// KASAN: use-after-free Read in __ext4_check_dir_entry
// https://syzkaller.appspot.com/bug?id=871e52ccd40e45efbdb30bb839c711c6e5cad72d
// status:dup
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void loop()
{
  long res = 0;
  memcpy((void*)0x200001c0, "./file0", 8);
  res = syscall(__NR_open, 0x200001c0, 0x3fffa, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000280, "./file0", 8);
  res = syscall(__NR_open, 0x20000280, 0x400000000, 0x1d4);
  if (res != -1)
    r[1] = res;
  syscall(__NR_close, r[0]);
  *(uint32_t*)0x20000000 = 9;
  *(uint32_t*)0x20000004 = 1;
  memcpy((void*)0x20000008, "\x0b", 1);
  syscall(__NR_open_by_handle_at, r[1], 0x20000000, 0);
  syscall(__NR_lseek, r[0], 0x1ffc, 0);
  syscall(__NR_getdents64, r[0], 0x20001540, 0x200015fc);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}