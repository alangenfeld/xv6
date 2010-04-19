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
};

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
  cprintf("jip %x\n", jip);

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
  struct buf *bp;
  struct t_block;


}

int jmap(struct inode *ip, uint bn)
{
  /* here we need to mirror bmap without changing the FS
   * we need to take the metadata changes and write them to
   * the journal.
   * ISSUE: need to handle multiple calls to jmap changing
   *        the same inode block
   */

  return 0;
} 
