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

int j_init()
{

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

int j_writei()
{

  return 0;
}
