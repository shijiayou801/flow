#include <iostream>
#include <thread>
#include <chrono>

#include <spdk/stdinc.h>
#include <spdk/string.h>
#include <spdk/event.h>
#include <spdk/thread.h>
#include <spdk/log.h>
#include <spdk/env.h>
#include <spdk/nvme.h>

static struct spdk_nvme_transport_id g_trid = {};


static bool probe_cb(void *cb_ctx, const struct spdk_nvme_transport_id *trid,
    struct spdk_nvme_ctrlr_opts *opts) {
  SPDK_NOTICELOG("Attaching to %s\n", trid->traddr);
  return true;
}

static void attach_cb(void *cb_ctx, const struct spdk_nvme_transport_id *trid,
    struct spdk_nvme_ctrlr *ctrlr, const struct spdk_nvme_ctrlr_opts *opts) {

}

int client() {
  int rc = 0;
  struct spdk_env_opts opts;

  spdk_env_opts_init(&opts);
  opts.name = "flow";
  if (spdk_env_init(&opts) < 0) {
    SPDK_ERRLOG("failed to init env\n");
    return 1;
  }

  SPDK_NOTICELOG("initializing NVMe Controllers\n");
  rc = spdk_nvme_probe(&g_trid, nullptr, probe_cb, attach_cb, nullptr);
  if (rc != 0) {
    SPDK_ERRLOG("spdk_nvme_probe() failed\n");
    rc = 1;
    goto exit;
  }


exit:
  spdk_env_fini();
  return 1;
}

int server() {
  
  return 0;
}

int main() {
  return server();
}
