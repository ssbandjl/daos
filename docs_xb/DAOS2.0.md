# DAOS 2.0

https://docs.daos.io/v2.0/
https://github.com/daos-stack/daos/

# 欢迎来到 DAOS 2.0 版！[¶](https://docs.daos.io/v2.0/#welcome-to-daos-version-20)

分布式异步对象存储 (DAOS) 是一种开源对象存储，专为大规模分布式非易失性内存 (NVM) 而设计。DAOS 利用下一代 NVM 技术，如 Intel © Optane ™ Persistent Memory 和 NVM express (NVMe)，同时在商用硬件之上提供键值存储接口，提供事务性非阻塞 I/O 等功能，具有自我修复、端到端数据完整性、细粒度数据控制和弹性存储的高级数据保护，以优化性能和成本。

除了本地部署，DAOS 还可以部署在云环境中。有关云部署的更多信息，请参阅“云中的[DAOS”](https://docs.daos.io/v2.0/cloud/)部分。

包含的文档版本与 DAOS v2.0 相关联。它们还可能描述当前正在为未来的 DAOS 版本开发的功能。这些功能将被清楚地标明。

[介绍](https://docs.daos.io/v2.0/)

[社区维基](http://wiki.daos.io/)

[社区路线图](http://wiki.daos.io/spaces/DC/pages/4836661105/Roadmap)

[社区邮件列表](https://daos.groups.io/g/daos)

[云中的 DAOS](https://docs.daos.io/v2.0/cloud/)

[问题跟踪器](http://jira.daos.io/)



# 架构[Architecture](https://docs.daos.io/v2.0/overview/architecture/#architecture)

DAOS 是一种开源软件定义的横向扩展对象存储，可为应用程序提供高带宽和高 IOPS 存储容器，并支持结合模拟、数据分析和机器学习的下一代以数据为中心的工作流。

与主要为旋转媒体设计的传统存储堆栈不同，DAOS 从头开始构建以利用新的 NVM 技术，并且非常轻量级，因为它在用户空间中运行端到端 (E2E) 并完全绕过操作系统。DAOS 提供了一种从为基于块和高延迟存储而设计的 I/O 模型向本质上支持细粒度数据访问并释放下一代存储技术性能的模型的转变。

DAOS 是一个高性能的独立容错存储层，不依赖第三方层来管理元数据和数据弹性。

## DAOS 功能[¶](https://docs.daos.io/v2.0/overview/architecture/#daos-features)

DAOS 依靠[开放结构接口](https://openfabrics.org/downloads/ofiwg/Industry_presentations/2015_HotI23/paper.pdf)(OFI) 进行低延迟通信，并将数据存储在存储级内存 (SCM) 和 NVMe 存储上。DAOS 提供了一个原生的键-数组-值存储接口，它提供了一个统一的存储模型，在该模型上可以移植特定领域的数据模型，例如[HDF5](https://docs.daos.io/v2.0/user/hdf5/)、 [MPI-IO](https://docs.daos.io/v2.0/user/mpi-io/)和 Apache [Hadoop](https://docs.daos.io/v2.0/user/spark/)。[POSIX](https://docs.daos.io/v2.0/user/filesystem/) I /O 仿真层通过本机 DAOS API 实现文件和目录也是可用的。

DAOS I/O 操作被记录下来，然后插入到 SCM 中维护的持久索引中。每个 I/O 都使用称为 epoch 的特定时间戳进行标记，并与数据集的特定版本相关联。内部不执行读-修改-写操作。写操作是非破坏性的并且对对齐不敏感。根据读取请求，DAOS 服务遍历持久索引并创建一个复杂的分散-聚集远程直接内存访问 (RDMA) 描述符，以直接在应用程序提供的缓冲区中重建请求版本的数据。

SCM 存储直接映射到 DAOS 服务的地址空间，该服务通过直接加载/存储来管理持久索引。根据 I/O 特性，DAOS 服务可以决定将 I/O 存储在 SCM 或 NVMe 存储中。如图 2-1 所示，对延迟敏感的 I/O，如应用程序元数据和字节粒度数据，通常存储在前者中，而检查点和批量数据将存储在后者中。这种方法允许 DAOS 通过将数据流式传输到 NVMe 存储并在 SCM 中维护内部元数据索引，为批量数据提供原始 NVMe 带宽。持久内存开发工具包 (PMDK) 允许管理对 SCM 的事务访问，而存储性能开发工具包 (SPDK) 支持 NVMe 设备的用户空间 I/O。

![img](https://docs.daos.io/v2.0/admin/media/image1.png) 图 2-1。DAOS 存储

DAOS 旨在提供：

- 任意对齐和大小的高吞吐量和 IOPS
- 具有真正零拷贝 I/O 到 SCM 的细粒度 I/O 操作
- 通过跨存储服务器的可扩展集体通信支持大规模分布式 NVM 存储
- 非阻塞数据和元数据操作，允许 I/O 和计算重叠
- 考虑故障域的高级数据放置
- 通过在线重建支持复制和纠删码的软件管理冗余
- 端到端数据完整性
- 具有保证数据一致性和自动恢复的可扩展分布式事务
- 数据集快照
- 用于管理对存储池的访问控制的安全框架
- 软件定义的存储管理，用于通过 COTS 硬件配置、配置、修改和监控存储池
- 通过 DAOS 数据模型原生支持分层数据格式 (HDF)5、MPI-IO 和 POSIX 命名空间
- 灾难恢复工具
- 与 Lustre 并行文件系统无缝集成
- Mover 代理在 DAOS 池之间迁移数据集，从并行文件系统迁移到 DAOS，反之亦然

## DAOS系统[¶](https://docs.daos.io/v2.0/overview/architecture/#daos-system)

数据中心可能有数十万个计算实例通过可扩展的高性能网络互连，其中所有或称为存储节点的实例子集都可以直接访问 NVM 存储。DAOS 安装涉及多个可以并置或分布的组件。

A DAOS *system* is identified by a system name, and consists of a set of DAOS *storage nodes* connected to the same network. The DAOS storage nodes run one DAOS *server* instance per node, which in turn starts one DAOS *Engine* process per physical socket. Membership of the DAOS servers is recorded into the system map, that assigns a unique integer *rank* to each *Engine* process. Two different DAOS systems comprise two disjoint sets of DAOS servers, and do not coordinate with each other.

The DAOS *server* is a multi-tenant daemon running on a Linux instance (either natively on the physical node or in a VM or container) of each *storage node*. Its *Engine* sub-processes export the locally-attached SCM and NVM storage through the network. It listens to a management port (addressed by an IP address and a TCP port number), plus one or more fabric endpoints (addressed by network URIs). The DAOS server is configured through a YAML file in /etc/daos, including the configuration of its Engine sub-processes. The DAOS server startup can be integrated with different daemon management or orchestration frameworks (for example a systemd script, a Kubernetes service, or even via a parallel launcher like pdsh or srun).

Inside a DAOS Engine, the storage is statically partitioned across multiple *targets* to optimize concurrency. To avoid contention, each target has its private storage, its own pool of service threads, and its dedicated network context that can be directly addressed over the fabric independently of the other targets hosted on the same storage node.

- The SCM modules are configured in *AppDirect interleaved* mode. They are thus presented to the operating system as a single PMem namespace per socket (in `fsdax` mode).

Note

When mounting the PMem devices with the `dax` option, the following warning will be logged in dmesg: `EXT4-fs (pmem0): DAX enabled. Warning: EXPERIMENTAL, use at your own risk` This warning can be safely ignored: It is issued because DAX does not yet support the `reflink` filesystem feature, but DAOS does not use this feature.

- When *N* targets per engine are configured, each target is using *1/N* of the capacity of the `fsdax` SCM capacity of that socket, independently of the other targets.
- Each target is also using a fraction of the NVMe capacity of the NVMe drives that are attached to this socket. For example, in an engine with 4 NVMe disks and 16 targets, each target will manage 1/4 of a single NVMe disk.

A target does not implement any internal data protection mechanism against storage media failure. As a result, a target is a single point of failure and the unit of fault. A dynamic state is associated with each target: Its state can be either "up and running", or "down and not available".

A target is the unit of performance. Hardware components associated with the target, such as the backend storage medium, the CPU core(s), and the network, have limited capability and capacity.

The number of targets exported by a DAOS Engine instance is configurable, and depends on the underlying hardware (in particular, the number of SCM modules and the number of NVMe SSDs that are served by this engine instance). As a best practice, the number of targets of an engine should be an integer multiple of the number of NVMe drives that are served by this engine.

## SDK and Tools[¶](https://docs.daos.io/v2.0/overview/architecture/#sdk-and-tools)

Applications, users, and administrators can interact with a DAOS system through two different client APIs. The management API offers the ability to administrate a DAOS system. It is intended to be integrated with different vendor-specific storage management or open-source orchestration frameworks. The `dmg` CLI tool is built over the DAOS management API. On the other hand, the DAOS library (`libdaos`) implements the DAOS storage model. It is primarily targeted at application and I/O middleware developers who want to store datasets in a DAOS system. User utilities like the `daos` command are also built over the API to allow users to manage datasets from a CLI.

应用程序可以直接通过原生 DAOS API、通过 I/O 中间件库（例如 POSIX 仿真、MPI-IO、HDF5）或通过已经与原生 DAOS 存储集成的 Spark 或 TensorFlow 等框架访问存储在 DAOS 中的数据集模型。

## 代理人[Agent](https://docs.daos.io/v2.0/overview/architecture/#agent)

DAOS 代理是驻留在客户端节点上的守护进程，它与 DAOS 库交互以验证应用程序进程。它是一个受信任的实体，可以使用证书对 DAOS 库凭证进行签名。该代理可以支持不同的身份验证框架，并使用 Unix Domain Socket 与 DAOS 库进行通信。



# 存储模型[¶](https://docs.daos.io/v2.0/overview/storage/#storage-model)

## 概述[¶](https://docs.daos.io/v2.0/overview/storage/#overview)

下[图](https://docs.daos.io/v2.0/overview/storage/#f4.1)代表了 DAOS 存储模型的基本抽象。

![../graph/daos_abstractions.png](https://docs.daos.io/v2.0/graph/daos_abstractions.png)

DAOS*池*是跨*目标*集合分布的存储预留。每个目标上分配给池的实际空间称为*池*分片。分配给池的总空间在创建时确定。可以通过调整所有池分片的大小（在每个目标专用的存储容量限制内）或跨越更多目标（添加更多池分片）来扩展它。池提供存储虚拟化，是配置和隔离的单元。DAOS 池不能跨越多个系统。

一个池可以托管多个称为 DAOS*容器*的事务对象存储。每个容器都是一个私有对象地址空间，可以事务性地修改它，并且独立于存储在同一池中的其他容器。容器是快照和数据管理的单元。属于容器的 DAOS 对象可以分布在池的任何目标上以提高性能和弹性，并且可以通过不同的 API 访问以有效地表示结构化、半结构化和非结构化数据

下表显示了每个 DAOS 概念的目标可扩展性水平。

| DAOS 概念 | 可扩展性（数量级）                              |
| :-------- | :---------------------------------------------- |
| 系统      | 10 5 个服务器（数十万个）和 10 2 个池（数百个） |
| 服务器    | 10 1 个目标（十个）                             |
| 水池      | 10 2 个容器（数百个）                           |
| 容器      | 10 9 个对象（十亿）                             |

## DAOS 矿池[¶](https://docs.daos.io/v2.0/overview/storage/#daos-pool)

池由唯一的*池 UUID*标识，并在称为*池映射的持久版本列表中维护目标成员资格*. 成员资格是确定的和一致的，成员资格的变化是按顺序编号的。池映射不仅记录活动目标列表，还包含树形存储拓扑，用于识别共享通用硬件组件的目标。例如，树的第一级可以表示共享同一主板的目标，然后第二级可以表示共享同一机架的所有主板，最后第三级可以表示同一机箱中的所有机架。该框架有效地表示分层故障域，然后用于避免将冗余数据放置在遭受相关故障的目标上。在任何时间点，都可以将新目标添加到池映射中，并且可以排除失败的目标。此外，池图是完全版本化的，

池分片是持久内存的保留，可选择与特定目标上的 NVMe 存储上的预分配空间相结合。它具有固定容量，并且在满时无法操作。可以随时查询当前空间使用情况，并报告存储在池分片中的任何数据类型使用的总字节数。

在目标失败并从池映射中排除时，池内的数据冗余会自动在线恢复。这种自我修复过程称为*重建*。重建进度会定期记录在存储在持久内存中的池中的特殊日志中，以解决级联故障。添加新目标时，数据会自动迁移到新添加的目标，以在所有成员之间平均分配空间使用量。此过程称为*空间重新平衡*，并使用专用的持久日志来支持中断和重新启动。池是分布在不同存储节点上的一组目标，数据和元数据分布在这些节点上以实现水平可扩展性，并进行复制或纠删码以确保持久性和可用性。

创建池时，必须定义一组系统属性来配置池支持的不同功能。此外，用户可以定义将永久存储的属性。

只有经过身份验证和授权的应用程序才能访问池。可以支持多种安全框架，从 NFSv4 访问控制列表到基于第三方的身份验证（例如 Kerberos）。连接到池时会强制执行安全性。成功连接到池后，将连接上下文返回给应用程序进程。

如前所述，池存储许多不同类型的持久元数据，例如池映射、身份验证和授权信息、用户属性、属性和重建日志。此类元数据至关重要，需要最高级别的弹性。因此，池元数据被复制到来自不同高级故障域的几个节点上。对于具有数十万个存储节点的非常大的配置，只有很小一部分节点（大约几十个）运行*池元数据服务*。在存储节点数量有限的情况下，DAOS 可以依靠共识算法达成一致，在出现故障的情况下保证一致性，并避免脑裂综合症。

要访问一个池，用户进程应该连接到这个池并通过安全检查。一旦被授予，一个池连接可以与它的任何或所有对等应用程序进程共享（通过`local2global()`和`global2local()`操作）（类似于`openg()`POSIX 扩展）。当在数据中心上运行大规模分布式作业时，这种集体连接机制有助于避免元数据请求风暴。当发出连接请求的原始进程与池断开连接时，池连接将被撤销。



## DAOS 容器[¶](https://docs.daos.io/v2.0/overview/storage/#daos-container)

容器表示池内的对象地址空间，并由*容器 UUID*标识。下图展示了用户（I/O 中间件、特定领域的数据格式、大数据或 AI 框架……）如何使用容器概念来存储相关数据集。

![../graph/containers.png](https://docs.daos.io/v2.0/graph/containers.png)

像池一样，容器可以存储用户属性。必须在容器创建时传递一组属性以配置不同的功能，如校验和。

要访问容器，应用程序必须首先连接到池，然后打开容器。如果应用程序被授权访问容器，则返回容器句柄。这包括授权应用程序中的任何进程访问容器及其内容的功能。打开进程可以与任何或所有对等方共享此句柄。他们的能力在容器关闭时被撤销。

容器中的对象可能具有不同的数据分布模式和目标冗余。动态或静态条带化、复制或纠删码是定义对象模式所需的一些参数。对象类定义一组对象的通用模式属性。每个对象类都分配有一个唯一标识符，并与池级别的给定模式相关联。可以随时使用可配置的模式定义新的对象类，该模式在创建后是不可变的（或至少在属于该类的所有对象都被销毁之前）。为方便起见，在创建池时会默认预定义几个预计最常用的对象类，如下[表](https://docs.daos.io/v2.0/overview/storage/#t4.2)所示。

**预定义对象类示例**

| 对象类别（RW = 读/写，RM = 主要读 | 冗余     | 布局（SC = 条带计数，RC = 副本计数，PC = 奇偶校验计数，TGT = 目标 |
| :-------------------------------- | :------- | :----------------------------------------------------------- |
| 小尺寸&RW                         | 复制     | 静态 SCxRC，例如 1x4                                         |
| 小尺寸 & RM                       | 擦除代码 | 静态 SC+PC，例如 4+2                                         |
| 大尺寸&RW                         | 复制     | 静态 SCxRC 超过最大 #targets）                               |
| 大号&RM                           | 擦除代码 | 静态 SCx(SC+PC) 带最大 #TGT)                                 |
| 未知尺寸和RW                      | 复制     | SCxRC，例如 1x4 最初和增长                                   |
| 尺寸未知 & RM                     | 擦除代码 | SC+PC，例如 4+2 最初和增长                                   |

如下所示，容器中的每个对象都由一个唯一的 128 位*对象地址标识*。对象地址的高 32 位是为 DAOS 保留的，用于对对象类等内部元数据进行编码。剩下的 96 位由用户管理，在容器内应该是唯一的。只要保证唯一性，堆栈的上层可以使用这些位来编码其元数据。DAOS API 中提供了每个容器的 64 位可扩展对象 ID 分配器。应用程序要存储的对象 ID 是完整的 128 位地址，仅供一次性使用，并且只能与单个对象模式关联。

**DAOS 对象 ID 结构**

```
<---------------------------------- 128 bits ---------------------------------->
--------------------------------------------------------------------------------
|DAOS Internal Bits|                Unique User Bits                           |
--------------------------------------------------------------------------------
<---- 32 bits ----><------------------------- 96 bits ------------------------->
```



容器是事务和版本控制的基本单元。*DAOS 库使用称为epoch*的时间戳隐式标记所有对象操作。DAOS 事务 API 允许将多个对象更新组合到单个原子事务中，并具有基于 epoch ordering 的多版本并发控制。所有版本化更新都可以定期 *聚合*，以回收重叠写入使用的空间并降低元数据复杂性。快照是永久引用，可以放置在特定时期以防止聚合。

容器元数据（快照列表、容器打开句柄、对象类、用户属性、属性等）存储在持久内存中，并由专用容器元数据服务维护，该服务使用与父元数据池服务相同的复制引擎，或者有自己的引擎。这在创建容器时是可配置的。

像池一样，对容器的访问由容器句柄控制。要获取有效句柄，应用程序进程必须打开容器并通过安全检查。然后可以通过容器`local2global()`和 `global2local()`操作与其他对等应用程序进程共享此容器句柄。



## DAOS 对象[¶](https://docs.daos.io/v2.0/overview/storage/#daos-object)

为了避免传统存储系统常见的扩展问题和开销，DAOS 对象有意简单。没有提供超出类型和模式的默认对象元数据。这意味着系统不维护时间、大小、所有者、权限甚至跟踪开启者。为了实现高可用性和水平可伸缩性，提供了许多对象模式（复制/纠删码、静态/动态条带化等）。架构框架灵活且易于扩展，以允许将来使用新的自定义架构类型。布局是根据对象标识符和池映射在对象打开的算法上生成的。通过在网络传输和存储期间使用校验和保护对象数据来确保端到端完整性。

可以通过不同的 API 访问 DAOS 对象：

- **多级键数组**API 是具有局部性功能的本机对象接口。密钥分为分布（dkey）和属性（akey）密钥。dkey 和 akey 都可以是可变长度和类型（字符串、整数甚至是复杂的数据结构）。保证同一 dkey 下的所有条目都配置在同一目标上。与 akey 关联的值可以是不能被部分覆盖的单个可变长度值，也可以是固定长度值的数组。akeys 和 dkeys 都支持枚举。
- **Key-value** API 提供了一个简单的键和变长值接口。它支持传统的 put、get、remove 和 list 操作。
- **Array API**实现了一个由 64 位偏移寻址的固定大小元素的一维数组。DAOS 数组支持任意范围的读、写和打孔操作。





# 交易模型[¶](https://docs.daos.io/v2.0/overview/transaction/#transaction-model)

DAOS API 支持分布式事务，允许针对属于同一容器的对象的任何更新操作组合成单个 ACID 事务。分布式一致性是通过基于多版本时间戳排序的无锁乐观并发控制机制提供的。DAOS 事务是可序列化的，可以临时用于需要它的部分数据集。

DAOS 版本控制机制允许创建持久的容器快照，这些快照提供容器的时间点分布式一致视图，可用于构建生产者-消费者管道。



## 纪元和时间戳排序[¶](https://docs.daos.io/v2.0/overview/transaction/#epoch-and-timestamp-ordering)

每个 DAOS I/O 操作都带有一个称为*epoch*的时间戳。一个纪元是一个 64 位整数，它集成了逻辑时钟和物理时钟（参见[HLC 论文](https://cse.buffalo.edu/tech-reports/2014-04.pdf)）。DAOS API 提供了帮助函数来将纪元转换为传统的 POSIX 时间（即`struct timespec`，请参阅 参考资料`clock_gettime(3)`）。



## 容器快照[¶](https://docs.daos.io/v2.0/overview/transaction/#container-snapshot)

如下[图](https://docs.daos.io/v2.0/overview/transaction/#f4.4)所示，容器的内容可以随时进行快照。

![../graph/container_snapshots.png](https://docs.daos.io/v2.0/graph/container_snapshots.png)

DAOS 快照非常轻量级，并带有与快照创建时间相关联的纪元标记。一旦成功创建，快照在被显式销毁之前仍然是可读的。容器的内容可以回滚到特定的快照。

容器快照功能允许支持原生生产者/消费者管道，如下图所示。

![../graph/producer_consumer.png](https://docs.daos.io/v2.0/graph/producer_consumer.png)

一旦成功写入数据集的一致版本，生产者将生成快照。消费者应用程序可以订阅容器快照事件，以便在生产者提交它们时处理新的更新。快照的不变性保证了消费者看到一致的数据，即使生产者继续进行新的更新。生产者和消费者确实在不同版本的容器上运行，不需要任何序列化。一旦生产者生成了新版本的数据集，消费者可以查询两个快照之间的差异并仅处理增量更改。



## 分布式事务[¶](https://docs.daos.io/v2.0/overview/transaction/#distributed-transactions)

与 POSIX 不同，DAOS API 没有强加任何最坏情况的并发控制机制来解决冲突的 I/O 操作。相反，单独的 I/O 操作使用不同的 epoch 进行标记，并按 epoch 顺序应用，而不管执行顺序如何。此基准模型为不会生成冲突 I/O 工作负载的数据模型和应用程序提供最大的可扩展性和性能。典型示例是集体 MPI-IO 操作、POSIX 文件读/写或 HDF5 数据集读/写。

对于需要冲突序列化的部分数据模型，DAOS提供了基于多版本并发控制的分布式可序列化事务。当不同的用户进程可以覆盖与 dkey/akey 对关联的值时，通常需要事务。例如，DAOS 上的 SQL 数据库或不协调的客户端同时访问的一致 POSIX 命名空间。在同一操作的上下文中提交的所有 I/O 操作（包括读取）将使用相同的 epoch。DAOS 事务机制自动检测传统的读/写、写/读和写/写冲突，并中止其中一个冲突事务（事务提交失败`-DER_RESTART`）。然后必须由用户/应用程序重新启动失败的事务。

在初始实现中，事务 API 具有以下限制，这些限制将在未来的 DAOS 版本中解决：

- 不支持数组 API
- 通过在同一事务的上下文中执行的对象获取/列表和键值获取/列表操作，事务对象更新和键值放置操作不可见。



# 数据的完整性[¶](https://docs.daos.io/v2.0/overview/data_integrity/#data-integrity)

DAOS 在内部使用校验和来发现静默数据损坏。虽然系统中的每个组件（网络层、存储设备）都可以提供针对无声数据损坏的保护，但 DAOS 提供端到端数据完整性以更好地保护用户数据。如果检测到静默数据损坏，DAOS 将尝试使用数据冗余机制（复制或纠删码）恢复损坏的数据。

## 端到端数据完整性[¶](https://docs.daos.io/v2.0/overview/data_integrity/#end-to-end-data-integrity)

简单来说，端到端意味着 DAOS 客户端库将为发送到 DAOS 服务器的数据计算校验和。DAOS 服务器将存储校验和并在数据检索时返回。然后客户端通过计算新的校验和并与从服务器接收到的校验和进行比较来验证数据。根据受保护数据的类型，这种方法会有所不同，但下图显示了基本的校验和流程。 ![基本校验和流程](https://docs.daos.io/v2.0/graph/data_integrity/basic_checksum_flow.png)

## 配置[¶](https://docs.daos.io/v2.0/overview/data_integrity/#configuring)

为每个容器配置数据完整性。有关如何在 DAOS 中组织数据的更多信息，请参阅[存储模型](https://docs.daos.io/v2.0/overview/storage/)。有关如何设置具有数据完整性的容器的详细信息，请参阅容器用户指南中的数据完整性[。](https://docs.daos.io/v2.0/user/container/#data-integrity)

## 键和值对象[¶](https://docs.daos.io/v2.0/overview/data_integrity/#keys-and-value-objects)

由于 DAOS 是键/值存储，因此键和值的数据都受到保护，但是方法略有不同。对于单值和数组这两种不同的值类型，方法也略有不同。

### 钥匙[¶](https://docs.daos.io/v2.0/overview/data_integrity/#keys)

在更新和获取时，客户端计算用作分布和属性键的数据的校验和，并将其发送到 RPC 中的服务器。服务器使用校验和验证密钥。在枚举密钥时，服务器将计算密钥的校验和并打包到 RPC 消息中给客户端。客户端将验证收到的密钥。

笔记

密钥的校验和不存储在服务器上。计算密钥的哈希并将其用于在密钥的服务器树中索引密钥，请参阅[VOS 密钥数组存储](https://github.com/daos-stack/daos/blob/release/2.0/src/vos/README.md#key-array-stores)。还期望密钥仅存储在具有可靠数据完整性保护的存储类内存中。

### 价值观[¶](https://docs.daos.io/v2.0/overview/data_integrity/#values)

在更新时，客户端将为值的数据计算校验和，并将其发送到 RPC 中的服务器。如果启用“服务器验证”，服务器将为该值计算一个新的校验和，并与从客户端接收到的校验和进行比较，以验证该值的完整性。如果校验和不匹配，则发生数据损坏并向客户端返回错误，指示客户端应再次尝试更新。无论是否启用“服务器验证”，服务器都会存储校验和。有关[VOS](https://github.com/daos-stack/daos/blob/release/2.0/src/vos/README.md) 中校验和管理和存储的更多信息，请参阅 VOS。

在获取时，服务器会将存储的校验和与获取的值一起返回给客户端，以便客户端可以验证接收到的值。如果校验和不匹配，则客户端将从另一个副本中获取（如果可用）以尝试获取未损坏的数据。

对于两种不同类型的值，这种方法有一些细微的变化。下图说明了一个基本示例。（有关单值和数组值类型的更多详细信息，请参阅[存储模型）](https://docs.daos.io/v2.0/overview/storage/)

![基本校验和流程](https://docs.daos.io/v2.0/graph/data_integrity/basic_checksum_flow.png)

#### 单值[¶](https://docs.daos.io/v2.0/overview/data_integrity/#single-value)

单个值是一个原子值，这意味着写入单个值将更新整个值并读取检索整个值。其他 DAOS 功能（例如纠删码）可能会将单个值拆分为多个分片，以分布在多个存储节点之间。整个单一值（如果去单个节点）或每个分片（如果分布式）将计算校验和，发送到服务器，并存储在服务器上。

请注意，单个值或单个值的分片可能小于从它派生的校验和。建议如果应用程序需要许多小的单个值来使用数组类型。

#### 数组值[¶](https://docs.daos.io/v2.0/overview/data_integrity/#array-values)

与单个值不同，数组值可以在数组的任何部分更新和获取。此外，对数组的更新是版本化的，因此 fetch 可以包含来自多个版本的数组的部分。数组的这些版本化部分中的每一个都称为范围。下图说明了几个示例（有关详细信息，另请参阅[VOS 密钥阵列存储](https://github.com/daos-stack/daos/blob/release/2.0/src/vos/README.md#key-array-stores)）：

来自索引 2-13 的单个范围更新（蓝线）。从索引 2-6 获取的范围（橙色线）。提取只是写入的原始范围的一部分。

![img](https://docs.daos.io/v2.0/graph/data_integrity/array_example_1.png)

许多范围更新和不同的时期。从索引 2-13 提取需要来自每个范围的部分。

![数组示例 2](https://docs.daos.io/v2.0/graph/data_integrity/array_example_2.png)

数组类型的性质要求使用更复杂的方法来创建校验和。DAOS 使用“分块”方法，其中每个范围将被分解为具有预定“块大小”的“块”。校验和将从这些块中派生。块与绝对偏移量（从 0 开始）对齐，而不是 I/O 偏移量。下图说明了配置为 4 的块大小（在此示例中单位是任意的）。尽管并非所有块的完整大小都是 4，但仍保持绝对偏移对齐。范围周围的灰色框表示块。

![img](https://docs.daos.io/v2.0/graph/data_integrity/array_with_chunks.png)

有关对象更新和获取的校验和过程的更多详细信息，请参阅[对象层](https://github.com/daos-stack/daos/blob/release/2.0/src/object/README.md) 。

## 校验和计算[¶](https://docs.daos.io/v2.0/overview/data_integrity/#checksum-calculations)

实际的校验和计算由 [isa-l](https://github.com/intel/isa-l) 和[isa-l_crypto](https://github.com/intel/isa-l_crypto)库完成。但是，这些库是从大部分 DAOS 中抽象出来的，并且使用了一个通用校验和库，并与实际的 isa-l 实现的适当适配器一起使用。 [通用校验和库](https://github.com/daos-stack/daos/blob/release/2.0/src/common/README.md#checksum)

## 性能影响[¶](https://docs.daos.io/v2.0/overview/data_integrity/#performance-impact)

计算校验和可能会占用大量 CPU，并且会影响性能。为了减轻性能影响，应选择具有硬件加速的校验和类型。例如，最近的 Intel CPU 支持 CRC32C，并且许多通过 SIMD 加速。

## 质量[¶](https://docs.daos.io/v2.0/overview/data_integrity/#quality)

单元和功能测试在许多层上执行。

| 测试可执行文件     | 测试了什么                                                   | 关键测试文件                          |
| :----------------- | :----------------------------------------------------------- | :------------------------------------ |
| common_test        | daos_csummer，帮助块对齐的实用函数                           | src/common/tests/checksum_tests.c     |
| vos_test           | vos_obj_update/fetch 带有校验和参数的 api，以确保更新和获取校验和 | src/vos/tests/vts_checksum.c          |
| srv_checksum_tests | 用于将获取的校验和添加到数组请求的服务器端逻辑。校验和根据范围布局被适当地复制或创建。 | src/object/tests/srv_checksum_tests.c |
| daos_test          | daos_obj_update/fetch 启用校验和。-z 标志可用于特定的校验和测试。此外 --csum_type 标志可用于启用与任何其他 daos_tests 的校验和 | src/tests/suite/daos_checksum.c       |

### 运行测试[¶](https://docs.daos.io/v2.0/overview/data_integrity/#running-tests)

**daos_server 未运行**

```
./commont_test
./vos_test -z
./srv_checksum_tests
```

**随着 daos_server 运行**

```
export DAOS_CSUM_TEST_ALL_TYPE=1
./daos_server -z
./daos_server -i --csum_type crc64
```





# 故障模型[¶](https://docs.daos.io/v2.0/overview/fault/#fault-model)

DAOS 依赖于大规模分布式单端口存储。因此，每个目标实际上都是一个单点故障。DAOS 通过为不同故障域中的目标提供冗余来实现数据和元数据的可用性和持久性。DAOS 内部池和容器元数据通过强大的共识算法进行复制。然后通过在内部透明地利用 DAOS 分布式事务机制来安全地复制或擦除 DAOS 对象。本节的目的是提供有关 DAOS 如何实现容错和保证对象弹性的详细信息。



## 分级故障域[¶](https://docs.daos.io/v2.0/overview/fault/#hierarchical-fault-domains)

故障域是一组共享相同故障点的服务器，因此它们很可能完全失败。DAOS 假设故障域是分层的并且不重叠。实际的层次结构和故障域成员必须由 DAOS 用来生成池映射的外部数据库提供。

池元数据从不同的高级故障域复制到多个节点上以实现高可用性，而对象数据可以在可变数量的故障域上复制或纠删码，具体取决于所选的对象类。



## 故障检测[¶](https://docs.daos.io/v2.0/overview/fault/#fault-detection)

DAOS 引擎在 DAOS 系统内通过称为[SWIM](https://doi.org/10.1109/DSN.2002.1028914)的基于 gossip 的协议进行监控，该协议 提供准确、高效和可扩展的故障检测。附加到每个 DAOS 目标的存储通过定期本地健康评估进行监控。每当本地存储 I/O 错误返回到 DAOS 服务器时，将自动调用内部健康检查过程。此过程将通过分析 IO 错误代码和设备 SMART/Health 数据来进行整体健康评估。如果结果是否定的，目标将被标记为有故障，并且此目标的进一步 I/O 将被拒绝并重新路由。



## 误隔离[¶](https://docs.daos.io/v2.0/overview/fault/#fault-isolation)

一旦检测到，故障目标或引擎（实际上是一组目标）必须从池映射中排除。此过程由管理员手动或自动触发。排除后，新版本的池映射会急切地推送到所有存储目标。此时，池进入降级模式，可能需要对访问进行额外处理（例如，从纠删码中重建数据）。因此，DAOS 客户端和存储节点重试 RPC，直到它们从新池映射中找到替代替换目标或遇到 RPC 超时。此时，与被驱逐目标的所有未完成的通信都将中止，并且在目标明确重新集成之前（可能仅在维护操作之后）之前不应向目标发送更多消息。

池服务会及时通知所有存储目标池映射更改。客户端节点不是这种情况，它们在每次与任何引擎通信时都会延迟通知池映射无效。为此，客户端在每个 RPC 中打包他们当前的池映射版本。服务器不仅回复当前的池地图版本。因此，当 DAOS 客户端遇到 RPC 超时时，它会定期与其他 DAOS 目标通信以确保其池映射始终是最新的。然后客户端最终将被告知目标排除并进入降级模式。

这种机制保证全局节点驱逐，并且所有节点最终共享相同的目标活跃度视图。



## 故障恢复[¶](https://docs.daos.io/v2.0/overview/fault/#fault-recovery)

从池映射中排除后，每个目标都会自动启动重建过程以恢复数据冗余。首先，每个目标创建一个受目标排除影响的本地对象列表。这是通过扫描由底层存储层维护的本地对象表来完成的。然后对于每个受影响的对象，确定新对象分片的位置，并为整个历史（即快照）恢复对象的冗余。一旦重建了所有受影响的对象，池映射将第二次更新以报告目标失败。这标志着集体重建过程的结束以及此特定故障从降级模式的退出。此时，池已完全从故障中恢复，客户端节点现在可以从重建的对象分片中读取。

此重建过程在应用程序继续访问和更新对象时在线执行。







# 安全模型[¶](https://docs.daos.io/v2.0/overview/security/#security-model)

DAOS 使用灵活的安全模型，将身份验证与授权分开。它旨在对 I/O 路径产生最小的影响。

DAOS 不为用于 I/O 传输的结构网络提供任何传输安全性。部署 DAOS 时，管理员负责对其特定结构网络进行安全配置。对于以太网上的 RDMA，建议启用 IPsec。有关详细信息，请参阅 [RDMA 协议规范 (RFC 5040)](https://tools.ietf.org/html/rfc5040#section-8.2) 。

DAOS 在两个领域实现了自己的安全层。在用户级别，客户端必须能够读取和修改它们已被授予访问权限的*池*和*容器。*在系统和管理级别，只有授权组件才能访问 DAOS 管理网络。

## 验证[¶](https://docs.daos.io/v2.0/overview/security/#authentication)

有不同的身份验证方式，具体取决于调用者是访问客户端资源还是访问 DAOS 管理网络。

### 客户端库[¶](https://docs.daos.io/v2.0/overview/security/#client-library)

客户端库`libdaos`是一个不受信任的组件。使用客户端库的`daos`用户级命令也是一个不受信任的组件。一个受信任的进程，即 DAOS 代理 ( `daos_agent`)，在每个客户端节点上运行并对用户进程进行身份验证。

DAOS 安全模型旨在支持客户端进程的不同身份验证方法。目前，我们仅支持 AUTH_SYS 身份验证风格，如 [RFC 2623](https://datatracker.ietf.org/doc/html/rfc2623#section-2.2.1)中为 NFS 定义的那样。

### DAOS 管理网络[¶](https://docs.daos.io/v2.0/overview/security/#daos-management-network)

DAOS 管理组件使用 [gRPC 协议](https://grpc.io/)通过网络进行通信。

每个受信任的 DAOS 组件（`daos_server`、`daos_agent`和`dmg` 管理工具）都通过系统管理员为该组件生成的证书进行身份验证。所有组件证书必须使用相同的根证书生成并分发到适当的 DAOS 节点，如 [DAOS 管理指南](https://docs.daos.io/v2.0/admin/deployment/#certificate-configuration)中所述。

DAOS 组件在 DAOS 管理网络上通过 gRPC 在相互验证的 TLS 上使用它们各自的组件证书相互识别。DAOS 验证证书链以及证书中的通用名称 (CN)，以验证组件的身份。

## 授权[¶](https://docs.daos.io/v2.0/overview/security/#authorization)

资源的客户端授权由资源上的访问控制列表 (ACL) 控制。管理网络的授权是通过设置 DAOS 系统时生成的[证书的设置来实现的。](https://docs.daos.io/v2.0/admin/deployment/#certificate-configuration)

### 组件证书[¶](https://docs.daos.io/v2.0/overview/security/#component-certificates)

通过在每个管理组件证书中设置的 CommonName (CN) 控制对 DAOS 管理 RPC 的访问。给定的管理 RPC 只能由与正确证书连接的组件调用。

### 访问控制列表[¶](https://docs.daos.io/v2.0/overview/security/#access-control-lists)

*客户端对池*和*容器*等资源的访问由 DAOS 访问控制列表 (ACL) 控制。这些 ACL 部分源自 NFSv4 ACL，并针对分布式系统的独特需求进行了调整。

客户端可以请求对资源的只读或读写访问。如果资源 ACL 未授予他们请求的访问级别，他们将无法连接。连接后，他们对该资源的句柄会授予特定操作的权限。

句柄的权限在其存在期间持续存在，类似于 POSIX 系统中的打开文件描述符。当前无法撤销句柄。

DAOS ACL 由零个或多个访问控制条目 (ACE) 组成。ACE 是 用于向请求访问资源的用户授予或拒绝特权的[规则。](https://docs.daos.io/v2.0/overview/security/#enforcement)

#### 访问控制条目[¶](https://docs.daos.io/v2.0/overview/security/#access-control-entries)

在 DAOS 工具的输入和输出中，访问控制条目 (ACE) 使用冒号分隔的字符串定义，格式如下： `TYPE:FLAGS:PRINCIPAL:PERMISSIONS`

所有字段的内容都区分大小写。

##### 类型[¶](https://docs.daos.io/v2.0/overview/security/#type)

ACE 条目的类型（强制）。目前仅支持一种类型的 ACE。

- A（允许）：允许对给定权限的指定主体进行访问。

##### 标志[¶](https://docs.daos.io/v2.0/overview/security/#flags)

（可选）标志提供了有关如何解释 ACE 的附加信息。

- G（Group）：主体应该被解释为一个组。

##### 主要的[¶](https://docs.daos.io/v2.0/overview/security/#principal)

主体（也称为身份）以`name@domain`格式指定。如果名称是本地域上的 UNIX 用户/组，则应保留该域。目前，这是 DAOS 支持的唯一案例。

有 3 个特殊主体 、`OWNER@`和`GROUP@`，`EVERYONE@`它们与传统 POSIX 权限位中的 User、Group 和 Other 一致。当以 ACE 字符串格式提供它们时，它们的拼写必须与此处所写的完全一致，大写且不附加域。该`GROUP@`条目还必须具有`G`（组）标志。

##### 权限[¶](https://docs.daos.io/v2.0/overview/security/#permissions)

资源的 ACE 中的权限允许某种类型的用户访问该资源。ACE 字段内的许可“位”（字符）的顺序 `PERMISSIONS`并不重要。

| 允许             | 池含义           | 容器含义                      |
| :--------------- | :--------------- | :---------------------------- |
| r（读取）        | 't' 的别名       | 读取容器数据和属性            |
| w（写）          | 'c' + 'd' 的别名 | 写入容器数据和属性            |
| c（创建）        | 创建容器         | 不适用                        |
| d（删除）        | 删除任何容器     | 删除此容器                    |
| t（获取道具）    | 连接/查询        | 获取容器属性                  |
| T（设置道具）    | 不适用           | 设置/更改容器属性             |
| 一个（获取 ACL） | 不适用           | 获取容器 ACL                  |
| A（设置 ACL）    | 不适用           | 设置/更改容器 ACL             |
| o（集所有者）    | 不适用           | 设置/更改容器的所有者用户和组 |

包含不适用于给定资源的权限的 ACE 被视为无效。

要允许用户/组连接到资源，该主体的权限必须至少包括某种形式的读取访问权限（例如，`read`或`get-prop`）。`write`当请求对资源进行 RW 访问时，具有 -only 权限的用户将被拒绝。

##### 拒绝访问[¶](https://docs.daos.io/v2.0/overview/security/#denying-access)

目前，仅支持“允许”访问控制条目。

但是，可以通过在没有权限的情况下为特定用户创建允许条目来拒绝对特定用户的访问。这与删除用户的 ACE 根本不同，后者允许 ACL 中的其他 ACE 确定其访问权限。

由于[强制执行组权限的方式，](https://docs.daos.io/v2.0/overview/security/#enforcement)*无法*以这种方式拒绝对特定组的访问 。

##### ACE 示例[¶](https://docs.daos.io/v2.0/overview/security/#ace-examples)

- ```
  A::daos_user@:rw
  ```

  - 允许指定的 UNIX 用户`daos_user`具有读写访问权限。

- ```
  A:G:project_users@:tc
  ```

  - 允许 UNIX 组中的任何人`project_users`访问池的内容并创建容器。

- ```
  A::OWNER@:rwdtTaAo
  ```

  - 允许拥有容器的 UNIX 用户拥有完全控制权。

- ```
  A:G:GROUP@:rwdtT
  ```

  - 允许拥有容器的 UNIX 组读取和写入数据、删除容器和操作容器属性。

- ```
  A::EVERYONE@:r
  ```

  - 允许其他规则未涵盖的任何用户具有只读访问权限。

- ```
  A::daos_user@:
  ```

  - 拒绝指定的 UNIX 用户`daos_user`对该资源的任何访问。

#### 执法[¶](https://docs.daos.io/v2.0/overview/security/#enforcement)

访问控制条目 (ACE) 将按以下顺序执行：

- 所有者-用户
- 命名用户
- 所有者组和命名组
- 每个人

通常，执行将基于第一个匹配项，忽略较低优先级的条目。

如果用户是资源的所有者并且有`OWNER@`条目，他们将仅获得所有者权限。他们将不会收到指定用户/组条目中的任何权限，即使他们会匹配那些其他条目。

如果用户不是所有者，或者没有`OWNER@`条目，但他们的用户身份有一个 ACE，他们将只收到他们的用户身份的权限。他们将不会获得任何组的权限，即使这些组条目具有比用户条目更广泛的权限。预计用户最多匹配一个用户条目。

如果没有找到匹配的用户条目，但条目匹配一个或多个用户组，则执行将基于所有匹配组的权限的联合，包括 owner-group `GROUP@`。

如果未找到匹配的组，则将`EVERYONE@`使用条目的权限（如果存在）。

默认情况下，如果用户在 ACL 列表中不匹配任何 ACE，则访问将被拒绝。

#### ACL 文件[¶](https://docs.daos.io/v2.0/overview/security/#acl-file)

接受 ACL 文件的工具期望它是一个简单的文本文件，每行有一个 ACE。通过使用 a`#`作为该行的第一个非空白字符，可以将一行标记为注释。

例如：

```
# ACL for my container
# Owner can't touch data - just do admin-type things
A::OWNER@:dtTaAo
# My project's users can generate and access data
A:G:my_great_project@:rw
# Bob can use the data to generate a report
A::bob@:r
```

权限位和 ACE 本身不需要按任何特定顺序排列。但是，当生成的 ACL 由 DAOS 解析和显示时，顺序可能会有所不同。

#### 限制[¶](https://docs.daos.io/v2.0/overview/security/#limitations)

DAOS ACL 内部数据结构中 ACE 列表的最大大小为 64KiB。

要计算 ACL 的内部数据大小，请对每个 ACE 使用以下公式：

- ACE 的基本大小为 256 字节。
- 如果 ACE 主体*不是*特殊主体之一：
- 添加主体字符串的长度 + 1。
- 如果该值不是 64 字节对齐的，则向上舍入到最接近的 64 字节边界。





# 用例[¶](https://docs.daos.io/v2.0/overview/use_cases/#use-cases)

本节提供了一个非详尽的用例列表，展示了如何在真正的 HPC 集群上使用 DAOS 存储模型和堆栈。

本文档包含以下部分：

- [存储管理和工作流程集成](https://docs.daos.io/v2.0/overview/use_cases/#61)
- 工作流程执行
  - [批量同步检查点](https://docs.daos.io/v2.0/overview/use_cases/#63)
  - [生产者/消费者](https://docs.daos.io/v2.0/overview/use_cases/#64)
  - [并发生产者](https://docs.daos.io/v2.0/overview/use_cases/#65)
- [存储节点故障和重新同步](https://docs.daos.io/v2.0/overview/use_cases/#66)



## 存储管理和工作流程集成[¶](https://docs.daos.io/v2.0/overview/use_cases/#storage-management-workflow-integration)

在本节中，我们考虑两种不同的集群配置：

- 集群 A：所有或大部分计算节点都具有本地持久内存。也就是说，每个计算节点也是一个存储节点。
- 集群 B：存储节点专用于存储并在整个结构中传播。它们不用于计算，因此不运行任何应用程序代码。

在启动时，每个存储节点都会启动实例化服务线程的 DAOS 服务器。在集群 A 中，DAOS 线程绑定到有噪声的内核，如果使用 mOS，则与 FWK 交互。在集群 B 中，DAOS 服务器可以使用存储节点的所有核心。

DAOS 服务器然后加载存储管理模块。该模块扫描节点上的本地存储并将结果报告给指定的主 DAOS 服务器，该服务器汇总有关整个集群的已用和可用存储的信息。管理模块还检索故障域层次结构（从数据库或特定服务）并将其与存储信息集成。

然后，资源管理器使用 DAOS 管理 API 来查询可用存储并为要调度的新工作流分配一定数量的存储（即永久内存）。在集群 A 中，此分配请求可能会列出应该运行工作流的计算节点，而在情况 B 中，它可能会要求在一些已分配的计算节点附近进行存储。

一旦成功分配，主服务器将通过格式化 VOS 布局（即 fallocate(1) 一个 PMEM 文件并创建 VOS 超级块）来初始化一个覆盖已分配存储的 DAOS 池，并启动池服务，这将启动 Raft 引擎负责池成员和元数据。此时，DAOS 池已准备好移交给实际工作流。

当工作流开始时，一个 rank 连接到 DAOS 池，然后使用 local2global() 生成一个全局连接句柄，并与使用 global2local() 创建本地连接句柄的所有其他应用程序 rank 共享它。此时，可以创建新容器并通过应用程序任务集体或单独打开现有容器。



## 工作流程执行[¶](https://docs.daos.io/v2.0/overview/use_cases/#workflow-execution)

我们考虑下[图](https://docs.daos.io/v2.0/overview/use_cases/#6a)中表示的工作流程。

![../graph/Fig_007.png](https://docs.daos.io/v2.0/graph/Fig_007.png)

每个绿色框代表一个不同的容器。所有容器都存储在灰色框表示的同一个 DAOS 池中。模拟从输入容器读取数据并将原始时间步长写入另一个容器。它还定期将检查点转储到专用的 ckpt 容器。下采样作业读取原始时间步并生成采样时间步以供后处理分析，后处理将分析数据存储到另一个容器中。



### 批量同步检查点[¶](https://docs.daos.io/v2.0/overview/use_cases/#bulk-synchronous-checkpoint)

防御性 I/O 用于管理运行时间大于平台平均故障间隔时间 (MTBF) 的大型仿真。模拟定期将当前计算状态转储到专用容器，用于在发生故障时保证前进进度。本节详细说明如何在 DAOS 存储堆栈之上实现检查点。我们首先考虑依赖阻塞屏障的传统方法，然后考虑更松散耦合的执行。

**阻挡屏障**

当模拟作业开始时，一项任务打开检查点容器并获取当前的全局 HCE。然后它获得一个 epoch 持有并与对等任务共享数据（容器句柄、当前 LHE 和全局 HCE）。每个任务通过读取等于全局 HCE 的 epoch 来检查保存到检查点容器的最新计算状态，并从上次检查点的位置恢复计算。

对于检查点，每个任务执行一个屏障以与其他任务同步，将其当前计算状态写入 epoch LHE 的检查点容器，刷新所有更新并最终执行另一个屏障。一旦所有任务都完成了最后一个屏障，一个指定的任务（例如等级 0）提交 LHE，然后在成功提交时增加一个。这个过程会定期重复，直到模拟成功完成。

**非阻塞屏障**

我们现在考虑另一种执行更松耦合的检查点方法。与前一种情况一样，一个任务负责打开检查点容器、获取全局 HCE、获取 epoch 持有并与其他对等任务共享数据。但是，任务现在可以按照自己的节奏检查其计算状态，而无需相互等待。在计算了 N 个时间步之后，每个任务在 LHE+1 时期将其状态转储到检查点容器，刷新更改并在完成后调用非阻塞屏障（例如 MPI_Ibarrier()）。然后再经过 N 个时间步，新的检查点被写入 epoch LHE+2，以此类推。对于每个检查点，epoch 编号都会递增。

此外，每个任务都会定期调用 MPI_Test() 来检查屏障是否完成，这允许它们回收 MPI_Request。在屏障完成后，一个指定的任务（通常为 0 级）也提交相关的纪元数。所有 epoch 都保证按顺序提交，并且每个提交的 epoch 都是一个新的一致检查点，可以从中重新启动。失败时，已由单个任务写入但未提交的检查点状态将自动回滚。



### 生产者/消费者[¶](https://docs.daos.io/v2.0/overview/use_cases/#producerconsumer)

在[上图中](https://docs.daos.io/v2.0/overview/use_cases/#6a)，我们有两个生产者/消费者示例。下采样作业使用模拟作业生成的原始时间步，并生成由后处理作业分析的采样时间步。DAOS 堆栈为生产者/消费者工作流程提供了特定机制，甚至允许消费者将其分析结果转储到与生产者相同的容器中。

**私人集装箱**

下采样作业打开采样时间步长容器，获取当前全局 HCE，获得一个 epoch 保持，并在 epoch LHE 将新的采样数据写入该容器。发生这种情况时，后处理作业会打开存储分析数据以进行写入的容器，检查最新分析的时间步长并在此容器上获得一个 epoch 保持。然后它打开采样的时间步长容器进行读取，并检查下一个要使用的时间步长是否准备好。如果不是，它等待一个新的全局 HCE 被提交（通过事件队列上的异步事件完成通知）并再次检查。当请求的时间步可用时，下采样作业会处理此新时间步的输入数据，将结果转储到其自己的容器中，并在其元数据中更新最新分析的时间步。

另一种方法是生产者作业为感兴趣的时期创建显式快照，并让分析作业等待和处理快照。这避免了处理每个提交的时期。

**共享容器**

我们现在假设存储采样时间步长的容器和存储分析数据的容器是一个容器。换句话说，下采样作业消耗输入数据并将输出数据写入同一个容器。

下采样作业打开共享容器，获得一个保持并将新的采样时间步转储到容器中。和以前一样，后处理作业也会打开容器，获取最新分析的时间步长，但在新的全局 HCE 准备好之前不会获得 epoch hold。一旦通知后处理作业有一个新的全局 HCE，它就可以分析新的采样时间步长，获得一个保持并将其分析的数据写入同一个容器。完成此操作后，后处理作业刷新其更新，提交持有的 epoch 并释放持有的 epoch。此时，它可以再次等待下采样作业生成新的全局 HCE。



### 并发生产者[¶](https://docs.daos.io/v2.0/overview/use_cases/#concurrent-producers)

在上一节中，我们考虑了生产者和消费者作业同时读取和写入同一个容器，但在不相交的对象中。我们现在考虑一个由并发生产者作业组成的工作流，这些作业以冲突和不协调的方式修改同一个容器。这实际上意味着两个生产者可以更新相同 KV 对象或文档存储的相同键或相同字节数组的重叠范围。该模型需要实现并发控制机制（不是 DAOS 的一部分）来协调冲突的访问。本节介绍了这种基于锁定的机制的示例，但也可以考虑替代方法。

工作流由两个应用程序组成，它们使用分布式锁管理器来序列化对 DAOS 对象的竞争访问。每个应用程序单独打开同一个容器，并在它想要修改容器中的某些对象时获取一个 epoch 保持。在修改对象之前，应用程序应该获取对象上的写锁。这个锁带有一个锁值块（LVB），存储了这个对象最后一次修改和提交的最后一个纪元号。一旦获得锁，写入者必须：

- 从一个 epoch 中读取，该 epoch 等于 LVB 中指定的 epoch 和句柄 LRE 中的最大值。
- 提交新的写入，其时代高于直播中的时代和当前持有的时代。

在应用程序完成、刷新和提交所有 I/O 操作后，使用修改对象的提交 epoch 更新 LVB，最终可以释放锁。



## 存储节点故障和重新同步[¶](https://docs.daos.io/v2.0/overview/use_cases/#storage-node-failure-and-resilvering)

在本节中，我们考虑一个连接到 DAOS 池和一个突然发生故障的存储节点的工作流。DAOS 客户端和与故障服务器通信的服务器都会遇到 RPC 超时并通知 RAS 系统。失败的 RPC 会重复发送，直到 RAS 系统或池元数据服务本身决定宣布存储节点死亡并将其从池映射中逐出。池映射更新与新版本一起传播到所有存储节点，这些节点会延迟（在 RPC 回复中）通知客户端新的池映射版本可用。因此，客户端和服务器最终都会收到故障通知并进入恢复模式。

服务器节点将合作为受影响的对象恢复不同服务器上的冗余，而客户端将进入降级模式并从其他副本读取，或从纠删码重建数据。此重建过程在容器仍在被访问和修改时在线执行。一旦为所有对象恢复了冗余，池映射将再次更新以通知所有人系统已从故障中恢复并且系统可以退出降级模式。





# 术语[¶](https://docs.daos.io/v2.0/overview/terminology/#terminology)

| Acronym                                                      | Expansion                                                    |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| [ABT](https://github.com/pmodels/argobots/wiki/Introduction-to-Argobots) | Argobots                                                     |
| ACL                                                          | Access Control List                                          |
| ACE                                                          | Access Control Entry                                         |
| ACID                                                         | Atomicity, consistency, isolation, durability                |
| BIO                                                          | Blob I/O                                                     |
| CART                                                         | Collective and RPC Transport                                 |
| CGO                                                          | Go tools that enable creation of Go packages that call C code |
| CN                                                           | Compute Node                                                 |
| COTS                                                         | Commercial off-the-shelf                                     |
| CPU                                                          | Central Processing Unit                                      |
| Daemon                                                       | A process offering system-level resources.                   |
| [DAOS](https://docs.daos.io/)                                | Distributed Asynchronous Object Storage                      |
| PMEM                                                         | Intel Optane Persistent Memory                               |
| DPDK                                                         | Data Plane Development Kit                                   |
| dRPC                                                         | DAOS Remote Procedure Call                                   |
| [gRPC](https://grpc.io/)                                     | gRPC Remote Procedure Calls                                  |
| GURT                                                         | A common library of Gurt Useful Routines and Types           |
| [HLC](https://cse.buffalo.edu/tech-reports/2014-04.pdf)      | Hybrid Logical Clock                                         |
| HLD                                                          | High-level Design                                            |
| [ISA-L](https://01.org/intel®-storage-acceleration-library-open-source-version) | Intel Storage Acceleration Library                           |
| I/O                                                          | Input/Output                                                 |
| KV store                                                     | Key-Value store                                              |
| [libfabric](https://ofiwg.github.io/libfabric/)              | Open Fabrics Interface                                       |
| Mercury                                                      | A user-space RPC library that can use libfabrics as a transport |
| MTBF                                                         | Mean Time Between Failures                                   |
| NVM                                                          | Non-Volatile Memory                                          |
| NVMe                                                         | Non-Volatile Memory express                                  |
| [OFI](https://ofiwg.github.io/libfabric/)                    | Open Fabrics Interface                                       |
| OS                                                           | Operating System                                             |
| PM                                                           | Persistent Memory                                            |
| [PMDK](https://pmem.io/pmdk/)                                | Persistent Memory Devevelopment Kit                          |
| RAFT                                                         | Raft is a consensus algorithm used to distribute state transitions among DAOS server nodes. |
| RAS                                                          | Reliability, Availability & Serviceability                   |
| RDB                                                          | Replicated Database, containing pool metadata and maintained across DAOS servers using the Raft algorithm. |
| RDMA/RMA                                                     | Remote (Direct) Memory Access                                |
| RDG                                                          | Redundancy Group                                             |
| RPC                                                          | Remote Procedure Call                                        |
| SCM                                                          | Storage-Class Memory                                         |
| [SPDK](https://spdk.io/)                                     | Storage Performance Development Kit                          |
| SSD                                                          | Solid State Drive                                            |
| [SWIM](https://doi.org/10.1109/DSN.2002.1028914)             | Scalable Weakly-consistent Infection-style process group Membership Protocol |
| [ULT](https://github.com/pmodels/argobots/wiki/User-level-Thread-(ULT)) | User Level Thread                                            |
| UPI                                                          | Intel Ultra Path Interconnect                                |
| UUID                                                         | Universal Unique Identifier                                  |
| [VOS](https://github.com/daos-stack/daos/tree/release/2.0/src/vos/README.md) | Versioning Object Store                                      |















# 管理指南

# 硬件要求[¶](https://docs.daos.io/v2.0/admin/hardware/#hardware-requirements)

本节的目的是描述部署 DAOS 系统的处理器、存储和网络要求。

## 部署选项[¶](https://docs.daos.io/v2.0/admin/hardware/#deployment-options)

DAOS 存储系统部署为**池存储模型**。DAOS 服务器可以在单独机架中的专用存储节点上运行。这是一种传统的池模型，其中存储由所有计算节点统一访问。为了尽量减少 I/O 机架的数量并优化占地面积，这种方法通常需要高密度的存储服务器。

## 处理器要求[¶](https://docs.daos.io/v2.0/admin/hardware/#processor-requirements)

DAOS 需要 64 位处理器架构，主要在 Intel x86_64 架构上开发。DAOS 软件和它所依赖的库（例如，[ISA-L](https://github.com/intel/isa-l)、 [SPDK](https://pmem.io/pmdk/)、[PMDK](https://spdk.io/)和 [DPDK](https://www.dpdk.org/)）可以利用英特尔英特尔流 SIMD (SSE) 和英特尔高级矢量 (AVX) 扩展。

社区还报告说在以 Little Endian 小端模式配置的 64 位 ARM 处理器上运行 DAOS 客户端取得了一些成功。话虽如此，ARM 测试并不是当前 DAOS CI 持续集成管道的一部分，因此不会定期进行验证。

## 网络要求[¶](https://docs.daos.io/v2.0/admin/hardware/#network-requirements)

DAOS 数据平面依赖于[OFI libfabrics](https://ofiwg.github.io/libfabric/) ，并支持以太网/tcp 和 InfiniBand/verbs 的 OFI 提供程序。具有 RDMA 功能的网络是首选，以获得更好的性能。

DAOS 通过将 DAOS 引擎的不同实例绑定到各个网卡来支持服务器上的多个网络接口。DAOS 可以通过将节点上的不同客户端进程分配给不同的网络接口来支持客户端上的多个网络接口。请注意，DAOS 不*支持*多个网络接口上的网络级条带化，因此*单个*客户端进程将始终使用单个网络链接。

DAOS 控制平面提供了使用安全套接字层接口管理和管理 DAOS 服务器的方法。客户端和服务器之间的管理流量使用 IP over Fabric。然而，在大型集群上，DAOS 服务器的管理通常使用额外的带外网络连接 DAOS 服务集群中的节点。

## 存储要求[¶](https://docs.daos.io/v2.0/admin/hardware/#storage-requirements)

DAOS 要求每个存储节点都可以直接访问存储级内存 (SCM)。[虽然 DAOS 主要针对 Intel Optane^TM^ Persistent Memory 进行测试和调整，但 DAOS 软件堆栈是基于SNIA NVM](https://www.snia.org/sites/default/files/technical_work/final/NVMProgrammingModel_v1.2.pdf)中所述的 Linux 操作系统的 Persistent Memory Development Kit (PMDK) 和 Direct Access (DAX) 功能构建的[编程模型](https://www.snia.org/sites/default/files/technical_work/final/NVMProgrammingModel_v1.2.pdf)。因此，开源 DAOS 软件堆栈应该能够在 PMDK 支持的任何存储级内存上透明地运行。

存储节点可以选择配备[NVMe](https://nvmexpress.org/) (non-volatile memory express)[^10] SSD 以提供容量。DAOS 不支持 HDD 以及 SATA 和 SAS SSD。支持 NVMe 3D-NAND 和 Optane SSD。对于以非常高的 IOPS 速率为目标的 DAOS 安装，Optane SSD 是首选。用户空间存储堆栈也支持 NVMe-oF 设备，但从未经过测试。

至少 6% 的 SCM 与 SSD 容量的比率将保证 DAOS 在 SCM 中有足够的空间来存储其内部元数据（例如，池元数据、SSD 块分配跟踪）。较低的比率是可能的，但所需的 SCM 数量将取决于访问 DAOS 存储的应用程序的使用模式。由于 DAOS 将 SCM 用于其元数据，如果该比率太低，可能会有大容量存储可用，但 DAOS 元数据的 SCM 不足。

出于测试目的，可以通过挂载 tmpfs 文件系统使用 DRAM 模拟 SCM，也可以使用 DRAM 或环回文件模拟 NVMe SSD。

## 存储服务器设计[¶](https://docs.daos.io/v2.0/admin/hardware/#storage-server-design)

DAOS 存储服务器的硬件设计平衡了结构的网络带宽和 NVMe 存储设备的总存储带宽。这种关系根据应用程序工作负载的读/写平衡设置 NVMe 驱动器的数量。由于 NVMe SSD 的读取速度快于写入速度，因此 200Gbps PCIe4 x4 NIC 可以通过 4 个 NVMe4 x4 SSD 平衡只读工作负载，但可以通过 8 个 NVMe4 x4 SSD 平衡写入工作负载。SSD 的容量将决定为 DAOS 元数据提供 6% 比率所需的 Optane PMem DIMM 的最小容量。

![img](https://docs.daos.io/v2.0/admin/media/image2.png)

## CPU 亲和性[¶](https://docs.daos.io/v2.0/admin/hardware/#cpu-affinity)

最近的英特尔至强数据中心平台使用两个处理器 CPU，通过超路径互连 (UPI) 连接在一起。这些服务器中的 PCIe 通道与一个 CPU 具有天然的亲和力。尽管可以从任何系统内核全局访问，但通过 PCIe 总线连接的 NVMe SSD 和网络接口卡可以为每个 CPU 提供不同的性能特征（例如，更高的延迟、更低的带宽）。访问非本地 PCIe 设备可能涉及 UPI 链路上的流量，这可能会成为拥塞点。类似地，持久内存是非统一可访问的 (NUMA-**non-uniform memory access**非统一内存访问架构)，并且必须尊重 CPU 亲和性以获得最大性能。

因此，在多插槽和多轨环境中运行时，DAOS 服务必须能够检测 CPU 到 PCIe 设备和持久内存亲和性，并尽可能减少非本地访问(尽量本地访问)。这可以通过为每个 CPU 生成一个 I/O 引擎实例来实现，然后从该引擎实例实例仅访问该 CPU 本地的持久内存和 PCI 设备。DAOS 控制平面负责检测存储和网络亲和性并相应地启动 I/O 引擎。

## 故障域[¶](https://docs.daos.io/v2.0/admin/hardware/#fault-domains)

DAOS 依赖于大规模分布在不同存储节点上的单端口存储。因此，每个存储节点都是单点故障。DAOS 通过在不同故障域中的存储节点之间提供数据冗余来实现容错。

DAOS 假设故障域是分层的并且不重叠。例如，故障域的第一级可以是机架，第二级可以是存储节点。

为了高效放置和优化数据弹性，故障域越多越好。因此，最好将存储节点分布在尽可能多的机架上。



# 部署前清单[¶](https://docs.daos.io/v2.0/admin/predeployment_check/#pre-deployment-checklist)

本节介绍在部署 DAOS 之前在计算和存储节点上所需的初步设置。

## 启用 IOMMU[¶](https://docs.daos.io/v2.0/admin/predeployment_check/#enable-iommu)

为了使用 NVMe 设备以非 root 用户身份运行 DAOS 服务器，硬件必须支持虚拟化设备访问，并且必须在系统 BIOS 中启用。在英特尔® 系统上，此功能称为英特尔® 定向 I/O 虚拟化技术 (VT-d)。一旦在 BIOS 中启用，IOMMU 支持也必须在 Linux 内核中启用。确切的细节取决于分布，但以下示例应该是说明性的：

```bash
# Enable IOMMU on CentOS 7 and EL 8
# All commands must be run as root/sudo!

$ sudo vi /etc/default/grub # add the following line:
GRUB_CMDLINE_LINUX_DEFAULT="intel_iommu=on"

# after saving the file, run the following to reconfigure
# the bootloader:
$ sudo grub2-mkconfig --output=/boot/grub2/grub.cfg

# if the command completed with no errors, reboot the system
# in order to make the changes take effect
$ sudo reboot
```

笔记

要强制 SPDK 在 daos_server 运行时使用 UIO 而不是 VFIO，请在[服务器配置文件](https://github.com/daos-stack/daos/blob/master/utils/config/daos_server.yml#L109)中设置“disable_vfio” ，但请注意，这需要以 root 身份运行 daos_server。

警告

如果未在 RHEL 8.x 和衍生版本上启用 VFIO，您将遇到如下所述的问题：https://github.com/spdk/spdk/issues/1153

该问题在内核日志中显示为以下签名：

```
[82734.333834] genirq: Threaded irq requested with handler=NULL and !ONESHOT for irq 113 [82734.341761] uio_pci_generic: probe of 0000:18:00.0 failed with error -22
```

因此，由于不支持 UIO，因此需要在这些发行版上使用 VFIO。

## 时间同步[¶](https://docs.daos.io/v2.0/admin/predeployment_check/#time-synchronization)

DAOS 事务模型依赖于时间戳，需要时间在所有存储节点之间进行同步。这可以使用 NTP 或任何其他等效协议来完成。

## 用户和组管理[¶](https://docs.daos.io/v2.0/admin/predeployment_check/#user-and-group-management)

### 服务器上的 DAOS 用户/组[¶](https://docs.daos.io/v2.0/admin/predeployment_check/#daos-usergroups-on-the-servers)

`daos_server`和`daos_engine`进程在非特权用户 ID 下运行`daos_server`。如果在安装 RPM 时该用户不存在，则`daos-server`该用户将作为 RPM 安装的一部分创建。`daos-server`还将创建一个组作为其主要组，`daos_metrics`以及另外两个`daos_daemons`将`daos_server`添加用户的组。

如果创建用户和组有特定于站点的规则，建议在安装 RPM*之前按照特定于站点的约定创建这些用户和组。*`daos-server`

### 客户端上的 DAOS 用户/组[¶](https://docs.daos.io/v2.0/admin/predeployment_check/#daos-usergroups-on-the-clients)

该`daos_agent`进程在非特权用户 ID 下运行`daos_agent`。如果在安装 RPM 时该用户不存在，则`daos-client`该用户将作为 RPM 安装的一部分创建。`daos-agent`还将创建一个组作为其主要组，以及`daos_daemons`将`daos_agent`添加用户的附加组。

如果创建用户和组有特定于站点的规则，建议在安装 RPM*之前按照特定于站点的约定创建这些用户和组。*`daos-client`

### 最终用户的用户/组同步[¶](https://docs.daos.io/v2.0/admin/predeployment_check/#usergroup-synchronization-for-end-users)

用于池和容器的 DAOS ACL 存储实际的用户名和组名（而不是数字 ID）。因此，服务器不需要访问同步的用户/组数据库。DAOS 代理（在客户端节点上运行）负责将用户的 UID/GID 解析为用户/组名，然后将其添加到签名凭证中并发送到 DAOS 存储节点。

## 设置多个网络连接[¶](https://docs.daos.io/v2.0/admin/predeployment_check/#setup-for-multiple-network-links)

使用 libfabric，不支持将多个网络链接聚合（或条带化）作为单个端点。所以单个 DAOS 引擎只能使用单个网络链接。但是当一个存储节点运行多个引擎实例时，每个引擎实例可以并且应该管理自己的网络链路以实现最佳性能。

### 子网[¶](https://docs.daos.io/v2.0/admin/predeployment_check/#subnet)

由于所有引擎都需要能够通信，因此不同的网络接口必须位于同一子网中，或者您必须配置跨不同子网的路由。

### 无限带宽设置[¶](https://docs.daos.io/v2.0/admin/predeployment_check/#infiniband-settings)

需要一些特殊配置来配置 IP 层并在同一个 IP 网络中将 librdmacm 与多个接口一起使用。

首先，必须在 DAOS 使用的网络接口上启用 accept_local 功能。这可以使用以下命令完成：

```
$ sudo sysctl -w net.ipv4.conf.all.accept_local=1
```

其次，必须将 Linux 配置为仅在 ARP 请求的目标接口上发送 ARP 回复。这是通过 arp_ignore 参数配置的。如果客户端和存储节点上的所有 IPoIB 接口都在同一个逻辑子网中（例如 ib0 == 10.0.0.27, ib1 == 10.0.1.27, prefix=16），则该值应设置为 2。

```
$ sysctl -w net.ipv4.conf.all.arp_ignore=2
0 - (default): reply for any local target IP address, configured on any interface 回复所有
1 - reply only if the target IP address is local address, configured on the incoming interface
2 - reply only if the target IP address is local address, configured on the incoming interface and both with the  sender's IP address are part from same subnet on this interface 仅当目标 IP 地址是本地地址时才回复，在传入接口上配置，并且发送者的 IP 地址都来自该接口上的同一子网
```

如果使用单独的逻辑子网（例如前缀 = 24），则该值必须设置为 1。

```
$ sysctl -w net.ipv4.conf.all.arp_ignore=1
```

最后，在几个发行版（例如在 CentOS 7 和 EL 8 上），rp_filter 默认设置为 1，应该设置为 0 或 2，其中 2 更安全。即使配置使用单个逻辑子网也是如此。必须替换为接口名称）

```
$ sysctl -w net.ipv4.conf.<ifaces>.rp_filter=2
rp_filter参数用于控制系统是否开启对数据包源地址的校验
0：不开启源地址校验。
1：开启严格的反向路径校验。对每个进来的数据包，校验其反向路径是否是最佳路径。如果反向路径不是最佳路径，则直接丢弃该数据包。
2：开启松散的反向路径校验。对每个进来的数据包，校验其源地址是否可达，即反向路径是否能通（通过任意网口），如果反向路径不同，则直接丢弃该数据包
```

通过在 /etc/sysctl.d 下添加一个带有所有相关设置的新 sysctl 文件（例如 /etc/sysctl.d/95-daos-net.conf），所有这些参数都可以在 /etc/sysctl.conf 中持久化。

有关详细信息，请参阅[librdmacm 文档](https://github.com/linux-rdma/rdma-core/blob/master/Documentation/librdmacm.md)

## 从源码安装[¶](https://docs.daos.io/v2.0/admin/predeployment_check/#install-from-source)

当 DAOS 从源代码（而不是预构建的包）安装时，需要在本节中详细说明的额外手动设置。

### 运行时目录设置[¶](https://docs.daos.io/v2.0/admin/predeployment_check/#runtime-directory-setup)

DAOS 使用一系列 Unix 域套接字在其各个组件之间进行通信。在现代 Linux 系统上，Unix 域套接字通常存储在 /run 或 /var/run（通常是 /run 的符号链接）下，并且是一个挂载的 tmpfs 文件系统。有几种方法可以确保设置必要的目录。

在启动 daos_server 或 daos_agent 时，可能会错过这一步的迹象，您可能会看到以下消息：

```bash
$ mkdir /var/run/daos_server: permission denied
Unable to create socket directory: /var/run/daos_server
```

#### 非默认目录[¶](https://docs.daos.io/v2.0/admin/predeployment_check/#non-default-directory)

默认情况下，daos_server 和 daos_agent 将分别使用目录 /var/run/daos_server 和 /var/run/daos_agent。要更改 daos_server 用于其运行时目录的默认位置，请取消注释并设置 /etc/daos/daos_server.yml 中的 socket_dir 配置值。对于 daos_agent，可以取消注释并在 /etc/daos/daos_agent.yml 中设置 runtime_dir 配置值，或者可以使用 --runtime_dir 标志 ( `daos_agent -d /tmp/daos_agent`) 在命令行上传递位置。

警告

在控制下运行时不要更改这些`systemd`。如果需要更改这些目录，请确保它们与 /usr/lib/systemd/system/daos_agent.service 和 /usr/lib/systemd/system/daos_server.service 配置文件中的 RuntimeDirectory 设置相匹配。套接字目录将`systemd`在服务启动和停止时创建和删除。

#### 默认目录（非持久性）[¶](https://docs.daos.io/v2.0/admin/predeployment_check/#default-directory-non-persistent)

在 /run 和 /var/run 中创建的文件和目录只能保留到下一次重新启动。后续运行需要这些目录；因此，如果不经常重新启动，在仍然使用默认位置的同时，一个简单的解决方案是手动创建所需的目录。为此，请执行以下命令。

daos_server:

```bash
$ mkdir /var/run/daos_server
$ chmod 0755 /var/run/daos_server
$ chown user:user /var/run/daos_server (where user is the user you
    will run daos_server as)
```

daos_agent：

```bash
$ mkdir /var/run/daos_agent
$ chmod 0755 /var/run/daos_agent
$ chown user:user /var/run/daos_agent (where user is the user you
    will run daos_agent as)
```

#### 默认目录（永久）[¶](https://docs.daos.io/v2.0/admin/predeployment_check/#default-directory-persistent)

如果 DAOS 是从 rpm 安装的，则不需要以下步骤。

如果服务器托管`daos_server`或`daos_agent`将经常重新启动，systemd 提供了一种持久机制来创建称为 tmpfiles.d 的所需目录。每次配置系统时都需要此机制，并且需要重新启动才能生效。

告诉 systemd 为 DAOS 创建必要的目录：

- 复制配置文件： `cp utils/systemd/daosfiles.conf /etc/tmpfiles.d`
- 修改复制的文件将用户和组字段（当前为daos）更改为用户daos将运行为
- 重新启动系统，目录将在所有后续重新启动时自动创建。

### 特权助手[¶](https://docs.daos.io/v2.0/admin/predeployment_check/#privileged-helper)

DAOS 使用特权帮助程序二进制文件 ( `daos_admin`) 来代表 执行需要提升特权的任务`daos_server`。

从 RPM 安装 DAOS 时，`daos_admin`助手会自动安装到具有正确权限的正确位置。RPM 创建一个`daos_server` 系统组并配置权限，以便`daos_admin`只能从`daos_server`.

对于非 RPM 安装，有两种支持的方案：

1. `daos_server`以 root 身份运行，这意味着它`daos_admin`也以 root 身份调用，因此不需要额外的设置。
2. `daos_server`以非root用户身份运行，这意味着`daos_admin`必须手动安装和配置。

启用第二种方案的步骤如下（假设步骤用尽了可能位于 NFS 共享上的 DAOS 源树）：

```bash
$ chmod -x $daospath/bin/daos_admin # prevent this copy from being executed
$ sudo cp $daospath/bin/daos_admin /usr/bin/daos_admin
$ sudo chmod 4755 /usr/bin/daos_admin # make this copy setuid root
$ sudo mkdir -p /usr/share/daos/control # create symlinks to SPDK scripts
$ sudo ln -sf $daospath/share/daos/control/setup_spdk.sh \
           /usr/share/daos/control
$ sudo mkdir -p /usr/share/spdk/scripts
$ sudo ln -sf $daospath/share/spdk/scripts/setup.sh \
           /usr/share/spdk/scripts
$ sudo ln -sf $daospath/share/spdk/scripts/common.sh \
           /usr/share/spdk/scripts
$ sudo ln -s $daospath/include \
           /usr/share/spdk/include
```

笔记

RPM 安装是生产场景的首选。手动安装最适合开发和部署前的概念验证方案。

### 内存锁定限制[¶](https://docs.daos.io/v2.0/admin/predeployment_check/#memory-lock-limits)

memlock 的低 ulimit 可能会导致 SPDK 失败并发出以下错误：

```bash
daos_engine:1 EAL: cannot set up DMA remapping, error 12 (Cannot allocate
    memory)
```

`daos_server`memlock 限制仅在不作为 systemd 服务运行时才需要手动调整。默认 ulimit 设置因操作系统而异。

[对于 RPM 安装，该服务通常由 systemd 启动，并且限制在单元文件](https://github.com/daos-stack/daos/blob/master/utils/systemd/daos_server.service)中预设为无限制`daos_server.service`

请注意，设置的值`/etc/security/limits.conf`会被 systemd 启动的服务忽略。

对于直接从命令行启动的非 RPM 安装`daos_server`（包括源代码构建），应根据本文调整限制 `/etc/security/limits.conf`（ [这](https://access.redhat.com/solutions/61334)是 RHEL 特定文档，但说明适用于大多数 Linux 发行版）。



# 系统部署[¶](https://docs.daos.io/v2.0/admin/deployment/#system-deployment)

DAOS 部署工作流程需要尽早启动 DAOS 服务器实例，以使管理员能够通过 dmg 管理实用程序跨多个存储节点并行执行远程操作。通过使用证书来保证安全性。安装后运行的第一类命令包括网络和存储硬件配置，通常从登录节点运行。

在`daos_server`每个存储节点上第一次启动实例后，`daos_server storage prepare --scm-only`将在每个主机上运行时将 PMem 存储设置为与 DAOS 一起使用的必要状态。然后`dmg storage format`在存储节点上格式化持久性存储设备（在服务器配置文件中指定），并在启动跨架构运行的 DAOS I/O 引擎进程之前写入必要的元数据。

综上所述，一个 DAOS 系统部署的典型工作流程包括以下步骤：

- 配置并启动[DAOS 服务器](https://docs.daos.io/v2.0/admin/deployment/#daos-server-setup)。
- 通过 dmg 实用程序在所有存储节点上[配置硬件。](https://docs.daos.io/v2.0/admin/deployment/#hardware-provisioning)
- [格式化](https://docs.daos.io/v2.0/admin/deployment/#storage-formatting)DAOS系统
- 在客户端节点上[设置并启动代理](https://docs.daos.io/v2.0/admin/deployment/#agent-setup)
- [验证](https://docs.daos.io/v2.0/admin/deployment/#system-validation)DAOS 系统是否正常运行

请注意，如果在 systemd 中注册了启动脚本，则可以在启动时自动启动 DAOS 服务器实例。

以下小节将更详细地介绍每个步骤。

## DAOS 服务器设置[¶](https://docs.daos.io/v2.0/admin/deployment/#daos-server-setup)

首先，应该启动 DAOS 服务器以允许通过 dmg 工具执行远程管理命令。本节介绍最小 DAOS 服务器配置以及如何在所有存储节点上启动它。

### 示例 RPM 部署工作流程[¶](https://docs.daos.io/v2.0/admin/deployment/#example-rpm-deployment-workflow)

启动和运行的推荐工作流程如下：

- 安装 DAOS 服务器 RPM - `daos_server`systemd 服务将以侦听模式启动，这意味着 DAOS I/O 引擎进程将不会启动，因为服务器配置文件（默认位置位于`/etc/daos/daos_server.yml`）尚未填充。
- `dmg config generate -l <hostset> -a <access_points>`跨整个主机集运行（`daos_server`安装 RPM 后现在正在运行服务的所有存储服务器）。只有在所有主机上的硬件设置都相似并且已获得合理的 NUMA 映射时，该命令才会生成配置。调整主机组，直到您拥有一组具有同质硬件配置的主机。
- 一旦可以生成推荐的配置文件，将其复制到`/etc/daos/daos_server.yml`每个 DAOS 服务器主机上的服务器配置文件默认位置 ( ) 并重新启动所有`daos_server`服务。重新启动服务的示例命令是 `clush -w machines-[118-121,130-133] "sudo systemctl restart daos_server"`. 服务应在重新启动时提示格式化，并且在从 触发格式化后`dmg`，应启动 DAOS I/O 引擎进程。

### 服务器配置文件[¶](https://docs.daos.io/v2.0/admin/deployment/#server-configuration-file)

`daos_server`启动`daos_server` 进程时会解析配置文件。可以在命令行上指定配置文件位置（`daos_server -h`以供使用），否则将从默认位置（`/etc/daos/daos_server.yml`）读取。

参数描述 在[示例](https://github.com/daos-stack/daos/tree/master/utils/config/examples/)[`daos_server.yml`](https://github.com/daos-stack/daos/blob/master/utils/config/daos_server.yml)目录 中的示例配置文件中 指定 。

作为命令行选项或标志提供的任何选项`daos_server`都将优先于等效的配置文件参数。

为方便起见，将活动解析的配置值写入临时文件以供参考，并将位置写入日志。

#### 配置选项[¶](https://docs.daos.io/v2.0/admin/deployment/#configuration-options)

示例配置文件列出了默认的空配置，列出了所有选项（配置文件的实时文档）。提供了实时[示例](https://github.com/daos-stack/daos/tree/master/utils/config/examples/) 。

此配置文件的位置是通过首先检查通过`daos_server`命令行的 -o 选项指定的路径来确定的，如果未指定则`/etc/daos/daos_server.yml`使用。

有关最新信息和示例，请参阅示例配置文件 [`daos_server.yml`](https://github.com/daos-stack/daos/blob/master/utils/config/daos_server.yml) 。

在此过程中，yaml 文件的 servers: 和 provider: 部分可以留空，并将在后续部分中填充。

#### 自动生成配置文件[¶](https://docs.daos.io/v2.0/admin/deployment/#auto-generate-configuration-file)

DAOS 可以尝试通过“dmg config generate”命令生成一个服务器配置文件，以优化使用给定主机集上的硬件：

```bash
$ dmg config generate --help
ERROR: dmg: Usage:
  dmg [OPTIONS] config generate [generate-OPTIONS]

Application Options:
...
  -l, --host-list=  comma separated list of addresses <ipv4addr/hostname>
...

[generate command options]
      -a, --access-points=                                 Comma separated list of access point
                                                           addresses <ipv4addr/hostname>
      -e, --num-engines=                                   Set the number of DAOS Engine sections to be
                                                           populated in the config file output. If unset
                                                           then the value will be set to the number of
                                                           NUMA nodes on storage hosts in the DAOS
                                                           system.
      -s, --min-ssds=                                      Minimum number of NVMe SSDs required per DAOS
                                                           Engine (SSDs must reside on the host that is
                                                           managing the engine). Set to 0 to generate a
                                                           config with no NVMe. (default: 1)
      -c, --net-class=[best-available|ethernet|infiniband] Network class preferred (default:
                                                           best-available)
```

如果满足提供的要求，该命令将输出推荐的配置文件。如果未在命令行上指定“--num-engines”，则将根据主机上存在的 NUMA 节点数得出要求。

- '--num-engines' 指定要在配置文件输出中填充的引擎部分的数量。除了绑定到同一个 NUMA 节点的结构网络接口和 SSD 之外，每个部分都将指定主机上必须存在的持久内存 (PMem) 块设备。如果未在命令行上显式设置，则默认为主机上检测到的 NUMA 节点数。
- '--min-ssds' 指定每个主机上需要存在的每个引擎的 NVMe SSD 的最小数量。对于生成的配置中的每个引擎条目，至少必须将这个数量的 SSD 绑定到与引擎关联的 PMem 设备和结构网络接口的亲和性相匹配的 NUMA 节点。如果未在命令行上设置，默认为“1”。如果设置为“0”，则不会将 NVMe SSD 添加到生成的配置中，并且将禁用 SSD 验证。
- '--net-class' 指定网络接口类的首选项，选项为'ethernet'、'infiband' 或'best-available'。'best-available' 将尝试选择性能最高（由 libfabric 判断）的接口集和受支持的提供程序，以匹配 PMem 设备的数量和 NUMA 亲和性。如果未在命令行上设置，默认为“最佳可用”。

由命令生成并输出到 stdout 的配置文件可以复制到文件中并在相关主机上使用，并用作服务器配置以确定“daos_server”实例的启动环境。

以下情况不会产生配置文件输出：

- PMem 设备计数、容量或 NUMA 映射在主机列表中的任何主机上都不同（主机列表可以在“dmg”配置文件或命令行中指定）。
- NVMe SSD 计数、PCI 地址分布或 NUMA 亲和性在主机列表中的任何主机上都不同。
- 无法在主机上检测到 NUMA 节点计数，或者在主机列表中的任何主机上都不同。
- PMem 设备计数或 NUMA 关联性不满足“num-engines”要求。
- NVMe 设备计数或 NUMA 关联性不满足“min-ssds”要求。
- 网络设备计数或 NUMA 关联与配置的 PMem 设备不匹配，考虑到任何指定的网络设备类首选项（以太网或 InfiniBand）。

已知 7.9 版本之前的一些 CentOS 7.x 内核存在缺陷，无法`ndctl`报告命名空间的 NUMA 亲和性，请参见[此处](https://github.com/pmem/ndctl/issues/130)。`dmg config generate` 这可以防止在使用上述受影响的内核之一运行时生成双引擎配置。

#### 证书配置[¶](https://docs.daos.io/v2.0/admin/deployment/#certificate-configuration)

除了加密 DAOS 控制平面通信之外，DAOS 安全框架还依赖证书来验证组件和管理员的身份。`gen_certificates.sh`如果没有现有的 TLS 证书基础设施，则可以通过运行随 DAOS 软件提供的脚本来生成给定 DAOS 系统的一组证书。该 `gen_certificates.sh`脚本使用该`openssl`工具生成所有必要的文件。我们强烈建议使用 OpenSSL 版本 1.1.1h 或更高版本，因为使用早期版本生成的密钥和证书容易受到攻击。

从 RPM 安装 DAOS 时，此脚本在基本`daos`RPM 中提供，并且可以在将写入证书的目录中调用。作为生成过程的一部分，创建了一个新的本地证书颁发机构来处理证书签名，并创建了三个角色证书：

```bash
# /usr/lib64/daos/certgen/gen_certificates.sh
Generating Private CA Root Certificate
Private CA Root Certificate created in ./daosCA
...
Generating Server Certificate
Required Server Certificate Files:
        ./daosCA/certs/daosCA.crt
        ./daosCA/certs/server.key
        ./daosCA/certs/server.crt
...
Generating Agent Certificate
Required Agent Certificate Files:
        ./daosCA/certs/daosCA.crt
        ./daosCA/certs/agent.key
        ./daosCA/certs/agent.crt
...
Generating Admin Certificate
Required Admin Certificate Files:
        ./daosCA/certs/daosCA.crt
        ./daosCA/certs/admin.key
        ./daosCA/certs/admin.crt
```

在 ./daosCA 下生成的文件应受到保护，防止未经授权的访问并保留以备将来使用。

然后必须将生成的密钥和证书安全地分发到参与 DAOS 系统的所有节点（服务器、客户端和管理节点）。应设置这些文件的权限，以防止未经授权访问密钥和证书。

客户端节点需要：

- CA 根证书
- 代理证书
- 代理密钥

管理节点需要：

- CA 根证书
- 管理员证书
- 管理员密钥

服务器节点需要：

- CA 根证书
- 服务器证书
- 服务器密钥
- DAOS 系统中所有有效的代理证书（在客户端证书目录中，请参阅下面的配置文件）

安全分发证书后，必须更新 DAOS 配置文件以启用身份验证和安全通信。这些示例假定配置和证书文件已安装在`/etc/daos`：

```yaml
# /etc/daos/daos_server.yml (servers)

transport_config:
  # Location where daos_server will look for Client certificates
  client_cert_dir: /etc/daos/certs/clients
  # Custom CA Root certificate for generated certs
  ca_cert: /etc/daos/certs/daosCA.crt
  # Server certificate for use in TLS handshakes
  cert: /etc/daos/certs/server.crt
  # Key portion of Server Certificate
  key: /etc/daos/certs/server.key
# /etc/daos/daos_agent.yml (clients)

transport_config:
  # Custom CA Root certificate for generated certs
  ca_cert: /etc/daos/certs/daosCA.crt
  # Agent certificate for use in TLS handshakes
  cert: /etc/daos/certs/agent.crt
  # Key portion of Agent Certificate
  key: /etc/daos/certs/agent.key
# /etc/daos/daos_control.yml (dmg/admin)

transport_config:
  # Custom CA Root certificate for generated certs
  ca_cert: /etc/daos/certs/daosCA.crt
  # Admin certificate for use in TLS handshakes
  cert: /etc/daos/certs/admin.crt
  # Key portion of Admin Certificate
  key: /etc/daos/certs/admin.key
```

### 服务器启动[¶](https://docs.daos.io/v2.0/admin/deployment/#server-startup)

DAOS 服务器作为 systemd 服务启动。从 RPM 安装时，DAOS 服务器单元文件安装在正确的位置。DAOS 服务器将以用户身份运行，`daos-server`该用户将在 RPM 安装期间创建。

如果您希望在开发版本中使用 systemd，则必须将服务文件从 utils/systemd 复制到`/usr/lib/systemd/system`. 复制文件后，修改 ExecStart 行以指向您的`daos_server`二进制文件。

修改 ExecStart 后，运行以下命令：

```bash
$ sudo systemctl daemon-reload
```

安装服务文件后，您可以`daos_server` 使用以下命令开始：

```bash
$ systemctl enable daos_server.service
$ systemctl start daos_server.service
```

要检查组件状态，请使用：

```bash
$ systemctl status daos_server.service
```

如果 DAOS 服务器启动失败，请检查日志：

```bash
$ journalctl --unit daos_server.service
```

RPM 安装后，`daos_server`服务开始以用户“daos”自动运行。读取服务器配置并从中读取`/etc/daos/daos_server.yml`证书`/etc/daos/certs`。除了加载证书之外没有其他管理员干预， `daos_server`将进入侦听状态，从而通过该`dmg`工具发现存储和网络硬件，而无需在配置文件中指定任何 I/O 引擎。在设备发现和配置之后，可以将具有填充的每个引擎部分的更新配置文件存储在 中 `/etc/daos/daos_server.yml`，并且在重新启动`daos_server`服务后，它就可以准备好对存储进行格式化。

## DAOS 服务器远程访问[¶](https://docs.daos.io/v2.0/admin/deployment/#daos-server-remote-access)

DAOS 系统和单个 DAOS 服务器进程的远程任务可以通过该`dmg`实用程序执行。

要设置要执行任务的 DAOS 服务器的地址，请提供：

- `-l <hostlist>`调用时在命令行上，或
- `hostlist: <hostlist>`在控制配置文件中 [`daos_control.yml`](https://github.com/daos-stack/daos/blob/master/utils/config/daos_control.yml)

where`<hostlist>`表示 slurm 风格的主机列表字符串，例如 `foo-1[28-63],bar[256-511]`. 主机列表中的第一个条目（按字母排序，然后按数字排序）将被假定为服务器配置文件中设置的接入点。

存储在用户目录中的本地配置文件将优先使用默认位置，例如`~/.daos_control.yml`.

## 硬件配置[¶](https://docs.daos.io/v2.0/admin/deployment/#hardware-provisioning)

一旦 DAOS 服务器启动，就可以通过 dmg 实用程序在存储节点上配置存储和网络。

笔记

`daos_server storage`命令不支持配置，这意味着它们不会从服务器配置文件中读取参数。

### 单片机准备[¶](https://docs.daos.io/v2.0/admin/deployment/#scm-preparation)

本节介绍如何验证 PMem（英特尔(R) Optane(TM) 持久内存）模块是否正确安装在存储节点上，以及如何将它们配置为交错模式以供 DAOS 使用。未来可能会涵盖其他类型的 SCM 的说明。

通过在特定套接字 (NUMA) 本地的模块组中配置交错内存区域（交错模式）中的 PMem 模块来提供 SCM，并且结果 PMem 名称空间由设备标识符（例如，/dev/pmem0）定义。

每次 DAOS 安装都需要一次 PMem 准备。

此步骤需要重新启动以启用 BIOS 读取 PMem 资源分配更改。

PMem 准备可以使用`daos_server storage prepare --scm-only`.

第一次运行该命令时，将创建 SCM 交错区域作为任何可用 PMem 模块上的资源分配（每个 NUMA 节点/套接字一个区域）。这些区域在 BIOS 读取新的资源分配后被激活。完成后，存储准备命令将提示管理员重新启动存储节点，以便 BIOS 激活新的存储分配。storage prepare 命令本身不会启动重新引导。

运行命令后需要重新启动，然后需要再次运行命令以公开 DAOS 使用的命名空间设备。

示例用法：

- `clush -w wolf-[118-121,130-133] daos_server storage prepare --scm-only` 运行后，应提示用户重新启动。
- `clush -w wolf-[118-121,130-133] reboot`
- `clush -w wolf-[118-121,130-133] daos_server storage prepare --scm-only` 运行后，PMem 设备（在新 SCM 区域上创建的 /dev/pmemX 命名空间）应该在每个主机上都可用。

在第二次运行时，每个区域创建一个命名空间，每个命名空间可能需要几分钟才能创建。pmem 设备的详细信息将在命令完成时以 JSON 格式显示。

成功创建 pmem 设备后，英特尔(R) 傲腾(TM) 持久内存已配置好，可以继续下一步。

如果需要，可以使用命令销毁 pmem 设备 `daos_server storage prepare --scm-only --reset`。

所有命名空间都被禁用和销毁。通过资源分配将模块重置为“MemoryMode”来移除 SCM 区域。

请注意，如果在运行 reset 之前挂载了命名空间/pmem 内核设备（根据打印的警告），可能会导致未定义的行为。

BIOS 需要随后重新启动才能读取新的资源分配。

### 存储发现和选择[¶](https://docs.daos.io/v2.0/admin/deployment/#storage-discovery-and-selection)

本节介绍如何手动检测和选择 DAOS 使用的存储设备。服务器配置文件使管理员能够控制存储选择。

笔记

`daos_server storage`命令不支持配置，这意味着它们不会从服务器配置文件中读取参数。

#### 发现[¶](https://docs.daos.io/v2.0/admin/deployment/#discovery)

`dmg storage scan`可以运行以`daos_server` 通过管理网络查询远程运行的进程。

`daos_server storage scan`可用于`daos_server`直接查询本地实例（扫描本地连接的 SSD 和 DAOS 可用的 Intel Persistent Memory Modules）。NVMe SSD 需要先通过运行才能访问 `daos_server storage prepare --nvme-only -u <current_user`。输出将等效于`dmg storage scan --verbose`远程运行。

```bash
bash-4.2$ dmg storage scan
Hosts        SCM Total             NVMe Total
-----        ---------             ----------
wolf-[71-72] 6.4 TB (2 namespaces) 3.1 TB (3 controllers)

bash-4.2$ dmg storage scan --verbose
------------
wolf-[71-72]
------------
SCM Namespace Socket ID Capacity
------------- --------- --------
pmem0         0         3.2 TB
pmem1         1         3.2 TB

NVMe PCI     Model                FW Revision Socket ID Capacity
--------     -----                ----------- --------- --------
0000:81:00.0 INTEL SSDPED1K750GA  E2010325    1         750 GB
0000:87:00.0 INTEL SSDPEDMD016T4  8DV10171    1         1.6 TB
0000:da:00.0 INTEL SSDPED1K750GA  E2010325    1         750 GB
```

上面的 NVMe PCI 字段应该在服务器配置文件中用于识别 NVMe SSD。

具有相同 NUMA 节点/套接字的设备应在服务器配置文件的相同每个引擎部分中使用，以获得最佳性能。

有关命令用法的更多信息，请运行`dmg storage --help`。

#### 健康[¶](https://docs.daos.io/v2.0/admin/deployment/#health)

SSD 健康状态可以通过以下方式验证`dmg storage scan --nvme-health`：

```bash
bash-4.2$ dmg storage scan --nvme-health
-------
wolf-71
-------
PCI:0000:81:00.0 Model:INTEL SSDPED1K750GA  FW:E2010325 Socket:1 Capacity:750 GB
  Health Stats:
    Timestamp:2021-09-13T11:12:34.000+00:00
    Temperature:318K(44.85C)
    Controller Busy Time:0s
    Power Cycles:15
    Power On Duration:10402h0m0s
    Unsafe Shutdowns:13
    Error Count:0
    Media Errors:0
    Read Errors:0
    Write Errors:0
    Unmap Errors:0
    Checksum Errors:0
    Error Log Entries:0
  Critical Warnings:
    Temperature: OK
    Available Spare: OK
    Device Reliability: OK
    Read Only: OK
    Volatile Memory Backup: OK
  Intel Vendor SMART Attributes:
    Program Fail Count:
       Normalized:100%
       Raw:0
    Erase Fail Count:
       Normalized:100%
       Raw:0
    Wear Leveling Count:
       Normalized:100%
       Min:0
       Max:9
       Avg:3
    End-to-End Error Detection Count:0
    CRC Error Count:0
    Timed Workload, Media Wear:65535
    Timed Workload, Host Read/Write Ratio:65535
    Timed Workload, Timer:65535
    Thermal Throttle Status:0%
    Thermal Throttle Event Count:0
    Retry Buffer Overflow Counter:0
    PLL Lock Loss Count:0
    NAND Bytes Written:222897
    Host Bytes Written:71

PCI:0000:da:00.0 Model:INTEL SSDPED1K750GA  FW:E2010325 Socket:1 Capacity:750 GB
  Health Stats:
    Temperature:320K(46.85C)
    Controller Busy Time:0s
    Power Cycles:15
    Power On Duration:10402h0m0s
    Unsafe Shutdowns:13
    Error Count:0
    Media Errors:0
    Read Errors:0
    Write Errors:0
    Unmap Errors:0
    Checksum Errors:0
    Error Log Entries:0
  Critical Warnings:
    Temperature: OK
    Available Spare: OK
    Device Reliability: OK
    Read Only: OK
    Volatile Memory Backup: OK
  Intel Vendor SMART Attributes:
    Program Fail Count:
       Normalized:100%
       Raw:0
    Erase Fail Count:
       Normalized:100%
       Raw:0
    Wear Leveling Count:
       Normalized:100%
       Min:0
       Max:9
       Avg:3
    End-to-End Error Detection Count:0
    CRC Error Count:0
    Timed Workload, Media Wear:65535
    Timed Workload, Host Read/Write Ratio:65535
    Timed Workload, Timer:65535
    Thermal Throttle Status:0%
    Thermal Throttle Event Count:0
    Retry Buffer Overflow Counter:0
    PLL Lock Loss Count:0
    NAND Bytes Written:222897
    Host Bytes Written:71

-------
wolf-72
-------
PCI:0000:81:00.0 Model:INTEL SSDPED1K750GA  FW:E2010435 Socket:1 Capacity:750 GB
  Health Stats:
    Temperature:316K(42.85C)
    Controller Busy Time:8m0s
    Power Cycles:23
    Power On Duration:10399h0m0s
    Unsafe Shutdowns:18
    Error Count:0
    Media Errors:0
    Read Errors:0
    Write Errors:0
    Unmap Errors:0
    Checksum Errors:0
    Error Log Entries:0
  Critical Warnings:
    Temperature: OK
    Available Spare: OK
    Device Reliability: OK
    Read Only: OK
    Volatile Memory Backup: OK
  Intel Vendor SMART Attributes:
    Program Fail Count:
       Normalized:100%
       Raw:0
    Erase Fail Count:
       Normalized:100%
       Raw:0
    Wear Leveling Count:
       Normalized:100%
       Min:0
       Max:9
       Avg:3
    End-to-End Error Detection Count:0
    CRC Error Count:0
    Timed Workload, Media Wear:65535
    Timed Workload, Host Read/Write Ratio:65535
    Timed Workload, Timer:65535
    Thermal Throttle Status:0%
    Thermal Throttle Event Count:0
    Retry Buffer Overflow Counter:0
    PLL Lock Loss Count:0
    NAND Bytes Written:222897
    Host Bytes Written:71

PCI:0000:da:00.0 Model:INTEL SSDPED1K750GA  FW:E2010435 Socket:1 Capacity:750 GB
  Health Stats:
    Temperature:320K(46.85C)
    Controller Busy Time:1m0s
    Power Cycles:23
    Power On Duration:10399h0m0s
    Unsafe Shutdowns:19
    Error Count:0
    Media Errors:0
    Read Errors:0
    Write Errors:0
    Unmap Errors:0
    Checksum Errors:0
    Error Log Entries:0
  Critical Warnings:
    Temperature: OK
    Available Spare: OK
    Device Reliability: OK
    Read Only: OK
    Volatile Memory Backup: OK
  Intel Vendor SMART Attributes:
    Program Fail Count:
       Normalized:100%
       Raw:0
    Erase Fail Count:
       Normalized:100%
       Raw:0
    Wear Leveling Count:
       Normalized:100%
       Min:0
       Max:9
       Avg:3
    End-to-End Error Detection Count:0
    CRC Error Count:0
    Timed Workload, Media Wear:65535
    Timed Workload, Host Read/Write Ratio:65535
    Timed Workload, Timer:65535
    Thermal Throttle Status:0%
    Thermal Throttle Event Count:0
    Retry Buffer Overflow Counter:0
    PLL Lock Loss Count:0
    NAND Bytes Written:222897
    Host Bytes Written:71
```

#### 选择[¶](https://docs.daos.io/v2.0/admin/deployment/#selection)

下一步包括在服务器配置 YAML 文件中指定 DAOS 应该使用的设备。配置`engines`部分是一个列表，其中包含要在主机上启动的每个 DAOS 引擎的详细信息（将为列表中的每个条目创建一个引擎）。

具有相同 NUMA 等级/节点/套接字的设备应尽可能位于单个 DAOS 引擎上。有关详细信息，请参阅[服务器配置文件部分](https://docs.daos.io/v2.0/admin/deployment/#server-configuration-file)。

为列表中的每个引擎指定存储，`storage`列表中的每个条目定义一个单独的存储层。

每个层都有一个`class`定义存储类型的参数。典型的类值为 PMem（英特尔(R) Optane(TM) 持久内存）的“dcpm”和 NVMe SSD 的“nvme”。

对于 class == "dcpm"，应填充以下参数：

- `scm_list`应该包含 PMem 交错集命名空间（例如`/dev/pmem1`）。目前列表的大小限制为 1。
- `scm_mount`给出所需的本地目录，用作 DAOS 持久存储的挂载点，该存储挂载在 `scm_list`.

对于 class == "nvme"，应填充以下参数：

- `bdev_list`应使用 NVMe PCI 地址填充。

#### 示例配置[¶](https://docs.daos.io/v2.0/admin/deployment/#example-configurations)

为了说明，假设一个具有同质硬件配置的集群从扫描中为每个主机返回以下内容：

```bash
[daos@wolf-72 daos_m]$ dmg -l wolf-7[1-2] storage scan --verbose
-------
wolf-7[1-2]
-------
SCM Namespace Socket ID Capacity
------------- --------- --------
pmem0         0         2.90TB
pmem1         1         2.90TB

NVMe PCI     Model                FW Revision Socket ID Capacity
--------     -----                ----------- --------- --------
0000:81:00.0 INTEL SSDPED1K750GA  E2010325    0         750.00GB
0000:87:00.0 INTEL SSDPEDMD016T4  8DV10171    0         1.56TB
0000:da:00.0 INTEL SSDPED1K750GA  E2010325    1         750.00GB
```

在这种情况下，`engines`可以按如下方式填充配置文件部分以建立 2 层存储：

```yaml
<snip>
port: 10001
access_points: ["wolf-71"] # <----- updated
<snip>
engines:
-
  targets: 16                 # number of I/O service threads per-engine
  first_core: 0               # offset of the first core to bind service threads
  nr_xs_helpers: 0            # count of I/O offload threads
  fabric_iface: eth0          # network interface to use for this engine
  fabric_iface_port: 31416    # network port
  log_mask: ERR               # debug level to start with the engine with
  log_file: /tmp/server1.log  # where to store engine logs
  storage:
  -
    class: dcpm               # type of first storage tier (SCM)
    scm_list: [/dev/pmem0]    # <----- updated
    scm_mount: /mnt/daos0     # where to mount SCM
  -
    class: nvme               # type of second storage tier (NVMe)
    bdev_list: ["0000:87:00.0"] # <----- updated
-
  targets: 16
  first_core: 0
  nr_xs_helpers: 0
  fabric_iface: eth0
  fabric_iface_port: 32416
  log_mask: ERR
  log_file: /tmp/server2.log
  storage:
  -
    class: dcpm               # type of first storage tier (SCM)
    scm_list: [/dev/pmem1]    # <----- updated
    scm_mount: /mnt/daos1     # where to mount SCM
  -
    class: nvme               # type of second storage tier (NVMe)
    bdev_list: ["0000:da:00.0"] # <----- updated
<end>
```

有一些默认情况下未构建的可选提供程序。有关详细信息，请参阅[DAOS 构建文档][6]。

> ***笔记\***
>
> 更新服务器配置文件后，需要在所有主机上重新启动 DAOS 控制服务器。
>
> 在系统中选择一台主机并设置`access_points`为该主机的主机名或 IP 地址列表（不需要指定端口）。这将是引导 DAOS 管理服务 (MS) 的主机。
>
> 可选提供者的支持并不能保证，并且可以在不另行通知的情况下删除。

### 网络配置[¶](https://docs.daos.io/v2.0/admin/deployment/#network-configuration)

#### 网络扫描[¶](https://docs.daos.io/v2.0/admin/deployment/#network-scan)

该`dmg`实用程序支持`network scan`显示每个设备的网络接口、相关的 OFI 结构提供程序和相关的 NUMA 节点的功能。此信息用于为存储节点上的每个 I/O 引擎配置全局结构提供程序和唯一的本地网络接口。本节将帮助您确定为文件中`provider`的 `fabric_iface`和`pinned_numa_node`条目提供什么`daos_server.yml`。

以下命令是典型示例：

```bash
$ dmg network scan
$ dmg network scan -p all
$ dmg network scan -p ofi+sockets
$ dmg network scan --provider 'ofi+verbs;ofi_rxm'
```

在 a`daos_server`尚未完全配置且缺少系统结构提供者声明的早期阶段，查看未过滤的扫描结果列表可能会有所帮助。

`dmg`在早期阶段使用以下任一命令来实现此目标：

```bash
$ dmg network scan
$ dgm network scan -p all
```

典型的网络扫描结果如下所示：

```bash
$ dmg network scan
-------
wolf-29
-------

    -------------
    NUMA Socket 1
    -------------

        Provider    Interfaces
        --------    ----------
        ofi+sockets ib1

---------
localhost
---------

    -------------
    NUMA Socket 0
    -------------

        Provider    Interfaces
        --------    ----------
        ofi+sockets ib0, eth0

    -------------
    NUMA Socket 1
    -------------

        Provider    Interfaces
        --------    ----------
        ofi+sockets ib1
```

使用这些提供程序之一来配置`provider`. `daos_server.yml`整个 DAOS 安装只能指定一个提供程序。客户端节点必须能够`daos_server`通过相同的提供者与节点通信。`daos_server`因此，选择与预期客户端节点配置兼容的网络设置会很有帮助。

`daos_server.yml`文件被编辑并包含提供程序后，后续命令`dmg network scan`将根据该提供程序过滤结果。如果希望再次查看未过滤的列表，请发出`dmg network scan -p all`.

无论`daos_server.yml`文件中的提供者是什么，都可以使用以下命令将结果过滤到指定的提供者：列表`dmg network scan -p ofi_provider`中`ofi_provider`的可用提供者之一。

网络扫描的结果可用于帮助配置 I/O 引擎。

每个 I/O 引擎都配置有唯一`fabric_iface`且可选 的`pinned_numa_node`. 扫描结果中列出的接口和 NUMA 套接字分别映射到 `daos_server.yml` `fabric_iface`和`pinned_numa_node`。的使用`pinned_numa_node`是可选的，但建议使用以获得最佳性能。当使用与网络接口匹配的值指定时，I/O 引擎将自己绑定到该 NUMA 节点以及纯粹在该 NUMA 节点内的核心。此配置产生对该网络设备的最快访问。

#### 更改网络提供商[¶](https://docs.daos.io/v2.0/admin/deployment/#changing-network-providers)

有关网络配置的信息作为元数据存储在 DAOS 存储中。

如果在初始部署后必须更改提供程序，则必须在使用`dmg storage format`新提供程序更新配置文件后重新格式化存储设备。

#### 提供者测试[¶](https://docs.daos.io/v2.0/admin/deployment/#provider-testing)

然后，该`fi_pingpong`测试可用于验证目标 OFI 提供商是否正常工作：

```bash
node1$ fi_pingpong -p psm2

node2$ fi_pingpong -p psm2 ${IP_ADDRESS_NODE1}

bytes #sent #ack total time  MB/sec  usec/xfer Mxfers/sec
64    10    =10  1.2k  0.00s 21.69   2.95      0.34
256   10    =10  5k    0.00s 116.36  2.20      0.45
1k    10    =10  20k   0.00s 379.26  2.70      0.37
4k    10    =10  80k   0.00s 1077.89 3.80      0.26
64k   10    =10  1.2m  0.00s 2145.20 30.55     0.03
1m    10    =10  20m   0.00s 8867.45 118.25    0.01
```

### CPU资源[¶](https://docs.daos.io/v2.0/admin/deployment/#cpu-resources)

I/O 引擎是多线程的，每个引擎应该使用的 I/O 服务线程和辅助线程的数量必须在文件的`engines:`部分中配置`daos_server.yml`。

I/O 服务线程的数量通过`targets:`设置进行配置。每个存储目标管理一小部分（交错的）SCM 存储空间，以及由该引擎管理的一个 NVMe SSD 的一小部分。每个引擎的最佳存储目标数取决于两个条件：

- 为了实现 NVMe 空间的最佳平衡，目标的数量应该是 `bdev_list:`引擎中配置的 NVMe 磁盘数量的整数倍。
- 为了获得最大的 SCM 性能，需要一定数量的目标。这取决于设备和工作负载，但大约 16 个目标通常运行良好。

虽然不是必需的，但建议还使用该设置指定多个 I/O 卸载线程`nr_xs_helpers:`。这些线程可以通过卸载诸如校验和计算之类的活动以及从主 I/O 服务线程分派服务器端 RPC 来提高性能。

服务器应该有足够多的物理内核来支持目标数量以及额外的服务线程。

## 存储格式化[¶](https://docs.daos.io/v2.0/admin/deployment/#storage-formatting)

一旦`daos_server`使用正确的存储设备、网络接口和 CPU 线程重新启动，就可以进入格式化阶段。`daos_server`首次启动时，进入“维护模式”，等待管理`dmg storage format`工具发出呼叫。此远程调用将使用服务器配置文件中定义的参数触发主机上本地附加存储的格式化，以便与 DAOS 一起使用。

`dmg -l <host>[,...] storage format`通常将在登录节点上运行，该节点指定`-l <host>[,...]`存储节点的主机列表 ( )，其中 SCM/PMem 模块和 NVMe SSD 已安装并准备好。

成功格式化后，DAOS 控制服务器将启动在服务器配置文件中指定的 DAOS I/O 引擎。

成功启动由标准输出上的以下内容指示： `DAOS I/O Engine (v2.0.1) process 433456 started on rank 1 with 8 target, 2 helper XS, firstcore 0, host wolf-72.wolf.hpdd.intel.com.`

### 单片机格式[¶](https://docs.daos.io/v2.0/admin/deployment/#scm-format)

运行该命令时，在 SCM/PMem 区域上创建的 pmem 内核设备将根据服务器配置文件中提供的参数进行格式化和挂载。

- `scm_mount`指定要创建的挂载点的位置。
- `scm_class`可以设置为`ram`在没有 SCM/PMem 可用的情况下使用 tmpfs（`scm_size`指示 tmpfs 的大小（以 GB 为单位）），当设置为`dcpm`下面指定的设备时，`scm_list`将安装在`scm_mount`路径上。

### NVMe 格式[¶](https://docs.daos.io/v2.0/admin/deployment/#nvme-format)

运行该命令时，会根据服务器配置文件中提供的参数对 NVMe SSD 进行格式化和设置以供 DAOS 使用。

`bdev_class`可以设置为`nvme`使用带有 SPDK 的实际 NVMe 设备进行 DAOS 存储。其他`bdev_class`值可用于模拟服务器配置文件中指定的 NVMe 存储。 `bdev_list`标识要与 PCI 地址列表一起使用的设备（这可以在查看`storage scan`命令结果后填充）。

运行 format 命令后，服务器配置文件`scm_mount`参数指定的路径应该被挂载，并且应该包含一个名为 `daos_nvme.conf`. `bdev_list`该文件应描述具有服务器配置文件参数中列出的 PCI 地址的设备 。该文件的存在和内容表明指定的 NVMe SSD 已正确配置为与 DAOS 一起使用。

服务器配置文件参数中列出的 NVMe SSD 的内容`bdev_list` 将在格式化时重置。

### 服务器格式[¶](https://docs.daos.io/v2.0/admin/deployment/#server-format)

`scm_mount`在运行 format 命令之前，服务器配置文件中参数指定的路径下不应存在 DAOS 元数据。

运行命令后`storage format`，服务器配置文件`scm_mount`参数指定的路径应该被挂载，并且应该包含必要的 DAOS 元数据，指示服务器已被格式化。

启动时，`daos_server`如果`maintenance mode`在`scm_mount`.

## 代理设置[¶](https://docs.daos.io/v2.0/admin/deployment/#agent-setup)

本节介绍如何在客户端节点上配置 DAOS 代理。

### 代理用户和组设置[¶](https://docs.daos.io/v2.0/admin/deployment/#agent-user-and-group-setup)

守护`daos_agent`进程作为非特权用户进程运行，使用用户名`daos_agent`和组名`daos_agent`。如果在安装 RPM 时这些 ID 不存在`daos-client`，它们将在 RPM 安装期间创建。有关详细信息，请参阅 [用户和组管理](https://docs.daos.io/v2.0/admin/predeployment_check/#user-and-group-management) 。

### 代理证书生成[¶](https://docs.daos.io/v2.0/admin/deployment/#agent-certificate-generation)

DAOS 安全框架依赖 SSL 证书来验证 DAOS 守护进程以及运行该 `dmg`命令的 DAOS 管理员。有关创建必要证书的详细信息，请参阅[证书生成](https://docs.daos.io/v2.0/admin/deployment/?h=gen_#certificate-configuration) 。

笔记

出于测试目的，可以禁用证书。这不应该*在*生产环境中完成。在不安全模式下运行将允许任意未经身份验证的用户进程访问并可能损坏 DAOS 存储。

### 代理配置文件[¶](https://docs.daos.io/v2.0/admin/deployment/#agent-configuration-file)

```
daos_agent`启动 `daos_agent`进程时会解析配置文件。可以在命令行上使用`-o`选项指定配置文件位置（请参阅`daos_agent -h`使用帮助）。否则，将使用默认位置`/etc/daos/daos_agent.yml`。在 systemd 控制下运行时，除非修改文件中的行以包含该选项`daos_agent`，否则将使用默认位置。`ExecStart``daos_agent.service``-o
```

参数说明在示例 [daos_agent.yml](https://github.com/daos-stack/daos/blob/master/utils/config/daos_agent.yml)`/etc/daos/daos_agent.yml`文件中指定， 在安装 daos-client RPM 期间 也会安装到该文件中。

作为命令行选项或标志提供的任何选项`daos_agent`都将优先于等效的配置文件参数。

以下部分列出了配置文件中可用的格式、选项、默认值和描述。

#### 手动定义结构接口[¶](https://docs.daos.io/v2.0/admin/deployment/#defining-fabric-interfaces-manually)

默认情况下，DAOS 代理会自动检测客户端节点上的所有结构接口。它根据客户端请求的 NUMA 节点和 DAOS 管理服务上报的接口类型偏好选择一个合适的 DAOS I/O。

如果 DAOS Agent 没有正确检测到 Fabric 接口，管理员可以在 Agent 配置文件中手动定义它们。这些`fabric_iface`条目必须由 NUMA 节点组织。如果使用动词提供者，还需要接口域。

例子：

```
fabric_ifaces:
-
  numa_node: 0
  devices:
  -
    iface: ib0
    domain: mlx5_0
  -
    iface: ib1
    domain: mlx5_1
-
  numa_node: 1
  devices:
  -
    iface: ib2
    domain: mlx5_2
  -
    iface: ib3
    domain: mlx5_3
```

### 代理启动[¶](https://docs.daos.io/v2.0/admin/deployment/#agent-startup)

DAOS 代理是在每个客户端节点上运行的独立应用程序。默认情况下，DAOS 代理将作为 systemd 服务运行。`/usr/lib/systemd/system/daos_agent.service`在 RPM 安装期间，DAOS 代理单元文件安装在正确的位置 ( )。

在 RPM 安装后，并且在创建了 Agent 配置文件之后，以下命令将使 DAOS Agent 在下次重新启动时启动，将立即启动它，并在启动后检查 Agent 的状态：

```bash
$ sudo systemctl enable daos_agent.service
$ sudo systemctl start  daos_agent.service
$ sudo systemctl status daos_agent.service
```

如果 DAOS 代理无法启动，请检查 systemd 日志是否有错误：

```bash
$ sudo journalctl --unit daos_agent.service
```

#### 使用非默认配置启动 DAOS 代理[¶](https://docs.daos.io/v2.0/admin/deployment/#starting-the-daos-agent-with-a-non-default-configuration)

要从命令行启动 DAOS 代理，例如使用非默认配置文件运行，请运行：

```bash
$ daos_agent -o <'path to agent configuration file/daos_agent.yml'> &
```

如果您希望在开发版本中使用 systemd，您必须将代理服务文件从 复制`utils/systemd/`到`/usr/lib/systemd/system/`. 然后修改该`ExecStart`行以指向您的代理配置文件：

```
ExecStart=/usr/bin/daos_agent -o <'path to agent configuration file/daos_agent.yml'>
```

安装服务文件并`systemctl daemon-reload`运行以重新加载配置后，`daos_agent`可以通过 systemd 启动，如上所示。

#### 禁用代理缓存（可选）[¶](https://docs.daos.io/v2.0/admin/deployment/#disable-agent-cache-optional)

在某些情况下（例如，对于 DAOS 开发或系统评估），可能需要禁用 DAOS 代理的缓存机制，以避免在系统重新格式化时保留陈旧的系统信息。DAOS 代理通常会缓存 rank-to-fabric URI 查找的映射以及客户端网络配置数据，以减少启动应用程序所需的管理 RPC 数量。当此信息过时时，必须重新启动代理才能用新信息重新填充缓存。或者，可以禁用缓存机制，权衡每个应用程序启动将调用管理 RPC 以获得系统连接信息。

`daos_agent`要禁用 DAOS 代理缓存机制，请在启动进程之前设置以下环境变量：

```
DAOS_AGENT_DISABLE_CACHE=true
```

如果从 systemd 运行，请在重新加载 systemd 并重新启动服务之前 将以下内容添加到`daos_agent`该部分的服务文件中：`[Service]``daos_agent`

```
Environment=DAOS_AGENT_DISABLE_CACHE=true
```







# 容器管理[¶](https://docs.daos.io/v2.0/user/container/#container-management)

DAOS 容器是由用户管理的数据集。与 S3 存储桶类似，DAOS 容器是对象的集合，可以通过不同的接口呈现给应用程序，包括 POSIX I/O（文件和目录）、HDF5、SQL 或您选择的任何其他数据模型。

一个容器属于单个池，并与其他容器共享空间。它由用户选择的标签和首次创建容器时分配的不可变 UUID 标识。

容器是 DAOS 中数据管理的单位，可以进行快照或克隆。

警告

DAOS 容器是存储容器，不应与 Linux 容器混淆。

## 容器基础[¶](https://docs.daos.io/v2.0/user/container/#container-basics)

可以通过 `daos(1)`实用程序或原生 DAOS API 创建、查询、重新标记、列出和销毁容器。

### 创建容器[¶](https://docs.daos.io/v2.0/user/container/#creating-a-container)

要创建并查询标记`mycont`在标记为 的池上的容器`tank`：

```bash
$ daos cont create tank --label mycont
  Container UUID : daefe12c-45d4-44f7-8e56-995d02549041
  Container Label: mycont
  Container Type : unknown
Successfully created container daefe12c-45d4-44f7-8e56-995d02549041

$ daos cont query tank mycont
  Container UUID             : daefe12c-45d4-44f7-8e56-995d02549041
  Container Label            : mycont
  Container Type             : unknown
  Pool UUID                  : 0d1fad71-5681-48d4-acdd-7bb2e786f12e
  Number of snapshots        : 0
  Latest Persistent Snapshot : 0
  Highest Aggregated Epoch   : 263546931609567249
  Container redundancy factor: 0
  Snapshot Epochs            :
```

虽然标签不是强制性的，但强烈建议这样做。与池一样，容器标签最长可达 127 个字符，并且只能包含字母数字字符、冒号 (':')、句点 ('.')、连字符 ('-') 或下划线 ('_')。不允许使用可以解析为 UUID 的标签。

容器类型（即 POSIX 或 HDF5）可以通过 --type 选项传递。

为方便起见，容器也可以通过支持扩展属性的文件系统的路径来标识。在这种情况下，池和容器 UUID 存储在目标文件或目录的扩展属性中，然后可以在后续命令调用中使用该属性来识别容器。

```bash
$ daos cont create tank --label mycont --path /tmp/mycontainer --type POSIX --oclass=SX
  Container UUID : 30e5d364-62c9-4ddf-9284-1021359455f2
  Container Type : POSIX

Successfully created container 30e5d364-62c9-4ddf-9284-1021359455f2 type POSIX

$ daos cont query --path /tmp/mycontainer
  Container UUID             : 30e5d364-62c9-4ddf-9284-1021359455f2
  Container Type             : POSIX
  Pool UUID                  : 0d1fad71-5681-48d4-acdd-7bb2e786f12e
  Number of snapshots        : 0
  Latest Persistent Snapshot : 0
  Highest Aggregated Epoch   : 263548861715283973
  Container redundancy factor: 0
  Snapshot Epochs            :
  Object Class               : SX
  Chunk Size                 : 1.0 MiB
```

默认情况下，创建容器时未启用数据保护。这可以通过在创建时更改冗余因子 (rf) 属性来修改。要创建可以支持一个引擎故障的容器，请使用冗余因子 1，如下所示：

```bash
$ daos cont create tank --label mycont1 --type POSIX --properties rf:1
  Container UUID : b396e2ca-2077-4908-9ff2-1af4b4b2fd4a
  Container Label: mycont1
  Container Type : unknown
Successfully created container b396e2ca-2077-4908-9ff2-1af4b4b2fd4a
```

有关详细信息，请参阅[冗余因子属性](https://docs.daos.io/v2.0/user/container/#redundancy-factor) 。

### 列出容器[¶](https://docs.daos.io/v2.0/user/container/#listing-containers)

列出池中所有可用的容器：

```bash
$ daos cont list tank
UUID                                 Label
----                                 -----
30e5d364-62c9-4ddf-9284-1021359455f2 container_label_not_set
daefe12c-45d4-44f7-8e56-995d02549041 mycont
```

### 销毁容器[¶](https://docs.daos.io/v2.0/user/container/#destroying-a-container)

销毁容器：

```bash
$ daos cont destroy tank mycont
Successfully destroyed container mycont

$ daos cont destroy --path /tmp/mycontainer
Successfully destroyed container 30e5d364-62c9-4ddf-9284-1021359455f2
```

如果容器正在使用中，则必须添加强制选项（即--force 或-f）。强制删除容器的活动用户将因句柄错误而失败。

小费

与单独打孔/删除每个对象相比，销毁容器要快几个数量级。

## 容器属性[¶](https://docs.daos.io/v2.0/user/container/#container-properties)

容器属性是可以用来控制容器行为的主要机制。这包括中间件的类型，是否启用了重复数据删除或校验和等功能。一些属性在创建后是不可变的，而另一些则可以动态更改。

### 列出属性[¶](https://docs.daos.io/v2.0/user/container/#listing-properties)

该`daos`实用程序可用于列出所有容器的属性，如下所示：

```bash
$ daos cont get-prop tank mycont
# -OR- --path interface shown below
$ daos cont get-prop --path=/tmp/mycontainer
Properties for container mycont
Name                  Value
----                  -----
Highest Allocated OID 0
Checksum              off
Checksum Chunk Size   32 KiB
Compression           off
Deduplication         off
Dedupe Threshold      4.0 KiB
EC Cell Size          1.0 MiB
Encryption            off
Group                 jlombard@
Label                 mycont
Layout Type           unknown (0)
Layout Version        1
Max Snapshot          0
Owner                 jlombard@
Redundancy Factor     rf0
Redundancy Level      rank (1)
Server Checksumming   off
Health                HEALTHY
Access Control List   A::OWNER@:rwdtTaAo, A:G:GROUP@:rwtT
```

此外，可以使用 libdaos API`daos_cont_query()`函数检索容器的属性。[请参阅文件 src/include/daos_cont.h Doxygen 注释和此处](https://docs.daos.io/v2.0/doxygen/html/)提供的在线文档。

### 更改属性[¶](https://docs.daos.io/v2.0/user/container/#changing-properties)

默认情况下，容器将为每个属性继承一组默认值。这些可以通过`--properties`选项在容器创建时被覆盖。

```bash
$ daos cont create tank --label mycont2 --properties cksum:sha1,dedup:hash,rf:1
  Container UUID : a6286ead-1952-4faa-bf87-00fc0f3785aa
  Container Label: mycont2
  Container Type : unknown
Successfully created container a6286ead-1952-4faa-bf87-00fc0f3785aa

$ daos cont query tank mycont2
Properties for container mycont2
Name                  Value
----                  -----
Highest Allocated OID 0
Checksum              sha1
Checksum Chunk Size   32 KiB
Compression           off
Deduplication         hash
Dedupe Threshold      4.0 KiB
EC Cell Size          1.0 MiB
Encryption            off
Group                 jlombard@
Label                 mycont2
Layout Type           unknown (0)
Layout Version        1
Max Snapshot          0
Owner                 jlombard@
Redundancy Factor     rf1
Redundancy Level      rank (1)
Server Checksumming   off
Health                HEALTHY
Access Control List   A::OWNER@:rwdtTaAo, A:G:GROUP@:rwtT
```

可变属性可以在容器创建后通过该`set-prop` 选项进行修改。

```bash
$ daos cont set-prop tank mycont2 --properties label:mycont3
Properties were successfully set
```

这有效地改变了容器标签。

```bash
$ daos cont get-prop tank mycont2
ERROR: daos: DER_NONEXIST(-1005): The specified entity does not exist

$ daos cont get-prop tank mycont3
Properties for container mycont3
Name                  Value
----                  -----
Highest Allocated OID 0
Checksum              sha1
Checksum Chunk Size   32 KiB
Compression           off
Deduplication         hash
Dedupe Threshold      4.0 KiB
EC Cell Size          1.0 MiB
Encryption            off
Group                 jlombard@
Label                 mycont3
Layout Type           unknown (0)
Layout Version        1
Max Snapshot          0
Owner                 jlombard@
Redundancy Factor     rf1
Redundancy Level      rank (1)
Server Checksumming   off
Health                HEALTHY
Access Control List   A::OWNER@:rwdtTaAo, A:G:GROUP@:rwtT
```

### 属性值[¶](https://docs.daos.io/v2.0/user/container/#property-values)

下表总结了可用的容器属性。

| **容器属性** | **不可变** | **描述**                                                     |
| :----------- | :--------- | :----------------------------------------------------------- |
| 标签         | 不         | 与容器关联的字符串。例如，“Cat_Pics”或“training_data”        |
| 所有者       | 是的       | 用户作为容器的所有者                                         |
| 团体         | 是的       | 作为容器所有者的组                                           |
| acl          | 不         | 容器访问控制列表                                             |
| 布局类型     | 是的       | 容器类型（例如，POSIX、HDF5、...）                           |
| layout_ver   | 是的       | 由 I/O 中间件决定使用的布局版本以实现互操作性                |
| 射频         | 是的       | 冗余因子，它是对象可以在不丢失数据的情况下支持的同时引擎故障的最大数量 |
| rf_lvl       | 是的       | 冗余级别，这是故障域层次结构中用于对象放置的级别             |
| 健康         | 不         | 容器的当前状态                                               |
| alloc_oid    | 不         | 容器分配器分配的最大对象 ID                                  |
| ec_cell      | 是的       | 纠删码对象的纠删码单元大小                                   |
| 校验和       | 是的       | 校验和关闭，或要使用的算法（adler32、crc[16,32,64] 或 sha[1,256,512]） |
| cksum_size   | 是的       | 校验和大小确定校验和可以覆盖的最大范围大小                   |
| srv_cksum    | 是的       | 写入数据前是否在服务器上验证校验和（默认：关闭）             |

此外，以下属性已添加为占位符，但尚未完全支持：

| **容器属性**    | **不可变** | **描述**                                                     |
| :-------------- | :--------- | :----------------------------------------------------------- |
| 最大快照        | 不         | 对要保留的快照数量施加上限（默认值：0，无限制）              |
| 压缩            | 是的       | 在线压缩关闭，或使用算法（关闭，lz4，放气[1-4]）             |
| 去重            | 是的       | 内联重复数据删除关闭，或要使用的算法（散列或 memcmp）        |
| dedup_threshold | 是的       | 重复数据删除要考虑的最小 I/O 大小                            |
| 加密            | 是的       | 内联加密关闭，或使用算法（XTS[128,256]、CBC[128,192,256] 或 GCM[128,256]） |

有关每个属性的更多详细信息，请参阅下一部分。

### 容器类型[¶](https://docs.daos.io/v2.0/user/container/#container-type)

DAOS 容器类型表示特定的存储中间件，它在通过`libdaos`. 容器类型是通过不可变`layout_type`容器属性指定的。在每种容器类型中，该`layout_version`属性提供了一种版本控制机制——该版本号的使用由各自的中间件决定。

一些容器布局被定义为主要 DAOS 项目的一部分。最著名的例子是`POSIX`容器类型，它用于实现带有文件和目录的 [DAOS 文件系统 (DFS)](https://docs.daos.io/v2.0/user/filesystem/)布局。其他容器布局由各种用户社区创建，这些用户社区在 DAOS 之上实现了自己的特定于域的存储中间件。

[`daos_prop.h`](https://github.com/daos-stack/daos/blob/release/2.0/src/include/daos_prop.h#L217) 已知的 DAOS 容器类型在头文件中作为枚举列表进行维护 。当前定义了以下容器类型，可以与`daos cont create --type`命令选项一起使用：

| **容器类型** | **描述**                                                     |
| :----------- | :----------------------------------------------------------- |
| 未知         | 创建容器时未指定容器类型，或指定的容器类型未知。             |
| POSIX        | [DAOS 文件系统 (DFS)](https://docs.daos.io/v2.0/user/filesystem/)，也用于 dfuse 和[MPI-IO DAOS 后端](https://docs.daos.io/v2.0/user/mpi-io/)。 |
| HDF5         | [HDF5 DAOS VOL 连接器，由](https://docs.daos.io/v2.0/user/hdf5/)[HDF 集团](https://www.hdfgroup.org/?s=DAOS)维护。 |
| PYTHON       | [PyDAOS](https://docs.daos.io/v2.0/user/python/)容器格式。   |
| 火花         | [Apache Spark](https://docs.daos.io/v2.0/user/spark/) shuffle的特定布局。 |
| 数据库       | SQL 数据库，由 MariaDB 的实验性 DAOS 接口使用。              |
| 根           | ROOT/RNTuple 格式，由[CERN](https://root.cern.ch/)维护。     |
| 地震         | [DAOS Seismic Graph，又名 SEG-Y，由segy-daos](https://github.com/daos-stack/segy-daos)项目维护。 |
| 气象         | [Meteorology，又名 Fields Database (FDB)，由ECMWF](https://www.ecmwf.int/search/site/FDB)维护。 |

要注册新的 DAOS 容器类型（在标头中表示为整数和该整数的相应`DAOS_PROP_CO_LAYOUT_*`助记符名称 `daos_prop.h`），请与 DAOS 工程团队联系。

### 冗余系数[¶](https://docs.daos.io/v2.0/user/container/#redundancy-factor)

DAOS 容器中的对象可能属于不同的对象类并具有不同级别的数据保护。虽然此模型为用户提供了很多控制权，但它也需要为每个对象仔细选择合适的类。如果具有不同数据保护级别的对象也存储在同一个容器中，用户也应该做好准备，一些对象可能会在多次级联故障后丢失数据，而另一些数据保护级别较高的对象可能不会。这导致了并非所有 I/O 中间件都必须处理的额外复杂性。

为了在保持灵活性的同时降低采用门槛，引入了两个容器属性：

- 冗余因子 (rf)，它描述了容器中的对象受到保护的并发引擎排除的数量。rf 值是介于 0（无数据保护）和 5（最多支持 5 个同时故障）之间的整数。
- `health`表示任何对象内容是否可能由于级联引擎故障而丢失的属性。此属性的值可以是`HEALTHY`（无数据丢失）或`UNCLEAN`（数据可能已丢失）。

冗余因子可以在容器创建时设置，创建后不能修改。

```bash
$ daos cont create tank --label mycont1 --type POSIX --properties rf:1
  Container UUID : b396e2ca-2077-4908-9ff2-1af4b4b2fd4a
  Container Label: mycont1
  Container Type : unknown
Successfully created container b396e2ca-2077-4908-9ff2-1af4b4b2fd4a
```

可以通过列出属性来检查它：

```bash
$ daos cont get-prop tank mycont1
Properties for container mycont1
Name                  Value
----                  -----
[...]
Redundancy Factor     rf1
Redundancy Level      rank (1)
Health                HEALTHY
[...]
```

只有启用了数据保护的对象才能存储在这样的容器中。这包括复制或擦除编码的对象。尝试使用不支持数据冗余的类（例如，SX）打开对象将失败。

对于 rf2，只有具有至少 3 路复制或具有两个或更多奇偶校验的纠删码的对象才能存储在容器中。

只要同时发生的引擎故障的数量低于冗余因子，容器就会被报告为健康的。如果不是，则容器被标记为不干净并且无法访问。

```bash
$ daos cont get-prop tank mycont1
Properties for container mycont1
Name                  Value
----                  -----
[...]
Redundancy Factor     rf1
Redundancy Level      rank (1)
Health                UNCLEAN
[...]
```

例如，尝试使用 dfuse 这个 POSIX 容器挂载失败，如下所示：

```bash
$ dfuse --pool tank --container mycont1 -m /tmp/dfuse
dfuse ERR  src/client/dfuse/dfuse_core.c:873 dfuse_cont_open(0x19b9b00) daos_cont_open() failed: DER_RF(-2031): 'Failures exceed RF'
Failed to connect to container (5) Input/output error
```

如果管理员可以将排除的引擎重新集成到池中，则容器状态将自动切换回健康状态并可以再次访问。

如果用户愿意访问不健康的容器（例如，恢复数据），则可以在容器打开时传递强制标志，或者可以通过 强制容器状态为健康`daos cont set-prop tank mycont1 --properties health:healthy`。

冗余级别 (rf_lvl) 是另一个用于指定用于放置的故障域级别的属性。

### 数据的完整性[¶](https://docs.daos.io/v2.0/user/container/#data-integrity)

DAOS 允许检测和修复（启用数据保护时）静默数据损坏。这是通过在客户端的 DAOS 库中计算数据和元数据的校验和并将这些校验和永久存储在 SCM 中来完成的。如果启用了服务器验证选项，则校验和将在访问和更新/写入以及服务器端进行验证。

损坏的数据永远不会返回给应用程序。当检测到损坏时，DAOS 将尝试从不同的副本中读取（如果有）。如果无法恢复原始数据，则会向应用程序报告错误。

要启用和配置校验和，在容器创建期间使用以下容器属性。

- cksum ( `DAOS_PROP_CO_CSUM`)：要使用的校验和算法的类型。支持的值为 adler32、crc[16|32|64] 或 sha[1|256|512]。默认情况下，对新容器禁用校验和。
- cksum_size ( `DAOS_PROP_CO_CSUM_CHUNK_SIZE`)：定义用于创建数组类型校验和的块大小。（默认为 32K）。
- srv_cksum ( `DAOS_PROP_CO_CSUM_SERVER_VERIFY`)：由于可能降低 IOPS，在大多数情况下，不需要在服务器端验证对象更新的校验和。客户端验证获取就足够了，因为任何数据损坏，无论是对象更新、存储还是获取，都将被捕获。但是，了解更新是否发生损坏是有好处的。更新将立即失败，指示客户端重试 RPC 或向上层报告错误。

例如，要创建一个启用 crc64 校验和并在服务器端进行校验和验证的新容器，可以使用以下命令行：

```bash
$ daos cont create tank --label mycont --properties cksum:crc64,srv_cksum:on
Successfully created container dfa09efd-4529-482c-b7cd-748c29ef7419

$ daos cont get-prop  tank mycont4 | grep cksum
Checksum              crc64
Checksum Chunk Size   32 KiB
Server Checksumming   on
```

笔记

请注意，目前，一旦创建容器，就无法更改其校验和配置。

### 擦除代码[¶](https://docs.daos.io/v2.0/user/container/#erasure-code)

DAOS 纠删码实现使用适用于容器中所有对象的固定单元大小。DAOS 中的单元大小是每个数据和奇偶校验片段（有时也称为块）的大小。单元格大小可以在容器创建时通过属性设置：

```bash
$ daos cont create tank --label mycont5 --type POSIX --properties rf:1,cell_size:65536
  Container UUID : 90185799-0e22-4a0b-be9d-1a20900a35ee
  Container Label: mycont5
  Container Type : unknown
Successfully created container 90185799-0e22-4a0b-be9d-1a20900a35ee
```

这将强制在此容器中创建的所有纠删码对象的单元格大小为 64KiB。如果未指定单元格大小，则将从池中继承。如果管理员在创建池时未修改，则池上的默认单元大小设置为 1MiB。

### 重复数据删除（预览版）[¶](https://docs.daos.io/v2.0/user/container/#deduplication-preview)

重复数据删除 (dedup) 是一个允许消除重复数据副本以降低容量要求的过程。DAOS 初步支持内联重复数据删除。

启用 dedup 后，每个 DAOS 服务器通过其哈希（即校验和）维护每个池表的索引范围。因此，将在此表中查找任何大于重复数据删除阈值的新 I/O，以查明是否已经存储了具有相同签名的现有扩展区。如果找到范围，则提供两个选项：

- 将数据从客户端传输到服务器并对两个范围进行内存比较（即 memcmp）以验证它们确实相同。
- 信任散列函数并跳过数据传输。为了尽量减少散列冲突的问题，在这种情况下使用加密散列函数（即 SHA256）。这种方法的好处是要写入的数据不需要传输到服务器。数据处理因此大大加快。

可以在每个容器的基础上启用内联重复数据删除功能。要启用和配置去重，使用以下容器属性：

- dedup ( `DAOS_PROP_CO_DEDUP`)：要使用的 dedup 机制的类型。支持的值为 off（默认）、memcmp（内存比较）或 hash（使用 SHA256 基于哈希）。
- dedup_threshold ( `DAOS_PROP_CO_DEDUP_THRESHOLD`)：定义考虑 dedup 的 I/O 的最小 I/O 大小（默认为 4K）。

警告

Dedup 是 2.0 中的一个功能预览，有一些已知的限制。不支持去重范围的聚合，并且校验和树还不是持久的。这意味着对启用了去重的容器禁用聚合，并且在服务器重新启动后不会匹配重复的范围。启用了 dedup 的容器不支持 NVMe，因此请确保不要在启用了 NVMe 的池上使用 dedup。

### 压缩（不支持）[¶](https://docs.daos.io/v2.0/user/container/#compression-unsupported)

压缩 ( `DAOS_PROP_CO_COMPRESS`) 属性保留用于配置在线压缩，尚未实现。

### 加密（不支持）[¶](https://docs.daos.io/v2.0/user/container/#encryption-unsupported)

加密 ( `DAOS_PROP_CO_ENCRYPT`) 属性保留用于配置在线加密，尚未实现。

## 快照和回滚[¶](https://docs.daos.io/v2.0/user/container/#snapshot-rollback)

该`daos`工具提供容器 {create/destroy}-snap 和 list-snaps 命令。

```bash
$ daos cont create-snap tank mycont
snapshot/epoch 262508437483290624 has been created

$ daos cont list-snaps tank mycont
Container's snapshots :
262508437483290624

$ daos cont destroy-snap tank mycont -e 262508437483290624
```

max_snapshot ( `DAOS_PROP_CO_SNAPSHOT_MAX`) 属性用于限制要保留的最大快照数。当拍摄新快照并达到阈值时，将自动删除最旧的快照。

计划在未来的 DAOS 版本中将容器的内容回滚到快照。

## 用户属性[¶](https://docs.daos.io/v2.0/user/container/#user-attributes)

与 POSIX 扩展属性类似，用户可以通过`daos cont [set|get|list|del]-attr`命令或通过 `daos_cont_{list/get/set}_attr()`libdaos API 的函数将一些元数据附加到每个容器。

```bash
$ daos cont set-attr tank mycont import_date "12/01/2021"

$ daos cont list-attr tank mycont
Attributes for container mycont:
Name
----
import_date

$ daos cont get-attr tank mycont import_date
Attributes for container mycont:
Name        Value
----        -----
import_date 12/01/2021

$ daos cont del-attr tank mycont import_date

$ daos cont list-attr tank mycont
Attributes for container mycont:
  No attributes found.
```

## 访问控制列表[¶](https://docs.daos.io/v2.0/user/container/#access-control-lists)

容器的客户端用户和组访问由 [访问控制列表 (ACL) 控制](https://docs.daos.io/v2.0/overview/security/#access-control-lists)。

访问控制的容器访问包括：

- 打开容器进行访问。
- 在容器中读取和写入数据。
- 读取和写入对象。
- 获取、设置和列出用户属性。
- 获取、设置和列出快照。
- 删除容器（如果池不授予用户权限）。
- 获取和设置容器属性。
- 获取和修改容器 ACL。
- 修改容器的所有者。

这反映在支持的 [容器权限](https://docs.daos.io/v2.0/overview/security/#permissions)集中。

### 池与容器权限[¶](https://docs.daos.io/v2.0/user/container/#pool-vs-container-permissions)

一般来说，池权限与容器权限是分开的，访问其中一个并不能保证访问另一个。但是，用户必须有权连接到容器的池，然后才能以任何方式访问容器，而不管他们对该容器的权限如何。用户连接到池后，容器访问决策将基于单个容器 ACL。例如，用户无需具有对池的读/写访问权限即可打开具有读/写访问权限的容器。

池可以授予容器级别权限的一种情况：容器删除。如果用户对池具有删除权限，这将授予他们删除池中*任何*容器的能力，而不管他们对该容器的权限如何。

如果用户对池没有删除权限，他们将只能删除在容器的 ACL 中明确授予他们删除权限的容器。

### 容器创建时的 ACL[¶](https://docs.daos.io/v2.0/user/container/#acl-at-container-creation)

要在带有自定义 ACL 的标记为 tank 的池中创建标记为 mycont 的容器：

```bash
$ export DAOS_POOL="tank"
$ export DAOS_CONT="mycont"
$ daos cont create $DAOS_POOL --label $DAOS_CONT --acl-file=<path>
```

[安全概述](https://docs.daos.io/v2.0/overview/security/#acl-file)中详细介绍了 ACL 文件格式 。

### 显示 ACL[¶](https://docs.daos.io/v2.0/user/container/#displaying-acl)

查看容器的 ACL：

```bash
$ daos cont get-acl $DAOS_POOL $DAOS_CONT
```

输出与创建期间 ACL 文件中使用的字符串格式相同，每行一个 ACE。

### 修改 ACL[¶](https://docs.daos.io/v2.0/user/container/#modifying-acl)

对于使用 ACL 文件的所有这些命令，ACL 文件必须采用上述用于创建容器的格式。

#### 覆盖 ACL[¶](https://docs.daos.io/v2.0/user/container/#overwriting-acl)

用新的 ACL 替换容器的 ACL：

```bash
$ daos cont overwrite-acl $DAOS_POOL $DAOS_CONT --acl-file=<path>
```

#### 添加和更新 ACE[¶](https://docs.daos.io/v2.0/user/container/#adding-and-updating-aces)

要在现有容器 ACL 中添加或更新多个条目：

```bash
$ daos cont update-acl $DAOS_POOL $DAOS_CONT --acl-file=<path>
```

要在现有容器 ACL 中添加或更新单个条目：

```bash
$ daos cont update-acl $DAOS_POOL $DAOS_CONT --entry <ACE>
```

如果 ACL 中没有主体的现有条目，则将新条目添加到 ACL。如果主体已经有一个条目，则该条目将替换为新条目。

#### 删除 ACE[¶](https://docs.daos.io/v2.0/user/container/#removing-an-ace)

要删除现有容器 ACL 中给定主体的条目：

```bash
$ daos cont delete-acl $DAOS_POOL $DAOS_CONT --principal=<principal>
```

该`principal`参数是指要删除的条目的 [主体](https://docs.daos.io/v2.0/overview/security/#principal)或身份。

对于删除操作，`principal`参数的格式必须如下：

- 指定用户：`u:username@`
- 命名组：`g:groupname@`
- 特殊校长：
- `OWNER@`
- `GROUP@`
- `EVERYONE@`

该主体的条目将被完全删除。这并不总是意味着主体将无权访问。相反，他们对容器的访问将根据剩余的 ACL 规则来决定。

### 所有权[¶](https://docs.daos.io/v2.0/user/container/#ownership)

容器的所有权对应于特殊主体`OWNER@` 和`GROUP@`ACL 中。这些值是容器属性的一部分。它们可以在容器创建时设置并在以后更改。

#### 特权[¶](https://docs.daos.io/v2.0/user/container/#privileges)

所有者用户 ( `OWNER@`) 对其容器具有一些隐式特权。这些权限与 ACL 中的条目显式授予用户的任何权限一起以静默方式包含在内。

所有者-用户将始终具有以下隐式功能：

- 打开容器
- 设置 ACL (A)
- 获取 ACL (一)

因为所有者的特殊权限是隐式的，所以不需要在`OWNER@`条目中指定它们。在 从容器 ACL[确定](https://docs.daos.io/v2.0/overview/security/#enforcement) 用户的权限后，DAOS 会检查请求访问的用户是否是 owner-user。如果是这样，除了 ACL 授予的任何权限外，DAOS 还会向该用户授予所有者的隐式权限。

相反，除了由ACL 中`GROUP@`的条目明确授予的权限之外，所有者组 ( ) 没有特殊权限。`GROUP@`

#### 在创建时设置所有权[¶](https://docs.daos.io/v2.0/user/container/#setting-ownership-at-creation)

默认所有者用户和组是创建容器的用户的有效用户和组。但是，可以在容器创建时指定特定的用户和/或组。

```bash
$ daos cont create $DAOS_POOL --label $DAOS_CONT --user=<owner-user> --group=<owner-group>
```

用户名和组名区分大小写，并且必须格式化为 [DAOS ACL user/group principals](https://docs.daos.io/v2.0/overview/security/#principal)。

#### 改变所有权[¶](https://docs.daos.io/v2.0/user/container/#changing-ownership)

要更改所有者用户：

```bash
$ daos cont set-owner $DAOS_POOL $DAOS_CONT --user=<owner-user>
```

要更改所有者组：

```bash
$ daos cont set-owner $DAOS_POOL $DAOS_CONT --group=<owner-group>
```

用户名和组名区分大小写，并且必须格式化为 [DAOS ACL user/group principals](https://docs.daos.io/v2.0/overview/security/#principal)。





# 文件系统[¶](https://docs.daos.io/v2.0/user/filesystem/#file-system)

一个容器可以作为共享 POSIX 命名空间安装在多个计算节点上。此功能由在本`libdfs`机库上实现文件和目录抽象的库提供`libdaos`。POSIX 仿真可以直接暴露给应用程序或 I/O 框架（例如，用于 Spark 或 TensorFlow 等框架，或支持不同存储后端插件的 IOR 或 mdtest 等基准测试）。它还可以通过 FUSE 守护程序透明地公开，可选地与拦截库相结合，通过为 POSIX 读/写操作提供完整的操作系统绕过来解决一些 FUSE 性能瓶颈。

![../graph/posix.png](https://docs.daos.io/v2.0/graph/posix.png)

直接使用 DFS API 时性能通常是最好的。将 IO 拦截库与 dfuse 一起使用应该会产生与 DFS API 相同的 IO 操作（读/写）性能，并且开销最小。dfuse 上的元数据操作（文件创建、删除、重命名等）的性能将比 DFS API 慢得多，因为没有拦截绕过 fuse/kernel 层。

## 库文件[¶](https://docs.daos.io/v2.0/user/filesystem/#libdfs)

DAOS 文件系统 (DFS) 在`libdfs`库中实现，并允许将 DAOS 容器作为分层 POSIX 命名空间进行访问。 `libdfs`支持文件、目录和符号链接，但不支持硬链接。访问权限是从父池继承的，不是在每个文件或每个目录的基础上实现的。

### 支持的操作[¶](https://docs.daos.io/v2.0/user/filesystem/#supported-operations)

DFS API 紧密地代表了 POSIX API。API 包括以下操作：

- 挂载：创建/打开超级块和根对象
- 卸载：释放打开的手柄
- 查找：遍历路径并返回打开的文件/目录句柄
- IO：使用 iovec 读写
- Stat：检索条目的属性
- mkdir：创建一个目录
- readdir：枚举一个目录下的所有条目
- 打开：创建/打开一个文件/目录
- 删除：取消链接文件/目录
- 移动：重命名
- 释放：关闭文件/目录的打开句柄
- 扩展属性：set、get、list、remove

### POSIX 合规性[¶](https://docs.daos.io/v2.0/user/filesystem/#posix-compliance)

不支持 POSIX 的以下功能：

- 硬链接
- 对 MAP_SHARED 的 mmap 支持将仅从单个客户端保持一致。请注意，这仅通过 DFUSE 支持（即不通过 DFS API）。
- 字符设备、块设备、套接字和管道
- 用户/组配额
- DFS 命名空间不支持 setuid()、setgid() 程序、补充组、POSIX ACL。
- [访问/更改/修改]时间未适当更新，可能仅在关闭时。
- Flock（可能仅在 dfuse 本地节点级别）
- stat buf 中的块大小不准确（不考虑孔，扩展属性）
- 通过 statfs 报告的各种参数，例如块数、文件数、可用/可用空间
- 封装命名空间内的 POSIX 权限
- 仍然在 DAOS 池/容器级别强制执行
- 实际上意味着所有文件都属于同一个“项目”

笔记

DFS 目录不包括从其他 POSIX 文件系统已知的`.`（当前目录）和（父目录）目录条目。`..`类似的命令`ls -al`不会在其输出中包含这些条目。POSIX 不需要这些目录条目，因此这不是对 POSIX 合规性的限制。但是，在假设存在这些点目录的情况下解析目录列表的脚本可能需要适应以正确处理这种情况。请注意，在 dfuse 安装的 POSIX 容器中，类似`cd .`或仍然会成功的操作。`cd ..`

可以`libdfs`在多个节点的并行应用程序中使用。DFS 提供了两种模式，可提供不同级别的一致性。可以在容器创建时间设置模式：

\1) 用于生成无冲突操作的表现良好的应用程序的宽松模式，将支持非常高的并发性。

\2) 平衡模式适用于需要以性能为代价更严格一致性的应用程序。目前不完全支持此模式，默认情况下 DFS 将使用宽松模式。

在容器访问时，如果容器是用平衡模式创建的，则只能用平衡模式访问。如果容器是在宽松模式下创建的，则可以在宽松或平衡模式下访问它。在任何一种模式下，都存在未正确处理的一致性语义问题：

- Open-unlink语义：当客户端获得对象（文件或目录）的打开句柄并访问该对象（读取/写入数据或创建其他文件），而另一个客户端删除另一个客户端已打开的该对象时，会发生这种情况从它下面。在 DAOS 中，我们不跟踪对象打开的句柄，因为这会非常昂贵，因此在这种冲突的情况下，最坏的情况是写入那些已从命名空间取消链接的孤立对象的丢失/泄漏空间。

两种一致性模式对其他一致性问题的处理方式不同：

- 同一操作同时执行（宽松模式和平衡模式都支持）：例如，客户端尝试同时创建或删除同一个文件，一个应该成功，另一个会失败。
- 创建/取消链接/重命名冲突（仅在平衡模式下支持）：例如，客户端重命名文件，但另一个客户端同时取消链接旧文件。
- 操作原子性（仅在平衡模式下支持）：如果客户端在重命名过程中崩溃，容器的状态应该保持一致，就好像操作从未发生过一样。
- 可见性（在平衡和放松模式下支持）：来自一个客户端的写入应该对另一个客户端可见，客户端之间的简单协调。

## DFuse (DAOS FUSE)[¶](https://docs.daos.io/v2.0/user/filesystem/#dfuse-daos-fuse)

DFuse 通过标准的 libc/kernel/VFS POSIX 基础设施提供 DAOS 文件系统访问。这允许现有应用程序无需修改即可使用 DAOS，并提供将这些应用程序升级到原生 DAOS 支持的途径。此外，DFuse 提供了一个拦截库`libioil`，以透明地允许 POSIX 客户端直接与 DAOS 服务器对话，为 I/O 提供 OS-Bypass，而无需修改或重新编译应用程序。

DFuse 大量建立在 DFS 之上。通过 DFuse 写入的数据可以被 DFS 访问，反之亦然。

### DFuse 守护进程[¶](https://docs.daos.io/v2.0/user/filesystem/#dfuse-daemon)

守护`dfuse`进程为每个节点运行一个实例，为用户提供对 DAOS 的 POSIX 访问。它应该使用用户的凭据运行，并且通常将作为正在使用的任何资源管理器或调度程序的 prolog 和 epilog 脚本的一部分在每个计算节点上启动和停止。

### 限制[¶](https://docs.daos.io/v2.0/user/filesystem/#restrictions)

DFuse 仅限于单个用户。其他用户（包括 root）对文件系统的访问将不被接受。因此，不支持`chown` and调用。`chgrp`不支持硬链接和特殊设备文件（符号链接除外），也不支持任何 ACL。

DFuse 可以在前台运行，保持终端窗口打开，或者它可以像系统守护程序一样守护程序运行。但是，要做到这一点并且仍然能够访问 DAOS，它需要在调用`daos_init()`. 这反过来意味着它无法在 stdout/stderr 上或通过其返回码报告某些类型的启动错误。最初使用 DFuse 启动时，建议在前台模式 ( `--foreground`) 下运行以更好地观察任何故障。

Inode 由 DFuse 在本地节点上管理。因此，虽然在会话期间节点上的 inode 编号将保持一致，但不能保证它们在 DFuse 重新启动或跨节点时保持一致。

无法通过 DFuse 查看池/容器列表。因此`readdir`，如果`ls`使用 或其他，DFuse 将返回`ENOTSUP`。

### 发射[¶](https://docs.daos.io/v2.0/user/filesystem/#launching)

DFuse 应该使用将要访问它的用户的凭据（用户/组）运行，并且谁拥有将使用的任何池。

有两个强制性命令行选项，它们是：

| **命令行选项**      | **描述**       |
| :------------------ | :------------- |
| --mountpoint=<路径> | dfuse 挂载路径 |

指定的挂载点应该是用户拥有的本地节点上的一个空目录。

此外，还有几个可选的命令行选项：

| **命令行选项**           | **描述**                |
| :----------------------- | :---------------------- |
| --pool=<标签\|uuid>      | 要连接的池标签或 uuid   |
| --container=<标签\|uuid> | 要打开的容器标签或 uuid |
| --sys-name=<名称>        | DAOS 系统名称           |
| - 前景                   | 在前台运行              |
| --单线程                 | 运行单线程              |

当 DFuse 启动时，它将在`--mountpoint`选项指定的位置向内核注册一个挂载。此挂载将在 中可见`/proc/mounts`，并且可能在 的输出中可见`df`。多个池/容器的内容可以通过这个单一的内核挂载点访问。

下面是在 /tmp/dfuse 挂载点下创建和挂载 POSIX 容器的示例。

```bash
$ mkdir /tmp/dfuse

$ dfuse -m /tmp/dfuse --pool tank --cont mycont

$ touch /tmp/dfuse/foo

$ ls -l /tmp/dfuse/
total 0
-rw-rw-r-- 1 jlombard jlombard 0 Jul 10 20:23 foo

$ df -h /tmp/dfuse/
Filesystem      Size  Used Avail Use% Mounted on
dfuse           9.4G  326K  9.4G   1% /tmp/dfuse
```

### 链接到其他容器[¶](https://docs.daos.io/v2.0/user/filesystem/#links-into-other-containers)

可以链接到 DFuse 中的其他容器，其中容器中的子目录不解析为常规目录，而是解析为完全不同的 POSIX 容器的根目录。

要创建一个新容器并将其链接到现有容器的命名空间，请使用以下命令。

```bash
$ daos container create <pool_label> --type POSIX --path <path_to_entry_point>
```

池应该已经存在，并且路径应该在 DFuse 挂载点中的某个位置指定一个位置，该位置解析为 POSIX 容器。创建链接后，可以通过新路径访问它。跟随链接几乎是透明的。不需要容器 uuid。如果没有提供，它将被创建。

要再次销毁容器，应使用以下命令。

```bash
$ daos container destroy --path <path to entry point>
```

这将删除容器之间的链接并删除链接到的容器。

不支持在不删除容器本身的情况下添加指向现有容器的链接或删除指向容器的链接。

有关容器的信息，例如容器之间是否存在入口点，或者链接到的容器的池和容器 uuid，可以使用以下命令读取。

```bash
$ daos container info --path <path to entry point>
```

请在下面找到一个示例。

```bash
$ dfuse -m /tmp/dfuse --pool tank --cont mycont
$ cd /tmp/dfuse/
$ ls
foo
$ daos cont create tank --label mycont3 --type POSIX --path ./link_to_externa_container
  Container UUID : 933944a9-ddf2-491a-bdbf-4442f0437d56
  Container Label: mycont3
  Container Type : POSIX

Successfully created container 933944a9-ddf2-491a-bdbf-4442f0437d56 type POSIX
$ ls -lrt
total 0
-rw-rw-r-- 1 jlombard jlombard  0 Jul 10 20:23 foo
drwxr-xr-x 1 jlombard jlombard 72 Jul 10 20:56 link_to_externa_container
$ daos cont destroy --path ./link_to_externa_container/
Successfully destroyed container 933944a9-ddf2-491a-bdbf-4442f0437d56
jlombard@wolf-151:/tmp/dfuse$ ls -l
total 0
-rw-rw-r-- 1 jlombard jlombard 0 Jul 10 20:23 foo
```

### 缓存[¶](https://docs.daos.io/v2.0/user/filesystem/#caching)

出于性能原因，DFuse 中默认启用缓存，包括数据和元数据缓存。可以在 DFuse 命令行的高级别调整这些设置，也可以通过容器属性进行细粒度控制。

默认情况下会缓存以下类型的数据。

- dentries 的内核缓存
- 负dentries的内核缓存
- inode 的内核缓存（文件大小、权限等）
- 文件内容的内核缓存
- 在 dfuse 中预读并将数据插入内核缓存
- MMAP写入优化

警告

dfuse 中默认启用缓存。这可能会导致某些并行应用程序失败。如果您遇到这种情况或希望在节点之间共享最新数据，请禁用缓存（--disable-caching 选项）。

为了有选择地控制容器内的缓存，应使用以下容器属性，如果设置了任何属性，则假定其余属性设置为 0 或关闭，除了默认为 dentry-time 的 dentry-dir-time

| **属性名称**          | **描述**                               |
| :-------------------- | :------------------------------------- |
| dfuse-attr-time       | 文件属性缓存多长时间                   |
| dfuse-dentry-time     | 目录条目缓存多长时间                   |
| dfuse-dentry-dir-time | 如果条目本身是目录，则缓存多长时间     |
| dfuse-ndentry-time    | 缓存多长时间的负数                     |
| dfuse 数据缓存        | 为该文件启用数据缓存（“on”/“off”）     |
| dfuse-direct-io 禁用  | 强制为此容器使用页面缓存（“on”/“off”） |

对于元数据缓存属性，指定缓存应有效的持续时间，以秒为单位指定，或使用“s”、“m”、“h”或“d”后缀表示秒、分钟、小时或天。

dfuse-data-cache 应设置为“on”，如果设置为“off”，任何其他值都会记录错误，并导致缓存关闭。打开文件的 O_DIRECT 标志将在启用此选项的情况下得到尊重，未设置 O_DIRECT 的文件将被缓存。

dfuse-direct-io-disable 将启用数据缓存，类似于 dfuse-data-cache，但是如果将其设置为“on”，则 O_DIRECT 标志将被忽略，所有文件都将使用页面缓存。此默认值为“关闭”。

如果没有指定选项 attr 和 dentry 超时将为 1 秒，dentry-dir 和 ndentry 超时将为 5 秒，并且将启用数据缓存。

这是控制 DFuse 进程本身的两个命令行选项。

| **命令行选项**    | **描述**     |
| :---------------- | :----------- |
| --禁用缓存        | 禁用所有缓存 |
| --disable-wb-缓存 | 禁用回写缓存 |

这些将影响通过 DFuse 访问的所有容器，而不考虑任何容器属性。

### 权限[¶](https://docs.daos.io/v2.0/user/filesystem/#permissions)

DFuse 可以提供来自任何用户容器的数据，但需要适当的权限才能执行此操作。

容器内的文件所有权由所服务的容器设置，容器的所有者拥有该容器内的所有文件，因此如果查看另一个用户的容器，则该容器内的所有条目都将归该用户所有，并且文件-内核将在此基础上进行基于权限的检查。

如果将写入权限授予另一个用户，那么任何新创建的文件也将归容器所有者所有，而与创建它们的用户无关。仅在连接时检查权限，因此如果权限被撤销，用户需要重新启动 DFuse 才能获取这些权限。

#### 池权限。[¶](https://docs.daos.io/v2.0/user/filesystem/#pool-permissions)

DFuse 仅需要池的 'r' 权限。

#### 容器权限。[¶](https://docs.daos.io/v2.0/user/filesystem/#container-permissions)

DFuse 需要“r”、“t”和“a”权限才能运行：读取用于访问数据，“t”用于读取容器属性以了解容器类型，“a”用于读取 ACL 以了解容器所有者。

容器的写权限是可选的；但是，没有它，容器将是只读的。

### 停止 DFuse[¶](https://docs.daos.io/v2.0/user/filesystem/#stopping-dfuse)

完成后，可以通过 fusermount 卸载文件系统：

```bash
$ fusermount3 -u /tmp/daos
```

完成后，本地 DFuse 守护程序应关闭挂载点，断开与 DAOS 服务器的连接，然后退出。您还可以验证挂载点是否不再列在`/proc/mounts`.

## 拦截库[¶](https://docs.daos.io/v2.0/user/filesystem/#interception-library)

一个名为的拦截库`libioil`可用于 DFuse。该库与 DFuse 结合使用，允许拦截 POSIX I/O 调用并直接从应用程序上下文发出 I/O 操作， `libdaos`而无需任何应用程序更改。这为 I/O 数据提供了内核绕过，从而提高了性能。

### 使用 libioil[¶](https://docs.daos.io/v2.0/user/filesystem/#using-libioil)

要使用拦截库，请设置`LD_PRELOAD`指向 DAOS 安装目录中的共享库：

```
LD_PRELOAD=/path/to/daos/install/lib/libioil.so
LD_PRELOAD=/usr/lib64/libioil.so # when installed from RPMs
```

例如：

```
$ dd if=/dev/zero of=./foo bs=1G count=20
20+0 records in
20+0 records out
21474836480 bytes (21 GB, 20 GiB) copied, 14.1946 s, 1.5 GB/s

$ LD_PRELOAD=/usr/lib64/libioil.so dd if=/dev/zero of=./bar bs=1G count=20
20+0 records in
20+0 records out
21474836480 bytes (21 GB, 20 GiB) copied, 5.0483 s, 4.3 GB/s
```

或者，可以在编译时使用`-lioil`标志简单地将拦截库链接到应用程序。

### 监控活动[¶](https://docs.daos.io/v2.0/user/filesystem/#monitoring-activity)

拦截库旨在对用户透明，除上述之外不需要其他设置。但是，这可能意味着很难判断它是否正确链接并且是否正常工作，要检测到这一点，您可以通过环境变量打开拦截库的活动报告，在这种情况下，它将向 stderr 打印报告。

如果`D_IL_REPORT`设置了环境变量，那么拦截库将在共享库的析构函数中打印一个简短的摘要，通常在程序退出时，如果您将其设置为一个数字，那么它还将记录第一次读取和写入调用。例如，如果您将此值设置为 2，那么拦截库将在前两个截获的读取调用、前两个写入调用和前两个统计调用上打印到 stderr。要打印所有调用，请将值设置为 -1。值 0 表示仅在程序退出时打印摘要。

```
D_IL_REPORT=2
```

例如：

```
$ D_IL_REPORT=1 LD_PRELOAD=/usr/lib64/libioil.so dd if=/dev/zero of=./bar bs=1G count=20
[libioil] Intercepting write of size 1073741824
20+0 records in
20+0 records out
21474836480 bytes (21 GB, 20 GiB) copied, 5.17297 s, 4.2 GB/s

$ D_IL_REPORT=3 LD_PRELOAD=/usr/lib64/libioil.so dd if=/dev/zero of=./bar bs=1G count=5
[libioil] Intercepting write of size 1073741824
[libioil] Intercepting write of size 1073741824
[libioil] Intercepting write of size 1073741824
5+0 records in
5+0 records out
5368709120 bytes (5.4 GB, 5.0 GiB) copied, 1.27362 s, 4.2 GB/s

$ D_IL_REPORT=-1 LD_PRELOAD=/usr/lib64/libioil.so dd if=/dev/zero of=./bar bs=1G count=5
[libioil] Intercepting write of size 1073741824
[libioil] Intercepting write of size 1073741824
[libioil] Intercepting write of size 1073741824
[libioil] Intercepting write of size 1073741824
[libioil] Intercepting write of size 1073741824
5+0 records in
5+0 records out
5368709120 bytes (5.4 GB, 5.0 GiB) copied, 1.29935 s, 4.1 GB/s
```

笔记

一些程序，'coreutils' 包中的大多数 GNU 实用程序都有一个析构函数来在退出时关闭 stderr，因此对于许多基本命令，如 cp 和 cat，虽然拦截库将工作，但无法看到拦截生成的摘要图书馆。

### 高级用法[¶](https://docs.daos.io/v2.0/user/filesystem/#advanced-usage)

DFuse 只会创建一个内核级别的挂载点，无论它是如何启动的。POSIX 容器在该挂载点内的表示方式取决于 DFuse 命令行选项。除了挂载单个 POSIX 容器外，DFuse 还可以在下面详述的其他两种模式下运行。

#### 池模式[¶](https://docs.daos.io/v2.0/user/filesystem/#pool-mode)

如果指定了池 uuid 但未指定容器 uuid，则可以通过路径访问容器`<mount point>/<container uuid>`。容器 uuid 必须从外部来源提供。

```bash
$ daos cont create tank --label mycont --type POSIX
   Container UUID : 8a8f08bb-5034-41e8-b7ae-0cdce347c558
   Container Label: mycont
   Container Type : POSIX
 Successfully created container 8a8f08bb-5034-41e8-b7ae-0cdce347c558

$ daos cont create tank --label mycont2 --type POSIX
  Container UUID : 0db21789-5372-4f2a-b7bc-14c0a5e968df
  Container Label: mycont2
  Container Type : POSIX

Successfully created container 0db21789-5372-4f2a-b7bc-14c0a5e968df

$ dfuse -m /tmp/dfuse --pool tank

$ ls -l /tmp/dfuse/
ls: cannot open directory '/tmp/dfuse/': Operation not supported

$ ls -l /tmp/dfuse/0db21789-5372-4f2a-b7bc-14c0a5e968df
total 0

$ ls -l /tmp/dfuse/8a8f08bb-5034-41e8-b7ae-0cdce347c558
total 0
-rw-rw-r-- 1 jlombard jlombard 0 Jul 10 20:23 foo

$ fusermount3 -u /tmp/dfuse/
```

#### 系统模式[¶](https://docs.daos.io/v2.0/user/filesystem/#system-mode)

如果未指定池或容器，则可以通过路径访问池和容器`<mount point>/<pool uuid>/<container uuid>`。但是应该注意，`readdir()`因此`ls`不能在此处表示池的挂载点或目录上工作。因此，必须从外部源提供池和容器 uuid。

```bash
$ dfuse -m /tmp/dfuse
$ df -h /tmp/dfuse
Filesystem      Size  Used Avail Use% Mounted on
dfuse              -     -     -    - /tmp/dfuse
$ daos pool query tank | grep -- -.*-
Pool 004abf7c-26c8-4cba-9059-8b3be39161fc, ntarget=32, disabled=0, leader=0, version=1
$ ls -l /tmp/dfuse/004abf7c-26c8-4cba-9059-8b3be39161fc/0db21789-5372-4f2a-b7bc-14c0a5e968df
total 0
$ ls -l /tmp/dfuse/004abf7c-26c8-4cba-9059-8b3be39161fc/8a8f08bb-5034-41e8-b7ae-0cdce347c558
total 0
-rw-rw-r-- 1 jlombard jlombard 0 Jul 10 20:23 foo
```

虽然这种模式预计不会被用户直接使用，但它对于统一命名空间集成很有用。









# MPI-IO 支持[¶](https://docs.daos.io/v2.0/user/mpi-io/#mpi-io-support)

[由MPI 论坛](https://www.mpi-forum.org/docs/)维护的消息传递接口 (MPI) 标准包括关于 MPI-IO 的一章。

[ROMIO](https://www.mcs.anl.gov/projects/romio/)是众所周知的 MPI-IO 实现，并包含在许多 MPI 实现中。DAOS 提供了自己的 MPI-IO ROMIO ADIO 驱动程序。此驱动程序已合并到上游 MPICH 存储库中，有关详细信息，请参阅 MPICH git 存储库中的 [adio/ad_daos](https://github.com/pmodels/mpich/tree/main/src/mpi/romio/adio/ad_daos) 部分。

## 支持的 MPI 版本[¶](https://docs.daos.io/v2.0/user/mpi-io/#supported-mpi-version)

### MPICH[¶](https://docs.daos.io/v2.0/user/mpi-io/#mpich)

DAOS ROMIO ADIO 驱动程序已被[MPICH](https://www.mpich.org/)接受。它包含在 mpich-3.4.1（2021 年 1 月发布）和 [mpich-3.4.2（2021 年 5 月发布）](https://www.mpich.org/downloads/)中。

笔记

从 DAOS 1.2 开始，`--svc`不再需要参数（服务副本数），DAOS API 也进行了相应更改。已向 MPICH 提供了检测 DAOS API 版本的补丁，以优雅地处理此更改。MPICH 3.4.2 包含这些更改，并且可以与 DAOS 2.0 一起使用。MPICH 3.4.1 不包括这些更改。[请在此处](https://github.com/pmodels/mpich/commits/main?author=mchaarawi)查看最新提交 ，了解如何将这些更改应用于 MPICH 3.4.1。

要构建 MPICH，包括带有 DAOS ADIO 驱动程序的 ROMIO：

```bash
export MPI_LIB=""

git clone https://github.com/pmodels/mpich

cd mpich

./autogen.sh

./configure --prefix=dir --enable-fortran=all --enable-romio \
 --enable-cxx --enable-g=all --enable-debuginfo --with-device=ch3:nemesis \
 --with-file-system=ufs+daos --with-daos=/usr

make -j8; make install
```

这假定 DAOS 已安装到`/usr`树中，这是 DAOS RPM 安装的情况。可以根据需要添加、修改或删除其他配置选项，如网络通信设备、fortran 支持等。有关这些，请咨询 mpich 用户指南。

将`PATH`and设置`LD_LIBRARY_PATH`为您要构建使用 MPI 的客户端应用程序或库的位置到已安装 MPICH 的路径。

### 英特尔 MPI[¶](https://docs.daos.io/v2.0/user/mpi-io/#intel-mpi)

[自2019.8 版本](https://software.intel.com/content/www/us/en/develop/articles/intel-mpi-library-release-notes-linux.html)以来，[英特尔 MPI 库](https://software.intel.com/content/www/us/en/develop/tools/mpi-library.html) 包括对 DAOS 的支持 。

请注意，英特尔 MPI 将`libfabric`其作为英特尔 MPI 安装的一部分使用并包括在内： * 2019.8 和 2019.9 包括`libfabric-1.10.1-impi` * 2021.1、2021.2 和 2021.3 包括`libfabric-1.12.1-impi`

必须注意确保使用的 libfabric 版本处于包含对 DAOS 至关重要的补丁的级别。DAOS 1.0.1 包括`libfabric-1.9.0`，DAOS 1.2 包括`libfabric-1.12`，DAOS 2.0 包括`libfabric-1.14`。

要将 DAOS 与 Intel MPI 一起使用，必须使用 DAOS提供的（默认`libfabric`安装的）。`/usr/lib64`英特尔 MPI 提供了一种机制来指示 `libfabric`不应**使用**英特尔 MPI 版本，方法是在 加载英特尔 MPI 环境**之前**设置此变量：

```bash
export I_MPI_OFI_LIBRARY_INTERNAL=0
```

这通常足以确保`libfabric`使用 DAOS 提供的内容。根据环境的设置方式，可能需要将系统库搜索路径添加回库搜索路径中的第一个路径：

```bash
export LD_LIBRARY_PATH="/usr/lib64/:$LD_LIBRARY_PATH"
```

为了确保 DAOS MPIIO 驱动程序正常运行，还需要在客户端设置其他环境变量，包括：

```bash
export FI_UNIVERSE_SIZE=16383
export FI_OFI_RXM_USE_SRX=1
```

### 打开 MPI[¶](https://docs.daos.io/v2.0/user/mpi-io/#open-mpi)

[Open MPI](https://www.open-mpi.org/) 4.0.5 尚不提供 DAOS 支持。由于其 MPI-IO 实现之一基于 ROMIO，因此它可能会在即将发布的版本中获得 DAOS 支持。

### MVAPICH2[¶](https://docs.daos.io/v2.0/user/mpi-io/#mvapich2)

[MVAPICH2](https://mvapich.cse.ohio-state.edu/) 2.3.4 尚未提供 DAOS 支持。由于其 MPI-IO 实现基于 ROMIO，因此它可能会在即将发布的版本中获得 DAOS 支持。

## 使用 DAOS 测试 MPI-IO[¶](https://docs.daos.io/v2.0/user/mpi-io/#testing-mpi-io-with-daos)

通常使用上面安装的 mpicc 命令和 mpich 库构建任何客户端（HDF5、ior、mpi 测试套件）。

### 使用 UNS[¶](https://docs.daos.io/v2.0/user/mpi-io/#using-the-uns)

DAOS UNS 允许将池和容器信息编码到文件系统上的路径中，因此可以使用该路径轻松访问该容器，而不是使用池和容器 uuid/标签使用显式寻址。

使用 dfuse 或 lustre 或任何支持扩展属性的文件系统上的路径创建容器：

```bash
daos cont create mypool --label mycont --path=/mnt/dfuse/ --type POSIX
```

然后使用该路径，只需附加 `daos:`到文件名/路径即可开始使用 DAOS MPIIO 驱动程序创建文件。例如： `daos:/mnt/dfuse/file` `daos:/mnt/dfuse/dir1/file`

### 使用前缀环境变量[¶](https://docs.daos.io/v2.0/user/mpi-io/#using-a-prefix-environment-variable)

使用 DAOS MPIIO 驱动程序的另一种方法是使用环境变量为文件设置前缀本身：

```bash
export DAOS_UNS_PREFIX="path"
```

该前缀路径可以是： 1. UNS 前缀（如果存在）（类似于上面的 UNS 模式）：/mnt/dfuse 2. 使用池和容器标签（或 uuid）的直接路径：daos://pool/container /

然后可以指定文件的路径相对于在前缀中设置的容器的根目录。所以在上面的例子中，如果要访问的文件在容器中的 /dir1 下，则可以将 `daos:/dir1/file' 传递给 MPI_File_open()。

### 使用池和容器环境变量[¶](https://docs.daos.io/v2.0/user/mpi-io/#using-pool-and-container-environment-variables)

此模式仅用于快速测试使用绕过 UNS 的 MPIIO DAOS 驱动程序并使用池和容器环境变量设置直接访问。在客户端，需要设置以下环境变量： `export DAOS_POOL={uuid/label}; export DAOS_CONT={uuid/label}; export DAOS_BYPASS_DUNS=1`. 用户仍然需要将`daos:`前缀附加到传递给 MPI_File_open() 的文件。

## 已知限制[¶](https://docs.daos.io/v2.0/user/mpi-io/#known-limitations)

当前实施的限制包括：

- 不支持 MPI 文件原子性、预分配或共享文件指针。







# 本机对象接口[¶](https://docs.daos.io/v2.0/user/interface/#native-object-interface)

## 针对 DAOS 库构建[¶](https://docs.daos.io/v2.0/user/interface/#building-against-the-daos-library)

要针对原生 DAOS API 构建应用程序或 I/O 中间件，`daos.h`请在程序中包含头文件并使用`-Ldaos`. 示例可在 下找到`src/tests`。

## DAOS API 参考[¶](https://docs.daos.io/v2.0/user/interface/#daos-api-reference)

`libdaos`用 C 编写并使用添加到 C 头文件中的 Doxygen 注释。Doxygen 文档可 [在此处](https://docs.daos.io/v2.0/doxygen/html/)获得。

## Python 绑定[¶](https://docs.daos.io/v2.0/user/interface/#python-bindings)

pydaos.raw 子模块通过 Ctypes 提供对 DAOS API 功能的访问，其开发重点是测试用例。虽然大多数单元测试是用 C 编写的，但更高级别的测试主要是使用 Python API 编写的。提供了用于从 Python 访问 DAOS 管理和 DAOS API 功能的接口。这个更高级别的接口允许更快的实现 DAOS 测试用例的周转时间。

#### 布局[¶](https://docs.daos.io/v2.0/user/interface/#layout)

Python API 根据功能分为几个文件：

- Python 对象 API： [daos_api.py](https://github.com/daos-stack/daos/blob/release/2.0/src/client/pydaos/raw/daos_api.py)。
- C 结构到 Python 类的映射 [daos_cref.py](https://github.com/daos-stack/daos/blob/release/2.0/src/client/pydaos/raw/daos_cref.py)

存在用于操作 DAOS 存储的高级抽象类：

```python
class DaosPool(object)
class DaosContainer(object)
class DaosObj(object)
class IORequest(object)
```

`DaosPool`是一个代表 DAOS 池的 Python 类。所有与池相关的功能都从此类公开。支持创建/销毁池、连接池、添加目标到存储池等操作。

`DaosContainer`是一个代表 DAOS 容器的 Python 类。与`DaosPool`类一样，所有与容器相关的功能都在这里公开。此类还公开了抽象的包装函数，用于创建对象并将其提交到 DAOS 容器的流程。

`DaosObj`是一个代表 DAOS 对象的 Python 类。诸如在容器中创建/删除对象、“打孔”对象（仅从指定事务中删除对象）和对象查询等功能。

`IORequest`是一个 Python 类，表示对 DAOS 对象的读取或写入请求。

出于管理目的也存在几个类：

```python
class DaosContext(object)
class DaosLog
class DaosApiError(Exception)
```

`DaosContext`是 DAOS 库的包装器。它使用可以找到 DAOS 库的路径进行初始化。

`DaosLog`公开将消息写入 DAOS 客户端日志的功能。

`DaosApiError`是一个由 API 在内部引发的自定义异常类，以防 DAOS 操作失败。

DAOS C API 中公开的大多数函数都支持同步和异步执行，Python API 也公开了相同的功能。每个 API 都接受一个输入事件。`DaosEvent`是此事件的 Python 表示。如果输入事件是`NULL`，则调用是同步的。如果提供了一个事件，该函数将在向底层堆栈提交 API 请求后立即返回，用户可以轮询和查询该事件是否完成。

#### 类型[¶](https://docs.daos.io/v2.0/user/interface/#ctypes)

Ctypes 是一个内置的 Python 模块，用于将 Python 与用 C/C++ 编写的现有库连接起来。Python API 被构建为使用 ctypes 围绕 DAOS 库的面向对象的包装器。

Ctypes 文档可以在[这里](https://docs.python.org/3/library/ctypes.html)找到。

下面演示了为 C 函数创建 Python 包装器的简化示例，其中 C 函数`daos_pool_tgt_exclude_out`的每个输入参数都通过 ctypes 进行强制转换。这也通过 ctypes 演示了结构表示：

```C
// daos_exclude.c

#include <stdio.h>

int
daos_pool_tgt_exclude_out(const uuid_t uuid, const char *grp,
                          struct d_tgt_list *tgts, daos_event_t *ev);
```

所有输入参数必须通过 ctypes 表示。如果需要 struct 作为输入参数，则可以创建相应的 Python 类。对于结构`d_tgt_list`：

```c
struct d_tgt_list {
    d_rank_t    *tl_ranks;
    int32_t     *tl_tgts;
    uint32_t    tl_nr;
};
class DTgtList(ctypes.Structure):
    _fields_ = [("tl_ranks", ctypes.POINTER(ctypes.c_uint32)),
                ("tl_tgts", ctypes.POINTER(ctypes.c_int32)),
                ("tl_nr", ctypes.c_uint32)]
```

`daos_pool_tgt_exclude_out`然后可以导入包含的共享对象并直接调用该函数：

```python
# api.py

import ctypes
import uuid
import conversion # utility library to convert C <---> Python UUIDs

# init python variables
p_uuid = str(uuid.uuid4())
p_tgts = 2
p_ranks = DaosPool.__pylist_to_array([2])

# cast python variables via ctypes as necessary
c_uuid = str_to_c_uuid(p_uuid)
c_grp = ctypes.create_string_buffer(b"daos_group_name")
c_tgt_list = ctypes.POINTER(DTgtList(p_ranks, p_tgts, 2))) # again, DTgtList must be passed as pointer

# load the shared object
my_lib = ctypes.CDLL('/full/path/to/daos_exclude.so')

# now call it
my_lib.daos_pool_tgt_exclude_out(c_uuid, c_grp, c_tgt_list, None)
```

#### 错误处理[¶](https://docs.daos.io/v2.0/user/interface/#error-handling)

API 是使用 EAFP（**E** asier to **A** sk **F** **orgiveness** than get Permission ）习语设计的。给定的函数将在错误状态下引发自定义异常，`DaosApiError`. API 的用户应根据需要捕获并处理此异常：

```python
# catch and log
try:
    daos_some_action()
except DaosApiError as e:
    self.d_log.ERROR("My DAOS action encountered an error!")
```

#### 日志记录[¶](https://docs.daos.io/v2.0/user/interface/#logging)

Python DAOS API 公开了将消息记录到 DAOS 客户端日志的功能。消息可以记录为`INFO`、`DEBUG`、`WARN`或`ERR`日志级别。DAOS 日志对象必须使用运行的环境上下文进行初始化：

```python
from pydaos.raw import DaosLog

self.d_log = DaosLog(self.context)

self.d_log.INFO("FYI")
self.d_log.DEBUG("Debugging code")
self.d_log.WARNING("Be aware, may be issues")
self.d_log.ERROR("Something went very wrong")
```

## 去绑定[¶](https://docs.daos.io/v2.0/user/interface/#go-bindings)

[Go](https://godoc.org/github.com/daos-stack/go-daos/pkg/daos)的API 绑定 也可用。











