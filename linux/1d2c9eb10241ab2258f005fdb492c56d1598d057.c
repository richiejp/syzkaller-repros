// kernel BUG at net/l2tp/l2tp_core.c:LINE!
// https://syzkaller.appspot.com/bug?id=1d2c9eb10241ab2258f005fdb492c56d1598d057
// status:invalid
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static void test();

void loop()
{
  while (1) {
    test();
  }
}

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;
static int collide;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    while (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE))
      syscall(SYS_futex, &th->running, FUTEX_WAIT, 0, 0);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&th->running, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &th->running, FUTEX_WAKE);
  }
  return 0;
}

static void execute(int num_calls)
{
  int call, thread;
  running = 0;
  for (call = 0; call < num_calls; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 128 << 10);
        pthread_create(&th->th, &attr, thr, th);
      }
      if (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE)) {
        th->call = call;
        __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
        __atomic_store_n(&th->running, 1, __ATOMIC_RELEASE);
        syscall(SYS_futex, &th->running, FUTEX_WAKE);
        if (collide && call % 2)
          break;
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (running)
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_bind
#define __NR_bind 361
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#ifndef __NR_close
#define __NR_close 6
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_getsockname
#define __NR_getsockname 367
#endif
#ifndef __NR_getsockopt
#define __NR_getsockopt 365
#endif
#ifndef __NR_connect
#define __NR_connect 362
#endif
#ifndef __NR_dup2
#define __NR_dup2 63
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[6];
uint64_t procid;
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0xe7d000, 3, 0x32, -1, 0);
    break;
  case 1:
    r[0] = syscall(__NR_socket, 0x200018, 0, 1);
    break;
  case 2:
    *(uint32_t*)0x20245ffc = 8;
    syscall(__NR_getsockname, r[0], 0x20e78000, 0x20245ffc);
    break;
  case 3:
    *(uint32_t*)0x207e8ff8 = 0;
    *(uint32_t*)0x207e8ffc = 0x20;
    *(uint32_t*)0x20e7a000 = 8;
    if (syscall(__NR_getsockopt, r[0], 0x84, 0x75, 0x207e8ff8, 0x20e7a000) !=
        -1)
      r[1] = *(uint32_t*)0x207e8ff8;
    break;
  case 4:
    *(uint32_t*)0x20e7aff8 = r[1];
    *(uint32_t*)0x20e7affc = 0xffff;
    *(uint32_t*)0x20e7affc = 8;
    syscall(__NR_getsockopt, r[0], 0x84, 0x76, 0x20e7aff8, 0x20e7affc);
    break;
  case 5:
    *(uint16_t*)0x20e79000 = 0x1a;
    *(uint16_t*)0x20e79002 = htobe16(0xf8);
    *(uint8_t*)0x20e79004 = 0xfe;
    *(uint8_t*)0x20e79005 = 2;
    *(uint8_t*)0x20e79006 = 1;
    *(uint8_t*)0x20e79007 = 4;
    *(uint8_t*)0x20e79008 = 0xaa;
    *(uint8_t*)0x20e79009 = 0xaa;
    *(uint8_t*)0x20e7900a = 0xaa;
    *(uint8_t*)0x20e7900b = 0xaa;
    *(uint8_t*)0x20e7900c = 0 + procid * 1;
    *(uint8_t*)0x20e7900d = 0xaa;
    *(uint8_t*)0x20e7900e = 0;
    *(uint8_t*)0x20e7900f = 0;
    syscall(__NR_bind, r[0], 0x20e79000, 0x13e);
    break;
  case 6:
    *(uint16_t*)0x20002000 = 0x1f;
    *(uint8_t*)0x20002002 = 1;
    *(uint8_t*)0x20002003 = 0;
    *(uint8_t*)0x20002004 = 0;
    *(uint8_t*)0x20002005 = 0;
    *(uint8_t*)0x20002006 = 0;
    *(uint8_t*)0x20002007 = 0;
    syscall(__NR_connect, r[0], 0x20002000, 0x32);
    break;
  case 7:
    r[2] = syscall(__NR_socket, 0xa, 2, 0);
    break;
  case 8:
    *(uint32_t*)0x200000c0 = 0x14;
    if (syscall(__NR_getsockname, r[0], 0x20000080, 0x200000c0) != -1)
      r[3] = *(uint32_t*)0x20000084;
    break;
  case 9:
    *(uint32_t*)0x20000180 = 0x20000000;
    *(uint16_t*)0x20000000 = 0x10;
    *(uint16_t*)0x20000002 = 0;
    *(uint32_t*)0x20000004 = 0;
    *(uint32_t*)0x20000008 = 0x1000;
    *(uint32_t*)0x20000184 = 0xc;
    *(uint32_t*)0x20000188 = 0x20000140;
    *(uint32_t*)0x20000140 = 0x20000100;
    *(uint32_t*)0x20000100 = 0x2c;
    *(uint16_t*)0x20000104 = 0x1c;
    *(uint16_t*)0x20000106 = 0x60a;
    *(uint32_t*)0x20000108 = 0x70bd2a + procid * 8;
    *(uint32_t*)0x2000010c = 0x25dfdbfd + procid * 4;
    *(uint8_t*)0x20000110 = 0x1c;
    *(uint8_t*)0x20000111 = 0;
    *(uint16_t*)0x20000112 = 0;
    *(uint32_t*)0x20000114 = r[3];
    *(uint16_t*)0x20000118 = 2;
    *(uint8_t*)0x2000011a = 8;
    *(uint8_t*)0x2000011b = 7;
    *(uint16_t*)0x2000011c = 8;
    *(uint16_t*)0x2000011e = 0xa;
    *(uint32_t*)0x20000120 = 9;
    *(uint16_t*)0x20000124 = 8;
    *(uint16_t*)0x20000126 = 0xb;
    *(uint32_t*)0x20000128 = 1;
    *(uint32_t*)0x20000144 = 0x2c;
    *(uint32_t*)0x2000018c = 1;
    *(uint32_t*)0x20000190 = 0;
    *(uint32_t*)0x20000194 = 0;
    *(uint32_t*)0x20000198 = 0x40;
    syscall(__NR_sendmsg, r[0], 0x20000180, 0);
    break;
  case 10:
    r[4] = syscall(__NR_socket, 0x18, 1, 1);
    break;
  case 11:
    *(uint16_t*)0x20e71000 = 0x18;
    *(uint32_t*)0x20e71002 = 1;
    *(uint32_t*)0x20e71006 = 0;
    *(uint32_t*)0x20e7100a = r[2];
    *(uint16_t*)0x20e7100e = 2;
    *(uint16_t*)0x20e71010 = 0;
    *(uint16_t*)0x20e71012 = 2;
    *(uint16_t*)0x20e71014 = 0;
    *(uint16_t*)0x20e71016 = 0xa;
    *(uint16_t*)0x20e71018 = htobe16(0x4e21 + procid * 4);
    *(uint32_t*)0x20e7101a = 3;
    *(uint8_t*)0x20e7101e = -1;
    *(uint8_t*)0x20e7101f = 1;
    *(uint8_t*)0x20e71020 = 0;
    *(uint8_t*)0x20e71021 = 0;
    *(uint8_t*)0x20e71022 = 0;
    *(uint8_t*)0x20e71023 = 0;
    *(uint8_t*)0x20e71024 = 0;
    *(uint8_t*)0x20e71025 = 0;
    *(uint8_t*)0x20e71026 = 0;
    *(uint8_t*)0x20e71027 = 0;
    *(uint8_t*)0x20e71028 = 0;
    *(uint8_t*)0x20e71029 = 0;
    *(uint8_t*)0x20e7102a = 0;
    *(uint8_t*)0x20e7102b = 0;
    *(uint8_t*)0x20e7102c = 0;
    *(uint8_t*)0x20e7102d = 1;
    *(uint32_t*)0x20e7102e = 4;
    syscall(__NR_connect, r[4], 0x20e71000, 0x32);
    break;
  case 12:
    r[5] = syscall(__NR_socket, 0x18, 1, 1);
    break;
  case 13:
    syscall(__NR_close, r[4]);
    break;
  case 14:
    syscall(__NR_dup2, r[5], r[2]);
    break;
  }
}

void test()
{
  memset(r, -1, sizeof(r));
  execute(15);
  collide = 1;
  execute(15);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}