// INFO: task hung in snd_pcm_oss_write
// https://syzkaller.appspot.com/bug?id=1106bd1fd40754fb1034e8ca884d23e08a4bbc2e
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/capability.h>
#include <linux/futex.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

static void fail(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
}

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* uctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  doexit(sig);
}

static void install_segv_handler()
{
  struct sigaction sa;

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);

  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                        \
  {                                                                            \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    }                                                                          \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
  }

static void use_temporary_dir()
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    fail("failed to mkdtemp");
  if (chmod(tmpdir, 0777))
    fail("failed to chmod");
  if (chdir(tmpdir))
    fail("failed to chdir");
}

static void vsnprintf_check(char* str, size_t size, const char* format,
                            va_list args)
{
  int rv;

  rv = vsnprintf(str, size, format, args);
  if (rv < 0)
    fail("tun: snprintf failed");
  if ((size_t)rv >= size)
    fail("tun: string '%s...' doesn't fit into buffer", str);
}

static void snprintf_check(char* str, size_t size, const char* format, ...)
{
  va_list args;

  va_start(args, format);
  vsnprintf_check(str, size, format, args);
  va_end(args);
}

#define COMMAND_MAX_LEN 128
#define PATH_PREFIX                                                            \
  "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin "
#define PATH_PREFIX_LEN (sizeof(PATH_PREFIX) - 1)

static void execute_command(const char* format, ...)
{
  va_list args;
  char command[PATH_PREFIX_LEN + COMMAND_MAX_LEN];
  int rv;

  va_start(args, format);
  memcpy(command, PATH_PREFIX, PATH_PREFIX_LEN);
  vsnprintf_check(command + PATH_PREFIX_LEN, COMMAND_MAX_LEN, format, args);
  rv = system(command);
  if (rv != 0)
    fail("tun: command \"%s\" failed with code %d", &command[0], rv);

  va_end(args);
}

static int tunfd = -1;
static int tun_frags_enabled;

#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define MAX_PIDS 32
#define ADDR_MAX_LEN 32

#define LOCAL_MAC "aa:aa:aa:aa:aa:%02hx"
#define REMOTE_MAC "bb:bb:bb:bb:bb:%02hx"

#define LOCAL_IPV4 "172.20.%d.170"
#define REMOTE_IPV4 "172.20.%d.187"

#define LOCAL_IPV6 "fe80::%02hxaa"
#define REMOTE_IPV6 "fe80::%02hxbb"

#define IFF_NAPI 0x0010
#define IFF_NAPI_FRAGS 0x0020

static void initialize_tun(uint64_t pid)
{
  if (pid >= MAX_PIDS)
    fail("tun: no more than %d executors", MAX_PIDS);
  int id = pid;

  tunfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
  if (tunfd == -1) {
    printf("tun: can't open /dev/net/tun: please enable CONFIG_TUN=y\n");
    printf("otherwise fuzzing or reproducing might not work as intended\n");
    return;
  }

  char iface[IFNAMSIZ];
  snprintf_check(iface, sizeof(iface), "syz%d", id);

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, iface, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_NAPI | IFF_NAPI_FRAGS;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) {
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
      fail("tun: ioctl(TUNSETIFF) failed");
  }
  if (ioctl(tunfd, TUNGETIFF, (void*)&ifr) < 0)
    fail("tun: ioctl(TUNGETIFF) failed");
  tun_frags_enabled = (ifr.ifr_flags & IFF_NAPI_FRAGS) != 0;

  char local_mac[ADDR_MAX_LEN];
  snprintf_check(local_mac, sizeof(local_mac), LOCAL_MAC, id);
  char remote_mac[ADDR_MAX_LEN];
  snprintf_check(remote_mac, sizeof(remote_mac), REMOTE_MAC, id);

  char local_ipv4[ADDR_MAX_LEN];
  snprintf_check(local_ipv4, sizeof(local_ipv4), LOCAL_IPV4, id);
  char remote_ipv4[ADDR_MAX_LEN];
  snprintf_check(remote_ipv4, sizeof(remote_ipv4), REMOTE_IPV4, id);

  char local_ipv6[ADDR_MAX_LEN];
  snprintf_check(local_ipv6, sizeof(local_ipv6), LOCAL_IPV6, id);
  char remote_ipv6[ADDR_MAX_LEN];
  snprintf_check(remote_ipv6, sizeof(remote_ipv6), REMOTE_IPV6, id);

  execute_command("sysctl -w net.ipv6.conf.%s.accept_dad=0", iface);

  execute_command("sysctl -w net.ipv6.conf.%s.router_solicitations=0", iface);

  execute_command("ip link set dev %s address %s", iface, local_mac);
  execute_command("ip addr add %s/24 dev %s", local_ipv4, iface);
  execute_command("ip -6 addr add %s/120 dev %s", local_ipv6, iface);
  execute_command("ip neigh add %s lladdr %s dev %s nud permanent", remote_ipv4,
                  remote_mac, iface);
  execute_command("ip -6 neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv6, remote_mac, iface);
  execute_command("ip link set dev %s up", iface);
}

static void setup_tun(uint64_t pid, bool enable_tun)
{
  if (enable_tun)
    initialize_tun(pid);
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 128 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);

#define CLONE_NEWCGROUP 0x02000000

  unshare(CLONE_NEWNS);
  unshare(CLONE_NEWIPC);
  unshare(CLONE_NEWCGROUP);
  unshare(CLONE_NEWNET);
  unshare(CLONE_NEWUTS);
  unshare(CLONE_SYSVSEM);
}

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
    close(fd);
    return false;
  }
  close(fd);
  return true;
}

static int real_uid;
static int real_gid;
__attribute__((aligned(64 << 10))) static char sandbox_stack[1 << 20];

static int namespace_sandbox_proc(void* arg)
{
  sandbox_common();

  write_file("/proc/self/setgroups", "deny");
  if (!write_file("/proc/self/uid_map", "0 %d 1\n", real_uid))
    fail("write of /proc/self/uid_map failed");
  if (!write_file("/proc/self/gid_map", "0 %d 1\n", real_gid))
    fail("write of /proc/self/gid_map failed");

  if (mkdir("./syz-tmp", 0777))
    fail("mkdir(syz-tmp) failed");
  if (mount("", "./syz-tmp", "tmpfs", 0, NULL))
    fail("mount(tmpfs) failed");
  if (mkdir("./syz-tmp/newroot", 0777))
    fail("mkdir failed");
  if (mkdir("./syz-tmp/newroot/dev", 0700))
    fail("mkdir failed");
  if (mount("/dev", "./syz-tmp/newroot/dev", NULL,
            MS_BIND | MS_REC | MS_PRIVATE, NULL))
    fail("mount(dev) failed");
  if (mkdir("./syz-tmp/newroot/proc", 0700))
    fail("mkdir failed");
  if (mount(NULL, "./syz-tmp/newroot/proc", "proc", 0, NULL))
    fail("mount(proc) failed");
  if (mkdir("./syz-tmp/pivot", 0777))
    fail("mkdir failed");
  if (syscall(SYS_pivot_root, "./syz-tmp", "./syz-tmp/pivot")) {
    if (chdir("./syz-tmp"))
      fail("chdir failed");
  } else {
    if (chdir("/"))
      fail("chdir failed");
    if (umount2("./pivot", MNT_DETACH))
      fail("umount failed");
  }
  if (chroot("./newroot"))
    fail("chroot failed");
  if (chdir("/"))
    fail("chdir failed");

  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    fail("capget failed");
  cap_data[0].effective &= ~(1 << CAP_SYS_PTRACE);
  cap_data[0].permitted &= ~(1 << CAP_SYS_PTRACE);
  cap_data[0].inheritable &= ~(1 << CAP_SYS_PTRACE);
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    fail("capset failed");

  loop();
  doexit(1);
}

static int do_sandbox_namespace(int executor_pid, bool enable_tun)
{
  int pid;

  setup_tun(executor_pid, enable_tun);

  real_uid = getuid();
  real_gid = getgid();
  mprotect(sandbox_stack, 4096, PROT_NONE);
  pid =
      clone(namespace_sandbox_proc, &sandbox_stack[sizeof(sandbox_stack) - 64],
            CLONE_NEWUSER | CLONE_NEWPID, NULL);
  if (pid < 0)
    fail("sandbox clone failed");
  return pid;
}

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

long r[1];
uint64_t procid;
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
    break;
  case 1:
    NONFAILING(memcpy((void*)0x206f48e5, "/dev/audio", 11));
    r[0] = syscall(__NR_openat, 0xffffffffffffff9c, 0x206f48e5, 1, 0);
    break;
  case 2:
    NONFAILING(*(uint64_t*)0x20d30fb0 = 0x20d12f42);
    NONFAILING(*(uint64_t*)0x20d30fb8 = 0xbe);
    NONFAILING(*(uint64_t*)0x20d30fc0 = 0x20d1afa7);
    NONFAILING(*(uint64_t*)0x20d30fc8 = 0x59);
    NONFAILING(*(uint64_t*)0x20d30fd0 = 0x20e4f000);
    NONFAILING(*(uint64_t*)0x20d30fd8 = 0xd6);
    NONFAILING(*(uint64_t*)0x20d30fe0 = 0x20116000);
    NONFAILING(*(uint64_t*)0x20d30fe8 = 0x1000);
    NONFAILING(*(uint64_t*)0x20d30ff0 = 0x20660000);
    NONFAILING(*(uint64_t*)0x20d30ff8 = 0x99);
    NONFAILING(memcpy(
        (void*)0x20d12f42,
        "\xdc\x5e\xd8\x16\xc0\x30\x99\x6d\x53\x33\x22\x01\x9c\x07\xbe\x1f\x77"
        "\x34\x5a\x3b\x8d\x72\x5b\x8a\x1b\x3e\x62\x5c\xd5\x59\x58\xfa\x2f\xcd"
        "\x87\xdc\xe6\x8b\x5b\x0d\x4f\xc1\x0b\xbe\x2c\xa3\x65\x55\xb8\x34\xa5"
        "\x79\x8c\x19\x5b\xf8\xfd\x78\x2d\x6f\xad\x37\x5b\xd0\x07\x98\x3e\x1d"
        "\x71\xba\x11\x56\x67\x27\x31\x9c\xa6\x91\x43\x1c\x7a\x26\x26\xe6\x1c"
        "\x58\x5b\x55\x4e\xd1\x2f\xec\xa6\xb2\xfa\x88\xd8\x50\x82\xd1\xe0\x06"
        "\x1b\xbe\x4c\x5f\xcc\xfb\x81\x58\x44\xbe\x94\x36\xd9\x9a\x1d\xd0\x6c"
        "\x96\xaa\xcb\x09\x0e\xfc\xfd\xfe\x7f\x44\x1e\x7d\xcf\x62\x5b\x79\x7e"
        "\x1d\x5e\x50\x4c\x2a\xdd\xcf\x2c\xf2\xa2\x3e\xb2\xdb\xa9\xbc\x66\x76"
        "\xe3\xbd\xcf\xd6\x96\xcc\x31\xa9\x5c\x19\x20\x24\xa1\x8c\x24\x87\x1c"
        "\xf8\xff\x3a\xf4\x37\x08\x79\xcd\xbd\x43\x0d\x44\x95\xb6\xf7\xd4\x75"
        "\xdd\xf2\x5d",
        190));
    NONFAILING(
        memcpy((void*)0x20d1afa7,
               "\x6a\x38\xe5\x37\x23\xee\xf9\x43\x1b\x20\x2d\x45\x65\x0b\xc7"
               "\xa5\xf3\x43\xc7\x48\xbf\x06\xe7\x41\x90\x6a\xac\x6e\x88\x69"
               "\x7c\x4e\xc7\x8a\x4c\xba\x02\x85\xc7\x64\xb1\x6d\xf5\xd2\x17"
               "\xc7\xe9\x73\xbd\x47\x6d\x83\xb6\x5d\x8a\x13\x25\x49\x00\xc9"
               "\xe9\x95\xde\x01\xb4\xb5\xe5\x8f\x63\x4b\x80\x9f\xc8\x0c\xa2"
               "\x10\x37\x19\x6e\xb0\xe8\x5b\x30\xef\xfc\x94\xca\xc7\xa0",
               89));
    NONFAILING(memcpy(
        (void*)0x20e4f000,
        "\x0a\x8b\x6a\x3b\x88\x50\x99\x46\x94\x3a\x74\xc6\x53\x82\x05\x34\x7d"
        "\x1c\x18\x8e\xf4\x4b\x98\x37\xaf\x69\xcd\x4f\xa9\x7a\x8a\x93\xdf\x3d"
        "\x6d\x15\xbd\xad\x87\xcd\xff\xb7\x14\x5e\xce\xee\xf5\xd7\xb5\x60\x09"
        "\x88\xb4\xed\xe9\xce\x14\x15\x79\x4f\x3a\x79\x15\x22\xd5\xb0\x1e\xc0"
        "\xe8\x94\x5a\x5d\xa1\xde\x2e\x3c\xce\xc9\x58\xd0\xdd\x63\x17\x84\x37"
        "\x45\xc8\x96\xe9\xc6\x00\xa3\x8e\x27\xb2\x92\x8a\x7b\x35\x9b\x5a\x24"
        "\xaa\x3e\x4b\x18\xe2\xdd\x6f\xc9\x41\xcd\x51\xd8\xce\x9f\x8e\x28\xa9"
        "\xce\x63\xab\x0f\xd8\xdc\x6d\x93\xc5\x0a\x40\x8a\xd1\x20\xa6\xbc\xaf"
        "\x8d\xdd\xaf\xa6\x5c\x2f\x82\x1e\x6d\x94\xb3\x28\xab\xf9\x0d\x91\x63"
        "\x70\x11\xd1\x03\x8c\xb9\x9b\x16\x67\x33\x4d\x77\x98\x03\x12\x66\x1b"
        "\x62\x57\xc1\xc7\x22\x0c\x9c\x46\xb1\x87\xbd\xbc\x5b\xa2\x4a\x95\x6f"
        "\xed\xdf\x54\xed\xe7\xec\x39\x75\xb7\x3d\xad\xde\xe0\x72\x33\xcf\xda"
        "\xd2\xf9\x04\xcc\x5f\x32\x10\x41\xe4\x07",
        214));
    NONFAILING(memcpy(
        (void*)0x20116000,
        "\x31\xf0\x1e\xef\x86\x0f\x54\x6e\x6d\x31\x27\x00\x4c\xba\x09\x3b\xa8"
        "\x48\xf5\xb8\xae\x07\x08\x34\xdd\x4a\x11\xf1\x2f\xbc\xc4\x03\xd8\xe4"
        "\xe5\x6f\x95\xd2\x06\x3a\x87\x42\xa9\x1c\x21\x45\x30\x9e\x04\x27\x72"
        "\xdb\xda\xa5\x3c\x6d\x00\x03\xa8\x8a\xd2\xc9\x7d\x55\x69\xac\xc5\x16"
        "\xb4\xb6\xa2\x64\xbc\x94\xed\xb7\x3f\xeb\x6d\x2f\xe4\xce\x47\x39\x71"
        "\xd2\xbe\x5e\x50\x38\xbd\x4e\xad\x88\xdd\x60\xd3\x3b\x2c\xc7\x0b\x98"
        "\xe5\xf9\x77\xdc\xe8\x10\xeb\x6b\x1b\xd4\x98\xc7\x6f\xd3\x51\xb0\x48"
        "\x0b\xa8\xdb\x26\xf7\xb6\x32\xfa\x37\x28\x6c\x9c\x50\x34\xb6\x69\xbe"
        "\x24\xd4\xf8\x50\x86\xc2\x8d\x2c\x56\x5b\xc3\xcc\x2b\x1e\xa3\xd7\x2d"
        "\x98\xd2\x52\x19\xec\x83\x73\xb3\xe5\x07\x08\x5e\x3e\x0c\xea\x24\x06"
        "\xb6\x59\x45\x75\xf4\xef\x2c\x53\x03\x93\x79\xe1\x62\x9b\xdf\x1f\xf1"
        "\xd6\xa9\x55\x3f\xd8\x38\x7f\xad\xb2\x54\xfe\x74\xaf\x72\x75\x0a\xc6"
        "\x09\x12\xbc\xb5\xcf\x66\x3a\x65\x53\xb8\xe7\xd4\xca\xf7\x42\xa2\x12"
        "\x0e\x64\x19\xe1\x9b\xe0\x45\xf2\x6e\xf7\xc0\x2f\xa4\x1b\x0c\xa7\xcd"
        "\x28\x64\x9b\xe0\x81\x93\xa0\xc2\x95\xf5\xdc\x2e\x20\x6a\xad\xb7\x08"
        "\x00\x00\x83\x2f\x13\x24\xf1\x01\xbe\x35\xc9\x00\x00\x06\x2b\x80\xf7"
        "\x0c\xab\xbe\xdb\x33\x94\x53\xe9\x25\x64\x05\x9c\xdd\x3e\x17\xf7\x30"
        "\x82\x29\x4d\x11\x19\x40\xed\x6c\x00\x39\x0b\x34\x4a\xdc\xf1\x49\x3d"
        "\xf0\xad\x15\x89\x16\xce\x82\x46\x49\x90\x5e\xb1\xb0\xdd\x0d\x8c\x91"
        "\xee\x2a\xa2\x30\x88\x28\x03\x88\x70\xef\x84\x37\x73\x28\x80\x73\xae"
        "\x89\x71\xfd\x0f\xe7\x72\xf1\x77\xcb\x64\x35\x6e\xf8\xce\x64\x26\x3b"
        "\x25\x88\xa4\xed\xb4\x84\x3e\xed\x75\x94\x80\x70\x0b\x54\x9b\x78\xf3"
        "\xf3\x16\xe8\x74\x70\xb3\xd8\x8b\x9b\x06\x0c\x0d\x67\x45\xdd\xdc\x1a"
        "\x83\x94\xf9\x75\x6c\x8b\xf0\xa6\xc0\x8a\xab\x71\xd0\xce\x51\xcf\xf7"
        "\xb2\x70\x6c\x30\x00\xef\x90\xdd\xd5\xd0\xe6\x56\xac\x68\x8c\xb8\x95"
        "\xdb\x20\x9f\x52\xd1\x01\xe1\x42\xa2\xdb\xdf\xbd\xbf\x48\x94\x9e\x12"
        "\xf0\x84\x6d\x5b\xee\x43\x29\xa5\x68\x26\x69\x2a\xff\x22\x83\x74\x79"
        "\x4f\x78\x88\x84\x7c\xa6\x01\xd8\x7e\x90\xb7\x16\xf9\xe7\xb7\x3d\xa0"
        "\xaf\xcf\x12\x5f\x43\xf7\x18\xcc\xef\xce\xee\x83\x2e\xef\xa1\x81\xed"
        "\x2a\xf1\x42\x89\xab\xdc\xad\x96\xca\x3c\x96\x4f\x57\x4e\xea\x6f\x53"
        "\xdc\x00\x41\x09\x74\x46\x9d\xe0\x4c\x66\x15\x83\xeb\xd0\xf7\x8b\x66"
        "\xaf\x5c\x30\x05\x4d\x81\xb9\xa5\x2a\x69\x2f\xd7\x92\x63\x76\x3d\x52"
        "\xfa\x98\x64\x5a\xf2\x9a\xc8\xa0\x60\xbd\x67\x17\x03\xf9\xa1\x6e\xb8"
        "\xf8\xa5\x35\xe9\x58\xca\xc7\xaa\x28\x5c\xdb\x56\xc9\xb9\xca\xd1\x9d"
        "\x2b\xd8\x99\x4e\xfe\xcd\x0b\x30\x25\x29\x03\x92\x81\x8a\xaf\x07\x90"
        "\x5e\x4e\x5c\xb3\x71\x6d\x2f\x0a\x7a\xfc\xae\x88\x69\x64\x46\x6f\xfa"
        "\xbe\xde\xe0\x6a\x41\x0e\x73\x3d\x31\xd4\xa0\xde\xb6\xed\x86\x13\x2d"
        "\x43\x5a\x65\xee\xff\xa5\xb3\xfa\xb1\x79\x6c\x64\x56\x7a\xe9\x6d\x7f"
        "\x2d\x7b\x58\x92\x91\x91\xf9\x09\x2a\x9f\x2d\x7e\xe9\x42\xeb\x1a\x4a"
        "\x1b\x86\x3d\x83\x47\x36\x08\x46\x10\x6d\x50\xc1\x26\x86\x70\xb5\x8b"
        "\x8c\xa0\x28\xac\x1c\x6e\x1f\x74\xac\x1c\x09\x57\x29\x6a\xc7\xdd\xf1"
        "\x52\x29\xcd\x28\x30\x31\xd4\x1c\x6e\x69\xe6\x77\xa3\xb8\xc0\x70\x87"
        "\x23\xdb\xf0\x89\x2f\x87\x0b\x98\x78\xba\xd6\xb1\x36\xa8\xfd\xa9\x6b"
        "\x2f\x81\x72\xbf\xac\xe8\x43\x52\x0d\x35\xf9\x75\x3b\xcd\xe0\x08\xc3"
        "\x63\x3d\xfc\x68\xbb\x06\xc4\x44\xeb\x93\xa6\x1c\x1b\x01\x42\xbb\x6e"
        "\x70\x9d\xa4\x27\x91\xb4\x3b\xeb\xa3\xb4\x60\x9c\x05\xb1\x93\x60\x63"
        "\x42\x08\x53\x45\x9d\xed\x28\xff\x7b\x2c\x86\x07\x5a\x94\x25\x36\x63"
        "\x9f\x2f\x0a\xe3\x59\x4c\xf5\x69\x17\xa0\x74\xf5\xe0\xa4\x20\x2b\x0d"
        "\xb8\x35\xfe\xb7\x12\xda\x6f\x93\x98\xa3\x5f\x79\x26\x67\x13\x83\x0a"
        "\x98\x3d\x97\xa4\xe1\x19\xfa\x6c\xc6\x35\x04\xa9\x7e\x2e\x55\xab\x52"
        "\x30\xcb\x9a\x2f\x44\xdb\xfc\x2c\x9a\xd2\xe0\x0b\x92\x82\x0e\xde\x1b"
        "\xb5\x4f\x50\x1a\xe8\x3b\x6e\x30\x10\xa6\x7c\xa6\x95\x14\x22\x22\xa2"
        "\x8f\x12\xc7\x05\xce\x47\xab\x51\x7c\x49\xcb\xd0\x60\xe0\x3b\x98\x2c"
        "\x6d\x31\xc7\x65\x28\x0a\xda\xfe\xd7\xa3\xc2\xaa\x55\x04\x90\x73\x83"
        "\x4e\xb0\xc1\xa1\x23\xef\x43\xa9\xe6\x38\x2c\xbf\x6d\x13\xe4\x1b\xa3"
        "\x73\x1e\x0f\xad\x72\xcb\x6d\x7c\x6f\x90\xe1\xff\x4b\x98\x84\x80\x9d"
        "\xfe\x7d\x17\xf3\x11\x51\xdd\x17\x42\x11\x43\x44\x7d\xb1\x0f\xb5\x91"
        "\x75\x91\x16\x43\xbc\xb0\xa2\xdf\x70\x0a\xe2\xda\x6a\xb9\xa1\x34\xe1"
        "\x3a\xfb\x67\x8c\x8f\x3c\x94\x73\xdd\x56\x97\xf0\x34\xca\xa1\x72\x16"
        "\xf6\x20\x28\x33\x4a\x58\x59\x13\x20\x5f\x04\xf1\x28\xab\x3a\x10\x09"
        "\xb3\xf4\x1b\xd2\x38\xa1\x5d\x71\xad\x78\x4b\x3b\xf7\x3b\xa1\x94\x8c"
        "\x6f\x52\xe0\x46\x03\x70\x72\xfa\x34\x37\x53\x73\x8d\x8a\x1e\xef\x4e"
        "\xa7\x6b\xae\xbc\x6b\x06\x18\x28\x21\xff\xa0\xff\x56\xd2\x1c\xa9\xb3"
        "\x0a\xa0\x07\x88\x73\x11\x62\xef\x50\xcf\xdd\x15\xa5\xb0\xb7\x7f\x6f"
        "\x04\xd1\x01\xed\x86\x01\x5d\x9d\x82\x39\x24\x4d\xa3\x18\xa9\x9d\x2e"
        "\xe7\x45\xd4\x61\xe1\x77\x68\x52\xba\xb1\xa5\x57\x85\x7f\x70\x5e\x1f"
        "\x99\xcd\xab\xe7\x95\x5a\x5e\xe7\xfc\x92\x5d\x47\xbb\xef\xb2\x14\x2c"
        "\x9c\x6e\xf5\x7b\x84\x18\x12\x01\x93\x87\xe3\xcf\xa1\x2d\x79\x7c\x15"
        "\x3d\x58\xb3\x28\xc3\x36\x5f\xae\x4a\x1d\xb8\x84\xc8\x06\x16\x25\xf0"
        "\xf7\x35\xdc\xf0\xc2\x03\x60\xa4\xb8\xa2\x74\x5f\xd5\x2d\x9d\xde\x72"
        "\x51\x5a\xe0\xe6\xca\x26\x82\xc4\x34\x96\x3f\xbd\x1d\xbc\x6f\x97\x8d"
        "\x10\xa2\x28\x89\xa9\x21\x54\xd7\xe0\xb5\x00\xdb\xbd\x29\x78\xef\xc2"
        "\xac\x71\xbb\x49\xc8\xb1\x2f\x93\x05\xc9\x24\x70\x88\xd8\x16\xfe\xb4"
        "\x37\x29\x8b\xc8\xbc\x29\x15\xf9\x5a\x5a\x77\x62\x08\xe5\x11\x33\xc6"
        "\x72\x33\x04\x91\x2c\xbd\xbb\x7e\xb5\xc9\x71\xe1\x0c\xbb\x08\x25\xba"
        "\x4d\xff\x67\x38\xa0\x9a\x1a\x7f\x99\x91\x15\x90\x7d\xc9\xd3\xe6\x2d"
        "\x77\xd2\x68\xc7\xef\x42\xf9\x86\xdc\x74\x55\x3c\x8e\x20\x09\xc9\xc4"
        "\xbe\x43\x6f\x4e\xb0\xe9\xe8\x15\xb3\xf4\xa8\xdb\x96\x68\x7b\xa7\x1e"
        "\xd5\xb3\x04\x26\x76\x41\xb7\x4e\xd0\x1f\xa9\x0b\x2f\xd8\x45\xdb\x56"
        "\x9b\x4d\xf2\x15\xbe\x6d\x87\x9d\xbc\x30\xd7\xa1\x79\xe3\xac\x20\x3b"
        "\x76\xc9\x15\xed\x80\x2d\x1d\x78\x9f\xc4\xf4\x92\xbe\x41\xbd\xd1\x2e"
        "\xe3\x22\x31\x77\x16\x42\x14\x6e\x03\x4e\x11\x08\x9a\x8e\xd3\xb5\xfc"
        "\x0d\xc2\xcc\x27\x13\x1b\x61\xb8\x7c\x22\xc9\xc1\xe6\xeb\xfa\x96\x6e"
        "\x78\x92\xd7\x0d\x1b\xf9\x0d\xf3\xa5\xb4\xb2\xf0\x88\xd6\x2b\x31\x8f"
        "\xef\xad\xeb\x4b\xcc\x23\xb0\x85\x16\x5e\xa0\xc2\x1d\x06\x9d\xba\x07"
        "\x09\xf1\xa1\xa2\x26\xc7\xad\xdf\xd3\x0d\x4d\xc7\x90\xa0\xc8\xcc\x23"
        "\x4d\x4d\x86\x7b\xb9\xab\x32\x14\x46\x42\x75\x5a\x72\x6d\x0d\x8a\x80"
        "\xc8\x7f\xd3\x39\x2f\x26\xc8\xe9\xec\x02\xd8\xa3\x6b\xd0\xf1\x7e\xbd"
        "\x09\x57\xc2\x38\x93\x94\x81\x90\x74\x41\x62\x50\xde\xb9\xa0\x7c\x26"
        "\xbf\x11\x27\x63\x6c\x7f\xd3\x6a\xfd\xea\xce\xfb\xb0\xb8\xa7\xb3\x56"
        "\xb5\xb6\xdd\xea\xc9\x58\x7c\x29\x91\x9a\xe4\x14\x54\x2a\xca\xb6\x98"
        "\x86\xbb\x7b\xd6\xc8\x81\x99\x65\x17\x1e\x12\xb2\x8f\xfb\xe0\xc7\xf7"
        "\x7f\x54\xea\xab\xd9\x32\x87\x76\x37\xeb\x2a\xe7\xb2\x1f\x36\x4c\x86"
        "\x74\x61\x22\xe0\x59\x93\xc4\x52\xcf\x54\x29\x89\x15\x91\xcf\x07\x37"
        "\x5b\x6a\x26\xe2\x7d\x0d\x32\xec\x19\x72\x91\xc0\x02\x50\x00\x78\x26"
        "\xae\xf4\x60\x13\x74\xc2\x5f\xb0\x49\xf7\x41\xe0\x55\x6d\xfe\x7e\x59"
        "\xdc\xd4\x3a\xc2\x66\xcc\x69\x14\xb4\x40\x58\x81\x02\x11\x21\x04\xd4"
        "\x9e\x55\x1d\x65\xaf\x6a\xb5\xa1\xf8\x22\x3d\x60\xb0\x06\x3c\x90\x21"
        "\x02\x25\xea\xee\x27\xb3\x1b\xb8\x8b\x98\xcb\x7c\xcb\x87\xf3\x83\x09"
        "\x7f\xdb\x81\x28\xdc\xf9\x0e\xb2\xa3\x5d\x34\x22\xf5\x59\xd8\x19\xb8"
        "\x91\x4c\x4b\x6d\x84\xf0\x71\x53\x9e\xbf\xfb\xcf\xa9\x63\xda\xff\x9b"
        "\x7e\x72\x44\x5c\x54\xcb\x1e\x33\x5e\x43\x6c\x6c\x61\x5d\x4c\xaf\x62"
        "\x7d\xac\x35\x83\x0d\x17\xf3\xed\x04\xf9\x57\xa1\xf7\xe1\x63\x88\x0f"
        "\x98\xdb\x80\x66\xc0\x13\xf6\x69\x17\x65\x7f\xed\xe0\xc3\x83\x19\x42"
        "\xf4\x68\x06\x0d\x55\x4b\x18\x4a\x96\x8a\x66\x52\x47\x13\x95\x90\x3e"
        "\x2a\x20\xb5\xdf\x05\xe1\xde\xa8\x98\x9a\xc9\x40\x4d\x0a\x99\x8e\x7e"
        "\x7c\x38\x7b\x11\x80\x09\x16\x9a\xe5\x01\x32\x71\x25\xf7\x53\x54\x5f"
        "\xba\xda\x92\x5c\x8c\xc8\x5d\x57\x8f\x90\x0d\xd4\x5f\x01\xb7\x2a\x8f"
        "\xa9\xaf\xe8\xef\x75\xfd\xa2\x0f\x4f\x4e\xe9\xdf\x1e\xdb\xab\xff\x09"
        "\xaf\x89\x5a\x7c\x3b\xd1\xf5\xcc\x38\xe1\x87\xef\xfc\x49\x95\x61\x34"
        "\xf4\x16\xe9\xdc\xab\xef\xef\x3e\xa8\x21\x98\x4c\x0e\x01\xb7\x39\x19"
        "\xc0\xc9\x74\xfa\xa5\x89\x53\xdd\x79\x1c\xdd\x6f\x58\x28\xd6\xc5\x6d"
        "\x9b\xd5\x5a\x16\x37\xbc\x09\xf7\x34\xbd\x0a\x1d\x3f\x45\x80\x70\xb5"
        "\xbe\x87\x6a\xd1\x5a\x57\x45\x4b\x0a\x74\xca\x5c\x36\x63\x0a\x63\x07"
        "\x62\xba\x74\xba\x0d\xd0\xbf\x4b\x90\xd0\x74\x21\x17\x19\x7a\x08\x41"
        "\x77\x06\x78\x3d\x59\xb4\x9b\x88\x95\x50\x7d\x6f\x99\xbf\x6f\xc6\x9a"
        "\xe5\xae\xc5\xea\x31\x79\x83\x2f\x95\x5b\x91\x29\xb2\xcc\x94\x51\x15"
        "\xde\x93\xe0\x48\xab\x65\xd4\xb9\xc1\x32\xf9\x68\x7a\x23\x2a\x74\xc2"
        "\x7e\xde\x64\xb1\xa5\xd7\xd8\xd9\xd8\xfe\x8f\xc5\x1a\x89\xb9\x70\xa4"
        "\x08\xa6\xce\x26\x1b\x73\x03\x7e\x3f\x43\x97\x0f\xdc\x8d\xae\xcf\xe6"
        "\x65\xa4\xee\x1c\x6c\xf5\x0e\x35\x51\x34\x44\x6d\x5c\x39\xb4\x38\xb6"
        "\x06\xd7\xff\x89\xed\x53\xa8\x6f\x49\x61\x67\xec\x8e\x9a\x90\x15\x47"
        "\x29\xae\x4c\xc2\x54\xf6\x63\x88\x07\xf2\x8b\xb3\x3a\xa5\xca\x8c\x84"
        "\x86\x82\xc4\x7e\x5f\x65\x6b\xc8\x27\x80\x0f\xcb\xdd\xb6\xc3\xef\xbd"
        "\x23\x2a\x3f\x5c\x30\xd7\x4e\x80\xc9\xef\xca\x0f\x9e\x30\x03\x0e\x6b"
        "\x88\x83\xf4\x57\x67\x09\xcc\xa5\x85\xe0\x94\xdd\xa5\xb2\x27\x80\x2a"
        "\x8c\x71\x3f\x7a\x76\x8f\x03\x44\x69\x72\xd7\x30\xd2\xc9\x7d\x8e\x02"
        "\x9b\x5a\x2f\x8c\x18\xb7\x3c\xe2\xd9\x59\xe3\xd5\xee\xac\x12\x81\x9f"
        "\xa0\x22\x4c\x92\x01\x62\xa3\xbc\x16\x6b\x0a\x90\x92\xa0\x68\x60\xca"
        "\x48\x82\xd9\x3a\x25\x83\xe0\x5b\xdd\xda\x7b\x01\xb7\xa1\x8a\xec\xe1"
        "\x15\x95\xa6\x8d\x2d\x9f\x91\xc1\x9b\x90\x49\x9a\xd0\x0c\x7a\x8e\x7c"
        "\x04\xd7\xb6\xc1\x34\x9b\x94\xf1\xf5\x94\xcf\xd7\x4b\x6d\xf9\x32\x2c"
        "\x67\x4f\xc2\x7a\x2b\x8a\xfa\x85\x41\xb7\x5f\x65\xc8\x07\xea\xfa\x00"
        "\x7f\x93\x35\xd9\x6b\xe8\xc6\x84\xbf\xfe\xc2\x1a\x6a\x9f\x0d\x18\x9b"
        "\x76\x99\x96\xef\x4a\x70\x53\xa4\x60\xcc\x81\x99\x44\xf5\x0a\xe2\x4c"
        "\x9e\x17\x58\x71\x84\x4f\x96\xf9\xd7\x18\x5d\x4f\xa0\x29\xd8\x1b\x54"
        "\xd6\xd6\xf8\x8c\xdc\x67\xc4\xb7\x98\x2b\x33\xe0\xec\xcf\x7e\x11\x8d"
        "\x82\x25\x0a\x5e\xcc\x59\xc0\x0a\xcd\x95\x10\xca\x69\x3a\xd0\x14\x23"
        "\x5f\x1d\x9c\x50\xbb\xab\x97\x5b\x6a\x6a\x7b\x9b\x4b\x15\x25\x31\x4d"
        "\x97\x7c\xeb\xdb\xe4\xde\x52\x4c\x8b\xab\x9a\x51\xbf\x45\xbd\xd1\x45"
        "\x09\x3b\x75\xd6\xe6\x97\x2a\x96\x93\x5f\x80\x09\x39\xbf\xe6\xe2\x14"
        "\x4c\x67\x2b\x8d\xf2\x30\x4a\x19\xaf\x1f\x6e\x13\x5a\xdd\x2e\x38\x01"
        "\x33\xa2\x0b\x25\x7b\xa9\x01\x29\x3c\xbc\xc7\x37\xea\xad\x5c\xd9\x1b"
        "\xca\xf9\xb1\xbe\x1d\x25\x8b\x9b\xe3\x91\x84\x73\x7e\x9d\x95\x01\xf7"
        "\x9c\x42\x32\x9b\xe0\xeb\x41\xc1\x97\xc1\x6a\x13\x40\x1c\xfa\xe5\x9b"
        "\x9b\x3d\x4f\x12\x1b\xad\x72\xdd\xa4\x73\x3b\xaf\xf5\xcd\x8a\x66\x97"
        "\xd0\x04\xd4\x4e\xaf\xc5\xeb\x9f\xc2\x53\xca\x63\x5b\x99\x81\x74\x33"
        "\x06\x57\xb4\x30\xde\x12\x0a\xed\x19\xcb\x5a\x3d\x72\x20\xc8\x10\x6a"
        "\xe8\x70\x46\xf6\x38\x35\xa8\x39\xcb\xc1\x28\x15\x24\x4f\x72\xbb\x54"
        "\x58\xc4\xdf\x40\x4d\x35\x44\xfc\xe8\xc9\x62\xc9\x7d\x25\x13\xc8\x25"
        "\x6c\x35\xe4\x27\xc0\xbe\x92\x6b\xbe\xdb\xd4\xc3\x11\x4b\x6e\x48\x3b"
        "\x00\x39\x9f\xee\xc1\x02\xbe\x2f\xb8\x47\xde\xf7\x35\x4f\xef\xc3\x4d"
        "\xcd\x5a\x06\x8d\xe8\x6d\x91\x77\x84\x2d\x7d\x48\xbf\x80\xaf\xc7\xbc"
        "\xf2\x56\xde\xcc\x90\xc6\xc1\x3a\x2d\xf7\x09\xd3\x48\x47\x9e\x81\xb9"
        "\xfa\x37\xf6\x75\x4f\x4b\x71\xb2\xe4\x24\xda\x20\xa3\xf1\xe3\x4c\x4c"
        "\xc0\x9b\xfa\x2b\x1e\x7c\xee\x2f\xda\x3a\x7a\x80\xb6\x64\x34\xda\x3d"
        "\x3b\x17\x71\xc7\x0a\x2b\x04\x87\xd0\x48\x98\x37\x0f\x11\xbe\x6f\xd0"
        "\x67\xf5\xd6\x75\xee\x5a\x1c\xd8\xb4\x40\x34\xb6\x3c\x08\x2a\xf0\x8d"
        "\xf7\x97\x2b\x12\x18\x07\x5c\x3b\xce\x06\x49\x4b\xbe\x6c\x71\xdb\x38"
        "\x80\x36\xc2\xd8\x3e\x2c\xf1\x69\x52\xa7\x81\x41\x96\xa9\xdd\xf1\x32"
        "\xcf\xea\xc3\xfc\x8b\xab\xfa\xe0\xf2\x9b\xd0\xd6\x9b\xc3\x10\x52\x4c"
        "\xa5\x67\xe3\xd4\xf3\xb0\x8b\x17\x90\xcb\xf3\x66\x3c\x66\x0d\x04\xdd"
        "\x27\xd4\x49\x3d\xc1\x87\xf5\xbe\x68\x3f\xe2\xcc\xff\xb5\xc9\x0d\x8d"
        "\xeb\x42\x3a\x8d\x89\xc2\xe8\xf8\x9a\x2c\x4d\xf9\xdd\x95\x6d\x5b\x12"
        "\xcc\x8b\x2c\x5c\x62\x42\xc5\x3a\xda\x34\x22\xf2\x5d\xa7\x29\xac\xe0"
        "\xd7\x39\x20\xbc\x1a\xc9\x36\xfc\x9d\x72\xb6\x44\xde\x13\xcc\xa1\xf5"
        "\xc5\xfe\xf6\x80\xb8\x09\x59\x63\x77\x38\x69\x51\xc7\xe8\x02\x2f\x7c"
        "\xd7\x95\x2b\x95\x82\x90\xd9\x09\x9d\xb2\x79\x0a\x5e\x84\x11\x69\xee"
        "\xd8\xc7\xe0\x47\x45\x48\xd3\x5a\x7e\x6e\x27\x7e\xc8\x52\xb6\x88\x32"
        "\xa4\xb8\xda\x9d\xcb\x52\xe0\x2c\xd9\x3b\x10\xa5\x49\xe2\xde\x8e\xcb"
        "\x2a\x4e\xdc\x57\x41\xd0\xe5\xb5\xa7\xee\x06\x27\xbe\xbb\x4a\x55\xd0"
        "\x9c\x56\x94\xd2\x44\x7c\xcc\x8f\x59\xb3\x56\xce\x57\x8d\x5c\xc0\xb7"
        "\xa5\xff\xcf\x0a\xf2\x3e\x0c\xf1\x56\x4e\x59\x25\xb9\x34\x1b\x1d\x3b"
        "\x29\x27\xff\xa3\x0a\xfe\x24\x85\x12\x84\x55\x58\x15\x5a\xa4\x7b\xac"
        "\x39\x8f\xf2\x96\x7d\x93\xa3\xa8\x50\xf3\x38\x96\xe2\x3a\x58\x4d\xbf"
        "\x17\xd3\x83\x4e\x84\xd8\x7d\xb0\xaf\x0f\x79\xc6\x4b\xd2\x1f\xb3\xd8"
        "\xc9\xf9\x81\x57\x5c\x0c\xb4\x34\x9f\xd8\x7e\xe4\x23\x02\xef\x9d\x44"
        "\x7d\xef\x81\x93\xf4\x61\xc3\xe5\xf5\xbe\x04\xf5\x99\xdf\x71\x32\x80"
        "\x86\x28\xfd\x85\xfa\x89\xbf\xe0\x6b\xc5\x18\xfd\x97\xf8\x66\x87\x2a"
        "\xb3\x76\xde\x1e\x1e\x0e\x35\x66\x95\x2f\x2e\x44\x95\x20\xc8\x09\x3c"
        "\xfd\x0e\x4d\xc4\x1e\xcd\x55\x05\x56\xd5\x19\xbe\x8d\x9b\xf6\x3c\xc7"
        "\x8e\x4a\x16\xc4\x30\x55\x79\x2c\xca\x27\xd9\x4e\x15\xe3\xe7\x1a\x50"
        "\xbb\xb0\x9d\xb8\xd3\x82\x6c\xb4\xfd\xf8\xad\x9c\x58\x1e\x9d\x32\x0d"
        "\x8f\xe1\x82\x44\x60\xdd\xce\x4c\x2a\x65\xc2\xe3\xf2\xcf\xcb\x08\xa1"
        "\x24\x80\x01\xab\x09\x6d\x5d\xf7\xaa\xf1\x29\x7c\xd6\x8d\x32\xf6\xd6"
        "\x67\xe9\xb5\xc3\x67\x2e\x1c\x5e\x51\xed\x04\x36\x12\x7e\xc7\x05\x4e"
        "\x8a\xbd\xb9\x9a\x33\x15\x94\xf7\x01\x15\x36\x42\x28\xf8\x5a\x39\x16"
        "\xd2\x5a\x86\x4d\x19\x22\x73\x01\xde\x48\x77\xa3\x9c\xb1\x93\xea\xa8"
        "\x24\x37\x77\x54\x37\x70\x86\xcb\x72\x06\x15\x16\x18\x0f\x12\xe8\xad"
        "\x01\xea\x61\x2d\x9a\x14\xd2\x6d\xcd\xb1\x06\x0f\x52\x4d\x50\x19\xca"
        "\xf8\xaa\x41\xd7\xba\x58\xb4\x47\x9a\x34\x40\x85\x57\x7e\x35\x84\x6c"
        "\x3c\x05\x02\x5e\x88\xff\x58\x34\x15\xe3\xea\xb7\x01\x63\x43\xe6\xbe"
        "\x03\xce\x81\x51\xa0\xf5\x43\xc4\x45\x5e\x47\x4e\x4b\x19\xd8\x64\x8c"
        "\x9f\x99\xc7\x36\xd4\x28\x18\xff\xed\x1f\xae\x92\xed\x31\x0b\x84\x4b"
        "\x88\xe3\x64\xcb\x8c\x43\xd4\x7d\x4c\xdd\xa0\x51\x7e\xcd\xba\x3f\xc5"
        "\x57\xa4\xe0\x30\xc6\xa0\x50\xcb\x10\x73\x20\x60\x13\x80\xe3\xc5\x94"
        "\xe7\xf8\xef\x3f\x60\xef\xa3\xf0\xc8\x67\x27\x66\x63\xf5\x7e\xcb\x2f"
        "\x77\x19\x62\xa6\x90\x23\xbb\xb5\x93\xd7\x2c\xb6\x75\xe9\x2c\x25\xdc"
        "\xd7\xb1\x6f\x06\xf8\xb5\x4e\x6b\x6a\xc6\x06\x3a\x87\x9b\x59\x2b\xbc"
        "\xd0\x77\xda\x1c\x71\xae\xd6\xab\xe1\x98\xc0\xd5\xb0\x7e\xfc\x28\x6a"
        "\xe4\xe9\xbb\xdc\xe0\x2c\x5f\xba\x9e\xd4\x9d\x0a\x54\x0f\xad\xef\x11"
        "\x9f\x9e\xe6\x52\x36\x1e\x72\xfa\x71\x1d\x8e\xfa\xfb\x10\x75\x47\xbd"
        "\x58\xe4\x20\x30\xd2\x9c\x67\x5b\xe0\x03\x45\xb3\x19\xbd\x23\xf0\xbc"
        "\xa7\xac\x64\x23\x00\x93\x2c\xd2\x86\xf0\xb2\x65\x7c\x63\x92\xdb\x2f"
        "\xf1\xf7\x91\xf5\xc2\x5f\xee\x4f\x1e\x28\x8e\x48\xbb\xd1\xea\xb0\xb3"
        "\x00\xe2\x17\x3b\x69\xaa\xba\x1b\x4f\xac\x40\xd2\x39\xa0\x29\xa8\x5a"
        "\x9b\xde\x60\xe9\xc5\xc1\x7f\x72\x02\xe4\x89\x10\x21\xad\x12\x08\xc8"
        "\xe8\x08\x95\x5b\x12\xde\xcd\xb0\xc1\x71\xa4\xa1\x5c\x01\x32\x4d\xd9"
        "\xfc\x0f\xc0\xd7\x9f\xfa\x1a\xfc\xf8\x61\x7b\x82\x77\x73\x19\x7c\x6e"
        "\x8a\xc3\x1f\xda\xba\x29\x80\x2e\x5a\x92\x9a\x5b\xcb\x7c\x24\xa2\x01"
        "\xdf\x2b\x8c\xb5\xbf\xc4\xda\xb7\xb8\xa5\x2a\x60\x68\x51\x51\x09\xc4"
        "\x9e\xd5\x55\x8f\x40\x40\x66\x9b\x9e\x2f\x26\xf8\x29\x0d\x01\x3b\x21"
        "\xa1\x9c\x2c\x8e\x17\xde\xd3\xc7\xf3\xb7\x05\x6c\xe3\x53\x76\x12\xbf"
        "\x91\x3a\xfe\x57\x64\x95\x19\x55\x95\x05\x0a\xad\x3d\x9d\xe7\xb1\x1f"
        "\x65\x96\x91\xdb\xcb\x11\xb3\xe5\x64\xd1\x4d\x30\xed\xfe\x79\x78\x4a"
        "\xca\x05\xde\x70\x4b\x9f\xf3\x6c\xcb\xaa\xa1\xfc\xd2\xbe\xcb\xc1\x46"
        "\x38\x39\xef\x11\xb1\xf9\xea\xed\x56\x94\x11\x65\x98\x9f\x26\x72\x1f"
        "\x66\x18\xcb\xb0\x7a\x0f\x6c\x88\xdd\x58\x38\xdf\xb5\xac\xad\x2e\x1a"
        "\x40\xc4\x99\x1d\x29\x80\x38\xbf\x7a\x7d\x35\x53\x43\xfb\x40\x68\x50"
        "\x99\x75\xf8\x8f\x89\xc2\x76\x1f\xa2\x51\xf8\x53\x89\xfd\x65\x3d\xe5"
        "\x9d\x4e\xef\x3c\xe6\xec\xdb\xc5\xdc\x51\x1f\x65\xaf\xa4\x86\xad\x18"
        "\xe1\x1c\x40\x68\x28\x6d\xe0\xcc\x9a\x16\xb2\x8e\x5c\xf7\x59\xc4\xdb"
        "\x70\x2f\xd5\x61\xfe\xfc\x04\xa8\x6a\x5d\x1e\xa0\xe3\x7b\x34\x60\x22"
        "\xeb\x5c\x0a\x74\xa3\x89\x82\x6a\x61\x55\x5b\x4d\xc6\x94\x78\x86\x84"
        "\x49\x83\xdb\x32\xdd\xb5\x10\x33\x0c\x0b\x4a\x29\x84\x62\xd4\x46\x17"
        "\x83\x5d\x30\xec\x8c\xc1\x38\xd3\x7b\xab\xab\x65\x53\x76\xbe\xfb\x5d"
        "\x59\x93\x72\x26\xe9\xff\x49\x31\x35\xed\x2a\x96\x9c\x6e\x53\xaf\x91"
        "\xc3\xf7\x09\x6a\x72\x2b\xf7\x68\xfe\xe1\x16\xe5\xfc\xf8\x8e\x52\xdb"
        "\x16\xe4\x60\x72\xed\x5e\x2c\x1f\xe0\xcb\x1c\x91\x0b\x7c\x1b\xac\xe6"
        "\xe8\xaa\x3a\xec\x77\x13\x27\x06\x86\xa1\x8a\x55\x35\x20\x02\xb3\x3d"
        "\x63\x3a\x14\xde\xb8\x6b\x62\x31\xb3\x1f\x11\x13\x88\x92\xde\xaa\x3e"
        "\xea\x1a\xd1\xee\xe3\x6c\x00\x06\xec\xaf\x6f\x36\xe4\xca\xad\x0b\x21"
        "\x35\xdc\x2a\xb8\x79\xd9\x5f\x4e\xe2\xd4\x60\x41\xdd\xb2\x01\xcc\x02"
        "\x43\x22\xce\x5e\x87\xa6\xc5\xe1\x72\x09\x7e\x15\xb6\x67\x1b\xb4\x26"
        "\xd0\x8e\x30\x03\xe3\x2f\xba\xc0\x5b\xd3\xa8\xf2\x23\xba\xd5\x5d\xd9"
        "\xff\x6f\x21\xda\x27\x49\x97\x27\x9d\x9f\x7a\x63\x37\x73\xb8\x41\x1e"
        "\x11\xfc\x09\x8d\x10\x84\xf7\x30\xd5\x89\x40\x30\x04\x4e\x8a\xd9\xe4"
        "\x37\x54\x16\xb2\x86\xb7\x67\xdc\xdc\x0f\x5d\x70\xe7\x40\x17\x8a\x97"
        "\x78\xd7\x8a\xb5\x64\x44\xb0\x1d\x65\x8e\x4d\xda\x48\x8f\xe0\x57\x60"
        "\xb5\x18\x2f\x90\x64\x56\xee\xff\xce\xf3\x31\x8b\x3e\xbe\x25\x6e\x49"
        "\xe4\x38\xd2\xf9\x5b\x6c\xfd\xcb\x04\xf6\xbc\xfd\x29\x67\x79\x0f",
        4096));
    NONFAILING(memcpy(
        (void*)0x20660000,
        "\xcc\x93\x72\x74\xa5\x53\x06\x00\x31\xd6\xa7\xc1\x1e\xd9\x05\x75\x0b"
        "\x72\x0f\x6b\xdf\x48\xf6\xf2\x7f\xa4\x03\xaf\xc1\x70\x91\xa7\xcb\xdb"
        "\x16\xcc\x54\x69\x31\x2f\xef\xe2\x0b\xb8\x65\xf2\xd5\x31\xe1\x59\xd4"
        "\x68\xe1\xa3\x92\xf9\x47\x8a\x1c\xcf\x9c\xf5\x67\x0f\x69\xd8\x95\x7e"
        "\x12\x6e\xf3\x83\xab\x43\x8b\x35\x61\xac\xb7\x72\x02\x3e\xf2\xcc\x83"
        "\xe2\xcc\xcb\x82\x04\x28\xd5\xb1\x5d\xe7\x6d\xea\x7e\x50\xb6\x6f\x1d"
        "\x0a\x65\x29\x14\xee\xea\x91\xd7\x80\xf7\x3c\x23\x00\x09\xa3\x1d\x70"
        "\x7e\x45\x43\xa1\x59\xe7\x20\xd2\x18\xb1\x59\x31\x98\x5c\x78\x63\x64"
        "\x92\x53\x03\x7f\x59\xbb\x9e\x7b\x4d\xf8\xf6\x7f\x38\x83\x73\x95\xd4",
        153));
    syscall(__NR_writev, r[0], 0x20d30fb0, 5);
    break;
  }
}

void test()
{
  memset(r, -1, sizeof(r));
  execute(3);
  collide = 1;
  execute(3);
}

int main()
{
  char* cwd = get_current_dir_name();
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      install_segv_handler();
      for (;;) {
        if (chdir(cwd))
          fail("failed to chdir");
        use_temporary_dir();
        int pid = do_sandbox_namespace(procid, true);
        int status = 0;
        while (waitpid(pid, &status, __WALL) != pid) {
        }
      }
    }
  }
  sleep(1000000);
  return 0;
}