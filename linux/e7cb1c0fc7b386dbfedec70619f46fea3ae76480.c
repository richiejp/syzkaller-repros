// BUG: unable to handle kernel paging request in page_remove_rmap
// https://syzkaller.appspot.com/bug?id=e7cb1c0fc7b386dbfedec70619f46fea3ae76480
// status:fixed
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);

  memcpy((void*)0x20000000, "\x72\x61\x6d\x66\x73\x14", 6);
  syscall(__NR_memfd_create, 0x20000000, 0);
  syscall(__NR_mprotect, 0x20000000, 0x800000, 0);
  return 0;
}