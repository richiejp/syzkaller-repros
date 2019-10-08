// KASAN: global-out-of-bounds Read in load_next_firmware_from_table
// https://syzkaller.appspot.com/bug?id=9e4fafb6fbc53782278754488801c0bbe1fd2a85
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/usb/ch9.h>

unsigned long long procid;

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

#define USB_MAX_EP_NUM 32

struct usb_device_index {
  struct usb_device_descriptor* dev;
  struct usb_config_descriptor* config;
  unsigned config_length;
  struct usb_interface_descriptor* iface;
  struct usb_endpoint_descriptor* eps[USB_MAX_EP_NUM];
  unsigned eps_num;
};

static bool parse_usb_descriptor(char* buffer, size_t length,
                                 struct usb_device_index* index)
{
  if (length <
      sizeof(*index->dev) + sizeof(*index->config) + sizeof(*index->iface))
    return false;
  index->dev = (struct usb_device_descriptor*)buffer;
  index->config = (struct usb_config_descriptor*)(buffer + sizeof(*index->dev));
  index->config_length = length - sizeof(*index->dev);
  index->iface =
      (struct usb_interface_descriptor*)(buffer + sizeof(*index->dev) +
                                         sizeof(*index->config));
  index->eps_num = 0;
  size_t offset = 0;
  while (true) {
    if (offset + 1 >= length)
      break;
    uint8_t desc_length = buffer[offset];
    uint8_t desc_type = buffer[offset + 1];
    if (desc_length <= 2)
      break;
    if (offset + desc_length > length)
      break;
    if (desc_type == USB_DT_ENDPOINT) {
      index->eps[index->eps_num] =
          (struct usb_endpoint_descriptor*)(buffer + offset);
      index->eps_num++;
    }
    if (index->eps_num == USB_MAX_EP_NUM)
      break;
    offset += desc_length;
  }
  return true;
}

enum usb_fuzzer_event_type {
  USB_FUZZER_EVENT_INVALID,
  USB_FUZZER_EVENT_CONNECT,
  USB_FUZZER_EVENT_DISCONNECT,
  USB_FUZZER_EVENT_SUSPEND,
  USB_FUZZER_EVENT_RESUME,
  USB_FUZZER_EVENT_CONTROL,
};

struct usb_fuzzer_event {
  uint32_t type;
  uint32_t length;
  char data[0];
};

struct usb_fuzzer_init {
  uint64_t speed;
  const char* driver_name;
  const char* device_name;
};

struct usb_fuzzer_ep_io {
  uint16_t ep;
  uint16_t flags;
  uint32_t length;
  char data[0];
};

#define USB_FUZZER_IOCTL_INIT _IOW('U', 0, struct usb_fuzzer_init)
#define USB_FUZZER_IOCTL_RUN _IO('U', 1)
#define USB_FUZZER_IOCTL_EP0_READ _IOWR('U', 2, struct usb_fuzzer_event)
#define USB_FUZZER_IOCTL_EP0_WRITE _IOW('U', 3, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_EP_ENABLE _IOW('U', 4, struct usb_endpoint_descriptor)
#define USB_FUZZER_IOCTL_EP_WRITE _IOW('U', 6, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_EP_READ _IOWR('U', 7, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_CONFIGURE _IO('U', 8)
#define USB_FUZZER_IOCTL_VBUS_DRAW _IOW('U', 9, uint32_t)

int usb_fuzzer_open()
{
  return open("/sys/kernel/debug/usb-fuzzer", O_RDWR);
}

int usb_fuzzer_init(int fd, uint32_t speed, const char* driver,
                    const char* device)
{
  struct usb_fuzzer_init arg;
  arg.speed = speed;
  arg.driver_name = driver;
  arg.device_name = device;
  return ioctl(fd, USB_FUZZER_IOCTL_INIT, &arg);
}

int usb_fuzzer_run(int fd)
{
  return ioctl(fd, USB_FUZZER_IOCTL_RUN, 0);
}

int usb_fuzzer_ep0_read(int fd, struct usb_fuzzer_event* event)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP0_READ, event);
}

int usb_fuzzer_ep0_write(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP0_WRITE, io);
}

int usb_fuzzer_ep_write(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP_WRITE, io);
}

int usb_fuzzer_ep_read(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP_READ, io);
}

int usb_fuzzer_ep_enable(int fd, struct usb_endpoint_descriptor* desc)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP_ENABLE, desc);
}

int usb_fuzzer_configure(int fd)
{
  return ioctl(fd, USB_FUZZER_IOCTL_CONFIGURE, 0);
}

int usb_fuzzer_vbus_draw(int fd, uint32_t power)
{
  return ioctl(fd, USB_FUZZER_IOCTL_VBUS_DRAW, power);
}

#define USB_MAX_PACKET_SIZE 1024

struct usb_fuzzer_control_event {
  struct usb_fuzzer_event inner;
  struct usb_ctrlrequest ctrl;
  char data[USB_MAX_PACKET_SIZE];
};

struct usb_fuzzer_ep_io_data {
  struct usb_fuzzer_ep_io inner;
  char data[USB_MAX_PACKET_SIZE];
};

struct vusb_connect_string_descriptor {
  uint32_t len;
  char* str;
} __attribute__((packed));

struct vusb_connect_descriptors {
  uint32_t qual_len;
  char* qual;
  uint32_t bos_len;
  char* bos;
  uint32_t strs_len;
  struct vusb_connect_string_descriptor strs[0];
} __attribute__((packed));

static bool lookup_connect_response(struct vusb_connect_descriptors* descs,
                                    struct usb_device_index* index,
                                    struct usb_ctrlrequest* ctrl,
                                    char** response_data,
                                    uint32_t* response_length, bool* done)
{
  uint8_t str_idx;
  switch (ctrl->bRequestType & USB_TYPE_MASK) {
  case USB_TYPE_STANDARD:
    switch (ctrl->bRequest) {
    case USB_REQ_GET_DESCRIPTOR:
      switch (ctrl->wValue >> 8) {
      case USB_DT_DEVICE:
        *response_data = (char*)index->dev;
        *response_length = sizeof(*index->dev);
        return true;
      case USB_DT_CONFIG:
        *response_data = (char*)index->config;
        *response_length = index->config_length;
        return true;
      case USB_DT_STRING:
        str_idx = (uint8_t)ctrl->wValue;
        if (str_idx >= descs->strs_len && descs->strs_len > 0) {
          str_idx = descs->strs_len - 1;
        }
        *response_data = descs->strs[str_idx].str;
        *response_length = descs->strs[str_idx].len;
        return true;
      case USB_DT_BOS:
        *response_data = descs->bos;
        *response_length = descs->bos_len;
        return true;
      case USB_DT_DEVICE_QUALIFIER:
        *response_data = descs->qual;
        *response_length = descs->qual_len;
        return true;
      default:
        exit(1);
        return false;
      }
      break;
    case USB_REQ_SET_CONFIGURATION:
      *response_length = 0;
      *response_data = NULL;
      *done = true;
      return true;
    default:
      exit(1);
      return false;
    }
    break;
  default:
    exit(1);
    return false;
  }
  return false;
}

static volatile long syz_usb_connect(volatile long a0, volatile long a1,
                                     volatile long a2, volatile long a3)
{
  int64_t speed = a0;
  int64_t dev_len = a1;
  char* dev = (char*)a2;
  struct vusb_connect_descriptors* descs = (struct vusb_connect_descriptors*)a3;
  if (!dev) {
    return -1;
  }
  struct usb_device_index index;
  memset(&index, 0, sizeof(index));
  int rv = 0;
  rv = parse_usb_descriptor(dev, dev_len, &index);
  if (!rv) {
    return rv;
  }
  int fd = usb_fuzzer_open();
  if (fd < 0) {
    return fd;
  }
  char device[32];
  sprintf(&device[0], "dummy_udc.%llu", procid);
  rv = usb_fuzzer_init(fd, speed, "dummy_udc", &device[0]);
  if (rv < 0) {
    return rv;
  }
  rv = usb_fuzzer_run(fd);
  if (rv < 0) {
    return rv;
  }
  bool done = false;
  while (!done) {
    struct usb_fuzzer_control_event event;
    event.inner.type = 0;
    event.inner.length = sizeof(event.ctrl);
    rv = usb_fuzzer_ep0_read(fd, (struct usb_fuzzer_event*)&event);
    if (rv < 0) {
      return rv;
    }
    if (event.inner.type != USB_FUZZER_EVENT_CONTROL)
      continue;
    bool response_found = false;
    char* response_data = NULL;
    uint32_t response_length = 0;
    response_found = lookup_connect_response(
        descs, &index, &event.ctrl, &response_data, &response_length, &done);
    if (!response_found) {
      return -1;
    }
    if (done) {
      rv = usb_fuzzer_vbus_draw(fd, index.config->bMaxPower);
      if (rv < 0) {
        return rv;
      }
      rv = usb_fuzzer_configure(fd);
      if (rv < 0) {
        return rv;
      }
      unsigned ep;
      for (ep = 0; ep < index.eps_num; ep++) {
        rv = usb_fuzzer_ep_enable(fd, index.eps[ep]);
        if (rv < 0) {
        } else {
        }
      }
    }
    struct usb_fuzzer_ep_io_data response;
    response.inner.ep = 0;
    response.inner.flags = 0;
    if (response_length > sizeof(response.data))
      response_length = 0;
    response.inner.length = response_length;
    if (response_data)
      memcpy(&response.data[0], response_data, response_length);
    if (event.ctrl.wLength < response.inner.length)
      response.inner.length = event.ctrl.wLength;
    rv = usb_fuzzer_ep0_write(fd, (struct usb_fuzzer_ep_io*)&response);
    if (rv < 0) {
      return rv;
    }
  }
  sleep_ms(200);
  return fd;
}

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);

  *(uint8_t*)0x20000040 = 0x12;
  *(uint8_t*)0x20000041 = 1;
  *(uint16_t*)0x20000042 = 0x110;
  *(uint8_t*)0x20000044 = 0xcb;
  *(uint8_t*)0x20000045 = 0xd9;
  *(uint8_t*)0x20000046 = 0xd0;
  *(uint8_t*)0x20000047 = 8;
  *(uint16_t*)0x20000048 = 0x1286;
  *(uint16_t*)0x2000004a = 0x2001;
  *(uint16_t*)0x2000004c = 0xa08e;
  *(uint8_t*)0x2000004e = 0x7f;
  *(uint8_t*)0x2000004f = 1;
  *(uint8_t*)0x20000050 = 1;
  *(uint8_t*)0x20000051 = 1;
  *(uint8_t*)0x20000052 = 9;
  *(uint8_t*)0x20000053 = 2;
  *(uint16_t*)0x20000054 = 0x6c;
  *(uint8_t*)0x20000056 = 1;
  *(uint8_t*)0x20000057 = 4;
  *(uint8_t*)0x20000058 = 2;
  *(uint8_t*)0x20000059 = 0x20;
  *(uint8_t*)0x2000005a = 1;
  *(uint8_t*)0x2000005b = 9;
  *(uint8_t*)0x2000005c = 4;
  *(uint8_t*)0x2000005d = 0xaa;
  *(uint8_t*)0x2000005e = 0;
  *(uint8_t*)0x2000005f = 6;
  *(uint8_t*)0x20000060 = 0x5d;
  *(uint8_t*)0x20000061 = 0x67;
  *(uint8_t*)0x20000062 = 0x89;
  *(uint8_t*)0x20000063 = 0;
  *(uint8_t*)0x20000064 = 0x18;
  *(uint8_t*)0x20000065 = 0x21;
  *(uint16_t*)0x20000066 = 0xf2;
  *(uint8_t*)0x20000068 = 0;
  *(uint8_t*)0x20000069 = 6;
  *(uint8_t*)0x2000006a = 0x22;
  *(uint16_t*)0x2000006b = 0x8b3;
  *(uint8_t*)0x2000006d = 0x22;
  *(uint16_t*)0x2000006e = 0xf8e;
  *(uint8_t*)0x20000070 = 0x23;
  *(uint16_t*)0x20000071 = 0xa79;
  *(uint8_t*)0x20000073 = 0x23;
  *(uint16_t*)0x20000074 = 0xd4b;
  *(uint8_t*)0x20000076 = 0x23;
  *(uint16_t*)0x20000077 = 0xd62;
  *(uint8_t*)0x20000079 = 0x21;
  *(uint16_t*)0x2000007a = 0xa2a;
  *(uint8_t*)0x2000007c = 2;
  *(uint8_t*)0x2000007d = 9;
  *(uint8_t*)0x2000007e = 9;
  *(uint8_t*)0x2000007f = 5;
  *(uint8_t*)0x20000080 = 5;
  *(uint8_t*)0x20000081 = 7;
  *(uint16_t*)0x20000082 = 7;
  *(uint8_t*)0x20000084 = 0x9a;
  *(uint8_t*)0x20000085 = 0;
  *(uint8_t*)0x20000086 = 5;
  *(uint8_t*)0x20000087 = 9;
  *(uint8_t*)0x20000088 = 5;
  *(uint8_t*)0x20000089 = 0xf;
  *(uint8_t*)0x2000008a = 3;
  *(uint16_t*)0x2000008b = 0;
  *(uint8_t*)0x2000008d = 9;
  *(uint8_t*)0x2000008e = 1;
  *(uint8_t*)0x2000008f = 8;
  *(uint8_t*)0x20000090 = 9;
  *(uint8_t*)0x20000091 = 5;
  *(uint8_t*)0x20000092 = 0;
  *(uint8_t*)0x20000093 = 0;
  *(uint16_t*)0x20000094 = 5;
  *(uint8_t*)0x20000096 = 0;
  *(uint8_t*)0x20000097 = 0;
  *(uint8_t*)0x20000098 = 0;
  *(uint8_t*)0x20000099 = 2;
  *(uint8_t*)0x2000009a = 0x2f;
  *(uint8_t*)0x2000009b = 9;
  *(uint8_t*)0x2000009c = 5;
  *(uint8_t*)0x2000009d = 0xc;
  *(uint8_t*)0x2000009e = 2;
  *(uint16_t*)0x2000009f = 4;
  *(uint8_t*)0x200000a1 = 4;
  *(uint8_t*)0x200000a2 = 7;
  *(uint8_t*)0x200000a3 = 0;
  *(uint8_t*)0x200000a4 = 2;
  *(uint8_t*)0x200000a5 = 0x21;
  *(uint8_t*)0x200000a6 = 9;
  *(uint8_t*)0x200000a7 = 5;
  *(uint8_t*)0x200000a8 = 0xc;
  *(uint8_t*)0x200000a9 = 0;
  *(uint16_t*)0x200000aa = 1;
  *(uint8_t*)0x200000ac = 2;
  *(uint8_t*)0x200000ad = 0;
  *(uint8_t*)0x200000ae = 4;
  *(uint8_t*)0x200000af = 2;
  *(uint8_t*)0x200000b0 = 0x23;
  *(uint8_t*)0x200000b1 = 9;
  *(uint8_t*)0x200000b2 = 5;
  *(uint8_t*)0x200000b3 = 0x8d;
  *(uint8_t*)0x200000b4 = 0x1a;
  *(uint16_t*)0x200000b5 = 0xadb;
  *(uint8_t*)0x200000b7 = 3;
  *(uint8_t*)0x200000b8 = 0xdc;
  *(uint8_t*)0x200000b9 = 0xbb;
  *(uint8_t*)0x200000ba = 2;
  *(uint8_t*)0x200000bb = 0x1b;
  *(uint8_t*)0x200000bc = 2;
  *(uint8_t*)0x200000bd = 0x10;
  *(uint32_t*)0x20000940 = 0;
  *(uint64_t*)0x20000944 = 0;
  *(uint32_t*)0x2000094c = 0;
  *(uint64_t*)0x20000950 = 0;
  *(uint32_t*)0x20000958 = 7;
  *(uint32_t*)0x2000095c = 0;
  *(uint64_t*)0x20000960 = 0;
  *(uint32_t*)0x20000968 = 0;
  *(uint64_t*)0x2000096c = 0;
  *(uint32_t*)0x20000974 = 0;
  *(uint64_t*)0x20000978 = 0;
  *(uint32_t*)0x20000980 = 0;
  *(uint64_t*)0x20000984 = 0;
  *(uint32_t*)0x2000098c = 0;
  *(uint64_t*)0x20000990 = 0;
  *(uint32_t*)0x20000998 = 0;
  *(uint64_t*)0x2000099c = 0;
  *(uint32_t*)0x200009a4 = 0;
  *(uint64_t*)0x200009a8 = 0;
  syz_usb_connect(7, 0x7e, 0x20000040, 0x20000940);
  return 0;
}