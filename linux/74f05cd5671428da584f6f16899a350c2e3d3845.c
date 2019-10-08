// KASAN: stack-out-of-bounds Read in strlcpy
// https://syzkaller.appspot.com/bug?id=74f05cd5671428da584f6f16899a350c2e3d3845
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_setsockopt
#define __NR_setsockopt 366
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 2, 1, 0);
  memcpy(
      (void*)0x204c3000,
      "\x10\x1c\x07\x57\x5c\x02\xfd\xdc\x54\x2a\x41\x7e\x95\xf2\x7e\x33\x61\xbd"
      "\xf6\x8b\x36\xa0\xdd\xda\x7f\x61\x0e\x92\x44\xa3\x03\xb5\x31\x75\x3b\xf1"
      "\xb6\x64\xf9\x95\xc3\x66\x4e\x5d\x36\x11\xbc\x34\xcb\x24\x49\x24\xe2\x73"
      "\x7c\xc8\xbb\xa7\x38\xb5\xb6\x8e\x2a\xd3\x26\x34\xdd\x87\xe8\xc2\x35\xf3"
      "\x0f\xde\xad\x4e\x21\x1b\x30\x23\x8e\x4e\x91\x63\x51\x40\xda\xcc\xf1\x5f"
      "\x54\x82\x0c\xfd\x92\x27\x76\x1a\x9e\x3d\x73\xf0\x62\x25\x73\x89\xc8\xe0"
      "\xc6\xfe\xb4\x1b\x48\x42\x8c\x5e\x28\x42\x78\x66\xa8\x8e\x49\xf6\x8d\xa6"
      "\xbf\x50\x99\x85\xff\xd9\xad\x1c\x95\x8b\x75\x58\xdc\x76\x4f\xf5\x65\x05"
      "\xf2\x49\x94\xdd\x43\x54\x48\x02\xcb\x71\x09\x3f\xad\x8c\xdd\x34\xb1\xee"
      "\x22\x37\x18\x66\x7d\x85\x1b\x17\x4f\x6e\xc8\xac\x06\x4a\x8c\x9e\x66\xb6"
      "\xef\x26\x33\xb3\x4f\x3e\x36\xea\x9c\xe1\x54\xcd\x93\xa6\x6c\x6b\xd7\xf2"
      "\x8c\x70\xf6\x18\x7b\x31\x65\xc7\xd4\xf8\xe6\xb6\x1f\x48\x57\x19\x8a\xb6"
      "\x04\xdf\x5c\x2e\x0e\xc1\xba\x27\xaa\xf5\xa4\x82\xad\xbf\xf9\xe4\x8f\x6a"
      "\xa9\x72\x79\xef\x2f\x18\x63\xba\x83\x10\xa3\xcc\x41\xe7\xa9\x0b\x80\x3c"
      "\x46\x9a\xce\x5d\x80\x37\x52\x52\x4a\xf1\x2f\xa3\xdc\x76\x20\xf5\x9f\x34"
      "\xcf\x5a\xa1\xf0\x81\x85\x6c\x58\xf2\x19\x86\x6a\x82\xad\x3f\x09\xd5\x67"
      "\xcc\x81\xd1\xa2\xca\xeb\xba\xc9\x7b\x3c\x4b\x87\xd6\x68\x5e\xfd\x70\xe1"
      "\x29\x70\xc4\xac\x08\xb4\x95\x8c\xaf\x30\x76\xca\x7c\x55\x6c\x7d\xd6\xed"
      "\x81\xce\x2d\x76\x1c\xb0\x19\xc9\x1c\x50\x5a\x8c\xd0\x9a\xc0\x7a\x2f\xaa"
      "\x86\x8e\x4f\xd1\xe1\x0d\x76\x51\x0c\x26\xdb\x26\xa9\xbc\xb1\x4b\x40\xd8"
      "\x33\x11\x9c\xe7\x35\xb7\xc4\x16\x8b\x6a\xb3\xc3\xd7\x62\x4f\x16\x9c\xff"
      "\x72\x56\x24\x03\x24\x67\xb8\xfa\xaa\x4a\x2b\x70\x0e\xca\x72\xbf\xc8\x33"
      "\x1f\x67\xcf\x4e\x26\x01\xd4\x6b\x2f\xa7\x56\xc9\x5e\x5a\xf8\x34\xb2\x7d"
      "\x44\x51\xa4\xe2\x2f\x28\x6b\x06\xca\x32\x0d\xb3\x7c\x25\x94\xa7\x6d\x2e"
      "\xa1\xde\x14\x40\x99\xe8\x14\x66\xad\x12\xc9\x80\x7f\x2b\xee\x37\x36\xd6"
      "\xd1\x0f\x5d\x85\x45\xa1\x7c\xe2\xac\x9c\x2f\x5b\x2e\x92\xb2\xf0\x0b\x92"
      "\xc4\x50\x93\x7d\x55\xe4\x1c\x13\xcd\x2e\x8f\xe0\xdb\xa2\xfb\x00\x24\xaf"
      "\x77\x77\x68\x35\x19\x02\xfe\x47\xd4\x29\xfd\xc6\x38\xe0\x77\x74\xf9\xcd"
      "\x2b\x8d\x8c\x75\x85\x63\x5b\xb4\x02\x67\x7a\x76\x9a\xf2\xda\x09\x05\xf2"
      "\x86\x82\xe3\x5c\xd4\x40\x5f\xa3\x22\xf9\xef\x17\x3a\xd6\x60\x3c\x96\xff"
      "\x2a\x01\x34\xd9\x5e\xde\x12\x5d\x1d\x53\x6f\x37\x57\x15\xcd\x06\x2e\x5c"
      "\x24\xb3\x18\xfc\xc9\xc5\xae\x49\x83\xb9\x40\x59\x69\x39\xca\x28\x3d\x68"
      "\xf9\x60\x89\xe7\xff\x80\x4e\x8f\xf7\x78\xc0\xfc\xac\x1c\x46\x66\xca\xf0"
      "\x7b\xb1\xaa\xf1\xf3\xfe\xbe\x4b\xf2\xcd\x38\x74\x25\x6b\xa9\xd8\xf1\x1d"
      "\x7d\x2c\x87\xda\xb9\x90\x4f\x76\xff\xf4\x84\x6e\xe3\xd1\xb1\xdf\x5d\x8f"
      "\x26\xc0\x3a\x41\xa4\xe1\x8a\x39\x6d\xf5\x8c\xc6\x5c\x6a\x28\xd0\xf7\x12"
      "\x7d\xea\x5d\xa9\x68\xe1\x37\xf0\x8c\x02\x1f\x8d\xbd\xa7\x38\xc9\x71\xab"
      "\x74\x7f\xc1\x4a\x3b\xf6\x83\x04\x88\xbc\x2e\x64\x9a\x65\xa8\x0b\xa9\x19"
      "\x50\xe2\xa0\xd2\xa0\x36\x46\x81\x69\x46\xd3\xbc\x92\x62\x35\x88\x22\x27"
      "\xcf\xe4\x49\x3f\x1b\x88\x31\xfb\x0d\x88\xef\x76\xf2\x90\x7f\x34\x47\x36"
      "\x79\xc3\x34\xfa\x51\xd5\xeb\xee\x30\xc5\xd8\x3a\x43\xee\xac\xc7\x18\xb1"
      "\xc9\xb9\xae\x5a\xdb\x08\xd5\x31\x5a\x6c\x08\x33\x33\x82\x29\x6f\x7f\x3f"
      "\x54\x6d\xb1\x42\xe8\x72\x74\xc6\x8e\xdb\x7c\x86\x18\x1f\xfe\x51\x46\xf4"
      "\xfa\x14\xe4\x7b\xfc\x54\x21\x8c\xae\xe8\x53\x68\x1d\x6a\x9c\xce\xa6\x2a"
      "\x48\x3c\xdd\x23\x63\x37\x29\x68\xe6\xcb\x0f\xf9\xeb\x7c\x88\x30\xee\xdc"
      "\x6d\xfb\xc7\xff\xf9\x1a\x90\x7d\x26\x69\x77\xa7\x0a\x73\x17\xb4\x43\xdb"
      "\xc8\xe8\x83\xce\xa9\xbc\x7e\x76\x84\xa8\x2d\x13\x23\xe9\xcc\xe3\x29\xfa"
      "\x40\xa1\x22\x7d\x70\x2c\xb3\x41\x67\xf8\x9b\x77\xb5\x07\x96\x7a\x7a\xc7"
      "\x9d\x1d\x42\xc6\xa3\xd9\x94\xcf\x75\xdf\x40\x91\x02\x08\xce\xc7\x5b\x2a"
      "\x5e\x5e\x75\x70\x46\x0d\x45\x3b\x3e\x8f\x80\xd5\x5c\x50\xf0\xb5\x73\x7b"
      "\xca\x18\x58\xa1\xfc\x85\xc8\x7a\x49\xe9\x36\xdd\xd7\xb5\x37\x20\x17\x30"
      "\x85\x8d\x9f\x2c\x1b\xd3\x73\xe5\x36\xa4\x6d\x63\xf0\xa0\x5b\xeb\x1e\x54"
      "\x7a\x8a\x75\x91\xd0\x1c\xd7\x0c\x90\x44\x7f\xb7\x67\xb5\xf3\xea\x37\x89"
      "\x18\x1f\x6b\x5d\x08\x7c\xf9\x49\xf3\xf8\xcc\xaa\x1f\x06\xce\x2f\x49\x6a"
      "\x40\xe6\x3e\x5a\x4e\xfc\x03\xf3\xea\x80\x12\xa9\x87\x61\xcc\x16\x93\xa4"
      "\x9c\x00\x31\x24\x0d\x5f\x6a\x5f\x9f\x5e\xad\xf9\xf9\xf8\xa1\x13\x3d\xd3"
      "\xc9\xec\x1d\xf8\x60\x05\x23\x39\x63\xce\x66\x0d\xe9\x4d\x85\xd8\x64\x59"
      "\x98\xeb\x45\x1f\xa0\xbe\xf5\x64\x97\x09\x6e\xfc\x0a\x1d\x64\x2c\x60\x97"
      "\x39\x5b\x5a\xa7\x26\xae\xfb\x7b\x80\x10\xc3\xe4\xbe\x24\x78\x7b\xef\xe2"
      "\x70\x22\x0d\xf5\xd8\x0c\x7f\x91\xca\xdc\x58\xf1\x5e\xd3\x49\x7f\xf9\xdd"
      "\x14\x0b\xef\x3a\x29\x57\xdf\xd4\xbd\x8b\x7e\x4b\x18\xee\x1e\x73\xa8\x30"
      "\xb7\x42\x23\x9d\x12\x80\x95\xe2\x9c\xf6\xeb\x7d\x75\x59\xa0\xaf\x5f\x52"
      "\x47\x84\xae\x42\x08\x9d\xf4\x0b\x1f\x8e\x4a\x6d\xeb\x22\xf9\x98\x63\x6b"
      "\xbb\xd9\x6a\xbf\x16\x31\xdd\xdd\xaf\xf5\x0b\x0c\xdf\x6b\x92\x36\xe1\x2c"
      "\xa5\x6e\x88\x7b\xb1\xe4\xa7\x9e\x23\x12\x42\x6c\xeb\xa4\x89\x74\x4c\xb9"
      "\x7a\x22\xc4\x4e\xd1\x99\xfb\x66\xbe\x93\xa2\x86\x51\x30\x3d\x60\x9c\x37"
      "\xa0\x66\xc0\x7d\x06\x60\x29\xc7\xf0\xba\x4b\x01\x94\xe9\x14\x9a\x92\x4f"
      "\xea\xdd\xa0\x6d\x58\xae\x05\xf6\x88\x80\xe4\x82\x61\xfc\xe4\xee\x90\xac"
      "\x7a\x86\x00\x18\x14\xef\xcb\xec\x4e\xab\x28\x50\x87\xf3\x6a\x59\x48\x72"
      "\x10\xd4\xa3\xae\xdc\xca\x20\xaa\x66\x2b\xc3\xe1\x93\x8f\x94\xe4\x0a\x35"
      "\xf7\xcb\x94\x28\x02\x50\xb1\xcb\x07\xa2\x29\x11\x0a\x21\x78\xba\x42\xdd"
      "\x09\xc0\xd7\x51\xa9\x4c\x5d\xfb\x6b\xcc\x45\x0c\xe8\xb6\xef\x8a\x20\x25"
      "\x0e\xaa\xdb\x02\x32\x19\xdb\x32\x21\x2b\xed\xba\x30\x3d\xe0\x53\x5d\x80"
      "\xb4\x91\x93\x47\xe3\x4a\x1d\x18\x3e\x7a\xb0\xc8\x47\x3c\xa7\xf2\x1f\xc9"
      "\x51\xdb\xd1\xa0\xe7\xe5\x2f\x1b\xfb\x2b\x59\xe3\x8d\x21\xaa\x0d\x2d\x25"
      "\x87\xcf\x0f\x6f\x77\xf8\x6e\x5d\xd9\x09\x44\x3c\xca\xd3\xb8\xca\x31\x07"
      "\xe5\x1d\xf7\x46\x1d\x9f\x23\x2a\xa0\x93\x33\xe3\xea\xe5\x91\x59\xd4\x9c"
      "\x04\x81\xde\xf5\x6c\x02\xab\x26\x08\x91\xd9\xc5\xb9\x93\xa9\xc8\x45\x5c"
      "\xd0\x8d\xd3\xa1\xa5\x79\xb9\x02\xad\x66\x0d\x9e\xd1\x78\x07\xa7\x6a\x8c"
      "\x51\x71\xaa\xcc\xd7\xff\x23\xbc\x58\xbf\x52\x66\xee\x0b\x55\x6f\x73\x32"
      "\x30\xcb\x30\xff\xca\xf2\x61\xf5\x82\xc7\x49\xe8\x8d\xe3\x27\x74\xad\xae"
      "\x8d\x50\x78\x07\x5e\x5c\xf4\xbb\x67\xc0\x42\x6a\x81\xe8\x1e\xd6\xaf\xb8"
      "\x74\x27\x1e\x60\xbc\xe0\xfe\x5a\xb6\xd3\xbd\xe4\xaa\x42\xe0\xd6\x6c\x2a"
      "\xfc\xd5\xe7\xb9\x16\x81\xee\x70\x3f\xbb\xa2\xc2\x27\xf3\x69\xe0\x95\x66"
      "\xdf\x10\xed\x5a\xbe\xda\x8b\xb4\x7f\x58\xca\x7e\x5c\x49\x6f\x33\xbc\xb0"
      "\xb8\x7e\x1a\x2e\x70\x01\x8c\xf2\xf1\x0d\xb6\x25\xf7\xa4\x9b\x7f\xb3\x49"
      "\xd7\x19\xe9\x99\x04\x56\x09\x10\x01\x50\xe5\xa3\x56\x5d\x2c\x53\x13\xf8"
      "\x07\x19\x6e\x01\xa2\xa8\xac\x51\x20\xaa\x62\x6a\x96\x2e\xa1\x6f\x97\x2f"
      "\x50\x60\x73\x9e\x12\xa5\xf1\x6b\xac\xea\x2f\xd2\x1f\x4c\xc8\x79\x60\xb5"
      "\x52\x89\x29\xf1\x3a\x7e\x9a\xe0\xde\xd1\x2e\x39\xfe\x22\xcf\xaa\x1a\x0b"
      "\x13\x51\x5e\xeb\x9e\x75\x7c\x61\x45\xf3\x35\x3e\x62\xdd\x27\x10\xf2\x59"
      "\x89\x26\x6c\x7d\x26\x65\x42\x28\xbd\x88\x57\x01\x2a\x6d\x3e\x45\x8c\xb8"
      "\x07\xcf\x18\x85\x92\xa1\xda\x2b\x27\x23\xa1\xf7\x74\x49\xe7\xc6\x88\x81"
      "\xd5\xc4\xee\x9b\x69\x2c\x93\x04\x4d\x02\x81\xa7\x43\x22\xa9\xc1\xb3\x31"
      "\xfe\x4a\x61\x0e\xfc\x28\x17\xf0\xee\x42\x2f\x01\xef\x09\xa8\x12\xa8\x67"
      "\xf0\x08\x14\xcd\xc1\x1b\x7d\x40\x4c\x01\xa4\xe1\x19\x90\x6c\x3c\x26\x30"
      "\x11\xa1\xb9\xad\xdf\xc3\x74\x49\x7e\xee\x54\xcb\x2b\xfe\x44\xec\xa7\x35"
      "\x08\x8d\x95\x24\x1c\x2a\x6c\x9c\x4c\x91\x91\x98\x00\x63\x52\x33\xbd\x92"
      "\xbb\x14\xc8\x1f\xe8\x2a\x25\x01\x25\x77\xe4\x8c\x75\x1c\xe4\xec\x41\x3a"
      "\x79\x90\xcd\x9d\xa4\x8f\xe0\x0f\x0b\xc9\x15\x1c\x8f\x6b\xe4\x1d\xc6\x75"
      "\x93\xe5\x78\xa9\x39\x66\x2f\x05\xeb\xa1\x8d\x84\x5c\xa0\x03\x97\x11\xf4"
      "\x2e\xb8\x3f\x60\x99\xcd\x1b\x66\x52\xee\xfe\x91\xfc\x60\x29\x57\x76\x40"
      "\x42\xdf\xe5\x55\xc5\x97\x52\x2a\x9b\x16\xaa\x68\x1c\x72\x3f\x39\xdc\x7d"
      "\x39\xb8\xd9\xb7\x8c\x37\x80\xa5\x35\x85\x50\xb6\x1f\x27\x9e\xb3\x75\xcb"
      "\xca\x11\x80\x5a\x2c\x53\x02\x44\xa0\x12\x9b\x20\xae\x37\xfa\xd5\x74\x78"
      "\xf6\x77\x06\x7e\x1d\xa3\xf7\x86\x2c\x6c\xec\x65\x18\x43\xe9\x3d\xe0\x59"
      "\x29\xce\x8c\xaa\x52\x5a\x37\x57\xa3\x23\x7b\x1b\x08\x0b\x93\xbe\xb4\x22"
      "\x8f\xaa\xd0\x46\x4a\xc8\x4d\xda\xde\x81\x1d\x46\xed\xf3\xd3\x45\xea\xcf"
      "\xac\xb8\x9d\xd1\xef\x47\x7e\xf7\x4d\x3d\x11\x7a\x28\x09\x76\x53\x92\x21"
      "\xb9\x22\x8e\xd8\x54\xa6\x4b\xc7\xfc\xd0\xbb\x1f\x4a\x14\x1f\xe7\x50\xc6"
      "\xb1\xcc\x92\x90\x9b\x48\xb0\xab\x01\x0f\x9a\x20\x9e\xa9\x63\xf7\x85\xd9"
      "\x3e\xd1\x57\x19\x32\x93\x72\x98\x56\x1e\x11\xc3\x4a\x25\x59\xff\x9f\xea"
      "\x66\x61\x38\xb5\xf3\xa3\xc2\xee\xa2\x73\x43\xb7\x1e\xe2\x09\xab\x2f\x2a"
      "\x33\xe0\x4e\x14\xd3\x1b\x86\xd6\x35\x7a\x70\x98\xaa\x15\x57\xa5\x1c\xaf"
      "\xdf\xa3\x30\xfc\x58\x16\x86\x31\x73\x08\x83\xb8\x78\xa2\x6c\x07\x72\x42"
      "\x79\xbb\x31\xfb\xa5\xba\x13\xbb\x4e\x4e\xdc\x9a\x59\x02\xab\x7a\x00\xbf"
      "\x78\x10\x4e\xe4\x8c\xfc\x44\x44\x0f\xfb\x3d\xbf\x19\xfb\x67\xe4\x19\x40"
      "\xd8\x64\xb1\x82\x9a\x87\x17\xed\x8b\xbb\x0a\x30\x7d\x8a\x79\x09\xcd\xfd"
      "\x14\x68\x32\xb8\x11\xa4\x35\xe8\x80\xef\xd9\xf1\x35\x08\x35\x3a\x55\x0e"
      "\xb5\x02\xbe\xbb\x1f\x36\x7f\x24\xf8\x9e\xc4\x54\x5b\x73\x3a\x97\xd1\xff"
      "\x2c\x7e\x88\xf8\xc7\xec\x22\x74\x01\x24\xd9\x40\x65\xf3\x7f\x87\x44\x8c"
      "\xe5\x8c\xab\x49\x78\xee\x15\xe8\x88\xd8\xc6\x3b\x52\xcc\x7c\x7b\xaa\xa5"
      "\x63\xa5\x8d\x3c\x94\x00\x18\x69\x15\x16\x53\x3e\x37\x70\xd0\x17\xe4\x0d"
      "\xc4\x84\xaf\x8a\x89\xcc\x0a\xee\x69\x4e\xa2\x67\x49\x24\xcb\x86\xf4\x14"
      "\xc8\x0f\x82\xfe\x15\xc3\x16\xb6\x83\x72\xd7\xee\x29\x44\x46\xb6\x92\x45"
      "\x4d\x92\xef\x6f\x54\xe9\x92\xc6\xb6\xef\xf1\x8e\xc7\xa2\xf0\xb6\x2e\xf5"
      "\x99\x5f\x04\x65\xf5\xf9\x11\xa7\xdc\x5a\x5f\xdf\xa0\xe6\x88\x39\xc1\xc2"
      "\x2b\x32\xee\xb3\x63\xba\x5c\x37\xc6\x6c\xaf\x56\xa4\x41\x02\x7f\xf1\x06"
      "\xd4\x47\x1b\x21\x22\x7d\x15\xc3\x25\x2f\xe9\xdd\xe1\x60\xa1\xbf\xe3\x19"
      "\xc1\x99\xf0\x2b\xa4\xd4\xde\x72\xa6\xf2\x6d\x99\x39\x95\x5a\x31\x4f\x4c"
      "\xca\xe7\x8b\xf7\xa9\x85\xc7\xd1\xb8\x58\xa8\xf6\x84\xe4\x28\xee\x26\xc8"
      "\xa4\x6d\x5b\xc5\xa2\xe0\x7c\x8c\x60\x96\x78\xcd\x8e\xf9\xd3\xe2\x1a\x4f"
      "\x0e\x3f\x41\xfd\xdc\x95\x0f\x10\xa4\xfc\xd9\xcd\xf6\x84\xc0\x4a\x27\x1d"
      "\x99\xae\x0d\x1f\x32\x47\xd5\xa0\x0f\x19\x05\x47\x65\x6c\xd7\xad\xb2\x6f"
      "\xa1\x84\x13\xa0\xa3\x4c\x40\x7b\xd7\x83\xbc\x0a\x92\xf2\x0c\x9b\xd3\xd4"
      "\x65\xec\x65\x7e\x6c\x87\x89\xc4\x4d\x4c\xcb\x78\x64\x1c\x1f\x4a\x49\x45"
      "\x1e\x97\xb6\x81\xa3\x56\xb2\x95\x61\x55\x22\x65\x56\x00\xf3\x2c\x83\x08"
      "\xb3\x2a\x4b\x98\x2b\xdd\xf2\xb4\x27\xb6\x1d\x8b\xe1\x1d\x50\xb7\xca\xfd"
      "\x6b\xe6\x51\x64\x97\x4b\xab\x37\x04\x04\xc4\xde\xc7\x99\xdf\x4f\xa3\xe9"
      "\x2a\xec\x9b\xf5\x7b\x6e\x64\x2e\x07\xff\xf4\x63\xcc\x00\xbd\xb5\xef\x98"
      "\x11\xc9\x9d\xf0\xb6\x6b\x8b\x5c\xf4\x25\x84\x6a\x11\x81\x38\x98\xe8\x3b"
      "\x7f\x7b\xcf\x77\xe1\x22\xab\xe3\xec\xe4\xec\xc7\x29\x0b\x97\x3d\xb8\x78"
      "\x80\x3f\xba\x78\x70\xff\x2e\x8c\xc3\xd3\x8a\x58\x8d\x68\x78\x33\x1a\x56"
      "\x26\x3c\xcd\xd0\x18\xd6\x41\x6c\x92\x34\xab\x75\xf6\x9b\xa9\x73\xf1\x3c"
      "\x07\x94\xc5\xc3\xa8\x8a\x4c\x43\x7e\x41\x6a\x2c\xcc\xab\x6b\x90\xaf\x8d"
      "\xc4\xc2\xe5\x21\x06\x9d\xbb\xf4\xab\x20\xd0\x58\x42\x0f\x57\xbc\x99\x1c"
      "\x92\xe9\xb6\x47\xe0\x9f\x7c\xc1\xc3\x86\x37\x90\xc1\xab\xad\xdc\x0d\xa7"
      "\xfe\xa1\x26\x44\xb9\xb2\x96\x86\xab\x66\xb5\xc0\xac\xb9\x70\x26\x80\x19"
      "\x61\xad\x4e\x0c\xb6\x91\xd8\xaa\xea\x42\xa8\x82\x5b\x7e\x03\x96\x68\x7c"
      "\x68\xc5\x2a\xef\xd3\xef\x3b\xfc\x4e\xec\xf6\xd8\x42\x35\x1a\xe1\xd7\x98"
      "\x98\x17\x18\x75\xe8\xa2\x3e\x86\x82\xf0\xa8\x67\x8c\xa3\xdc\x16\x9f\xba"
      "\xd2\x61\x32\xaa\x52\xc1\xb1\x21\xe0\x2b\x9e\x2e\x52\x37\xcd\x89\xb8\x04"
      "\xd1\xb8\xf1\x34\x19\x24\x8a\x61\xe6\x28\xef\x7c\xc5\xd2\x8d\xa0\xd9\x6f"
      "\xee\x4a\x21\x63\x84\x16\xb9\xf3\xbe\xe5\xcf\xd5\x04\xf0\x5c\x4f\x91\xa6"
      "\x3c\xc5\x5d\x2c\x10\x74\x36\xe1\x62\xe4\xb5\x4c\x8b\xda\xf0\x77\x4a\x35"
      "\xa4\x05\x81\x9a\xa2\x64\x2b\x08\x37\x07\xd8\x9e\xcb\xe8\x10\x9e\xbd\xcd"
      "\x98\x01\xa5\x95\x92\x22\xb6\x18\x93\x7c\xfb\xd0\xf1\xb0\x4c\xd9\x96\x20"
      "\x09\xc2\x66\xea\x8e\x5c\xe0\xb9\x24\xf9\x13\xb8\xe8\x53\x68\x10\x67\x8e"
      "\xf6\x30\x0d\xa9\xaf\xa4\xbe\x42\x1b\x87\x23\x24\x7a\xf8\x08\xe5\x0a\xfe"
      "\x5c\x83\x02\xbd\x83\xb6\x51\x70\x2c\x3c\xf9\xac\x36\x03\x8c\xd1\x11\x61"
      "\x8a\xc0\x34\xa1\x1c\x44\x00\xa6\x83\x43\x2d\x6c\xe3\xfb\x8a\x30\xc3\xbb"
      "\x76\xf6\x53\x58\x55\x65\x9e\x85\xe0\x7e\x41\xb4\x8d\x0b\xd8\xdc\xcc\xd0"
      "\x5a\x23\x6b\x23\xd0\x0f\x22\x9b\x13\x37\x46\x8e\x80\x77\x5a\x9c\xe4\x15"
      "\xfb\x5e\x8b\x07\xa0\xbc\xa6\xf0\x26\x33\xdb\x39\x8e\x73\x6a\xe6\x76\xac"
      "\x1c\x2f\x4d\x21\x1d\xd3\xe6\x57\xd4\x63\xeb\x74\xb4\x9b\x45\x9e\xfa\xbd"
      "\xc1\x54\x25\xf8\xae\x90\x42\x77\x9b\x12\x69\x9d\x40\xb7\xa6\xcd\xa1\xb9"
      "\xab\xae\xca\x76\x5d\x0c\x77\x48\x45\x72\x6c\x08\x48\x37\xdc\x5f\x56\xff"
      "\xd0\x8d\xa5\xe1\xd1\xf6\x7b\x7f\x91\x61\x0f\x98\x97\x90\x01\xd9\x72\x30"
      "\xec\x1a\xeb\xc5\xcd\xde\x6d\x16\x08\xc5\x8e\x92\xb9\x34\xc2\x69\x6f\xc9"
      "\xd7\x1b\x6a\x89\x11\x29\xa4\x75\x91\x55\x2c\xac\x2a\xd4\xe3\x42\x5c\x93"
      "\xce\xfc\xcb\x05\x5a\xa3\xff\xf9\x64\x1f\x35\xba\x0f\x8b\xc9\x17\x81\x3c"
      "\x7c\xb0\x81\xc0\xb8\xc2\x20\x73\x40\x06\xd0\x5e\x79\x66\xb7\x43\x15\xe3"
      "\x25\x41\x58\x0d\xc7\x2b\xc8\x55\xf5\xbf\x09\xc7\xe3\x3d\xa4\x83\x49\xe4"
      "\x7f\xcc\xbc\xfd\x30\x01\x0d\x7b\xa5\x7c\xf7\x0b\x20\x35\xee\x18\xe9\xbe"
      "\x50\x95\x5a\x51\xa9\x87\x28\x95\x38\x8a\x08\x15\x77\x79\x28\x05\x7a\x6d"
      "\xf4\xbd\x44\x76\xa5\x21\x69\x46\x50\x97\xe3\x3b\xbb\xae\xac\x9d\x73\xfb"
      "\x30\xf1\x47\x3a\xca\xde\xb1\xe3\xce\x17\xb9\xfa\xd4\x67\xf1\x5a\xdb\xf0"
      "\xb3\x0d\x3d\xec\x58\x17\x99\xc6\xaa\xcc\x0f\x04\x74\x4d\x23\x45\x3a\x03"
      "\x26\x31\x58\x79\xa0\xf5\x34\x6d\x67\x11\x81\xd5\xc0\xa0\xfb\x73\x94\x70"
      "\x83\x32\xa1\xc8\x4f\x1d\x79\x60\xe2\x3e\x13\x6e\x7f\x7d\x5e\x6b\xe3\x26"
      "\x3d\x77\x7e\x84\xda\x5c\x89\x70\x3c\xf0\x53\xa9\xba\x67\x3e\x66\xfc\x60"
      "\x7f\x38\x40\x57\x15\x80\x37\x09\xc5\xe8\x6e\xc6\x8b\xcd\xb6\x96\x44\x93"
      "\xf0\x2f\xa9\xba\xcf\x4e\x63\x2d\x75\x7e\x15\xfa\xde\x77\x5e\xc2\x80\xd4"
      "\xc9\xae\x2d\x3a\x06\xe2\x9d\x10\x14\x88\xf8\xdf\x5e\x26\xaa\xd2\xa9\xaa"
      "\x0c\xad\x4f\x59\x0c\x13\xf0\x9d\xcc\x81\x8b\x30\x10\xfe\x0d\x5c\x7e\x35"
      "\x03\x0d\xb5\xae\x27\x69\x34\xac\xda\xcc\x82\x1d\x24\x06\x72\x01\xbf\xe3"
      "\x3b\x0b\x9d\x62\xd9\x70\x3c\x7d\x94\xad\x13\x1a\x76\xbe\xa8\x9b\xca\x3b"
      "\x59\xce\xd7\xe4\x26\x27\xe3\xcb\x1b\x25\xa4\x87\x66\x45\x8d\x40\x6a\xb4"
      "\xf8\x7f\xc5\xe2\xb1\x71\x49\xbc\xdc\xfe\xf3\xf0\xb2\x9c\x10\xa1\xcf\x90"
      "\x7d\x15\xc7\x87\x9c\x33\xf0\x74\x6f\x39\x34\xf5\x57\xa1\x16\x1d\x0e\xba"
      "\x28\x31\x86\x87\x39\x3f\x47\x96\xd1\xb5\x1f\xe6\xb8\xed\xfe\x22\x99\x3a"
      "\xfa\x54\xb4\x54\xeb\x5a\xf2\xb2\x31\x1a\xf3\x36\xa6\xaf\x2b\xe1\x11\xa0"
      "\xff\x9a\x32\xcf\x95\x61\x11\xd3\x0f\xa1\xac\xb4\xca\x6c\x88\x77\x0b\xa7"
      "\xfb\x8b\x73\x53\x6d\x52\x26\xd6\xf8\xc0\x3e\x79\x73\x0b\x96\xf5\x41\x18"
      "\xb8\x95\x16\x07\x75\xcf\x34\x33\x19\x04\x91\xa5\xfa\x8a\x64\xd3\xd6\xb3"
      "\xd1\x82\xab\x1e\xf5\xbf\x39\x67\x68\x61\x4c\x08\xc1\xae\xcc\x25\x8a\x0f"
      "\x75\x1e\xfb\x92\x70\x86\xe6\x95\xf7\x91\xa5\xaa\xfd\x69\x25\x16\xd5\x57"
      "\x52\xeb\x84\x17\x62\x85\xf4\x5e\xc1\xce\x14\xec\x2b\x32\x29\x2c\x99\x7c"
      "\x95\x66\x2c\x9d\x11\x18\xaf\xbb\x7d\xe3\xaf\x0e\x46\xb9\x34\x83\x1f\x58"
      "\x62\xeb\x21\xa8\xbf\xb8\xf7\x4c\xcb\x97\x6b\x6e\x3a\x37\xdf\xdb\x74\x6a"
      "\x68\xf6\x7d\x49\x7f\x21\xb7\x6f\xa1\x10\x0c\x31\xcb\x52\xab\xca\xe7\x44"
      "\xf7\x3b\x62\xcd\x6d\xac\xfb\x71\x01\x3e\x56\x07\xdd\x00\x34\x61\x83\x76"
      "\x7a\x38\x52\x62\x94\x65\x76\xe0\x5b\xa5\x35\x8b\xee\x7c\xa2\xd0\xcf\x9f"
      "\x6a\x0e\x2a\x89\xd5\xf1\x8e\xdb\x46\x2c\xfe\x38\x91\xc2\xec\x7d\xa1\x6b"
      "\x44\xce\x89\xe2\x34\xfc\x41\xac\xbe\xb9\x8b\xf0\x75\xb0\xa6\xd8\x61\xce"
      "\xdf\x2e\x2e\x69\x37\x56\xe5\xb3\xa6\xfe\xda\xe7\x98\x45\x27\x10\xfb\xc6"
      "\x4d\x62\x8f\xf0\x14\xbd\x1c\xce\x98\x96\x35\x7a\xfe\x72\x02\x62\xad\x5b"
      "\xe3\x1e\x1c\xa4\x16\x2a\xda\xb5\x96\xff\xeb\xf9\x5f\xfc\x4a\x25\xbd\x31"
      "\xfe\x33\x3c\x33\x15\x7c\x68\xe5\xcb\x4e\xdc\x73\x2e\x33\x36\xb8\x82\x3b"
      "\x16\xab\x5e\x08\x2d\xfa\x43\x31\x7d\xe1\xd0\x8d\xff\xc2\x96\x42\xc4\x2f"
      "\xb1\x88\x97\xa0\x9a\x65\x19\xca\x26\x2a\xc2\x43\x51\x00\x16\xfe\x9a\x24"
      "\x3d\x24\xd5\x1f\xeb\x0b\x97\xe4\xd3\x09\xda\x32\x95\xcc\xdb\x55\xe2\xc5"
      "\x0b\x09\xc4\xba\xfd\x05\xec\x23\x25\xf0\xd1\x56\x3e\x98\x1e\xcd\xfa\x79"
      "\x4a\x13\xdc\x78\xa5\x3f\x0b\x91\x96\x0d\x26\x61\x09\xec\x85\xc4\x3e\x0e"
      "\xe0\xa8\xc9\x9e\xdc\x75\x51\x7a\x20\x35\xb2\xc7\xf7\xeb\xa6\x7a\x8e\xf4"
      "\x5b\xac\x73\x2e\xde\xec\x0f\x0a\xee\x6d\x06\xe6\xa4\xfc\x60\x75\x0d\x7b"
      "\x18\x4b\x71\x6d\xba\x78\xc7\x4b\x24\x59\x33\x81\x13\x1f\xb5\x0b\x3a\x3a"
      "\x44\x21\x86\x8e\xc1\x4a\x42\xf2\x8c\x20\xb2\x04\x50\x42\x38\x3e\xd6\xae"
      "\xe4\xc0\x3c\x37\x8f\xb5\xba\xe5\xf0\x9b\x37\xc4\x62\x02\x46\x47\x70\x14"
      "\xe6\xd6\x18\x1f\x9b\x0c\xff\x59\x03\xb4\xad\x7e\x9c\xc7\xfb\x13\x13\x7f"
      "\x49\x9e\x4c\x74\xdc\x42\x83\xad\x15\x8e\x4e\x70\x3a\x34\xb5\x70\x29\x4e"
      "\x37\xde\x92\x69\x87\x34\x77\x7a\x03\xaa\x0f\x02\x4b\x2d\xfa\xc4\x54\xb3"
      "\xd4\xf7\x18\xc0\x1d\x10\x94\xc8\xc6\xeb\x56\x65\x90\x69\x0e\x26\x84\xe7"
      "\x58\xae\xa5\x6d\x6d\x71\x29\x6d\x1e\x7f\x02\xd9\x09\x05\xa4\xb4\xba\x70"
      "\xf7\xc9\xd2\xe8\x60\x7e\xee\xe5\xe8\x42\x54\x17\x1b\xca\xe1\xb4\x17\x82"
      "\x9d\xb6\xe1\xe3\xa2\x8a\xd6\x40\x14\x44\xac\x37\xcb\xc1\xde\x84\xcf\xac"
      "\x5f\xab\xf7\x4d\xc5\x82\xd4\xb5\x77\x1f\x5b\xdf\xb0\xd6\xbd\x09\x1f\x49"
      "\xa8\x08\x9c\x04\xc9\x03\x33\x70\x11\x83",
      4096);
  syscall(__NR_setsockopt, r[0], 0, 0x41, 0x204c3000, 0x1000);
}

int main()
{
  loop();
  return 0;
}