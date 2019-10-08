// kernel BUG at ./include/linux/scatterlist.h:LINE!
// https://syzkaller.appspot.com/bug?id=c0d2a4940611ffcce896d6a29d80376fa0bca8af
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

static bool write_file(const char* file, const char* what, ...)
{
  char buf[1024];
  va_list args;
  va_start(args, what);
  vsnprintf(buf, sizeof(buf), what, args);
  va_end(args);
  buf[sizeof(buf) - 1] = 0;
  int len = strlen(buf);
  int fd = open(file, O_WRONLY | O_CLOEXEC);
  if (fd == -1)
    return false;
  if (write(fd, buf, len) != len) {
    int err = errno;
    close(fd);
    errno = err;
    return false;
  }
  close(fd);
  return true;
}

static long syz_open_dev(volatile long a0, volatile long a1, volatile long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

static long syz_open_procfs(volatile long a0, volatile long a1)
{
  char buf[128];
  memset(buf, 0, sizeof(buf));
  if (a0 == 0) {
    snprintf(buf, sizeof(buf), "/proc/self/%s", (char*)a1);
  } else if (a0 == -1) {
    snprintf(buf, sizeof(buf), "/proc/thread-self/%s", (char*)a1);
  } else {
    snprintf(buf, sizeof(buf), "/proc/self/task/%d/%s", (int)a0, (char*)a1);
  }
  int fd = open(buf, O_RDWR);
  if (fd == -1)
    fd = open(buf, O_RDONLY);
  return fd;
}

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  int i;
  for (i = 0; i < 100; i++) {
    if (waitpid(-1, status, WNOHANG | __WALL) == pid)
      return;
    usleep(1000);
  }
  DIR* dir = opendir("/sys/fs/fuse/connections");
  if (dir) {
    for (;;) {
      struct dirent* ent = readdir(dir);
      if (!ent)
        break;
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        continue;
      char abort[300];
      snprintf(abort, sizeof(abort), "/sys/fs/fuse/connections/%s/abort",
               ent->d_name);
      int fd = open(abort, O_WRONLY);
      if (fd == -1) {
        continue;
      }
      if (write(fd, abort, 1) < 0) {
      }
      close(fd);
    }
    closedir(dir);
  } else {
  }
  while (waitpid(-1, status, __WALL) != pid) {
  }
}

#define SYZ_HAVE_SETUP_TEST 1
static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[19] = {0xffffffffffffffff, 0xffffffffffffffff,
                  0xffffffffffffffff, 0xffffffffffffffff,
                  0xffffffffffffffff, 0xffffffffffffffff,
                  0xffffffffffffffff, 0x0,
                  0xffffffffffffffff, 0xffffffffffffffff,
                  0xffffffffffffffff, 0xffffffffffffffff,
                  0xffffffffffffffff, 0xffffffffffffffff,
                  0xffffffffffffffff, 0xffffffffffffffff,
                  0xffffffffffffffff, 0xffffffffffffffff,
                  0xffffffffffffffff};

void execute_one(void)
{
  long res = 0;
  *(uint32_t*)0x2001d000 = 1;
  *(uint32_t*)0x2001d004 = 0x70;
  *(uint8_t*)0x2001d008 = 0;
  *(uint8_t*)0x2001d009 = 0;
  *(uint8_t*)0x2001d00a = 0;
  *(uint8_t*)0x2001d00b = 0;
  *(uint32_t*)0x2001d00c = 0;
  *(uint64_t*)0x2001d010 = 0x41c1;
  *(uint64_t*)0x2001d018 = 0;
  *(uint64_t*)0x2001d020 = 0;
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 29, 35);
  *(uint32_t*)0x2001d030 = 0;
  *(uint32_t*)0x2001d034 = 0;
  *(uint64_t*)0x2001d038 = 0;
  *(uint64_t*)0x2001d040 = 0;
  *(uint64_t*)0x2001d048 = 0;
  *(uint64_t*)0x2001d050 = 0;
  *(uint32_t*)0x2001d058 = 0;
  *(uint32_t*)0x2001d05c = 0;
  *(uint64_t*)0x2001d060 = 0;
  *(uint32_t*)0x2001d068 = 0;
  *(uint16_t*)0x2001d06c = 0;
  *(uint16_t*)0x2001d06e = 0;
  syscall(__NR_perf_event_open, 0x2001d000, 0, 0, -1, 0);
  memcpy((void*)0x20000000, "/dev/snd/controlC#\000", 19);
  res = syz_open_dev(0x20000000, 1, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_dup3, r[0], r[0], 0x80000);
  if (res != -1)
    r[1] = res;
  syscall(__NR_setsockopt, r[1], 0, 0x60, 0, 0);
  syscall(__NR_ioctl, -1, 0x1000, 0);
  syscall(__NR_socket, 2, 2, 0);
  syscall(__NR_ioctl, r[0], 0xc0505510, 0);
  res = syscall(__NR_socket, 0x26, 5, 0);
  if (res != -1)
    r[2] = res;
  syscall(__NR_setsockopt, r[2], 0x117, 1, 0x20000100, 0);
  syscall(__NR_recvmmsg, -1, 0, 0, 0, 0);
  syscall(__NR_openat, 0xffffffffffffff9c, 0, 0, 0);
  *(uint32_t*)0x2001d000 = 1;
  *(uint32_t*)0x2001d004 = 0x70;
  *(uint8_t*)0x2001d008 = 0;
  *(uint8_t*)0x2001d009 = 0;
  *(uint8_t*)0x2001d00a = 0;
  *(uint8_t*)0x2001d00b = 0;
  *(uint32_t*)0x2001d00c = 0;
  *(uint64_t*)0x2001d010 = 0x7f;
  *(uint64_t*)0x2001d018 = 0;
  *(uint64_t*)0x2001d020 = 0;
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0x103, 5, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 29, 35);
  *(uint32_t*)0x2001d030 = 0;
  *(uint32_t*)0x2001d034 = 0;
  *(uint64_t*)0x2001d038 = 0;
  *(uint64_t*)0x2001d040 = 0;
  *(uint64_t*)0x2001d048 = 0;
  *(uint64_t*)0x2001d050 = 0;
  *(uint32_t*)0x2001d058 = 0;
  *(uint32_t*)0x2001d05c = 0;
  *(uint64_t*)0x2001d060 = 0;
  *(uint32_t*)0x2001d068 = 0;
  *(uint16_t*)0x2001d06c = 0;
  *(uint16_t*)0x2001d06e = 0;
  syscall(__NR_perf_event_open, 0x2001d000, 0, -1, -1, 0);
  syscall(__NR_msgget, 0x798e7454, 0x10);
  res = syscall(__NR_socket, 2, 1, 0);
  if (res != -1)
    r[3] = res;
  memcpy((void*)0x20000000,
         "lo\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 16);
  *(uint16_t*)0x20000010 = 0;
  syscall(__NR_ioctl, r[3], 0x8914, 0x20000000);
  res = syscall(__NR_socket, 0x26, 5, 0);
  if (res != -1)
    r[4] = res;
  *(uint16_t*)0x20000080 = 0x26;
  memcpy((void*)0x20000082, "aead\000\000\000\000\000\000\000\000\000\000", 14);
  *(uint32_t*)0x20000090 = 0;
  *(uint32_t*)0x20000094 = 0;
  memcpy((void*)0x20000098, "gcm(aes)"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000",
         64);
  syscall(__NR_bind, r[4], 0x20000080, 0x58);
  memcpy((void*)0x20000100,
         "\x71\xe6\x7a\x11\x1f\xde\x54\xfe\x46\xb9\x04\x83\x2c\x8f\xff\x73",
         16);
  syscall(__NR_setsockopt, r[4], 0x117, 1, 0x20000100, 0x10);
  res = syscall(__NR_accept4, r[4], 0, 0, 0);
  if (res != -1)
    r[5] = res;
  *(uint64_t*)0x20003b80 = 0;
  *(uint32_t*)0x20003b88 = 0;
  *(uint64_t*)0x20003b90 = 0x20000000;
  *(uint64_t*)0x20000000 = 0x20000380;
  memcpy((void*)0x20000380,
         "\xad\xfb\x98\x7f\x77\xad\x8a\xe2\xd6\xfc\x0b\xcc\x7f\x0b\xb4\xb5",
         16);
  *(uint64_t*)0x20000008 = 0x10;
  *(uint64_t*)0x20003b98 = 1;
  *(uint64_t*)0x20003ba0 = 0;
  *(uint64_t*)0x20003ba8 = 0;
  *(uint32_t*)0x20003bb0 = 0;
  syscall(__NR_sendmsg, r[5], 0x20003b80, 0);
  *(uint64_t*)0x200001c0 = 0x77359400;
  *(uint64_t*)0x200001c8 = 0;
  syscall(__NR_recvmmsg, r[5], 0x20002480, 0x871, 0, 0x200001c0);
  syscall(__NR_openat, 0xffffffffffffff9c, 0, 0, 0);
  syscall(__NR_perf_event_open, 0, 0, -1, -1, 0);
  memcpy((void*)0x20000000,
         "lo\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 16);
  *(uint16_t*)0x20000010 = 0;
  syscall(__NR_ioctl, -1, 0x8914, 0x20000000);
  syscall(__NR_msgget, 0x798e7454, 0x10);
  res = syscall(__NR_socket, 2, 1, 0);
  if (res != -1)
    r[6] = res;
  syscall(__NR_ioctl, r[6], 0x8914, 0);
  syscall(__NR_openat, 0xffffffffffffff9c, 0, 0, 0);
  syscall(__NR_perf_event_open, 0, 0, -1, -1, 0);
  res = syscall(__NR_msgget, 0x798e7454, 0x10);
  if (res != -1)
    r[7] = res;
  syscall(__NR_msgsnd, r[7], 0, 0xffffffffffffff07, 0);
  res = syscall(__NR_accept4, -1, 0, 0, 0);
  if (res != -1)
    r[8] = res;
  *(uint64_t*)0x200001c0 = 0x77359400;
  *(uint64_t*)0x200001c8 = 0;
  syscall(__NR_recvmmsg, r[8], 0x20002480, 0x871, 0, 0x200001c0);
  sprintf((char*)0x20000000, "%020llu", (long long)-1);
  sprintf((char*)0x20000014, "%023llo", (long long)-1);
  *(uint64_t*)0x2000002b = 0;
  syscall(__NR_write, -1, 0x20000000, 0x33);
  syscall(__NR_clock_settime, 7, 0);
  syscall(__NR_fcntl, -1, 0x409, 9);
  memcpy((void*)0x20000380, "/dev/zero\000", 10);
  syscall(__NR_openat, 0xffffffffffffff9c, 0x20000380, 0x80000, 0);
  syscall(__NR_bind, -1, 0, 0);
  syscall(__NR_pread64, -1, 0, 0, 0);
  syscall(__NR_ioctl, -1, 0xc0e85667, 0);
  syscall(__NR_sched_setscheduler, 0, 5, 0);
  syscall(__NR_close, -1);
  *(uint32_t*)0x200000c0 = 1;
  *(uint32_t*)0x200000c4 = 0x70;
  *(uint8_t*)0x200000c8 = 0;
  *(uint8_t*)0x200000c9 = 0;
  *(uint8_t*)0x200000ca = 0;
  *(uint8_t*)0x200000cb = 0;
  *(uint32_t*)0x200000cc = 0;
  *(uint64_t*)0x200000d0 = 0x41c1;
  *(uint64_t*)0x200000d8 = 0;
  *(uint64_t*)0x200000e0 = 0;
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 3, 5, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 29, 35);
  *(uint32_t*)0x200000f0 = 0;
  *(uint32_t*)0x200000f4 = 0;
  *(uint64_t*)0x200000f8 = 0;
  *(uint64_t*)0x20000100 = 0;
  *(uint64_t*)0x20000108 = 0;
  *(uint64_t*)0x20000110 = 0;
  *(uint32_t*)0x20000118 = 0;
  *(uint32_t*)0x2000011c = 0;
  *(uint64_t*)0x20000120 = 0;
  *(uint32_t*)0x20000128 = 0;
  *(uint16_t*)0x2000012c = 0;
  *(uint16_t*)0x2000012e = 0;
  syscall(__NR_perf_event_open, 0x200000c0, 0, -1, -1, 0);
  syscall(__NR_keyctl, 0x11, 0, 0, 0, 0);
  res = syscall(__NR_socket, 0x10, 3, 0xc);
  if (res != -1)
    r[9] = res;
  memcpy((void*)0x200003c0, "bond0\000\341\000\n\000!!\000\001\000\000", 16);
  memcpy((void*)0x200003d0, "bond_slave_1\000\000\000\000", 16);
  syscall(__NR_ioctl, r[9], 0x8990, 0x200003c0);
  syscall(__NR_perf_event_open, 0, 0, -1, -1, 0);
  syscall(__NR_shmget, 0x798dd814, 0x4000, 0x78000401, 0x20ffc000);
  syscall(__NR_prctl, 0x1b, 0, 0, 0);
  syscall(__NR_ioctl, -1, 0x80045500, 0);
  syscall(__NR_socket, 2, 2, 0);
  syscall(__NR_ioctl, -1, 0x40086414, 0);
  syscall(__NR_setsockopt, -1, 0x110, 3, 0, 0);
  res = syscall(__NR_socket, 0xa, 0x1000000000002, 0);
  if (res != -1)
    r[10] = res;
  syscall(__NR_sendmsg, r[10], 0, 0);
  syscall(__NR_setsockopt, -1, 6, 0x1d, 0, 0);
  *(uint32_t*)0x20001fde = 0;
  syscall(__NR_setsockopt, -1, 0x29, 0x40, 0x20001fde, 4);
  res = syscall(__NR_accept4, -1, 0, 0, 0);
  if (res != -1)
    r[11] = res;
  syscall(__NR_sendmsg, r[11], 0, 0);
  syscall(__NR_recvmmsg, r[11], 0x20002480, 0, 0, 0);
  res = syscall(__NR_socket, 0x26, 5, 0);
  if (res != -1)
    r[12] = res;
  syscall(__NR_setsockopt, r[12], 0x117, 1, 0, 0);
  res = syscall(__NR_accept4, r[12], 0, 0, 0);
  if (res != -1)
    r[13] = res;
  syscall(__NR_sendmsg, r[13], 0, 0);
  syscall(__NR_recvmmsg, r[13], 0x20002480, 0, 0, 0);
  syscall(__NR_socket, 2, 2, 0x88);
  syscall(__NR_ioctl, -1, 0xc0306201, 0);
  res = syz_open_dev(0, 0, 0);
  if (res != -1)
    r[14] = res;
  syscall(__NR_ioctl, r[14], 0x4c00, -1);
  syscall(__NR_socket, 2, 1, 0);
  syz_open_procfs(0, 0);
  syscall(__NR_socket, 0xa, 1, 0);
  syscall(__NR_ioctl, -1, 0x891c, 0);
  syscall(__NR_ioctl, -1, 0x40087705, 0);
  *(uint32_t*)0x20000340 = 7;
  *(uint16_t*)0x20000348 = 2;
  *(uint16_t*)0x2000034a = htobe16(0x4e22);
  *(uint32_t*)0x2000034c = htobe32(0xe0000002);
  *(uint32_t*)0x200003c8 = 0;
  *(uint32_t*)0x200003cc = 0;
  syscall(__NR_setsockopt, -1, 0, 0x30, 0x20000340, 0x90);
  syscall(__NR_write, -1, 0, 0);
  syscall(__NR_fcntl, -1, 0x409, 9);
  memcpy((void*)0x20000380, "/dev/zero\000", 10);
  syscall(__NR_openat, 0xffffffffffffff9c, 0x20000380, 0x80000, 0);
  syscall(__NR_bind, -1, 0, 0);
  syscall(__NR_pread64, -1, 0x20000280, 0xaa, 0);
  syscall(__NR_prctl, 0x26, 1, 0, 0);
  res = syscall(__NR_socket, 0xa, 0, 0xb);
  if (res != -1)
    r[15] = res;
  syscall(__NR_sched_setscheduler, 0, 5, 0);
  memcpy((void*)0x200003c0, "bond0\000\341\000\n\000!!\000\001\000\000", 16);
  memcpy((void*)0x200003d0, "bond_slave_1\000\000\000\000", 16);
  syscall(__NR_ioctl, -1, 0x8990, 0x200003c0);
  syscall(__NR_ioctl, r[15], 0x8903, 0x20000140);
  *(uint32_t*)0x2001d000 = 1;
  *(uint32_t*)0x2001d004 = 0x70;
  *(uint8_t*)0x2001d008 = 0;
  *(uint8_t*)0x2001d009 = 0;
  *(uint8_t*)0x2001d00a = 0;
  *(uint8_t*)0x2001d00b = 0;
  *(uint32_t*)0x2001d00c = 0;
  *(uint64_t*)0x2001d010 = 0x7f;
  *(uint64_t*)0x2001d018 = 0;
  *(uint64_t*)0x2001d020 = 0;
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 3, 5, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 29, 35);
  *(uint32_t*)0x2001d030 = 0;
  *(uint32_t*)0x2001d034 = 0;
  *(uint64_t*)0x2001d038 = 0;
  *(uint64_t*)0x2001d040 = 0;
  *(uint64_t*)0x2001d048 = 0;
  *(uint64_t*)0x2001d050 = 0;
  *(uint32_t*)0x2001d058 = 0;
  *(uint32_t*)0x2001d05c = 0;
  *(uint64_t*)0x2001d060 = 0;
  *(uint32_t*)0x2001d068 = 0;
  *(uint16_t*)0x2001d06c = 0;
  *(uint16_t*)0x2001d06e = 0;
  syscall(__NR_perf_event_open, 0x2001d000, 0, -1, -1, 0);
  syscall(__NR_shmctl, 0, 0xe, 0x200004c0);
  syscall(__NR_setsockopt, -1, 0x84, 0x6e, 0, 0xfffffdba);
  syscall(__NR_prctl, 0x1b, 0, 0, 0);
  res = syscall(__NR_socket, 0x26, 5, 0);
  if (res != -1)
    r[16] = res;
  memcpy((void*)0x20000100,
         "\x71\xe6\x7a\x11\x1f\xde\x54\xfe\x46\xb9\x04\x83\x2c\x8f\xff\x73",
         16);
  syscall(__NR_setsockopt, r[16], 0x117, 1, 0x20000100, 0x10);
  *(uint64_t*)0x20003b80 = 0;
  *(uint32_t*)0x20003b88 = 0;
  *(uint64_t*)0x20003b90 = 0x20000000;
  *(uint64_t*)0x20000000 = 0x20000380;
  memcpy((void*)0x20000380,
         "\xad\xfb\x98\x7f\x77\xad\x8a\xe2\xd6\xfc\x0b\xcc\x7f\x0b\xb4\xb5",
         16);
  *(uint64_t*)0x20000008 = 0x10;
  *(uint64_t*)0x20003b98 = 1;
  *(uint64_t*)0x20003ba0 = 0;
  *(uint64_t*)0x20003ba8 = 0;
  *(uint32_t*)0x20003bb0 = 0;
  syscall(__NR_sendmsg, -1, 0x20003b80, 0);
  syscall(__NR_prctl, 0x16, 2, 0, 0);
  syscall(__NR_inotify_init);
  syscall(__NR_socket, 2, 0x4000000000000001, 0);
  syscall(__NR_socket, 0xa, 2, 0);
  syscall(__NR_socket, 0xa, 1, 0);
  syscall(__NR_socket, 0x2b, 1, 0);
  syz_open_dev(0, 3, 0);
  syscall(__NR_socket, 0x28, 1, 0);
  syscall(__NR_msgctl, 0, 0xc, 0);
  res = syscall(__NR_socket, 0x26, 5, 0);
  if (res != -1)
    r[17] = res;
  syscall(__NR_setsockopt, r[17], 0x117, 1, 0, 0);
  res = syscall(__NR_accept4, r[17], 0, 0, 0);
  if (res != -1)
    r[18] = res;
  syscall(__NR_sendmsg, r[18], 0, 0);
  syscall(__NR_recvmmsg, -1, 0, 0, 0, 0);
  syscall(__NR_munlock, 0x20ff2000, 0x3000);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}