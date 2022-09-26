#include <iostream>

#include <spdk/stdinc.h>
#include <spdk/string.h>
#include <spdk/event.h>
#include <spdk/thread.h>
#include <spdk/log.h>
#include <spdk/bdev_zone.h>
#include <spdk/env.h>

struct hello_context_t {
  struct spdk_bdev *bdev;
  struct spdk_bdev_desc *bdev_desc;
  struct spdk_io_channel *bdev_io_channel;
  char *buff;
  char *bdev_name;
  struct spdk_bdev_io_wait_entry bdev_io_wait;
};

static void hello_start(void *arg1) {
  struct hello_context_t *ctx = (struct hello_context_t *)arg1;

  SPDK_NOTICELOG("Successfully started the application\n");

}

int main() {
  struct spdk_app_opts opts = {};
  struct hello_context_t hello_context = {};
  int rc = 0;

  spdk_app_opts_init(&opts, sizeof(opts));
  opts.name = "hello_bdev";

  //rc = spdk_app_parse_args(argc, argv, &opts, "b:", nullptr,
  //                         hello_bdev_parse_arg, hello_bdev_usage);
  
  rc = spdk_app_start(&opts, hello_start, &hello_context);
  if (rc) {
    SPDK_ERRLOG("ERROR starting application\n");
  }

  return 0;
}
