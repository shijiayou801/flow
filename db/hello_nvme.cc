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
#include <spdk/nvmf.h>
#include <spdk/nvmf_cmd.h>

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
  int r = 0;
  struct spdk_env_opts opts;

  spdk_env_opts_init(&opts);
  opts.name = "flow";
  if (spdk_env_init(&opts) < 0) {
    SPDK_ERRLOG("failed to init env\n");
    return 1;
  }

  SPDK_NOTICELOG("initializing NVMe Controllers\n");
  r = spdk_nvme_probe(&g_trid, nullptr, probe_cb, attach_cb, nullptr);
  if (r != 0) {
    SPDK_ERRLOG("spdk_nvme_probe() failed\n");
    r = 1;
    goto exit;
  }


exit:
  spdk_env_fini();
  return 1;
}

int server() {
  int r = 0;


  struct spdk_env_opts spdk_opts;
  spdk_env_opts_init(&spdk_opts);
  spdk_opts.name = "hello_nvme";

  r = spdk_env_init(&spdk_opts);
  if (r != 0) {
    SPDK_NOTICELOG("spdk env init failed\n");
    return 1;
  }

  r = spdk_thread_lib_init(nullptr, 0);
  if (r != 0) {
    SPDK_NOTICELOG("spdk thread lib init failed\n");
    return 1;
  }

  struct spdk_thread *thread = spdk_thread_create("hello_nvme", nullptr);

  spdk_set_thread(thread);

  SPDK_NOTICELOG("%d is nullptr?\n", thread == nullptr);


  struct spdk_nvmf_target_opts target_opts;
  struct spdk_nvmf_tgt *target; 

  snprintf(target_opts.name, sizeof(target_opts.name), "%s", "hello_nvmf");
  target_opts.max_subsystems = 5;


  spdk_log_set_level(SPDK_LOG_DEBUG);
  spdk_log_set_print_level(SPDK_LOG_DEBUG);

  enum spdk_log_level level = spdk_log_get_level();
  enum spdk_log_level level_2 = spdk_log_get_print_level();

  SPDK_NOTICELOG("%d %d %d\n", level, level_2, SPDK_LOG_DEBUG);

  target = spdk_nvmf_tgt_create(&target_opts);
  if (target == nullptr) {
    fprintf(stderr, "spdk_nvmf_tgt_create() failed\n");
    return 1;
  }

  spdk_thread_destroy(thread);

  return 0;
}

int main() {


  server();

  return 0;
}
