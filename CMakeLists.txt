project(Flow)
cmake_minimum_required(VERSION 3.6.13)
set(CMAKE_CXX_STANDARD 17)

set(std_libs
  pthread
  rt
  uuid
  numa
  dl)

set(spdk_libs
  "${LINK_FLAGS_BEGIN}"
  "spdk_thread"
  "spdk_event"
  "spdk_env_dpdk"
  "spdk_bdev"
  "spdk_bdev_malloc"
  "spdk_util"
  "spdk_log"
  "spdk_rpc"
  "spdk_jsonrpc"
  "spdk_rpc"
  "spdk_nvmf"
  "spdk_event_nvmf"
  "spdk_iscsi"
  "spdk_scsi"
  "spdk_event_iscsi"
  "spdk_vhost"
  "spdk_event_vhost"
  "spdk_virtio"
  "spdk_bdev_virtio"
  "spdk_trace"
  "spdk_sock"
  "spdk_sock_posix"
  "spdk_accel"
  "${LINK_FLAGS_END}")

set(dpdk_libs
  "${LINK_FLAGS_BEGIN}"
  "rte_mempool"
  "rte_mempool_ring"
  "rte_ring"
  "rte_eal"
  "rte_telemetry"
  "rte_power"
  "rte_pci"
  "rte_bus_pci"
  "rte_vhost"
  "${LINK_FLAGS_END}")


set(DPDK_BUILD_PATH /home/jyshi/Jiayou/flow/third-party/spdk/dpdk/build)
set(SPDK_BUILD_PATH /home/jyshi/Jiayou/flow/third-party/spdk/build)


include_directories(${DPDK_BUILD_PATH}/include)
link_directories(${DPDK_BUILD_PATH}/lib)

include_directories(${SPDK_BUILD_PATH}/include)
link_directories(${SPDK_BUILD_PATH}/lib)

add_executable(test_it
  db/test.cc)
target_link_libraries(test_it
  ${spdk_libs}
  ${dpdk_libs}
  ${std_libs})
target_link_options(test_it PUBLIC 
  "LINKER:-no-as-needed,-copy-dt-needed-entries")

enable_testing()
add_test(NAME FlowTest COMMAND test_it)

