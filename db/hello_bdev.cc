#include <iostream>
#include <thread>
#include <chrono>


#include <spdk/stdinc.h>
#include <spdk/string.h>
#include <spdk/event.h>
#include <spdk/thread.h>
#include <spdk/log.h>
#include <spdk/bdev_zone.h>
#include <spdk/env.h>

static char *g_bdev_name = "Malloc0";

struct hello_context_t {
  struct spdk_bdev *bdev;
  struct spdk_bdev_desc *bdev_desc;
  struct spdk_io_channel *bdev_io_channel;
  char *buff;
  char *bdev_name;
  struct spdk_bdev_io_wait_entry bdev_io_wait;
};

static void hello_bdev_event_cb(enum spdk_bdev_event_type type, 
    struct spdk_bdev *bdev, void *event_ctx) {
  SPDK_NOTICELOG("Unsupported bdev event: type %d\n", type);
}

static void hello_start(void *arg1) {
  struct hello_context_t *hello_context = (struct hello_context_t *)arg1;
  uint32_t blk_size, buf_align;
  int rc = 0;

  hello_context->bdev = nullptr;
  hello_context->bdev_desc = nullptr;

  SPDK_NOTICELOG("Opening the bdev %s\n", hello_context->bdev_name);
  rc = spdk_bdev_open_ext(hello_context->bdev_name, true, hello_bdev_event_cb,
      nullptr, &hello_context->bdev_desc);
  if (rc) {
    SPDK_ERRLOG("Failed to open bdev %s\n", hello_context->bdev_name);
    spdk_app_stop(-1);
    return;
  }

}

int main() {
  struct spdk_app_opts opts = {};
  struct hello_context_t hello_context = {};
  int rc = 0;

  spdk_app_opts_init(&opts, sizeof(opts));
  opts.name = "hello_bdev";
  hello_context.bdev_name = g_bdev_name;
 
  SPDK_NOTICELOG("starting...\n");


  rc = spdk_app_start(&opts, hello_start, &hello_context);
  if (rc) {
    SPDK_ERRLOG("failed to start application\n");
  }

  SPDK_NOTICELOG("stopping...\n");

  spdk_dma_free(hello_context.buff);

  spdk_app_fini();

  return 0;
}
