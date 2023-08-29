# Blob I/O

The Blob I/O (BIO) module was implemented for issuing I/O over NVMe SSDs. The BIO module covers NVMe SSD support, faulty device detection, device health monitoring, NVMe SSD hot plug functionality, and also SSD identification with the use of Intel VMD devices.

Blob I/O (BIO) 模块的实现是为了通过 NVMe SSD 发出 I/O。 BIO 模块涵盖 NVMe SSD 支持、故障设备检测、设备运行状况监控、NVMe SSD 热插拔功能以及使用 Intel VMD 设备的 SSD 识别

This document contains the following sections:

- <a href="#1">NVMe SSD Support</a>
    - <a href="#2">Storage Performance Development Kit (SPDK)</a>
    - <a href="#3">Per-Server Metadata Management (SMD)</a>
    - <a href="#4">DMA Buffer Management</a>
- <a href="#5">NVMe Threading Model</a>
- <a href="#6">Device Health Monitoring</a>
- <a href="#7">Faulty Device Detection (SSD Eviction)</a>
- <a href="#8">NVMe SSD Hot Plug</a>
- <a href="#9">SSD Identification</a>
    - <a href="#10">Intel Volume Management Device (VMD)
- <a href="#11">Device States</a>
- <a href="#12">User Interfaces</a>

<a id="1"></a>
## NVMe SSD Support
The DAOS service has two tiers of storage: Storage Class Memory (SCM) for byte-granular application data and metadata, and NVMe for bulk application data. Similar to how PMDK is currently used to facilitate access to SCM, the Storage Performance Development Kit (SPDK) is used to provide seamless and efficient access to NVMe SSDs. DAOS storage allocations can occur on either SCM by using a PMDK pmemobj pool, or on NVMe, using an SPDK blob. All local server metadata will be stored in a per-server pmemobj pool on SCM and will include all current and relevant NVMe device, pool, and xstream mapping information. Background aggregation allows for data migration from SCM to an NVMe SSD by coalescing smaller data records into a larger one. The DAOS control plane handles all SSD configuration, and the DAOS data plane handles all allocations through SPDK, with finer block allocations using the in-house Versioned Extent Allocator (VEA).

<a id="2"></a>
### Storage Performance Development Kit (SPDK)
SPDK is an open source C library that when used in a storage application, can provide a significant performance increase of more than 7X over the standard NVMe kernel driver. SPDK's high performance can mainly be attributed to the user space NVMe driver, eliminating all syscalls and enabling zero-copy access from the application. In SPDK, the hardware is polled for completions as opposed to relying on interrupts, lowering both total latency and latency variance. SPDK also offers a block device layer called bdev which sits immediately above the device drivers like in a traditional kernel storage stack. This module offers pluggable module APIs for implementing block devices that interface with different types of block storage devices. This includes driver modules for NVMe, Malloc (ramdisk), Linux AIO, Ceph RBD, and others.

存储性能开发套件 (SPDK)
SPDK 是一个开源 C 库，在存储应用程序中使用时，与标准 NVMe 内核驱动程序相比，可以提供超过 7 倍的显着性能提升。 SPDK的高性能主要归功于用户空间NVMe驱动程序，消除了所有系统调用并实现了应用程序的零复制访问。 在 SPDK 中，轮询硬件是否完成，而不是依赖中断，从而降低了总延迟和延迟方差。 SPDK 还提供了一个名为 bdev 的块设备层，它位于设备驱动程序的正上方，就像在传统的内核存储堆栈中一样。 该模块提供可插入模块 API，用于实现与不同类型的块存储设备接口的块设备。 其中包括 NVMe、Malloc（ramdisk）、Linux AIO、Ceph RBD 等的驱动程序模块

![/docs/graph/Fig_065.png](/docs/graph/Fig_065.png "SPDK Software Stack")

#### SPDK NVMe Driver
The NVMe driver is a C library linked to a storage application providing direct, zero-copy data transfer to and from NVMe SSDs. Other benefits of the SPDK NVMe driver are that it is entirely in user space, operates in polled-mode vs. interrupt-dependent, is asynchronous and lock-less.
#### SPDK Block Device Layer (bdev)
The bdev directory contains a block device abstraction layer used to translate from a common block protocol to specific protocols of backend devices, such as NVMe. Additionally, this layer provides automatic queuing of I/O requests in response to certain conditions, lock-less sending of queues, device configuration and reset support, and I/O timeout trafficking.
#### SPDK Blobstore
The blobstore is a block allocator for a higher-level storage service. The allocated blocks are termed 'blobs' within SPDK. Blobs are designed to be large (at least hundreds of KB), and therefore another allocator is needed in addition to the blobstore to provide efficient small block allocation for the DAOS service. The blobstore provides asynchronous, un-cached, and parallel blob read and write interfaces

### SPDK Integration
The BIO module relies on the SPDK API to initialize/finalize the SPDK environment on the DAOS server start/shutdown. The DAOS storage model is integrated with SPDK by the following:

* Management of SPDK blobstores and blobs:
NVMe SSDs are assigned to each DAOS server xstream. SPDK blobstores are created on each NVMe SSD. SPDK blobs are created and attached to each per-xstream VOS pool.
* Association of SPDK I/O channels with DAOS server xstreams:
Once SPDK I/O channels are properly associated to the corresponding device, NVMe hardware completion pollers are integrated into server polling ULTs.

<a id="3"></a>
## Per-Server Metadata Management (SMD)
One of the major subcomponenets of the BIO module is per-server metadata management. The SMD submodule consists of a PMDK pmemobj pool stored on SCM used to track each DAOS server's local metadata.

Currently, the persistent metadata tables tracked are :
  - **NVMe Device Table**: NVMe SSD to DAOS server xstream mapping (local PCIe attached NVMe SSDs are assigned to different server xstreams to avoid hardware contention). A persistent device state is also stored (supported device states are: NORMAL and FAULTY).
  - **NVMe Pool Table**: NVMe SSD, DAOS server xstream, and SPDK blob ID mapping (SPDK blob to VOS pool:xstream mapping). Blob size is also stored along with the SPDK blob ID in order to support creating new blobs on a new device in the case of NVMe device hotplug.

On DAOS server start, these tables are loaded from persistent memory and used to initialize new, and load any previous blobstores and blobs. Also, there is potential to expand this module to support other non-NVMe related metadata in the future.

Useful admin commands to query per-server metadata:
   <a href="#80">dmg storage query (list-devices | list-pools)</a> [used to query both SMD device table and pool table]

<a id="4"></a>
## DMA Buffer Management
BIO internally manages a per-xstream DMA safe buffer for SPDK DMA transfer over NVMe SSDs. The buffer is allocated using the SPDK memory allocation API and can dynamically grow on demand. This buffer also acts as an intermediate buffer for RDMA over NVMe SSDs, meaning on DAOS bulk update, client data will be RDMA transferred to this buffer first, then the SPDK blob I/O interface will be called to start local DMA transfer from the buffer directly to NVMe SSD. On DAOS bulk fetch, data present on the NVMe SSD will be DMA transferred to this buffer first, and then RDMA transferred to the client.

<a id="5"></a>
## NVMe Threading Model
  - Device Owner Xstream: In the case there is no direct 1:1 mapping of VOS XStream to NVMe SSD, the VOS xstream that first opens the SPDK blobstore will be named the 'Device Owner'. The Device Owner Xstream is responsible for maintaining and updating the blobstore health data, handling device state transitions, and also media error events. All non-owner xstreams will forward events to the device owner.
  - Init Xstream: The first started VOS xstream is termed the 'Init Xstream'. The init xstream is responsible for initializing and finalizing the SPDK bdev, registering the SPDK hotplug poller, handling and periodically checking for new NVMe SSD hot remove and hotplug events, and handling all VMD LED device events.

![/docs/graph/NVME_Threading_Model_Final](/docs/graph/NVME_Threading_Model_Final.PNG "NVMe Threading Model")

Above is a diagram of the current NVMe threading model. The 'Device Owner' xstream is responsible for all faulty device and device reintegration callbacks, as well as updating device health data. The 'Init' xstream is responsible for registering the SPDK hotplug poller and maintaining the current device list of SPDK bdevs as well as evicted and unplugged devices. Any device metadata operations or media error events that do not occur on either of these two xstreams will be forwarded to the appropriate xstream using the SPDK event framework for lockless inter-thread communication. All xstreams will periodically poll for I/O statistics (if enabled in server config), but only the device owner xstream will poll for device events, making necessary state transitions, and update device health stats, and the init xstream will poll for any device removal or device hot plug events.

<a id="6"></a>
## Device Health Monitoring
The device owner xstream is responsible for maintaining anf updating all device health data and all media error events as apart of the device health monitoring feature. Device health data consists of raw SSD health stats queried via SPDK admin APIs and in-memory health data. The raw SSD health stats returned include useful and critical data to determine the current health of the device, such as temperature, power on duration, unsafe shutdowns, critical warnings, etc. The in-memory health data contains a subset of the raw SSD health stats, in addition to I/O error (read/write/unmap) and checksum error counters that are updated when a media error event occurs on a device and stored in-memory.

The DAOS data plane will monitor NVMe SSDs every 60 seconds, including updating the health stats with current values, checking current device states, and making any necessary blobstore/device state transitions. Once a FAULTY state transition has occurred, the monitoring period will be reduced to 10 seconds to allow for quicker transitions and finer-grained monitoring until the device is fully evicted.

 Useful admin command to query device health:
  - <a href="#81">dmg storage query device-health</a> [used to query SSD health stats]

While monitoring this health data, an admin can now make the determination to manually evict a faulty device. This data will also be used to set the faulty device criteria for automatic SSD eviction (available in a future release).

<a id="7"></a>
## Faulty Device Detection (SSD Eviction)
Faulty device detection and reaction can be referred to as NVMe SSD eviction. This involves all affected pool targets being marked as down and the rebuild of all affected pool targets being automatically triggered. A persistent device state is maintained in SMD and the device state is updated from NORMAL to FAULTY upon SSD eviction. The faulty device reaction will involve various SPDK cleanup, including all I/O channels released, SPDK allocations (termed 'blobs') closed, and the SPDK blobstore created on the NVMe SSD unloaded. Currently only manual SSD eviction is supported, and a future release will support automatic SSD eviction.

 Useful admin commands to manually evict an NVMe SSD:
  - <a href="#82">dmg storage set nvme-faulty</a> [used to manually set an NVMe SSD to FAULTY (ie evict the device)]

<a id="8"></a>
## NVMe SSD Hot Plug

**Full NVMe hot plug capability will be available and supported in DAOS 2.0 release. Use is currently intended for testing only and is not supported for production.**

The NVMe hot plug feature includes device removal (an NVMe hot remove event) and device reintegration (an NVMe hotplug event) when a faulty device is replaced with a new device.

For device removal, if the device is a faulty or previously evicted device, then nothing further would be done when the device is removed. The device state would be displayed as UNPLUGGED. If a healthy device that is currently in use by DAOS is removed, then all SPDK memory stubs would be deconstructed, and the device state would also display as UNPLUGGED.

For device reintegration, if a new device is plugged to replace a faulty device, the admin would need to issue a device replacement command. All SPDK in-memory stubs would be created and all affected pool targets automatically reintegrated on the new device. The device state would be displayed as NEW initially and NORMAL after the replacement event occurred. If a faulty device or previously evicted device is re-plugged, the device will remain evicted, and the device state would display EVICTED. If a faulty device is desired to be reused (NOTE: this is not advised, mainly used for testing purposes), the admin can run the same device replacement command setting the new and old device IDs to be the same device ID. Reintegration will not occur on the device, as DAOS does not currently support incremental reintegration.

NVMe hot plug with Intel VMD devices is currently not supported in this release, but will be supported in a future release.

 Useful admin commands to replace an evicted device:
  - <a href="#83">dmg storage replace nvme</a> [used to replace an evicted device with a new device]
  - <a href="#84">dmg storage replace nvme</a> [used to bring an evicted device back online (without reintegration)]

<a id="9"></a>
## SSD Identification
The SSD identification feature is a way to quickly and visually locate a device. It requires the use of Intel VMD, which needs to be physically available on the hardware as well as enabled in the system BIOS. The feature supports two LED events: locating a healthy device and locating an evicted device.

<a id="10"></a>
### Intel Volume Management Device (VMD)
Intel VMD is a technology embedded in the processor silicon that aggregates the NVMe PCIe SSDs attached to its root port, acting as an HBA does for SATA and SAS. Currently, PCIe storage lacks a standardized method to blink LEDs and indicated the status of a device. Intel VMD, along with NVMe, provides this support for LED management.

![/docs/graph/Intel_VMD.png](/docs/graph/Intel_VMD.png "Intel VMD Technology")
Intel VMD places a control point in the PCIe root complex of the servers, meaning that NVMe drives can be hot-swapped, and the status LED is always reliable.

![/docs/graph/VMD_Amber_LED.png](/docs/graph/VMD_Amber_LED.png "Status Amber VMD LED")
The Amber LED (status LED) is what VMD provides. It represents the LED coming from the slot on the backplane. The Green LED is the activity LED.

The status LED on the VMD device has four states: OFF, FAULT, REBUILD, and IDENTIFY. These are communicated by blinking patterns specified in the IBPI standard (SFF-8489).
![/docs/graph/VMD_LED_states.png](/docs/graph/VMD_LED_states.png "Status LED states")

#### Locate a Healthy Device
Upon issuing a device identify command with a specified device ID and optional custom timeout value, an admin now can quickly identify a device in question.
The timeout value will be 2 minutes if unspecified on the commandline, any value specified should be in units of a minute.
The status LED on the VMD device would be set to an IDENTIFY state, represented by a quick, 4Hz blinking amber light.
The device will quickly blink until the timeout value is reached, after which returning to the default OFF state.

#### Locate an Evicted Device
If an NVMe SSD is faulty, the status LED on the VMD device will be set to a EVICTED state, represented by a solidly ON amber light.
This LED activity visually indicates a fault and that the device needs to be replaced and is no longer in use by DAOS.
The LED of the VMD device will remain in this state until replaced by a new device.

 Useful admin command to locate a VMD-enabled NVMe SSD:
  - <a href="#85">dmg storage identify vmd</a> [used to change the status LED state on the VMD device to quickly blink until timeout expires]

<a id="11"></a>
## Device States
The device states that are returned from a device query by the admin are dependent on both the persistently stored device state in SMD, and the in-memory BIO device list.

  - NORMAL: A fully functional device in use by DAOS (or in setup).
  - EVICTED: A device has been manually evicted and is no longer in use by DAOS.
  - UNPLUGGED: A device previously used by DAOS is unplugged.
  - NEW: A new device is available for use by DAOS.

![/docs/graph/dmg_device_states.png](/docs/graph/dmg_device_states.png "Device States")

 Useful admin command to query device states:
   - <a href="#31">dmg storage query list-devices</a> [used to query NVMe SSD device states]

<a id="12"></a>
## User Interfaces:
<a id="80"></a>
- Query Per-Server Metadata (SMD): **$dmg storage query (list-devices | list-pools)**

To list all devices:

```
$ dmg storage query list-devices
Devices
        UUID:9fb3ce57-1841-43e6-8b70-2a5e7fb2a1d0 [TrAddr:0000:8d:00.0]
            Targets:[0] Rank:0 State:NORMAL
        UUID:a0e34f6b-06f7-4cb8-aec6-9162e45b8648 [TrAddr:0000:8a:00.0]
            Targets:[1] Rank:0 State:NORMAL
        UUID:0c87e88d-44bf-4b9f-a69d-77b2a26ed4c4 [TrAddr:0000:8b:00.0]
            Targets:[2] Rank:0 State:NORMAL
        UUID:f1623ce1-b383-4927-929f-449fccfbb340 [TrAddr:0000:8c:00.0]
            Targets:[] Rank:0 State:NEW
```
To list all pools:
```
$ dmg storage query list-pools --verbose
Pools
        UUID:8131fc39-4b1c-4662-bea1-734e728c434e
            Rank:0 Targets:[0 2 1] Blobs:[4294967296 4294967296 4294967296]
        UUID:8131fc39-4b1c-4662-bea1-734e728c434e
            Rank:1 Targets:[0 1 2] Blobs:[4294967296 4294967296 4294967296]

```

<a id="81"></a>
- Query Device Health Data: **$dmg storage query device-health**

```
$ dmg storage query device-health --uuid=9fb3ce57-1841-43e6-8b70-2a5e7fb2a1d0
Devices:
        UUID:9fb3ce57-1841-43e6-8b70-2a5e7fb2a1d0 [TrAddr:0000:8d:00.0]
           Targets:[0] Rank:0 State:NORMAL
           Health Stats:
               Timestamp:Tue Jul 28 20:08:57 UTC 19029
               Temperature:314K(40.85C)
               Controller Busy Time:37m0s
               Power Cycles:96
               Power On Duration:14128h0m0s
               Unsafe Shutdowns:89
               Media errors: 0
               Read errors: 0
               Write errors: 0
               Unmap errors: 0
               Checksum errors: 0
               Error log entries: 0
              Critical Warnings:
               Temperature: OK
               Available Spare: OK
               Device Reliability: OK
               Read Only: OK
               Volatile Memory Backup: OK
```
<a id="82"></a>
- Manually Set Device State to FAULTY: **$dmg storage set nvme-faulty**
```
$ dmg storage set nvme-faulty --uuid=9fb3ce57-1841-43e6-8b70-2a5e7fb2a1d0
Devices
        UUID:9fb3ce57-1841-43e6-8b70-2a5e7fb2a1d0 [TrAddr:0000:8d:00.0]
            Targets:[0] Rank:0 State:EVICTED

```

<a id="83"></a>
- Replace an evicted device with a new device: **$dmg storage replace nvme**
```
$ dmg storage replace nvme --old-uuid=9fb3ce57-1841-43e6-8b70-2a5e7fb2a1d0 --new-uuid=8131fc39-4b1c-4662-bea1-734e728c434e
Devices
        UUID:8131fc39-4b1c-4662-bea1-734e728c434e [TrAddr:0000:8d:00.0]
            Targets:[0] Rank:0 State:NORMAL

```

<a id="84"></a>
- Reuse a previously evicted device: **$dmg storage replace nvme**
```
$ dmg storage replace nvme --old-uuid=9fb3ce57-1841-43e6-8b70-2a5e7fb2a1d0 --new-uuid=9fb3ce57-1841-43e6-8b70-2a5e7fb2a1d0
Devices
        UUID:9fb3ce57-1841-43e6-8b70-2a5e7fb2a1d0 [TrAddr:0000:8a:00.0]
            Targets:[0] Rank:0 State:NORMAL

```

<a id="#85"></a>
- Identify a VMD-enabled NVMe SSD: **$dmg storage identify vmd**
```
$ dmg storage identify vmd --uuid=57b3ce9f-1841-43e6-8b70-2a5e7fb2a1d0
Devices
       UUID:57b3ce9f-1841-43e6-8b70-2a5e7fb2a1d0 [TrAddr:5d0505:01:00.0]
           Targets:[1] Rank:1 State:IDENTIFY
```


Blob I/O (BIO) 模块的实现是为了通过 NVMe SSD 发出 I/O。 BIO 模块涵盖 NVMe SSD 支持、故障设备检测、设备运行状况监控、NVMe SSD 热插拔功能以及使用 Intel VMD 设备的 SSD 识别。

本文档包含以下部分：

NVMe SSD 支持
存储性能开发套件 (SPDK)
每服务器元数据管理 (SMD)
DMA 缓冲区管理
NVMe 线程模型
设备健康监测
故障设备检测（SSD 驱逐）
NVMe SSD热插拔
SSD识别
英特尔卷管理设备 (VMD)
设备状态
用户界面

NVMe SSD 支持
DAOS 服务有两层存储：用于字节粒度应用程序数据和元数据的存储类内存 (SCM)，以及用于批量应用程序数据的 NVMe。 与目前使用 PMDK 促进 SCM 访问的方式类似，存储性能开发套件 (SPDK) 用于提供对 NVMe SSD 的无缝、高效访问。 DAOS 存储分配可以使用 PMDK pmemobj 池在 SCM 上进行，也可以使用 SPDK blob 在 NVMe 上进行。 所有本地服务器元数据将存储在 SCM 上的每服务器 pmemobj 池中，并将包括所有当前和相关的 NVMe 设备、池和 xstream 映射信息。 后台聚合通过将较小的数据记录合并为较大的记录，允许将数据从 SCM 迁移到 NVMe SSD。 DAOS 控制平面处理所有 SSD 配置，DAOS 数据平面通过 SPDK 处理所有分配，并使用内部版本化盘区分配器 (VEA) 进行更精细的块分配。


SPDK NVMe驱动程序
NVMe 驱动程序是一个链接到存储应用程序的 C 库，提供与 NVMe SSD 之间的直接零复制数据传输。 SPDK NVMe 驱动程序的其他优点是它完全位于用户空间中，以轮询模式（与中断相关）运行，是异步且无锁的。

SPDK 块设备层 (bdev)
bdev 目录包含块设备抽象层，用于将通用块协议转换为后端设备的特定协议，例如 NVMe。 此外，该层还提供响应某些条件的 I/O 请求自动排队、队列的无锁发送、设备配置和重置支持以及 I/O 超时流量。

SPDK Blobstore
blobstore 是用于更高级别存储服务的块分配器。 分配的块在 SPDK 中称为“blob”。 Blob 设计得很大（至少数百 KB），因此除了 blobstore 之外还需要另一个分配器来为 DAOS 服务提供高效的小块分配。 blobstore 提供异步、非缓存和并行 blob 读写接口

SPDK集成
BIO 模块依赖 SPDK API 在 DAOS 服务器启动/关闭时初始化/完成 SPDK 环境。 DAOS存储模型通过以下方式与SPDK集成：

SPDK blobstore 和 blob 的管理：NVMe SSD 分配给每个 DAOS 服务器 xstream。 SPDK blobstore 在每个 NVMe SSD 上创建。 创建 SPDK blob 并将其附加到每个 xstream VOS 池。
SPDK I/O 通道与 DAOS 服务器 xstream 的关联：一旦 SPDK I/O 通道正确关联到相应的设备，NVMe 硬件完成轮询器就会集成到服务器轮询 ULT 中。

每服务器元数据管理 (SMD)
BIO 模块的主要子组件之一是每服务器元数据管理。 SMD 子模块由存储在 SCM 上的 PMDK pmemobj 池组成，用于跟踪每个 DAOS 服务器的本地元数据。

目前，跟踪的持久元数据表是：

NVMe 设备表：NVMe SSD 到 DAOS 服务器 xstream 映射（本地 PCIe 连接的 NVMe SSD 分配给不同的服务器 xstream，以避免硬件争用）。 还存储持久设备状态（支持的设备状态有：正常和故障）。
NVMe 池表：NVMe SSD、DAOS 服务器 xstream 和 SPDK blob ID 映射（SPDK blob 到 VOS 池：xstream 映射）。 Blob 大小还与 SPDK Blob ID 一起存储，以便支持在 NVMe 设备热插拔的情况下在新设备上创建新 Blob。
在 DAOS 服务器启动时，这些表将从持久内存中加载并用于初始化新表，并加载任何以前的 blobstore 和 blob。 此外，将来还有可能扩展该模块以支持其他非 NVMe 相关元数据。

用于查询每个服务器元数据的有用管理命令：dmg storage query (list-devices | list-pools) [用于查询 SMD 设备表和池表]


DMA 缓冲区管理
BIO 在内部管理每个 xstream DMA 安全缓冲区，用于通过 NVMe SSD 进行 SPDK DMA 传输。 该缓冲区使用 SPDK 内存分配 API 进行分配，并且可以根据需要动态增长。 该缓冲区还充当 RDMA over NVMe SSD 的中间缓冲区，这意味着在 DAOS 批量更新时，客户端数据将首先通过 RDMA 传输到该缓冲区，然后调用 SPDK blob I/O 接口从缓冲区启动本地 DMA 传输 直接连接到 NVMe SSD。 在 DAOS 批量获取时，NVMe SSD 上存在的数据将首先通过 DMA 传输到此缓冲区，然后通过 RDMA 传输到客户端。


NVMe 线程模型
设备所有者 Xstream：如果 VOS XStream 到 NVMe SSD 没有直接 1:1 映射，则首先打开 SPDK blobstore 的 VOS xstream 将被命名为“设备所有者”。 设备所有者 Xstream 负责维护和更新 blobstore 运行状况数据、处理设备状态转换以及媒体错误事件。 所有非所有者 xstream 都会将事件转发给设备所有者。
Init Xstream：第一个启动的 VOS xstream 称为“Init Xstream”。 init xstream 负责初始化和完成 SPDK bdev、注册 SPDK 热插拔轮询器、处理和定期检查新的 NVMe SSD 热移除和热插拔事件，以及处理所有 VMD LED 设备事件。
/docs/graph/NVME_Threading_Model_Final

上图是当前 NVMe 线程模型的图。 “设备所有者”xstream 负责所有故障设备和设备重新集成回调，以及更新设备运行状况数据。 “Init”xstream 负责注册 SPDK 热插拔轮询器并维护 SPDK bdev 的当前设备列表以及已逐出和拔出的设备。 任何未在这两个 xstream 上发生的设备元数据操作或媒体错误事件都将使用 SPDK 事件框架转发到适当的 xstream，以进行无锁线程间通信。 所有 xstream 将定期轮询 I/O 统计信息（如果启用d 在服务器配置中），但只有设备所有者 xstream 才会轮询设备事件，进行必要的状态转换并更新设备运行状况统计信息，并且 init xstream 将轮询任何设备删除或设备热插拔事件。


设备健康监测
设备所有者 xstream 负责维护和更新所有设备运行状况数据和所有媒体错误事件，作为设备运行状况监控功能的一部分。 设备运行状况数据包括通过 SPDK 管理 API 查询的原始 SSD 运行状况统计数据和内存运行状况数据。 返回的原始 SSD 运行状况统计数据包括确定设备当前运行状况的有用和关键数据，例如温度、开机持续时间、不安全关机、严重警告等。内存运行状况数据包含原始 SSD 运行状况的子集 除了 I/O 错误（读/写/取消映射）和校验和错误计数器之外，这些统计信息还会在设备上发生媒体错误事件时更新并存储在内存中。

DAOS 数据平面将每 60 秒监控 NVMe SSD，包括使用当前值更新运行状况统计信息、检查当前设备状态以及进行任何必要的 blobstore/设备状态转换。 一旦发生 FAULTY 状态转换，监控时间将缩短至 10 秒，以实现更快的转换和更细粒度的监控，直到设备被完全驱逐。

用于查询设备运行状况的有用管理命令：

dmg storage query device-health [用于查询SSD健康统计数据]
在监控此健康数据时，管理员现在可以决定手动驱逐有故障的设备。 此数据还将用于设置自动 SSD 驱逐的故障设备标准（在未来版本中提供）。


故障设备检测（SSD 驱逐）
故障设备检测和反应可称为 NVMe SSD 驱逐。 这涉及将所有受影响的池目标标记为已关闭，并自动触发所有受影响的池目标的重建。 在 SMD 中维护持久设备状态，并在 SSD 驱逐时将设备状态从 NORMAL 更新为 FAULTY。 故障设备反应将涉及各种 SPDK 清理，包括释放所有 I/O 通道、关闭 SPDK 分配（称为“blob”）以及卸载在 NVMe SSD 上创建的 SPDK blobstore。 目前仅支持手动 SSD 驱逐，未来版本将支持自动 SSD 驱逐。

用于手动驱逐 NVMe SSD 的有用管理命令：

dmg storage set nvme-faulty [用于手动将 NVMe SSD 设置为 FAULTY（即逐出设备）]

NVMe SSD热插拔
DAOS 2.0 版本将提供并支持完整的 NVMe 热插拔功能。 目前仅用于测试，不支持生产。

NVMe 热插拔功能包括用新设备更换故障设备时的设备移除（NVMe 热移除事件）和设备重新集成（NVMe 热插拔事件）。

对于设备移除，如果设备是有故障的或先前被逐出的设备，则移除设备时不会执行任何进一步操作。 设备状态将显示为“未插入”。 如果 DAOS 当前正在使用的健康设备被删除，则所有 SPDK 内存存根都将被解构，并且设备状态也将显示为 UNPLUGGED。

对于设备重新集成，如果插入新设备来替换故障设备，管理员将需要发出设备替换命令。 所有 SPDK 内存存根都将被创建，所有受影响的池目标将自动重新集成到新设备上。 设备状态最初会显示为 NEW，发生更换事件后会显示为 NORMAL。 如果重新插入有故障的设备或之前被驱逐的设备，该设备将保持被驱逐的状态，并且设备状态将显示 EVICTED。 如果需要重复使用故障设备（注意：不建议这样做，主要用于测试目的），管理员可以运行相同的设备替换命令，将新旧设备 ID 设置为相同的设备 ID。 设备上不会发生重新集成，因为 DAOS 目前不支持增量重新集成。

此版本当前不支持使用 Intel VMD 设备进行 NVMe 热插拔，但未来版本将支持。

用于替换被驱逐设备的有用管理命令：

dmg 存储替换 nvme [用于用新设备替换已驱逐的设备]
dmg 存储替换 nvme [用于使被逐出的设备重新上线（无需重新集成）]

SSD识别
SSD识别功能是一种快速直观地定位设备的方法。 它需要使用 Intel VMD，该功能需要在硬件上物理可用并在系统 BIOS 中启用。 该功能支持两种 LED 事件：定位正常设备和定位已逐出设备。


英特尔卷管理设备 (VMD)
英特尔 VMD 是一项嵌入处理器芯片中的技术，可聚合连接到其根端口的 NVMe PCIe SSD，就像 HBA 用于 SATA 和 SAS 一样。 目前，PCIe 存储缺乏一种标准化方法来闪烁 LED 并指示设备的状态。 Intel VMD 与 NVMe 一起为 LED 管理提供这种支持。

/docs/graph/Intel_VMD.png Intel VMD 在服务器的 PCIe 根联合体中放置了一个控制点，这意味着 NVMe 驱动器可以热插拔，并且状态 LED 始终可靠。

/docs/graph/VMD_Amber_LED.png VMD 提供了琥珀色 LED（状态 LED）。 它代表来自背板上插槽的 LED。 绿色 LED 是活动 LED。

VMD 设备上的状态 LED 有四种状态：OFF、FAULT、REBUILD 和 IDENTIFY。 这些通过 IBPI 标准 (SFF-8489) 中指定的闪烁模式进行传达。 /docs/graph/VMD_LED_states.png

找到一个健康的设备
在发出具有指定设备 ID 和可选自定义超时值的设备识别命令后，管理员现在可以快速识别有问题的设备。 如果在命令行上未指定，超时值为 2 分钟，指定的任何值都应以分钟为单位。 VMD 设备上的状态 LED 将设置为“识别”状态，以 4Hz 快速闪烁的琥珀色灯表示。 设备将快速闪烁，直到达到超时值，然后返回默认关闭状态。

找到被逐出的设备
如果 NVMe SSD 出现故障，VMD 设备上的状态 LED 将设置为 EVICTED 状态，由常亮的琥珀色灯表示。 此 LED 活动直观地表明存在故障，并且该设备需要更换并且 DAOS 不再使用。 VMD 设备的 LED 将保持此状态，直到被新设备更换为止。

用于查找启用 VMD 的 NVMe SSD 的有用管理命令：

dmg storage recognize vmd [用于更改 VMD 设备上的状态 LED 状态以快速闪烁直至超时]

设备状态
管理员从设备查询返回的设备状态取决于 SMD 中持久存储的设备状态和内存中的 BIO 设备列表。

NORMAL：DAOS 使用（或设置中）的功能齐全的设备。
已驱逐：设备已被手动驱逐，并且 DAOS 不再使用该设备。
UNPLUGGED：DAOS 先前使用的设备已拔出。
新功能：DAOS 可以使用新设备。
/docs/graph/dmg_device_states.png

用于查询设备状态的有用管理命令：

dmg storage query list-devices [用于查询NVMe SSD设备状态]

用户界面：

查询每服务器元数据 (SMD)：$dmg 存储查询（列表设备 | 列表池）
列出所有设备：

$ dmg 存储查询列表设备
设备
         UUID:9fb3ce57-1841-43e6-8b70-2a5e7fb2a1d0 [TrAddr:0000:8d:00.0]
             目标：[0] 等级：0 状态：正常
         UUID:a0e34f6b-06f7-4cb8-aec6-9162e45b8648 [TrAddr:0000:8a:00.0]
             目标：[1] 等级：0 状态：正常
         UUID:0c87e88d-44bf-4b9f-a69d-77b2a26ed4c4 [TrAddr:0000:8b:00.0]
             目标：[2] 等级：0 状态：正常
         UUID:f1623ce1-b383-4927-929f-449fccfbb340 [TrAddr:0000:8c:00.0]
             目标：[] 等级：0 状态：新
列出所有池：

$ dmg 存储查询列表池 --verbose
泳池
         UUID：8131fc39-4b1c-4662-bea1-734e728c434e
             排名：0 目标：[0 2 1] Blob：[4294967296 4294967296 4294967296]
         UUID：8131fc39-4b1c-4662-bea1-734e728c434e
             排名：1 目标：[0 1 2] Blob：[4294967296 4294967296 4294967296]


查询设备健康数据：$dmg storage query device-health
$ dmg 存储查询设备运行状况 --uuid=9fb3ce57-1841-43e6-8b70-2a5e7fb2a1d0
设备：
         UUID:9fb3ce57-1841-43e6-8b70-2a5e7fb2a1d0 [TrAddr:0000:8d:00.0]
            目标：[0] 等级：0 状态：正常
            健康统计：
                时间戳：7 月 28 日星期二 20:08:57 UTC 19029
                温度：314K(40.85C)
                控制器繁忙时间：37m0s
                电源周期：96
                开机时间：14128h0m0s
                不安全停机：89
                媒体错误：0
                读取错误：0
                写入错误：0
                取消映射错误：0
                校验和错误：0
                错误日志条目：0
               严重警告：
                温度： 还好
                可用备用： 好的
                设备可靠性：良好
                只读：确定
                易失性内存备份：好的

手动将设备状态设置为 FAULTY：$dmg storage set nvme-faulty
$ dmg 存储集 nvme-faulty --uuid=9fb3ce57-1841-43e6-8b70-2a5e7fb2a1d0
设备
         UUID:9fb3ce57-1841-43e6-8b70-2a5e7fb2a1d0 [TrAddr:0000:8d:00.0]
             目标：[0] 等级：0 状态：驱逐


用新设备替换被逐出的设备：$dmg storage Replace nvme
$ dmg 存储替换 nvme --old-uuid=9fb3ce57-1841-43e6-8b70-2a5e7fb2a1d0 --new-uuid=8131fc39-4b1c-4662-bea1-734e728c434e
设备
         UUID:8131fc39-4b1c-4662-bea1-734e728c434e [TrAddr:0000:8d:00.0]
             目标：[0] 等级：0 状态：正常


重用以前驱逐的设备：$dmg storage replacement nvme
$ dmg 存储替换 nvme --old-uuid=9fb3ce57-1841-43e6-8b70-2a5e7fb2a1d0 --new-uuid=9fb3ce57-1841-43e6-8b70-2a5e7fb2a1d0
设备
         UUID:9fb3ce57-1841-43e6-8b70-2a5e7fb2a1d0 [TrAddr:0000:8a:00.0]
             目标：[0] 等级：0 状态：正常


识别启用 VMD 的 NVMe SSD：$dmg storage recognize vmd
$ dmg 存储识别 vmd --uuid=57b3ce9f-1841-43e6-8b70-2a5e7fb2a1d0
设备
        UUID:57b3ce9f-1841-43e6-8b70-2a5e7fb2a1d0 [TrAddr:5d0505:01:00.0]
            目标：[1] 等级：1 状态：识别


