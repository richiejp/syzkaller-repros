// WARNING in ext4_direct_IO
// https://syzkaller.appspot.com/bug?id=5b1b9853e5504f1f32511691917fec1401a827cb
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/futex.h>

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

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  if (pthread_create(&th, &attr, fn, arg))
    exit(1);
  pthread_attr_destroy(&attr);
}

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_RELAXED))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
}

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 160 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 256;
  setrlimit(RLIMIT_NOFILE, &rlim);
  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
}

int wait_for_loop(int pid)
{
  if (pid < 0)
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  loop();
  exit(1);
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
}

#define SYZ_HAVE_RESET_TEST 1
static void reset_test()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
}

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void execute_one(void)
{
  int i, call, thread;
  for (call = 0; call < 11; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
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
      reset_test();
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
#ifndef __NR_creat
#define __NR_creat 8
#endif
#ifndef __NR_fchdir
#define __NR_fchdir 133
#endif
#ifndef __NR_fcntl
#define __NR_fcntl 55
#endif
#ifndef __NR_ftruncate
#define __NR_ftruncate 93
#endif
#ifndef __NR_io_setup
#define __NR_io_setup 245
#endif
#ifndef __NR_io_submit
#define __NR_io_submit 248
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_openat
#define __NR_openat 295
#endif
#ifndef __NR_pwrite64
#define __NR_pwrite64 181
#endif
#ifndef __NR_write
#define __NR_write 4
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0,
                 0xffffffffffffffff};

void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    memcpy((void*)0x20000100, "./cgroup.cpu", 13);
    res = syscall(__NR_openat, 0xffffff9c, 0x20000100, 0x200002, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    syscall(__NR_fchdir, (long)r[0]);
    break;
  case 2:
    memcpy((void*)0x20000140, "./bus", 6);
    res = syscall(__NR_creat, 0x20000140, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 3:
    syscall(__NR_fcntl, (long)r[1], 4, 0x4001);
    break;
  case 4:
    res = syscall(__NR_io_setup, 0x1002, 0x20000040);
    if (res != -1)
      r[2] = *(uint32_t*)0x20000040;
    break;
  case 5:
    memcpy((void*)0x200004c0, "./bus", 6);
    res = syscall(__NR_creat, 0x200004c0, 0);
    if (res != -1)
      r[3] = res;
    break;
  case 6:
    syscall(__NR_fcntl, (long)r[3], 4, 0xb88cc547);
    break;
  case 7:
    syscall(__NR_ftruncate, (long)r[3], 0x8008200);
    break;
  case 8:
    *(uint32_t*)0x20000000 = 0;
    *(uint32_t*)0x20000004 = 0x2710;
    *(uint16_t*)0x20000008 = 0;
    *(uint16_t*)0x2000000a = 0;
    *(uint32_t*)0x2000000c = 0;
    *(uint32_t*)0x20000010 = 0x77359400;
    *(uint32_t*)0x20000014 = 0;
    *(uint16_t*)0x20000018 = 0;
    *(uint16_t*)0x2000001a = 0;
    *(uint32_t*)0x2000001c = 0;
    *(uint32_t*)0x20000020 = 0;
    *(uint32_t*)0x20000024 = 0;
    *(uint16_t*)0x20000028 = 0;
    *(uint16_t*)0x2000002a = 0;
    *(uint32_t*)0x2000002c = 0;
    syscall(__NR_write, (long)r[3], 0x20000000, 0xfffffdc6);
    break;
  case 9:
    memcpy(
        (void*)0x20000580,
        "\x5f\x78\xce\x74\x97\xa1\x0f\x54\x1d\xe0\x1b\x3f\xbf\x60\x7b\xde\xec"
        "\x39\x71\xe6\xd2\x99\xc6\x7b\x54\xb2\xea\xe4\xd3\x79\x2a\x7e\x3d\x3a"
        "\x18\xdc\x13\x4c\xc0\xe8\xd5\x42\x97\xba\xb4\x52\x51\x09\x46\xdc\xaf"
        "\x8d\xbf\x1a\x01\xb5\x05\x6a\xa8\x3d\x85\x46\x21\x7f\xdb\x60\xe0\x39"
        "\xef\x43\x78\x06\x37\xea\x5a\x26\xbd\xdb\x9a\x64\xfb\xf7\x01\x33\x36"
        "\xe0\xd5\x5b\x80\x42\x54\xd9\xf1\x2e\x36\xc4\x33\xea\xfb\xd0\x70\x0a"
        "\x6e\x53\x31\xa2\x4a\x14\x0f\xd2\x96\x31\xee\x34\x18\x68\xbc\x6b\xd8"
        "\xf1\x31\x0d\x75\xcd\x9b\x94\x1b\x0f\xc1\xfc\x9d\xae\x71\xe1\x93\xbd"
        "\xbf\xa6\x3e\xa7\x94\xe8\x75\xbe\xea\x3a\xfb\x4e\xa2\x4e\x1e\xbc\xf5"
        "\x76\x6a\x73\x3b\x08\xf9\xc4\xc7\x42\x6f\xb5\x98\xb1\xaf\xf1\xfc\x43"
        "\x7f\xb8\xe0\xd2\x33\x3d\x6c\xab\x9a\x7b\x2a\x61\x01\x48\xe6\x50\xf7"
        "\x71\xb5\x78\x21\x46\x66\x06\x58\x04\xf8\xcc\xd4\xdb\xc0\x4e\x65\xf9"
        "\xa8\x76\xd3\xe6\x4b\xc0\xfe\x90\x9a\xc8\x7d\xbc\x69\xb7\xe8\x9c\x25"
        "\x0d\xcc\x1c\x42\x74\x51\xc7\x09\xfb\x5d\xa2\x8e\x82\x65\x94\x15\x0f"
        "\x01\xfe\xaa\x87\xa8\xd8\xcf\xe8\xa5\x78\x3c\x31\xcb\xe2\xbe\xeb\x19"
        "\xf9\x59\xe6\x3f\x4d\x73\x21\xf9\xac\x0c\x00\x64\x8b\x47\x7a\xc6\x12"
        "\x85\x24\xe8\x44\x5b\xf0\x90\x7d\x2e\x9d\x74\x5f\x54\xb9\xde\x28\x78"
        "\x1b\x39\x82\x4b\x66\xe2\x25\xca\x9d\x26\x34\x8e\x53\xb3\x1d\x0d\xaa"
        "\xbf\x1a\x72\xde\x59\xad\x5c\x10\x69\x88\x8f\x91\x88\x67\xbf\x8a\x4c"
        "\x09\xb9\xf6\x8c\x3d\x4c\xad\xb4\xfb\x93\xd0\x01\x67\xe6\xb4\x76\x4a"
        "\x54\xde\x90\x0c\xb2\x25\x3e\x7f\x5b\x8d\xe7\x4e\x9b\xfb\x7b\xff\x65"
        "\xdb\x21\xad\xf7\xd4\x4f\x1b\x03\xd5\xb8\x77\xfd\x39\x63\x80\x21\xd7"
        "\x8e\x22\x83\x40\x42\x5f\x15\x80\x6a\x94\x52\x7b\x2d\x12\x80\x49\x17"
        "\x99\xfc\xb2\xea\xde\x4b\x9e\xa4\xfd\x06\x2d\x2a\x82\x47\x02\xee\x5b"
        "\x92\x0f\x34\x20\x60\xe3\x17\x73\xcb\x98\x3c\x32\x35\x11\x58\x73\x6a"
        "\x1e\xe1\x36\xd0\x40\xb7\x80\xba\x1a\x40\xe8\x2e\xb0\x46\x3e\x1e\xfa"
        "\x45\x8c\x8f\x44\xf7\x16\x49\x6f\xb1\x20\x2d\x54\x46\xb6\x1e\x62\xf7"
        "\x32\xab\xcf\x57\x32\xcb\x77\xd0\x63\x70\x64\xd7\x1f\xbe\x8d\xb2\xf6"
        "\xca\x40\xbc\x4a\x35\x2e\x29\x2a\x57\x3b\xb4\xa2\x3a\x17\x68\x8b\x69"
        "\x80\xfc\xd4\x38\x28\xcc\x86\x2a\x44\xd0\xd7\x4f\x42\x56\x89\x75\x61"
        "\x05\xe6\x0a\x61\x3b\xf8\x09\xb0\x35\xb1\x8f\x7b\xa6\xf0\x3e\x86\xd4"
        "\xce\x84\xec\xe0\xc3\x37\xd7\x70\x5d\x81\x1c\x45\x0e\x7f\xdf\xe1\xe6"
        "\xe1\x2e\xe6\xca\xbb\x1f\xaa\xc5\xde\x73\x9b\x43\x1b\x39\x07\x14\x4c"
        "\x63\xe0\xd1\x30\xab\x83\x41\x3f\x1a\x60\xc8\xcc\x50\xb0\xb2\x89\x71"
        "\xb7\x68\x0b\xda\xdd\x2e\x2f\x13\x3f\xf8\x5e\xc6\xe7\x21\x83\x41\xf4"
        "\xf8\x30\x6d\xfc\xb2\xd8\x9d\x81\x18\x0d\xea\x18\x61\xe0\xb5\x28\x0e"
        "\xec\x64\x33\xae\xec\xcd\x65\xfe\xdf\xd7\x6b\xd4\x88\x0d\xe1\x4d\x99"
        "\x2b\x56\x76\x60\xe2\xb8\x22\x95\x2d\x54\xf5\xb1\x15\x16\x77\xcc\xe2"
        "\xbf\x63\x65\x1b\xfa\xf7\xb2\x66\x8a\xb0\x70\x02\x08\xed\x2d\x81\xfe"
        "\x1c\x03\xa4\xf0\x6f\x25\x6c\x1e\x80\x5e\xe7\x66\x19\xc5\xbf\x08\x26"
        "\x45\xec\x35\xf2\xfb\xfa\x47\x12\x6a\xcc\xcc\x72\x12\x0a\x6d\xd8\xb6"
        "\xbe\xf7\xd3\x65\x06\x05\xea\x6b\xb5\xeb\xb0\xf3\x9c\x03\x77\xe1\x68"
        "\x0f\x54\x5e\x25\x75\xe5\x4b\x62\xd7\x6e\x8c\x06\x46\x77\xc4\x93\x2f"
        "\x92\xc7\xc6\xde\x18\x2e\x69\x26\x42\x44\x04\x22\x97\xc5\xfc\x35\x46"
        "\xde\x3f\xff\xa7\x84\x31\x7c\xcc\x78\x76\xf8\xa9\x18\xa5\x8a\xa3\x93"
        "\x62\x45\xc3\x16\xa7\xdd\x64\x37\xda\x36\xcc\x1a\xcf\xfb\x37\x5c\xa8"
        "\x9e\x72\x91\xa5\xdd\x66\x59\x0a\xf9\x1c\x02\x87\xec\x5c\x68\x5f\xcc"
        "\x3a\xd5\xdc\xf1\xd9\x8f\x0c\xb2\x31\xab\xe6\x16\xed\x84\x52\xc5\x2c"
        "\xa7\xdf\xbf\x81\x93\x39\xdb\x14\x7d\xc7\x10\xc0\x51\xe7\x55\x4a\x89"
        "\x9d\x2b\xe9\xcc\x8e\x4e\x12\xc4\xa5\xaf\x76\xd7\xcd\x4f\x5b\x39\x31"
        "\x4a\x86\x50\xb0\x53\x19\x5f\xfa\xf5\xfe\xe3\x78\xbf\x40\x07\xc3\x8f"
        "\x68\x1a\x29\x1a\x08\xb3\x3c\x29\xc8\x93\xd3\xfc\x9d\x85\xc8\xf9\x83"
        "\xff\x7c\xe1\x62\x4b\x61\x2d\xb9\xb7\xbd\x00\xe4\xf0\xc2\x8f\x2d\xb3"
        "\x7b\x51\x21\x13\x86\x16\x47\x75\x48\x4a\x61\x9f\x71\x2f\x89\x56\x26"
        "\x8f\x1b\x73\xce\xa3\x89\x88\x3a\xce\x49\x3b\xbc\x0b\xc2\xe3\xb9\xbf"
        "\x4b\xca\xb8\xa0\x84\x40\x26\xd3\x04\x34\xd4\x31\xbb\xf8\x56\xcc\xba"
        "\x4b\x83\x6c\x91\x80\xe3\x49\xe2\x0d\xe5\xcb\x31\x4d\x34\xb8\x65\xad"
        "\x05\xda\xfa\xa9\x1d\x7d\x02\x24\xcf\x35\x0d\x88\x81\xbf\x87\xb8\x24"
        "\xc7\x25\xd5\xc8\xba\xc8\x60\x78\x74\x58\xb7\xa3\x9f\x42\x2a\xae\x72"
        "\x7a\x31\x6c\xc3\xe5\x46\x58\xeb\x6d\x45\x33\x5f\xb4\xa6\xfa\x1c\x95"
        "\x29\xf5\xb3\xca\x40\xc0\x63\xb1\xdd\xd1\xea\x63\xc7\xf9\xdb\x5a\xe4"
        "\x4b\xe2\xff\xde\x57\x90\x0c\x15\x7e\x18\x54\xc7\x60\x5b\x8a\x7b\x53"
        "\xbf\xe7\xe1\x43\xc5\x0a\xba\x9f\xc0\x17\x7f\xf9\xba\x97\x2c\xc0\x91"
        "\x98\xb1\xa6\x40\x61\x3d\x67\x2b\x70\xbe\x5e\xdd\xe1\x2e\x41\x9d\xfb"
        "\x5a\xd0\x09\x17\xbc\x47\xdb\x16\x2b\xe5\x53\x7a\x41\x4d\xbc\x8d\x72"
        "\x55\x23\x55\x2e\x9c\x65\x9d\x55\x1b\x2e\x24\xd7\xdd\xb4\x59\xbc\x75"
        "\x2a\x53\x37\x86\x62\xe3\x94\x06\xeb\x34\x84\xb8\x46\x44\x2d\xe8\x86"
        "\xc6\x3f\x8b\x87\x9c\x25\xf8\x4b\xa8\xa8\x74\xb9\xdd\xcc\x6b\x93\xb5"
        "\xa3\xa9\x42\xc0\xd6\xe7\xa5\x48\xf3\xbf\xd2\x4d\x48\xd0\x93\xc0\xd8"
        "\x37\xbd\x90\x3c\x74\x4f\xe2\x98\x26\xfa\xd4\xd3\x57\xda\x3b\x63\x33"
        "\x54\xbf\x2f\xb3\xc6\x01\x4f\x6f\x5c\x51\x8b\x2f\xb5\x3c\x30\xe0\x6c"
        "\x97\xe9\x4b\xd0\xad\x6e\x58\x88\xb3\xba\x5d\xe3\x1e\x1f\x2b\x04\xb9"
        "\x88\xb0\x82\x4b\x69\xeb\xb7\xd2\xef\xee\x33\x65\x5d\xa4\xaa\x18\x47"
        "\x66\x27\x6f\x28\xfd\x12\x2f\x17\x3b\x46\x02\xef\x48\xe4\x25\xa2\xb2"
        "\x1b\x7a\x8b\x5f\x79\x2a\xca\x69\x66\xa1\xda\xb6\xc0\xa3\xe7\x1a\x1e"
        "\x97\xe7\x4f\xec\x21\xcc\xad\x72\xb1\x2d\x4d\x59\xa4\xa1\xdc\xed\x17"
        "\x90\x71\x08\x1d\x96\x0c\x33\x76\xbc\x4a\xa9\xa0\x22\xbf\xcd\x40\x61"
        "\xe9\x36\x4e\xb5\xca\x08\x83\xa9\x18\xf5\xfa\xbd\xd6\xb4\xba\xb0\xf1"
        "\x39\x0a\x4a\xb9\xb1\x52\x11\x01\x17\x85\xab\xaf\x8c\x81\x5d\x0b\xd9"
        "\xc0\x02\xd8\xe9\x84\x65\x60\xfe\xf1\x6d\xf3\xa4\xbc\x21\x2d\xe8\x21"
        "\x2c\xd7\x9d\x82\x8d\xb5\xf1\xf3\xbb\xb5\x89\xe6\xdc\x87\x11\xd2\x35"
        "\x50\xba\x5b\x30\x64\x68\x80\x6c\x50\x53\x8a\xcd\x76\x50\x45\xfe\x99"
        "\xdb\x83\x5e\x30\xa2\x64\x6a\x80\x63\x54\x93\x51\xf2\x1c\x21\x1e\xc2"
        "\x15\xd2\xf2\xbc\x33\x82\x3c\xe6\xcf\xfb\x6d\xa1\x60\xbd\x8c\xec\x26"
        "\xef\x7e\xbe\xc0\x87\xe6\xa8\xf6\x30\x48\x09\x5b\xea\x73\xba\x5f\x4e"
        "\x26\x49\x81\x35\xeb\xea\x27\xb0\xe7\x81\x34\x06\x1d\xdd\x2a\xec\xb7"
        "\x4c\xb4\xba\x24\xb1\xaa\x7a\x0f\xb0\x64\x14\x1a\x98\xd0\x65\x4a\xe9"
        "\x1f\xd4\x12\x14\x64\x80\x6a\xf6\x0d\x7b\xb4\xe0\xe2\x9b\x01\x8d\x50"
        "\x1a\xb6\x6a\x39\x09\x69\xd0\xa8\x95\x7f\x1c\xe7\xe0\xae\xc6\xbd\x9f"
        "\x5e\xad\x9d\xb8\x73\x5f\xb2\xa1\x46\xc5\x9d\x28\x7f\x63\x58\xf3\x45"
        "\x3b\xa3\xb5\x59\xa8\xb4\xe7\xd6\x2b\x4a\x5b\x99\x02\x8b\x5c\x88\x9e"
        "\x5e\xbe\xba\x54\x21\xf6\xfe\x68\x6a\x07\xf9\x08\xa4\x28\x0d\x6e\x6d"
        "\xfa\xda\x54\x15\x03\x62\x3e\x81\xaa\x74\xb0\x57\xa6\xfc\xba\x76\x5e"
        "\xdf\xeb\x5a\x1d\xeb\xd7\x90\x04\x69\xa0\xae\x1e\x85\xfc\x8a\x83\x6a"
        "\xb9\x9a\x82\xbe\x11\x69\xf5\xb1\x44\x41\x33\x1d\x2a\xbd\xed\x21\xf0"
        "\x1f\x18\xee\x46\x2e\xb8\xc4\x14\xa4\xec\x89\x89\xce\xb2\xd4\xf7\x29"
        "\xe2\x8f\x89\x7a\x53\x77\x18\xb9\xed\x97\x10\x6d\x32\xbf\x86\xef\x2a"
        "\xc5\x5c\xcf\xda\x48\xb9\x2f\xe1\xc7\xda\xce\xde\x8c\x5e\x31\x89\xc4"
        "\xa4\x96\x22\x1f\x48\x21\x5d\x24\x18\x8f\x15\x25\xe9\xa4\xc2\x61\x80"
        "\xff\x7c\xfe\x62\x7b\x91\x60\x1d\x14\x11\x98\x69\xfc\x31\xc7\xa3\xa9"
        "\x6e\xca\x94\xe9\xa5\xdb\x60\xea\x43\xec\xd6\x7b\xb0\x0e\xc9\xe9\xfc"
        "\xb4\xc4\x9f\x9d\x64\x0f\xc2\x05\x64\x40\x3d\xa9\x30\x71\x15\xb1\x4e"
        "\x9a\xe1\xbb\xa6\x58\x36\x62\xc0\x53\xdd\x4d\x9a\xe1\xd7\x5b\xed\xe6"
        "\x0f\x17\x20\xbb\xbc\xd8\x3a\x48\xc3\x79\x0f\x69\x83\x8b\xdd\xc1\x63"
        "\x38\xbf\xff\x5d\xf9\xdc\xb7\x18\x79\x29\x3f\xf5\x1f\x9e\x3f\x02\x32"
        "\xee\x41\x45\xc3\xb3\x48\x32\x09\x7b\x82\x29\xd0\x7d\x3f\x93\x0b\xba"
        "\x47\x8b\x13\x11\xab\x34\xe4\x9d\x12\x5b\x2a\xc7\x31\xeb\xd7\x2d\x36"
        "\x0c\xb6\x46\x6d\x0e\x7e\xd9\x1c\x88\x96\x0c\xb3\x91\x54\xaa\x58\xd7"
        "\x59\xb2\xac\x44\x04\x37\xc7\x9c\x26\x55\x8c\xf7\xbd\xcd\x6c\xe2\x74"
        "\x71\x0e\x02\x96\x64\xed\xc7\xd8\xe9\x8e\x42\x60\x39\xff\x87\xe9\xfc"
        "\x94\x84\x03\xe8\x60\xeb\x1f\xb6\x55\x2b\x5c\x52\xb5\x11\x0d\xda\x43"
        "\xbd\xcc\x8a\x9f\x73\x7c\x11\x69\xb5\x81\xd9\xa0\x19\x3f\xba\xf2\x8e"
        "\x36\x15\x05\x45\xc5\xfd\x18\x3a\x78\xbb\xd8\x71\x56\x68\x0d\x3c\xba"
        "\x32\x49\x05\xe8\xcf\x42\x76\x06\x95\x82\x8a\xc0\x17\x96\xa8\xea\x3a"
        "\x16\xb5\x97\x3a\xa4\xf4\xd0\xe9\xa6\x76\x9a\xee\xe5\xec\x7c\xab\xe6"
        "\xe5\x60\x7b\xf0\xcf\x6f\x36\x1c\x3e\x42\xfa\xb9\x95\xd3\xce\x49\x31"
        "\x5f\x60\xbe\x50\x9f\x2f\xc1\x3d\x71\x54\xbf\x0b\xcb\x6b\x6a\xf9\x4e"
        "\x64\x35\x30\xef\xf0\xef\xdd\x39\x55\xaa\x3f\xd6\x85\xb4\xcf\x46\x53"
        "\xb2\x05\x61\x1e\xfb\xec\x87\x55\xdd\x74\xfb\x0f\xb0\xb4\xc9\x18\x20"
        "\x69\x4b\xfd\xf0\xb6\x2a\x79\x9b\x05\x4f\x66\x35\x58\x02\x4a\x9d\x49"
        "\x4a\xe0\xb6\x30\xa5\x02\x37\xa1\x8c\x33\x89\x8e\x1b\x69\x20\x75\xee"
        "\x1c\x15\xf1\x2c\x44\xe7\x4e\x4f\xe7\xa6\x6e\xe0\x1c\x32\xf9\xb8\xbd"
        "\x65\xc1\x9b\xc3\x75\xbf\x4e\x7d\x6d\x78\xa5\x14\xe7\x06\xfe\xeb\x91"
        "\xb6\x77\xad\x54\xe9\xc3\x5e\xbb\xb7\x8a\x24\x5c\xcd\x48\xbf\xe4\x62"
        "\x41\x29\x5a\x42\xc3\x20\x1e\xa8\x4b\xec\x13\x2c\x56\x6a\x6b\x25\x56"
        "\xd3\x23\xb0\x49\x5b\x01\x16\x46\x3c\x41\x81\x87\x9c\xdd\xb4\xe1\x92"
        "\xb1\x7c\x5a\x2f\x32\xbb\x85\x22\x42\xb1\xb4\xfa\xb1\x73\xcb\x62\xfb"
        "\x3a\xbb\x35\xf5\xa7\x6d\x84\xb5\x92\xc3\xef\x46\x9b\xf3\x94\xcf\xf5"
        "\xf1\x75\x97\xb0\xb9\x75\x54\xbe\x37\x32\x32\x36\x50\x42\xc2\x2b\x27"
        "\x92\xc2\x4e\x6e\x88\x7e\x61\xfb\x03\x1f\x05\xf4\x79\xb2\x19\x67\xfc"
        "\xa4\x0c\x67\xdb\x7c\xd8\x11\xfa\xa7\x94\x97\x8b\x0e\xfd\xda\xc2\xda"
        "\x9e\xaa\x74\x20\xdc\x59\xf8\x04\xce\x13\x3a\x23\x9c\x25\x5c\x49\x9d"
        "\xa8\x77\x19\xac\xe8\xb0\x9c\xf3\xfc\xf8\x44\xe9\xd1\x28\x3a\x44\x42"
        "\x07\x94\x16\xf7\xca\xbf\xcd\x69\x98\x86\x33\x04\x7a\x1c\xd7\xee\x8e"
        "\x75\x0f\x29\x79\xae\x10\x11\x71\xf9\x9f\xd5\x7b\x0f\x2d\xb2\x2c\x93"
        "\xb1\xc9\x68\x12\x01\x4e\x93\x75\xcd\xcd\xf2\x10\xa4\x23\x97\x33\x43"
        "\x24\xaa\xcb\xf9\x1c\x7c\x7e\xe8\xb0\x56\x1d\x85\x42\xdf\x08\xbd\xd9"
        "\xfe\xdf\x1a\x55\x6d\x82\x91\x7f\xc0\xfd\x58\xbc\x7a\xfd\xed\xa0\x11"
        "\xf3\xc7\x82\xee\xa0\xc2\x5a\x30\x54\xc8\x65\x8c\x23\xff\x26\xb9\xa7"
        "\x6a\xf1\xc0\x70\xcd\xf8\x2c\xe2\x47\x95\x4b\x88\x94\x8d\x47\x1c\xdd"
        "\x3e\xf8\x42\xd3\x6c\x03\x15\x3d\x46\x8a\xef\x45\x1b\x97\xb0\x2e\xd8"
        "\x9e\x05\x2d\x2c\xcb\x84\xd1\x94\x85\xd0\x4d\x3c\xcf\x28\xf0\xc6\x8d"
        "\xc6\x96\x19\x6d\x8a\x5d\x83\xb6\x0d\x19\xdf\xea\x07\x67\xba\x38\x71"
        "\x79\x13\xd8\xbe\x8c\xb4\x20\x16\x0a\x3c\x1b\xc7\xa0\x4e\xa0\xd8\xd3"
        "\x2c\xb8\x11\xbd\x05\x46\x6d\x11\x8c\xa3\x15\xa6\xa7\x5f\x45\x59\x8f"
        "\x14\x59\x9b\x37\xfc\xca\xde\x9a\xfa\x8a\x09\x02\x89\xfe\xb5\x01\xc7"
        "\xe8\xc4\xc3\xbc\x07\xd8\x2a\xc6\x0c\xb5\x6e\xdb\x49\xdb\x88\x7a\x36"
        "\x56\xaf\x14\x2a\xa1\xb6\xfa\x11\x44\x89\x05\xc8\x5a\xb1\x2a\xa1\x0f"
        "\x9a\xc1\xb2\x92\x67\xc0\x21\xf3\x5b\x0a\x34\xff\x1b\x1b\x2e\x23\x14"
        "\x85\x7e\x9b\xac\x7a\x54\x7f\xe5\xff\x41\x71\x6e\x1a\x30\xb4\x11\x93"
        "\x83\x46\xf9\xd0\xf0\x49\xff\xf7\x5e\x76\x4b\xbc\x61\x6f\xa8\xa3\xfc"
        "\x03\xe7\xe6\xa7\x54\xfc\x08\x29\x49\x9c\x16\x24\x6f\x04\xe1\xe8\xd3"
        "\x73\xe6\xe8\x3c\x4e\xdc\x88\x4a\x53\x1b\x50\x6b\xea\x77\x10\xf7\xad"
        "\xc8\xdb\x19\x70\x00\xac\x59\x17\x33\x90\x43\xab\x6e\x7f\x5f\x0c\x9b"
        "\xfd\x7b\x1c\x5f\x63\x9c\xd2\x0a\xe4\x1e\xe1\x86\xf5\x6e\xfc\x16\x6f"
        "\xd1\xe3\x8d\xc6\xbf\x33\x2e\x85\xe5\x91\x33\x95\xd2\x75\x44\x77\xc2"
        "\xe0\x76\x9a\xcf\x07\xf5\x91\x31\xa6\x66\xff\xdd\x62\x31\x89\xdc\x73"
        "\xbf\xed\x55\x15\xd3\x3a\x65\xee\xc7\x7f\x88\x0a\xa1\x90\xc3\xb4\x3b"
        "\x4c\x15\x13\x86\xe6\x30\x71\x6a\xbd\x0c\x05\x70\xa1\x7f\x15\x0e\x22"
        "\x34\xd8\xcf\x8f\x48\xb0\x10\xee\x5e\xca\x21\x7e\x09\xca\xa8\xe3\x9b"
        "\xf4\x4b\x61\x5d\xa9\x18\x72\xc3\x7c\xf7\xc9\x6c\x79\xcb\x10\x8e\xb4"
        "\x90\x2b\x9f\xd8\x79\x7e\x52\x7b\x78\xaf\x6c\x9b\xb3\x06\x58\xe5\x34"
        "\x83\x6d\xe6\x05\x6d\xbb\xd3\xdb\x3a\xd7\x18\x69\x15\x04\x11\x7e\x21"
        "\xa4\xda\x65\xa6\xed\x91\xfd\x95\x05\xd8\x58\xfb\xb9\xcf\x48\xdb\x5b"
        "\x84\xee\x7f\xe9\xc5\x99\xf7\x72\x36\x1e\x35\x5b\x91\xde\x69\x9c\xab"
        "\x92\xe6\x79\x9e\xcb\x7f\x49\xf9\x97\x18\xa8\x6e\x12\x27\x97\x90\x8d"
        "\x6b\x94\x41\xa1\x64\x87\xd4\x0d\xf5\x38\x39\x25\x5a\x0c\xe4\xc8\x80"
        "\x55\xfb\xec\xb5\xba\x8e\x1f\xa4\x4e\xa5\xe5\xb4\xcc\xe7\x1d\x8d\xd0"
        "\x83\x10\xda\xe5\x34\x93\xc5\x59\x72\x8e\x5f\x92\x73\x7f\x8d\x08\xc1"
        "\x6c\x96\x44\xb1\xf6\x33\xb1\xd4\x33\x0a\x7e\xd2\x21\x7e\xa9\xd9\xba"
        "\xac\x0f\xec\x16\xff\x39\x18\xe1\x38\x97\x4a\x75\x38\xbf\xd6\x0f\x18"
        "\xbe\x65\x59\x5b\x93\xb2\x43\xd3\xa9\x7c\xc5\x76\xba\x7d\x40\x3d\x94"
        "\x3e\x5e\x5f\x4b\x25\x25\x7b\xd3\x11\xae\x24\xd9\xdb\x41\x4b\xdf\x4a"
        "\xf6\x51\x6d\x16\x12\x2b\xf6\xe0\xec\xb4\x1f\x99\xfe\x68\x13\xb2\x55"
        "\xfe\xa7\x45\x68\x4f\x4b\xbd\x94\x6a\xe4\xc5\xac\xda\x93\x3f\x85\xdb"
        "\x1b\x61\xba\x17\x25\x94\x38\xa5\x7c\xa6\xe6\xfe\xbd\x01\x0c\x7c\x4f"
        "\x9e\x09\x89\xa8\xe8\x99\x5d\x8e\x5b\x2c\x26\xa1\x0a\x7f\xf8\x3c\x5a"
        "\x39\x62\xc0\x24\xa8\xc6\xba\x22\xa3\x69\x75\xf9\x61\x4a\x1f\xc6\x6f"
        "\x92\x06\x24\xf4\x1f\x4d\xa6\xdb\x43\xf4\x10\x89\xe2\x29\xdd\xcf\x0a"
        "\xd5\xc4\xf0\x70\xe3\x09\x58\x9d\xe3\xa6\xb0\x2c\x28\x72\xe1\x06\xe0"
        "\xa2\xf2\x00\x0b\x8d\xda\x3f\x9e\xd2\x7b\x78\xc5\x70\xea\x7a\xa1\x99"
        "\x76\x71\x81\x74\x29\xd5\x81\xd3\xf7\x4d\x1a\x70\x56\xd7\xeb\x53\x97"
        "\x66\x96\x5b\x4d\x57\xd2\x63\x1d\xe9\x7e\xb2\x6f\x7b\x0a\x20\x0e\x6a"
        "\x37\x4f\x0d\x31\x3e\xbf\x57\x79\x56\x79\x47\xf5\x4e\x8d\x99\xb0\xf5"
        "\x90\xa5\x04\x36\xef\x69\x92\xe1\x75\xa0\xb1\x74\x17\xf1\x1b\x39\x82"
        "\x16\xec\x7c\x44\xf3\x68\xa2\xcc\x04\xe7\x1c\x12\x55\x68\x78\x79\xc9"
        "\x04\xab\x07\x4a\x8c\x4b\xf5\x7a\x50\xe0\x3a\x46\x78\x20\xa5\x7d\x63"
        "\x14\xc7\x8e\xa3\xf7\xaa\xd6\xb3\xd7\x8b\x64\x98\xea\xa1\x65\x7d\x8c"
        "\x70\xc6\xa9\xb3\x66\x75\x90\x66\x4b\x21\xa6\xd1\xa2\xb6\x5b\xbc\xdf"
        "\xf7\x4f\x68\xa0\xa6\x56\xd7\x64\x60\x4c\xc1\xba\x97\x79\x85\x73\x15"
        "\xf2\xd0\xb2\x29\x86\x30\xd2\xb2\x02\x66\xfa\x35\x7e\xbe\x20\x06\x44"
        "\x0f\x4a\xe7\xa0\xa9\x58\x29\x0b\x1b\xc0\x1d\xa9\xde\x33\x9c\xdb\x84"
        "\x45\x2f\x3a\x2f\x0f\xe8\x76\xbf\xbf\xe4\x09\x99\x60\xf8\xd5\x0e\x0f"
        "\x94\x9e\x50\x87\xa4\x30\xd1\x5b\xb3\x67\x1d\xb1\x46\x77\x5d\xa6\xac"
        "\xa0\x03\x71\xe2\x00\x24\xbd\x47\x5a\x58\x02\x32\xbd\xbb\xf8\x6c\x17"
        "\xbb\xe6\xb3\x4f\x3a\x04\x88\x61\x67\x8c\x89\x4c\x20\x73\xd3\x3e\xb0"
        "\xf8\x15\xb6\x7c\x03\x56\x46\xb9\x6a\xe2\x0a\x2c\x55\x9c\x89\x52\x1b"
        "\x18\xc9\xec\xf9\x23\x96\x70\x35\xd9\x50\x32\x4f\xf3\xc4\xbd\xf1\xc1"
        "\x53\x39\x72\x96\x24\x6f\x3b\x78\x22\xfa\xfc\xd4\x14\x56\x70\xc6\xfc"
        "\xfb\x34\xe5\x13\x09\x96\xba\x10\x49\x9f\x54\xa3\x79\x3d\x0d\x83\x75"
        "\x8e\x87\x17\x3a\x55\x25\xc4\x23\xbf\xdf\x3d\x5d\x06\xfe\x6a\xba\x05"
        "\x01\x60\xe5\x13\x5c\x9c\xd6\xb5\xf3\x9c\x77\xaa\x7d\xea\xa9\x19\xdd"
        "\x51\xd8\xb9\xfc\x83\xc5\x71\x50\x86\x34\xa9\xba\x9c\x74\x0c\x5a\x37"
        "\xf2\x34\x6b\xdd\x49\xdd\x5e\xa3\x9a\xf5\x19\x34\x1b\xfa\x7c\x4b\xb2"
        "\x1f\x36\x20\x1b\x42\xc6\x08\x03\xe5\xd9\x29\x68\x3e\x38\xfb\x0d\xb2"
        "\x13\xa8\x23\x9b\xab\x25\x28\xf8\xe2\x09\x25\x7c\x66\xa7\x66\x9b\x72"
        "\x4f\x0b\xb2\xea\xc0\xd0\xa5\x0a\x72\xd9\x76\x13\xf3\xe4\x18\x7c\x99"
        "\xf7\x53\x1d\x90\x1c\x6e\xcf\xe4\x89\x5c\xfe\xb2\xe8\x3d\x93\xe8\x01"
        "\xa9\xc1\x03\x4b\xde\x33\xc1\x52\x01\x0b\xc3\x53\x8f\xb1\x65\xeb\x20"
        "\x3b\x8b\x7e\x3c\x3c\xc3\xca\xcf\xc4\xc6\x19\x73\xaf\xed\xbc\xc5\xf5"
        "\xf8\x5d\x98\xdb\xc6\xc6\xd1\x98\xda\x9a\xde\xde\x4d\x1f\x75\x10\x92"
        "\xab\xf4\x8c\x30\xf5\x21\x03\x45\x63\x74\x09\xd8\xea\x29\xfd\x7c\x26"
        "\x49\x56\xa3\x11\xa9\x29\xf7\x51\x39\x63\x1e\x64\x66\xb7\x5c\xae\xfb"
        "\xe2\xd2\xec\x90\x62\x99\x8f\xd2\x7f\xf9\x16\xa2\xe1\xf9\xe9\xb2\x10"
        "\x0e\xf9\x14\xb6\x8a\x34\xf8\x1e\xa2\xff\xc9\x07\xaa\xd8\x47\xe1\xa9"
        "\x05\x54\xd2\xc9\xb9\xba\xc3\xe2\x83\x2a\xa5\x34\x45\x6c\x3d\x19\x11"
        "\x4f\xce\x26\xe9\xb5\xcd\xea\x35\x0d\x39\x80\xd7\xd4\x78\x7f\xdf\xe4"
        "\xdb\x82\x34\xdd\x61\x77\x0c\x2d\x8c\xde\xf0\x0f\x10\x13\x45\xf6\x71"
        "\xb0\xb4\xf6\xa2\x7a\x36\x35\xed\x5b\xa6\x43\x13\xf1\xec\x81\x3d\x98"
        "\xf7\x49\xae\xc2\x56\x90\xd2\xad\x31\x34\xa5\x0e\xc5\xc0\xfe\xb2\x3a"
        "\x1b\xb1\x17\x96\xbc\xf3\x3a\x20\xf5\x01\x13\xab\x7f\x2a\xb5\x3f\x00"
        "\x1b\xb5\x74\x28\x83\xc7\x37\x7e\x60\x5a\x83\xef\x80\x12\xae\xb1\x27"
        "\xbe\x44\x85\x02\xca\xfb\x17\x1e\x2a\x18\x57\xd9\x27\xba\x72\xbb\x74"
        "\x44\xcd\x06\xf5\x1c\x81\x06\xb9\x85\xef\xd9\x8a\x23\xaf\xe6\x46\x4e"
        "\x77\x3a\x27\x4e\x6c\x85\x05\x63\x09\x5c\xc7\xa2\x4a\xde\x1a\x06\x91"
        "\x76\x12\xef\xdf\x88\xfd\x7e\x61\x49\x49\x39\x2f\x65\x80\xa0\xe6\x4c"
        "\x26\xca\x10\x9b\x42\xc1\xfb\x50\x57\xfb\xa0\x22\xed\x97\x06\x15\xb9"
        "\xea\xd5\xff\xa7\xf1\x13\xcb\xe2\x78\x00\x50\x8e\x1e\x1a\x64\x2d\x8b"
        "\x0e\xbf\x15\x62\xf6\xb2\xc2\x58\x4a\x0e\x71\x2e\xf9\x05\x08\xe3\x52"
        "\x2b\xb3\x2e\x9f\xf7\x80\x4a\xd6\x49\xdd\x9f\xc3\x42\xe5\x6b\xcc\xb3"
        "\x35\xcc\x41\x6f\x0e\x8c\x67\xb6\xd9\x9b\x6c\xe1\xc2\x91\x8c\x96\x1c"
        "\xae\x8d\xb5\x3a\x56\x42\x06\xe3\x56\x57\x68\x55\x2d\x5a\x22\xba\x1c"
        "\x68\x96\x87\x90\xcc\xbf\xfa\x40\xd1\x97\x8c\x9c\xef\x9d\x64\x78\x02"
        "\x7a\x4b\xb7\x0d\x2d\x94\xc6\xff\xf2\x4a\xcf\xb1\x63\x4a\x85\xdb\x1b"
        "\xe0\xec\x2d\x19\x9c\x77\x0b\x49\x43\xcb\x6d\x3f\x0f\x26\xd1\x94\x47"
        "\x2f\xb2\x5a\xf7\x61\xab\x34\x4c\xea\x3e\xb7\x1a\x9d\x4d\xd8\x38\x08"
        "\x60\xe0\xd5\xd7\x75\xfa\x51\x41\x1e\x5f\xbb\x18\xac\xdf\x10\xc8\x9c"
        "\x9c\xf1\x6a\x67\xc4\xfb\x72\xb4\x68\x0a\x74\x61\x41\x59\xcb\xaa\x34"
        "\xed\x2f\x95\x0c\x37\x73\xb6\xb3\x92\x37\x72\x8f\xcb\x5c\x26\x50\x04"
        "\x52\xa2\x71\xa0\xea\x35\xb6\x22\xb3\x01\x3a\x8c\xaa\x59\xd0\x02\x88"
        "\x15\x7f\x3e\x0d\x28\x96\xee\x43\x9c\x04\x69\x79\xb5\xf2\x02\xb5",
        4096);
    syscall(__NR_pwrite64, (long)r[3], 0x20000580, 0x1000, 0);
    break;
  case 10:
    *(uint32_t*)0x20000540 = 0x200000c0;
    *(uint64_t*)0x200000c0 = 0;
    *(uint32_t*)0x200000c8 = 0;
    *(uint32_t*)0x200000cc = 0;
    *(uint16_t*)0x200000d0 = 1;
    *(uint16_t*)0x200000d2 = 0;
    *(uint32_t*)0x200000d4 = r[1];
    *(uint64_t*)0x200000d8 = 0x20000000;
    *(uint64_t*)0x200000e0 = 0x377140be6b5ef4c7;
    *(uint64_t*)0x200000e8 = 0;
    *(uint64_t*)0x200000f0 = 0;
    *(uint32_t*)0x200000f8 = 0;
    *(uint32_t*)0x200000fc = -1;
    syscall(__NR_io_submit, (long)r[2], 1, 0x20000540);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  do_sandbox_none();
  return 0;
}