# Hardware Requirements


The purpose of this section is to describe processor, storage, and
network requirements to deploy a DAOS system.

## Deployment Options


A DAOS storage system is deployed as a **Pooled Storage Model**.
The DAOS servers can run on dedicated storage nodes in separate racks.
This is a traditional pool model where storage is uniformly accessed by
all compute nodes. In order to minimize the number of I/O racks and to
optimize floor space, this approach usually requires high-density storage
servers.


## Processor Requirements


DAOS requires a 64-bit processor architecture and is primarily developed
on Intel x86\_64 architecture. The DAOS software and the libraries it
depends on (e.g., [ISA-L](https://github.com/intel/isa-l),
[SPDK](https://pmem.io/pmdk/), [PMDK](https://spdk.io/), and
[DPDK](https://www.dpdk.org/) can take
advantage of Intel Intel Streaming SIMD (SSE) and Intel Advanced Vector (AVX) extensions.

Some success was also reported by the community on running the DAOS client
on 64-bit ARM processors configured in Little Endian mode. That being said,
ARM testing is not part of the current DAOS CI pipeline and is thus not
validated on a regular basis.

## Network Requirements

An RDMA-capable fabric is preferred for best performance.
The DAOS data plane relies on [OFI libfabric](https://ofiwg.github.io/libfabric/)
and supports OFI providers for Ethernet/tcp and InfiniBand/verbs.
Starting with a Technology Preview in DAOS 2.2, [UCX](https://www.openucx.org/)
is also supported as an alternative network stack for DAOS.
Refer to [UCX Fabric Support (DAOS 2.2 Technology Preview)](./ucx.md)
for details on setting up DAOS with UCX support.

DAOS supports multiple network interfaces on the servers
by binding different instances of the DAOS engine to individual
network cards.
DAOS can support multiple network interfaces on the clients,
by assigning different client processes on the node to different
network interfaces. Note that DAOS does *not* support network-level
striping over multiple network interfaces, so a *single* client process
will always use a single network link.

The DAOS control plane provides methods for administering and managing
the DAOS servers using a secure socket layer interface. Management
traffic between clients and servers uses IP over Fabric. On large
clusters, however, administration of DAOS servers typically uses an
additional out-of-band network connecting the nodes in the DAOS service
cluster.

网络要求¶
为了获得最佳性能，首选支持 RDMA 的结构。 DAOS 数据平面依赖于 OFI libfabric，并支持以太网/tcp 和 InfiniBand/verbs 的 OFI 提供程序。 从 DAOS 2.2 中的技术预览开始，UCX 也被支持作为 DAOS 的替代网络堆栈。 有关设置具有 UCX 支持的 DAOS 的详细信息，请参阅 UCX Fabric 支持（DAOS 2.2 技术预览）。

DAOS 通过将 DAOS 引擎的不同实例绑定到各个网卡来支持服务器上的多个网络接口。 通过将节点上的不同客户端进程分配给不同的网络接口，DAOS 可以支持客户端上的多个网络接口。 请注意，DAOS 不支持多个网络接口上的网络级条带化，因此单个客户端进程将始终使用单个网络链接。

DAOS 控制平面提供使用安全套接字层接口来管理和管理 DAOS 服务器的方法。 客户端和服务器之间的管理流量使用 IP over Fabric。 然而，在大型集群上，DAOS 服务器的管理通常使用连接 DAOS 服务集群中的节点的附加带外网络。

## Storage Requirements


DAOS requires each storage node to have direct access to storage-class
memory (SCM). While DAOS is primarily tested and tuned for Intel
Optane^TM^ Persistent Memory, the DAOS software stack is built over the
Persistent Memory Development Kit (PMDK) and the Direct Access (DAX) feature of the
Linux operating systems as described in the
[SNIA NVM Programming Model](https://www.snia.org/sites/default/files/technical\_work/final/NVMProgrammingModel\_v1.2.pdf).
As a result, the open-source DAOS software stack should be
able to run transparently over any storage-class memory supported by the
PMDK.

The storage node can optionally be equipped with [NVMe](https://nvmexpress.org/)
(non-volatile memory express)[^10] SSDs to provide capacity. HDDs,
as well as SATA andSAS SSDs, are not supported by DAOS.
Both NVMe 3D-NAND and Optane SSDs are supported. Optane SSDs are
preferred for DAOS installation that targets a very high IOPS rate.
NVMe-oF devices are also supported by the
userspace storage stack but have never been tested.

A minimum 6% ratio of SCM to SSD capacity will guarantee that DAOS has
enough space in SCM to store its internal metadata (e.g., pool metadata,
SSD block allocation tracking). Lower ratios are possible, but the
amount of SCM required will depend on the usage patterns of the
applications accessing the DAOS storage. Since DAOS uses the SCM for its
metadata, if the ratio is too low, it is possible to have bulk storage
available but insufficient SCM for DAOS metadata.

For testing purposes, SCM can be emulated with DRAM by mounting a tmpfs
filesystem, and NVMe SSDs can be also emulated with DRAM or a loopback
file.

存储要求¶
DAOS 要求每个存储节点能够直接访问存储级内存 (SCM)。 虽然 DAOS 主要针对 Intel Optane^TM^ 持久内存进行测试和调整，但 DAOS 软件堆栈是基于持久内存开发套件 (PMDK) 和 Linux 操作系统的直接访问 (DAX) 功能构建的，如 SNIA NVM 中所述 编程模型。 因此，开源 DAOS 软件堆栈应该能够在 PMDK 支持的任何存储级内存上透明地运行。

存储节点可以选择配备 NVMe（非易失性存储器 Express）[^10] SSD 以提供容量。 DAOS 不支持 HDD、SATA 和 SAS SSD。 支持 NVMe 3D-NAND 和 Optane SSD。 Optane SSD 是以极高 IOPS 速率为目标的 DAOS 安装的首选。 用户空间存储堆栈也支持 NVMe-oF 设备，但从未经过测试。

SCM 与 SSD 容量的比例至少为 6%，将保证 DAOS 在 SCM 中有足够的空间来存储其内部元数据（例如，池元数据、SSD 块分配跟踪）。 较低的比率是可能的，但所需的 SCM 数量将取决于访问 DAOS 存储的应用程序的使用模式。 由于 DAOS 将 SCM 用于其元数据，因此如果该比率太低，则可能有可用的大容量存储，但 SCM 不足以用于 DAOS 元数据。

出于测试目的，可以通过安装 tmpfs 文件系统来使用 DRAM 模拟 SCM，也可以使用 DRAM 或环回文件来模拟 NVMe SSD

## Storage Server Design


The hardware design of a DAOS storage server balances the network
bandwidth of the fabric with the aggregate storage bandwidth of the NVMe
storage devices. This relationship sets the number of NVMe drives
depending on the read/write balance of the application workload. Since
NVMe SSDs have read faster than they write, a 200Gbps PCIe4 x4 NIC can
be balanced for read only workloads by 4 NVMe4 x4 SSDs, but for write
workloads by 8 NVMe4 x4 SSDs. The capacity of the SSDs will determine
the minimum capacity of the Optane PMem DIMMs needed to provide the 6%
ratio for DAOS metadata.

![](media/image2.png)

## CPU Affinity


Recent Intel Xeon data center platforms use two processor CPUs connected
together with the Ultra Path Interconnect (UPI). PCIe lanes in these
servers have a natural affinity to one CPU. Although globally accessible
from any of the system cores, NVMe SSDs and network interface cards
connected through the PCIe bus may provide different performance
characteristics (e.g., higher latency, lower bandwidth) to each CPU.
Accessing non-local PCIe devices may involve traffic over the UPI link
that might become a point of congestion. Similarly, persistent memory is
non-uniformly accessible (NUMA), and CPU affinity must be respected for
maximal performance.

Therefore, when running in a multi-socket and multi-rail environment,
the DAOS service must be able to detect the CPU to PCIe device and
persistent memory affinity and minimize, as much as possible, non-local
access. This can be achieved by spawning one instance of the I/O Engine
per CPU, then accessing only the persistent memory and PCI devices local
to that CPU from that server instance. The DAOS control plane is
responsible for detecting the storage and network affinity and starting
the I/O Engines accordingly.


## Fault Domains

DAOS relies on single-ported storage massively distributed across
different storage nodes. Each storage node is thus a single point of
failure. DAOS achieves fault tolerance by providing data redundancy
across storage nodes in different fault domains.

DAOS assumes that fault domains are hierarchical and do not overlap. For
instance, the first level of a fault domain could be the racks and the
second one, the storage nodes.

For efficient placement and optimal data resilience, more fault domains
are better. As a result, it is preferable to distribute storage nodes
across as many racks as possible.

存储服务器设计¶
DAOS 存储服务器的硬件设计平衡结构的网络带宽与 NVMe 存储设备的聚合存储带宽。 此关系根据应用程序工作负载的读/写平衡设置 NVMe 驱动器的数量。 由于 NVMe SSD 的读取速度快于写入速度，因此 200Gbps PCIe4 x4 NIC 可以通过 4 个 NVMe4 x4 SSD 平衡只读工作负载，但通过 8 个 NVMe4 x4 SSD 平衡写入工作负载。 SSD 的容量将决定为 DAOS 元数据提供 6% 比率所需的 Optane PMem DIMM 的最小容量。



CPU亲和力¶
最近的英特尔至强数据中心平台使用两个通过超级路径互连 (UPI) 连接在一起的处理器 CPU。 这些服务器中的 PCIe 通道与一个 CPU 具有天然的亲和力。 尽管可以从任何系统核心进行全局访问，但通过 PCIe 总线连接的 NVMe SSD 和网络接口卡可能会为每个 CPU 提供不同的性能特征（例如，更高的延迟、更低的带宽）。 访问非本地 PCIe 设备可能涉及 UPI 链路上的流量，这可能会成为拥塞点。 同样，持久内存是非一致可访问的 (NUMA)，并且必须尊重 CPU 关联性才能获得最大性能。

因此，当在多插槽和多轨环境中运行时，DAOS 服务必须能够检测 CPU 与 PCIe 设备和持久内存的关联性，并尽可能减少非本地访问。 这可以通过为每个 CPU 生成一个 I/O 引擎实例，然后从该服务器实例仅访问该 CPU 本地的持久内存和 PCI 设备来实现。 DAOS 控制平面负责检测存储和网络亲和性并相应地启动 I/O 引擎。

故障域¶
DAOS 依赖于大规模分布在不同存储节点上的单端口存储。 因此，每个存储节点都是单点故障。 DAOS 通过在不同故障域的存储节点之间提供数据冗余来实现容错。

DAOS 假定故障域是分层的且不重叠。 例如，故障域的第一级可能是机架，第二级可能是存储节点。

为了实现高效放置和最佳数据恢复能力，故障域越多越好。 因此，最好将存储节点分布在尽可能多的机架上