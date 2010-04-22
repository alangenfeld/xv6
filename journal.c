#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "buf.h"
#include "fs.h"
#include "fsvar.h"
#include "file.h"
#include "fcntl.h"
#include "dev.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

struct buf *bp[20];
uint b_index;

struct t_start_blk{
  uint state;
  uint num_blks;
  uint sector[20];
};

#define START 0xDEADBEEF
#define READY 0xB00B1E55
#define END 0x69696969

static void
journal_start()
{
  struct inode *ip;
  int i;
  struct t_start_blk start;
  uint state = READY;

  ip = iget(1, 3);
  ilock(ip);
  start.state = START;
  start.num_blks = b_index;
  for(i=0;i<b_index;i++){
    start.sector[i] = bp[i]->sector;
  }
  // write trans start blk 
  writei(ip, &start, 0, sizeof(start));
  
  // write data blks
  for(i=0;i<b_index;i++){
    writei(ip, bp[i]->data, (i*512) + 512, sizeof(bp[i]->data));
  }
  
  // journal valid
  writei(ip, &state, 0, sizeof(state));
  
  iunlock(ip);
}

static void
journal_end()
{
  uint state = END;
  struct inode *ip;
  ip = iget(1, 3);
  ilock(ip);
  // write trans start blk 
  writei(ip, &state, 0, sizeof(state));
  iunlock(ip);
}

void
j_init()
{
  struct inode *ip;
  uchar buffer[512];
  int i, j;
  struct buf *bp;
  struct t_start_blk t_blk;
  
  for(j=0;j<512;j++)
    buffer[j] = 0;
  
  ip = iget(1, 3);
  ilock(ip);
  if(ip->size < 512*20){ 
    cprintf("alloc journal\n");
    // allocate journal if too small
    for(i=0;i<20;i++){
      writei(ip, buffer, i*512, sizeof(buffer));
    }
  }
  else {
    readi(ip, &t_blk, 0, sizeof(t_blk));
    // check if there is a valid not commited transaction
    if(t_blk.state == READY){
      cprintf("```~~~~~~~~XxXx~~~RECOVERING!!!!!~~XxXx~~~~```\n");
      for(i = 0; i < t_blk.num_blks; i++){
	readi(ip, buffer, i*512, sizeof(buffer));
	bp = bread(1, t_blk.sector[i]);	
	cprintf("recovering %d\n", t_blk.sector[i]);
	memmove(bp->data, buffer, sizeof(buffer));
	bwrite(bp);
	brelse(bp);
      }
      writei(ip, buffer, 0, sizeof(buffer));
    }
  }
  
  iunlock(ip);
}

// Allocate a disk block.
static uint
j_balloc(uint dev)
{
  int b, bi, m, i;
  //struct buf *bp;
  struct superblock sb;

  //  bp = 0;
  readsb(dev, &sb);
  for(b = 0; b < sb.size; b += BPB){
    /* check in dirty blocks */
    for(i = 0; i < b_index; i++)
      if(bp[i]->sector == BBLOCK(b, sb.ninodes)) {
	for(bi = 0; bi < BPB; bi++){
	  m = 1 << (bi % 8);
	  if((bp[i]->data[bi/8] & m) == 0){  // Is block free?
	    bp[i]->data[bi/8] |= m;  // Mark block in use on disk.
	    return b + bi;
	  }
	}
      }
    /* load new block out of mem */
    bp[b_index] = bread(dev, BBLOCK(b, sb.ninodes));
    for(bi = 0; bi < BPB; bi++){
      m = 1 << (bi % 8);
      if((bp[b_index]->data[bi/8] & m) == 0){  // Is block free?
	bp[b_index]->data[bi/8] |= m;  // Mark block in use on disk.
	/* keep dirty around, move index to next*/
	b_index++;
	return b + bi;
      }
    }
    //    panic("eh");
    brelse(bp[b_index]);
  }
  panic("balloc: out of blocks");
}

uint
j_lookup(struct inode *ip, uint bn)
{
  uchar found = 0;
  int i;
  uint addr, *a;

  //  struct buf *bp;
  if(bn < NDIRECT){
    if((addr = ip->addrs[bn]) == 0){
      panic("fs fail");
    }
    return addr;
  }
  bn -= NDIRECT;
  if(bn < (NINDIRECT * NINDIRECT)){
    // Load double indirect block, allocating if necessary.
    if((addr = ip->addrs[INDIRECT]) == 0){
      panic("fs fail");
    }
    // check dirty blocks
    for(i = 0; i < b_index; i++){
      if(bp[i]->sector == addr){
	found = 1;
	a = (uint*)bp[i]->data;
	if((addr = a[(bn / NINDIRECT)]) == 0){
	  panic("fs fail");
	}
	break;
      }
    }
    if(!found) panic("oh shit");
    found = 0;
    for(i = 0;i < b_index; i++){
      if(bp[i]->sector == addr){
	found = 1;
	a = (uint*)bp[i]->data;
	if((addr = a[(bn % NINDIRECT)]) == 0){
	  panic("fs fail");
	}
	break;
      }
    }
    if(!found)    panic("oswhesn ");

    return addr;
  }

  panic("bmap: out of range");

}

uint
j_bmap(struct inode *ip, uint bn)
{
  uchar found;
  int i;
  uint addr, *a;

  if(bn < NDIRECT){
    if((addr = ip->addrs[bn]) == 0){
      ip->addrs[bn] = addr = j_balloc(ip->dev);
    }
    return addr;
  }
  bn -= NDIRECT;
  if(bn < (NINDIRECT * NINDIRECT)){
 
    // Load double indirect block, allocating if necessary.
    if((addr = ip->addrs[INDIRECT]) == 0){
      ip->addrs[INDIRECT] = addr = j_balloc(ip->dev);
    }
    // check dirty blocks
    found = 0;
    for(i = 0; i < b_index; i++){
      if(bp[i]->sector == addr){
	found = 1;
	a = (uint*)bp[i]->data;
	if((addr = a[(bn / NINDIRECT)]) == 0){
	  a[(bn / NINDIRECT)] = addr = j_balloc(ip->dev);
	}
	break;
      }
    }
    if(!found){
      // load new block from mem    

      bp[b_index] = bread(ip->dev, addr);
      a = (uint*)bp[b_index]->data;

      b_index++;
      if((addr = a[(bn / NINDIRECT)]) == 0){	
	a[(bn / NINDIRECT)] = addr = j_balloc(ip->dev);
      }
    }
    //    brelse(bp);
    // Load indirect block, allocating if necessary.
    
    // check dirty blocks
    found = 0;
    for(i = 0;i < b_index; i++){
      if(bp[i]->sector == addr){
	found = 1;
	a = (uint*)bp[i]->data;
	if((addr = a[(bn % NINDIRECT)]) == 0){
	  a[(bn % NINDIRECT)] = addr = j_balloc(ip->dev);
	}
	break;
      }
    }
    if(!found){
      // load new block 
      bp[b_index] = bread(ip->dev, addr);
      a = (uint*)bp[b_index]->data;

      b_index++;
      if((addr = a[(bn % NINDIRECT)]) == 0){
	a[(bn % NINDIRECT)] = addr = j_balloc(ip->dev);
	//bwrite(bp);
      }
    }
    //brelse(bp);

    return addr;
  }

  panic("bmap: out of range");
}

void
j_iupdate(struct inode *ip)
{
  //  struct buf *bp;
  struct dinode *dip;
  cprintf("%d:iup %d\n", b_index, IBLOCK(ip->inum)); 
  bp[b_index] = bread(ip->dev, IBLOCK(ip->inum));
  dip = (struct dinode*)bp[b_index]->data + ip->inum%IPB;
  dip->type = ip->type;
  cprintf("type %d\n", ip->type);
  dip->major = ip->major;
  dip->minor = ip->minor;
  dip->nlink = ip->nlink;
  dip->size = ip->size;
  memmove(dip->addrs, ip->addrs, sizeof(ip->addrs));
  //  bwrite(bp);
  //brelse(bp);
  b_index++;
}

int 
j_writei(struct inode *ip, char *src, uint off, uint n)
{
  uint tot, m, i, j;
  struct buf *tbp;

  if(ip->type == T_DEV){
    if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].write)
      return -1;
    return devsw[ip->major].write(ip, src, n);
  }

  if(off + n < off)
    return -1;
  if(off + n > MAXFILE*BSIZE)
    n = MAXFILE*BSIZE - off;

  /* REAL CODE STARTS HERE */
  //  j_init();
  b_index = 0; // new xfer, start keeping track of open bufs

  /* allocate all space needed */
  for(i=0, j=off; i<n; i+=m, j+=m){
    j_bmap(ip, j/BSIZE);
    m = min(n - i, BSIZE - j%BSIZE);
  }

  for(tot=0; tot<n; tot+=m, off+=m, src+=m){
    bp[b_index] = bread(ip->dev, j_lookup(ip, off/BSIZE));
    m = min(n - tot, BSIZE - off%BSIZE);
    memmove(bp[b_index]->data + off%BSIZE, src, m);
    b_index++;
  }

  if(n > 0 && off > ip->size){
    ip->size = off;
    j_iupdate(ip);
  }
  
  journal_start();
  panic("octomom");
  for(i = 0; i < b_index; i++){
    bwrite(bp[i]);
    brelse(bp[i]);
  }
  
  journal_end();
  
  return n;

}
