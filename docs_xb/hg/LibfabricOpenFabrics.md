# LibfabricOpenFabrics

![OpenFabrics 接口概述](https://ofiwg.github.io/libfabric/images/openfabric-interfaces-overview.png)

# 最新版本

- libfabric 库、单元测试和文档：[libfabric v1.15.2](https://github.com/ofiwg/libfabric/releases/tag/v1.15.2)（或[查看所有以前的版本](https://github.com/ofiwg/libfabric/releases/)）。

# 概述

OpenFabrics Interfaces (OFI) 是一个专注于将结构通信服务导出到应用程序的框架。OFI 最好被描述为用于导出结构服务的库和应用程序的集合。OFI 的关键组件是：应用程序接口、提供程序库、内核服务、守护程序和测试应用程序。

![img](https://ofiwg.github.io/libfabric/images/ofa-logo.png)Libfabric 是 OFI 的核心组件。它是定义和导出 OFI 的用户空间 API 的库，通常是应用程序直接处理的唯一软件。它与提供程序库一起工作，这些提供程序库通常直接集成到 libfabric 中。

Libfabric 由[OpenFabrics Alliance - OFA](http://www.openfabrics.org/)的一个子组 OFI 工作组（OFIWG，发音为“o-fee-wig”）开发。任何人都可以参加 OFIWG，而不仅限于 OFA 的成员。

OFI 的目标，特别是 libfabric，是定义接口，在应用程序和底层结构服务之间实现紧密的语义映射。具体来说，libfabric 软件接口是与 Fabric 硬件提供商和应用程序开发人员共同设计的，重点是 HPC 用户的需求。Libfabric 支持多种接口语义，与结构和硬件实现无关，并利用和扩展现有的 RDMA 开源社区。

Libfabric 旨在最大限度地减少应用程序之间的阻抗失配，包括 MPI、SHMEM 和 PGAS 等中间件以及结构通信硬件。它的接口针对高带宽、低延迟的 NIC，目标是扩展到数万个节点。

Libfabric 的目标是支持 Linux、Free BSD、Windows 和 OS X。为支持所有主要的现代 Linux 发行版做出了合理的努力；但是，验证仅限于 Red Hat Enterprise Linux (RHEL) 和 SUSE Linux Enterprise Server (SLES) 的最新 2-3 个版本。Libfabric 将其支持的发行版与最新的 OpenFabrics Enterprise Distribution (OFED) 软件版本保持一致。对特定操作系统版本或发行版的支持是特定于供应商的。例外情况是基于 tcp 和 udp 的套接字提供程序可在所有平台上使用。

Libfabric 受到各种开源 HPC 中间件应用程序的支持，包括 MPICH、Open MPI、Sandia SHMEM、Open SHMEM、Charm++、GasNET、Clang、UPC 等。此外，一些专有软件应用程序（例如 Intel MPI）和非公共应用程序端口是已知的。Libfabric 支持各种高性能结构和网络硬件。它将在标准 TCP 和 UDP 网络、高性能结构（如 Omni-Path 架构、InfiniBand、Cray GNI、Blue Gene 架构、iWarp RDMA 以太网、融合以太网上的 RDMA (RoCE)）上运行，并支持多个网络和内存 CPU面料正在积极开发中。



查看： 

### verbs

------

The verbs provider enables applications using OFI to be run over any verbs hardware (Infiniband, iWarp, and RoCE). It uses the Linux Verbs API for network transport and translates OFI calls to appropriate verbs API calls. It uses librdmacm for communication management and libibverbs for other control and data transfer operations.

See the `fi_verbs(7)` man page for more details.

#### Dependencies

- The verbs provider requires libibverbs (v1.1.8 or newer) and librdmacm (v1.0.16 or newer). If you are compiling libfabric from source and want to enable verbs support, you will also need the matching header files for the above two libraries. If the libraries and header files are not in default paths, specify them in CFLAGS, LDFLAGS and LD_LIBRARY_PATH environment variables.

https://ofiwg.github.io/libfabric/v1.6.1/man/fi_verbs.7.html

# fi_verbs(7) Libfabric Programmer's Manual

# NAME

fi_verbs - The Verbs Fabric Provider

# OVERVIEW

The verbs provider enables applications using OFI to be run over any verbs hardware (Infiniband, iWarp, etc). It uses the Linux Verbs API for network transport and provides a translation of OFI calls to appropriate verbs API calls. It uses librdmacm for communication management and libibverbs for other control and data transfer operations.

# SUPPORTED FEATURES

The verbs provider supports a subset of OFI features.

### Endpoint types

FI_EP_MSG, FI_EP_RDM

New change in libfabric v1.6: FI_EP_RDM is supported through the OFI RxM utility provider. This is done automatically when the app requests FI_EP_RDM endpoint. Please refer the man page for RxM provider to learn more. The provider’s internal support for RDM endpoints is deprecated and would be removed from libfabric v1.7 onwards. Till then apps can explicitly request the internal RDM support by disabling ofi_rxm provider through FI_PROVIDER env variable (FI_PROVIDER=^ofi_rxm).

### Endpoint capabilities and features

#### MSG endpoints

FI_MSG, FI_RMA, FI_ATOMIC and shared receive contexts.

#### RDM endpoints (internal - deprecated)

FI_MSG, FI_TAGGED, FI_RMA

#### DGRAM endpoints

FI_MSG

### Modes

Verbs provider requires applications to support the following modes:

#### FI_EP_MSG endpoint type

- FI_LOCAL_MR / FI_MR_LOCAL mr mode.
- FI_RX_CQ_DATA for applications that want to use RMA. Applications must take responsibility of posting receives for any incoming CQ data.

#### FI_EP_RDM endpoint type (internal - deprecated)

- FI_CONTEXT

### Addressing Formats

Supported addressing formats include

- MSG and RDM (internal - deprecated) EPs support: FI_SOCKADDR, FI_SOCKADDR_IN, FI_SOCKADDR_IN6, FI_SOCKADDR_IB
- DGRAM supports: FI_ADDR_IB_UD

### Progress

Verbs provider supports FI_PROGRESS_AUTO: Asynchronous operations make forward progress automatically.

### Operation flags

Verbs provider supports FI_INJECT, FI_COMPLETION, FI_REMOTE_CQ_DATA, FI_TRANSMIT_COMPLETE.

### Msg Ordering

Verbs provider support the following message ordering:

- Read after Read
- Read after Write
- Read after Send
- Write after Write
- Write after Send
- Send after Write
- Send after Send

and the following completion ordering:

- TX contexts: FI_ORDER_STRICT
- RX contexts: FI_ORDER_DATA

### Fork

Verbs provider supports the fork system call by default. See the limitations section for restrictions. It can be turned off by setting the FI_FORK_UNSAFE environment variable to “yes”. This can improve the performance of memory registrations but it also makes the use of fork unsafe.

### Memory Registration Cache

The verbs provider features a memory registration cache. This speeds up memory registration calls from applications by caching registrations of frequently used memory regions. The user can control the maximum combined size of all cache entries and the maximum number of cache entries with the environment variables FI_VERBS_MR_MAX_CACHED_SIZE and FI_VERBS_MR_MAX_CACHED_CNT respectively. Look below in the environment variables section for details.

Note: The memory registration cache framework hooks into alloc and free calls to monitor the memory regions. If this doesn’t work as expected caching would not be optimal.

# LIMITATIONS

### Memory Regions

Only FI_MR_BASIC mode is supported. Adding regions via s/g list is supported only up to a s/g list size of 1. No support for binding memory regions to a counter.

### Wait objects

Only FI_WAIT_FD wait object is supported only for FI_EP_MSG endpoint type. Wait sets are not supported.

### Resource Management

Application has to make sure CQs are not overrun as this cannot be detected by the provider.

### Unsupported Features

The following features are not supported in verbs provider:

#### Unsupported Capabilities

FI_NAMED_RX_CTX, FI_DIRECTED_RECV, FI_TRIGGER, FI_RMA_EVENT

#### Other unsupported features

Scalable endpoints, FABRIC_DIRECT

#### Unsupported features specific to MSG endpoints

- Counters, FI_SOURCE, FI_TAGGED, FI_PEEK, FI_CLAIM, fi_cancel, fi_ep_alias, shared TX context, cq_readfrom operations.
- Completion flags are not reported if a request posted to an endpoint completes in error.

#### Unsupported features specific to RDM (internal - deprecated) endpoints

The RDM support for verbs have the following limitations:

- Supports iovs of only size 1.
- Wait objects are not supported.
- Not thread safe.

### Fork

The support for fork in the provider has the following limitations:

- Fabric resources like endpoint, CQ, EQ, etc. should not be used in the forked process.
- The memory registered using fi_mr_reg has to be page aligned since ibv_reg_mr marks the entire page that a memory region belongs to as not to be re-mapped when the process is forked (MADV_DONTFORK).

# RUNTIME PARAMETERS

The verbs provider checks for the following environment variables.

### Common variables:

- *FI_VERBS_TX_SIZE*

  Default maximum tx context size (default: 384)

- *FI_VERBS_RX_SIZE*

  Default maximum rx context size (default: 384)

- *FI_VERBS_TX_IOV_LIMIT*

  Default maximum tx iov_limit (default: 4). Note: RDM (internal - deprecated) EP type supports only 1

- *FI_VERBS_RX_IOV_LIMIT*

  Default maximum rx iov_limit (default: 4). Note: RDM (internal - deprecated) EP type supports only 1

- *FI_VERBS_INLINE_SIZE*

  Default maximum inline size. Actual inject size returned in fi_info may be greater (default: 64)

- *FI_VERBS_MIN_RNR_TIMER*

  Set min_rnr_timer QP attribute (0 - 31) (default: 12)

- *FI_VERBS_USE_ODP*

  Enable On-Demand-Paging (ODP) experimental feature. The feature is supported only on Mellanox OFED (default: 0)

- *FI_VERBS_CQREAD_BUNCH_SIZE*

  The number of entries to be read from the verbs completion queue at a time (default: 8).

- *FI_VERBS_IFACE*

  The prefix or the full name of the network interface associated with the verbs device (default: ib)

- *FI_VERBS_MR_CACHE_ENABLE*

  Enable Memory Registration caching (default: 0)

- *FI_VERBS_MR_MAX_CACHED_CNT*

  Maximum number of cache entries (default: 4096)

- *FI_VERBS_MR_MAX_CACHED_SIZE*

  Maximum total size of cache entries (default: 4 GB)

### Variables specific to RDM (internal - deprecated) endpoints

- *FI_VERBS_RDM_BUFFER_NUM*

  The number of pre-registered buffers for buffered operations between the endpoints, must be a power of 2 (default: 8).

- *FI_VERBS_RDM_BUFFER_SIZE*

  The maximum size of a buffered operation (bytes) (default: platform specific).

- *FI_VERBS_RDM_RNDV_SEG_SIZE*

  The segment size for zero copy protocols (bytes)(default: 1073741824).

- *FI_VERBS_RDM_THREAD_TIMEOUT*

  The wake up timeout of the helper thread (usec) (default: 100).

- *FI_VERBS_RDM_EAGER_SEND_OPCODE*

  The operation code that will be used for eager messaging. Only IBV_WR_SEND and IBV_WR_RDMA_WRITE_WITH_IMM are supported. The last one is not applicable for iWarp. (default: IBV_WR_SEND)

- *FI_VERBS_RDM_CM_THREAD_AFFINITY*

  If specified, bind the CM thread to the indicated range(s) of Linux virtual processor ID(s). This option is currently not supported on OS X. Usage: id_start[-id_end[:stride]][,]

### Variables specific to DGRAM endpoints

- *FI_VERBS_DGRAM_USE_NAME_SERVER*

  The option that enables/disables OFI Name Server thread. The NS thread is used to resolve IP-addresses to provider specific addresses (default: 1, if “OMPI_COMM_WORLD_RANK” and “PMI_RANK” environment variables aren’t defined)

- *FI_VERBS_NAME_SERVER_PORT*

  The port on which Name Server thread listens incoming connections and requests (default: 5678)

### Environment variables notes

The fi_info utility would give the up-to-date information on environment variables: fi_info -p verbs -e

# Troubleshooting / Known issues

When running an app over verbs provider with Valgrind, there may be reports of memory leak in functions from dependent libraries (e.g. libibverbs, librdmacm). These leaks are safe to ignore.

# SEE ALSO

[`fabric`(7)](https://ofiwg.github.io/libfabric/v1.6.1/man/fabric.7.html), [`fi_provider`(7)](https://ofiwg.github.io/libfabric/v1.6.1/man/fi_provider.7.html),

------

© 2022 OpenFabrics Interfaces Working Group with help from [Jekyll Bootstrap](http://jekyllbootstrap.com/) and [Twitter Bootstrap](http://twitter.github.com/bootstrap/)