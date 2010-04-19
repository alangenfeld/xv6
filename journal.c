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
  uint sectors[120];
}

struct inode *jip;
uint t_index;
// uint sectors[16396 + 128 + 2];
uint sectors[510];

/* Game Plan:
 * Make it so it writes in this order
 * 1. Bit Mask
 * 2. Inodes (indirect block)
 * 3. Data Blocks
 * after this, we will attempt logging these transacitons in
 * this order
 */



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


  /* AW FUCK:
   * we should structure this in a way that we know
   * what the final meta data state will be so we dont
   * have to troll through the write one at a time
   * penis
   */




  if(off + n < off)
    return -1;
  if(off + n > MAXFILE*BSIZE)
    n = MAXFILE*BSIZE - off;

  jtp = bread(jip->dev, bmap(jip, 0, 1));

  for(tot=0; tot<n; tot+=m, off+=m, src+=m){
    /* init buffers */
    bp = bread(ip->dev, bmap(ip, off/BSIZE, 0));
    if(bp == -1) {
     
      /* mapping new data blocks this changes the inode 
       * so we need to be sure to log them in this transaction
       */
      // bp = jmap(ip, off/BSIZE);

    }

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

int jmap(struct inode *ip, uint bn)
{
  /* here we need to mirror bmap without changing the FS
   * we need to take the metadata changes and write them to
   * the journal.
   * ISSUE: need to handle multiple calls to jmap changing
   *        the same inode block
   */

  /* 
    

} 
