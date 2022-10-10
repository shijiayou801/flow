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
#include <spdk/rpc.h>
#include <spdk/bdev.h>
#include <spdk/likely.h>

#define kNvmfDefaultSubsystems 32

enum nvmf_target_state {
	kNvmfInitSubsystem = 0,
  kNvmfInitTarget,
  kNvmfInitPollGroups,
  kNvmfInitStartSubsystems,
  kNvmfRunning,
  kNvmfFiniStopSubsystems,
  kNvmfFiniPollGroups,
  kNvmfFiniTarget,
  kNvmfFiniSubsystem,
};

TAILQ_HEAD(, nvmf_reactor) g_reactors = TAILQ_HEAD_INITIALIZER(g_reactors);


static struct nvmf_reactor *g_main_reactor = nullptr;
static struct nvmf_reactor *g_next_reactor = nullptr;
static struct spdk_thread *g_init_thread = nullptr;
static struct spdk_thread *g_fini_thread = nullptr;


static enum nvmf_target_state g_target_state;

struct nvmf_reactor {
  uint32_t core;

  struct spdk_ring *threads;
  TAILQ_ENTRY(nvmf_reactor) link;
};

struct nvmf_lw_thread {
  TAILQ_ENTRY(nvmf_lw_thread) link;
  bool resched;
};



static void 

static void nvmf_target_advance_state() {
  enum nvmf_target_state prev_state;

  do {
    prev_state = g_target_state;

    switch (g_target_state) {
      case kNvmfInitSubsystem:
        spdk_subsystem_init(nvmf_subsystem_init_done, nullptr);
        return;
    }

  } while (g_target_state != prev_state);
}

static void nvmf_target_app_start(void *arg) {
  g_target_state = kNvmfInitSubsystem;
  nvmf_target_advance_state();
}

static int nvmf_schedule_spdk_thread(struct spdk_thread *thread) {
  struct spdk_cpuset *cpumask;

  return 0;
}


static int nvmf_reactor_thread_op(struct spdk_thread *thread, 
                                  enum spdk_thread_op op) {
  switch (op) {
    case SPDK_THREAD_OP_NEW:
      return nvmf_schedule_spdk_thread(thread);
    case SPDK_THREAD_OP_RESCHED:
      //nvmf_request_spdk_thread_reschedule(thread);
      return 0;
    default:
      return -ENOTSUP;
  }
}

static bool nvmf_reactor_thread_op_supported(enum spdk_thread_op op)
{
	switch (op) {
  	case SPDK_THREAD_OP_NEW:
	  case SPDK_THREAD_OP_RESCHED:
		  return true;
	  default:
		  return false;
	}
}

static int nvmf_init_threads() {
  int rc;
  uint32_t i;
  char thread_name[32];
  struct nvmf_reactor *reactor;
  struct spdk_cpuset cpumask;
  uint32_t main_core = spdk_env_get_current_core();
  

  spdk_thread_lib_init_ext(nvmf_reactor_thread_op, 
                           nvmf_reactor_thread_op_supported,
                           sizeof(struct nvmf_lw_thread));

  // Create one system thread per core
  SPDK_ENV_FOREACH_CORE(i) {
    reactor = (struct nvmf_reactor *)calloc(1, sizeof(struct nvmf_reactor));

    if (!reactor) {
      SPDK_ERRLOG("failed to allocate nvmf_reactor\n");
      rc = -ENOMEM;
      goto err_exit;
    }

    reactor->core = i;

    reactor->threads = spdk_ring_create(
      SPDK_RING_TYPE_MP_SC, // Multiple producer, single consumer
      1024,
      SPDK_ENV_SOCKET_ID_ANY);
    if (!reactor->threads) {
      SPDK_ERRLOG("failed to allocate ring\n");
      free(reactor);
      rc = -ENOMEM;
      goto err_exit;
    }
    
    TAILQ_INSERT_TAIL(&g_reactors, reactor, link);

    if (main_core == i) {
      g_main_reactor = reactor;
      g_next_reactor = g_main_reactor;
    } else {
      assert(false);
      /*
      rc = spdk_env_thread_launch_pinned(
        i,
        nvmf_reactor_run,
        reactor);
      if (rc) {
        SPDK_ERRLOG("failed to pin reactor launch\n");
        goto err_exit;
      }*/
    }
  }

  // Create a lightweight thread only on this core to manage this application
  spdk_cpuset_zero(&cpumask);
  spdk_cpuset_set_cpu(&cpumask, main_core, true);
  snprintf(thread_name, sizeof(thread_name), "nvmf_main_thread");
  g_init_thread = spdk_thread_create(thread_name, &cpumask);
  if (!g_init_thread) {
    SPDK_ERRLOG("failed to create spdk thread\n");
    rc = -1;
    goto err_exit;
  }

  SPDK_NOTICELOG("nvmf threads initialized\n");
  return 0;

err_exit:
  return rc;
}

int main() {
  int rc;
  struct spdk_env_opts opts;

  spdk_env_opts_init(&opts);
  opts.name = "nvmf-example";

  if (spdk_env_init(&opts) < 0) {
    SPDK_ERRLOG("unable to initialize spdk env\n");
    return -EINVAL;
  }

  rc = nvmf_init_threads();
  assert(rc == 0);

  /* Send a message to the thread assigned to the main reactor
   * that continues initialization. This is how we bootstrap the
   * program so that all code from here on is running on an SPDK thread.
   */
  assert(g_init_thread != nullptr);

  //rc = nvmf_setup_signal_handlers();
  //assert(rc == 0);

  spdk_thread_send_msg(g_init_thread, nvmf_target_app_start, nullptr);

  return 0;
}
