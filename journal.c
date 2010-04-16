#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "proc.h"
#include "fs.h"
#include "fsvar.h"
#include "file.h"
#include "fcntl.h"

struct t_block{
  uint status;
  uint num_sectors;
  uint sectors[510];
}

struct inode *jip;
uint t_index;
// uint sectors[16396 + 128 + 2];
uint sectors[510];

int j_init()
{

  jip = create("./journal", 1, T_FILE, 0, 0);
  t_index = 0;
  /* read journal */

  return 0;
}

int j_iupdate(struct inode *ip)
{

  struct buf *jbp;
  struct dinode *jdip;

  jbp = bread(ip->dev, IBLOCK(ip->inum));
  // jdip = (struct dinode*)jbp->data + ip->inum%IPB;
  jdip->type = ip->type;
  jdip->major = ip->major;
  jdip->minor = ip->minor;
  jdip->nlink = ip->nlink;
  jdip->size = ip->size;
  memmove(jdip->addrs, ip->addrs, sizeof(ip->addrs));
  bwrite(jbp);
  brelse(jbp);

  iupdate(ip);

  return 0;
}

int j_writei(struct inode *ip, char *src, uint off, uint n)
{
  uint tot, m, i;
  struct buf *jbp, *bp, *jtp;
  struct t_block;
  /* calculate the number of blocks you will touch */


  if(off + n < off)
    return -1;
  if(off + n > MAXFILE*BSIZE)
    n = MAXFILE*BSIZE - off;

  jtp = bread(jip->dev, bmap(jip, 0, 1));

  for(tot=0; tot<n; tot+=m, off+=m, src+=m){
    /* init buffers */
    bp = bread(ip->dev, bmap(ip, off/BSIZE, 0));
    if(bp == -1)
      bp = jmap();

    jbuf = bread(jip->dev, bmap(jip, (t_index * BSIZE) + 1, 1)); 
    m = min(n - tot, BSIZE - off%BSIZE);

    /* prepare block */
    memmove(bp->data + off%BSIZE, src, m);
    /* copy block to journal buffer */
    memmove(jbp->data, bp->data, sizeof(data));
    sectors[t_index++] = bp->sector;

    /* write to journal */
    bwrite(jbp);

    /* release both blocks */
    brelse(jbp);
    brelse(bp);
  }

  if(n > 0 && off > ip->size){
    ip->size = off;
    iupdate(ip);
  }
  return n;

}
