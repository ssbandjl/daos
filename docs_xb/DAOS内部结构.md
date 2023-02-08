# DAOS 内部结构

本文档的目的是描述 DAOS 使用的内部代码结构和主要算法。它假定事先了解[DAOS 存储模型](https://github.com/daos-stack/daos/blob/master/docs/overview/storage.md) 和[首字母缩略词](https://github.com/daos-stack/daos/blob/master/docs/overview/terminology.md)。本文档包含以下部分：

- DAOS 组件
  - [DAOS系统](https://github.com/daos-stack/daos/blob/master/src/README.md#11)
  - [客户端 API、工具和 I/O 中间件](https://github.com/daos-stack/daos/blob/master/src/README.md#12)
  - [代理人](https://github.com/daos-stack/daos/blob/master/src/README.md#13)
- 网络传输和通信
  - [gRPC 和协议缓冲区](https://github.com/daos-stack/daos/blob/master/src/README.md#21)
  - [dRPC](https://github.com/daos-stack/daos/blob/master/src/README.md#22)
  - [CART](https://github.com/daos-stack/daos/blob/master/src/README.md#23)
- DAOS 分层和服务
  - [架构](https://github.com/daos-stack/daos/blob/master/src/README.md#31)
  - [代码结构](https://github.com/daos-stack/daos/blob/master/src/README.md#32)
  - [基础设施库](https://github.com/daos-stack/daos/blob/master/src/README.md#33)
  - [DAOS 服务](https://github.com/daos-stack/daos/blob/master/src/README.md#34)
- 软件兼容性
  - [协议兼容性](https://github.com/daos-stack/daos/blob/master/src/README.md#41)
  - [PM Schema 兼容性和升级](https://github.com/daos-stack/daos/blob/master/src/README.md#42)



## DAOS 组件

如下图所示，DAOS 安装涉及多个组件，这些组件既可以托管也可以分布式。DAOS 软件定义存储 (SDS) 框架依赖于两种不同的通信通道：用于管理的带外 TCP/IP 网络和用于数据访问的高性能结构。实际上，同一网络可用于管理和数据访问。IP over Fabric 也可用作管理网络。

[![DAOS SDS 组件](docs/graph/system_architecture.png)](https://github.com/daos-stack/daos/blob/master/docs/graph/system_architecture.png)



### DAOS系统

DAOS 服务器是在 Linux 实例（即物理节点、VM 或容器）上运行并管理分配给 DAOS 的本地连接的 SCM 和 NVM 存储的多租户守护进程。它侦听一个由 IP 地址和 TCP 端口号寻址的管理端口，以及一个或多个由网络 URI 寻址的fabric端点。DAOS 服务器是通过 YAML 文件（`/etc/daos/daos_server.yml`或命令行中提供的不同路径）配置的。启动和停止 DAOS 服务器可以与不同的守护进程管理或编排框架（例如 systemd 脚本、Kubernetes 服务，甚至通过 pdsh 或 srun 等并行启动器）集成。

DAOS 系统由系统名称标识，由一组连接到同一结构的 DAOS 服务器组成。两个不同的系统包括两组不相交的服务器并且彼此不协调。DAOS 池不能跨越多个系统。

在内部，一个 DAOS 服务器由多个守护进程组成。第一个要启动的是[控制平面](https://github.com/daos-stack/daos/blob/master/src/control/README.md) （二进制文件名`daos_server`），它负责解析配置文件、提供存储并最终启动和监控[数据平面](https://github.com/daos-stack/daos/blob/master/src/engine/README.md) （二进制文件名`daos_engine`）的一个或多个实例。控制平面是用 Go 编写的，并在 gRPC 框架上实现了 DAOS 管理 API，该框架提供了一个安全的带外通道来管理 DAOS 系统。每个服务器要启动的数据平面实例的数量以及存储、CPU 和fabric接口亲和度可以通过`daos_server.yml`YAML 配置文件进行配置。

数据平面是一个用 C 语言编写的运行 DAOS 存储引擎的多线程进程。它通过 **CART 通信中间件**处理传入的元数据和 I/O 请求，并通过 PMDK（用于存储级内存，也称为 SCM）和 SPDK（用于 NVMe SSD）库访问本地 NVM 存储。数据平面依靠 **Argobots** 进行基于事件的并行处理，并导出多个可以通过fabric独立寻址的目标。每个数据平面实例在 DAOS 系统内都被分配了一个唯一的等级rank。

**Argobots** 是一个轻量级的运行时系统，支持大规模并发的集成计算和数据移动。它将直接利用硬件和操作系统中的最底层结构：轻量级通知机制、数据移动引擎、内存映射和数据放置策略。

Argobots 被众多工业和学术合作伙伴使用，例如英特尔、HDF 集团、RIKEN 和 BSC。Argobots 被选为[2020 R&D 100 Awards 的决赛选手](https://www.rdworldonline.com/finalists-for-2020-rd-100-awards-are-unveiled/)

控制平面和数据平面进程通过 Unix 域套接字和称为 dRPC 的自定义轻量级协议进行本地通信。

进一步阅读：

- [DAOS 控制平面 (daos_server)](https://github.com/daos-stack/daos/blob/master/src/control/README.md)
- [DAOS 数据平面 (daos_engine)](https://github.com/daos-stack/daos/blob/master/src/engine/README.md)



### 客户端 API、工具和 I/O 中间件

应用程序、用户和管理员可以通过两个不同的客户端 API 与 DAOS 系统进行交互。

DAOS 管理 Go 软件包允许从任何可以通过带外管理通道与 DAOS 服务器通信的节点管理 DAOS 系统。此 API 是为通过特定证书进行身份验证的 DAOS 系统管理员保留的。DAOS 管理 API 旨在与不同供应商特定的存储管理或开源编排框架集成。一个名为的 CLI 工具`dmg`是基于 DAOS 管理 API 构建的。有关管理 API 和`dmg`工具的进一步阅读：

- [DAOS 管理 Go 包](https://godoc.org/github.com/daos-stack/daos/src/control/client)
- [DAOS 管理工具（又名 dmg）](https://github.com/daos-stack/daos/blob/master/src/control/cmd/dmg/README.md)

DAOS 库 ( `libdaos`) 实现了 DAOS 存储模型，主要针对希望将数据集存储到 DAOS 容器中的应用程序和 I/O 中间件开发人员。它可以从连接到目标 DAOS 系统使用的结构的任何节点上使用。应用程序进程通过 DAOS 代理进行身份验证（请参阅下一节）。导出的 API`libdaos`通常称为 DAOS API（与 DAOS 管理 API 不同），允许通过不同的接口（例如键值存储或数组 API）来管理容器和访问 DAOS 对象。该`libdfs`库模拟 POSIX 文件和目录抽象， `libdaos`并为需要 POSIX 命名空间的应用程序提供平滑的迁移路径。进一步阅读`libdaos`，不同编程语言的绑定和`libdfs`：

- [`libdaos`](https://github.com/daos-stack/daos/blob/master/src/client/api/README.md)基于原生 DAOS API 构建的[DAOS 库 ( ](https://github.com/daos-stack/daos/blob/master/src/client/api/README.md)[)](https://github.com/daos-stack/daos/blob/master/src/client/api/README.md)和[数组接口](https://github.com/daos-stack/daos/blob/master/src/client/array/README.md)和[KV 接口](https://github.com/daos-stack/daos/blob/master/src/client/kv/README.md)
- [Python API 绑定](https://github.com/daos-stack/daos/blob/master/src/src/client/pydaos/raw/README.md)
- [Go 绑定](https://github.com/daos-stack/go-daos)和[API 文档](https://godoc.org/github.com/daos-stack/go-daos/pkg/daos)
- [POSIX 文件和目录模拟 ( `libdfs`)](https://github.com/daos-stack/daos/blob/master/src/client/dfs/README.md)

和库为支持特定领域的数据格式（如 HDF5 和 Apache Arrow）提供了基础`libdaos`。`libdfs`如需进一步了解 I/O 中间件集成，请查看以下外部参考：

- [用于 HDF5 的 DAOS VOL 连接器](https://bitbucket.hdfgroup.org/projects/HDFFV/repos/hdf5/browse?at=refs%2Fheads%2Fhdf5_daosm)
- [用于 MPI-IO 的 ROMIO DAOS ADIO 驱动程序](https://github.com/daos-stack/mpich/tree/daos_adio)



### 代理人agent

[DAOS 代理](https://github.com/daos-stack/daos/blob/master/src/control/cmd/daos_agent/README.md)是驻留在客户端节点上的守护进程。它通过 dRPC 与 DAOS 客户端库交互，对应用进程进行身份验证。它是一个受信任的实体，可以使用本地证书签署 DAOS 客户端凭据。DAOS 代理可以支持不同的身份验证框架并使用 Unix 域套接字与客户端库进行通信。DAOS 代理是用 Go 编写的，通过 gRPC 与每个 DAOS 服务器的控制平面组件进行通信，以向客户端库提供 DAOS 系统成员信息并支持池列表。



## 网络传输和通信

如上一节所述，DAOS 使用三种不同的通信渠道。



### gRPC 和协议缓冲区Protocol Buffers

gRPC 为 DAOS 管理提供双向安全通道。它依靠 TLS/SSL 来验证管理员角色和服务器。协议缓冲区用于 RPC 序列化，所有 proto 文件都位于[proto](https://github.com/daos-stack/daos/blob/master/src/proto)目录中。



### dRPC

dRPC 是基于 Unix Domain Socket 构建的通信通道，用于进程间通信。它提供了[C](https://github.com/daos-stack/daos/blob/master/src/common/README.md#dRPC-C-API)和[Go](https://github.com/daos-stack/daos/blob/master/src/control/drpc/README.md) 接口来支持以下之间的交互：

- 和用于应用程序`daos_agent`的身份验证`libdaos`
- （`daos_server`控制平面）和`daos_engine`（数据平面）守护进程与 gRPC 一样，RPC 通过协议缓冲区进行序列化。



### CART

[CART](https://github.com/daos-stack/cart)是一个用户空间函数传送库，为 DAOS 数据平面提供低延迟高带宽通信。它支持 RDMA 功能和可扩展的集体操作。CART 建立在[Mercury](https://github.com/mercury-hpc/mercury)和 [libfabric 之上](https://ofiwg.github.io/libfabric/)。CART 库用于实例libdaos和daos_engine之间的所有通信。



## DAOS 分层和服务



### 架构

如下图所示，DAOS 堆栈被构建为客户端/服务器架构上的存储服务集合。DAOS 服务的示例是池、容器、对象和重建服务。

[![DAOS 内部服务和库](docs/graph/services.png)](https://github.com/daos-stack/daos/blob/master/docs/graph/services.png)

DAOS 服务可以分布在控制和数据平面上，并通过 dRPC 进行内部通信。大多数服务都有可以通过 gRPC 或 CART 同步的客户端和服务器组件。跨服务通信始终通过直接 API 调用完成。可以跨服务的客户端或服务器组件调用这些函数调用。虽然每个 DAOS 服务都被设计为相当自治和隔离，但有些服务比其他服务更紧密耦合。这通常是需要与池、容器和对象服务密切交互以在 DAOS 服务器故障后恢复数据冗余的重建服务的情况。

虽然基于服务的架构提供了灵活性和可扩展性，但它与一组基础设施库相结合，提供丰富的软件生态系统（例如通信、持久存储访问、具有依赖关系图的异步任务执行、加速器支持......）所有 DAOS 服务。



### 源代码结构

每个基础设施库和服务都在`src/`. 服务的客户端和服务器组件存储在单独的文件中。作为客户端组件一部分的函数带有前缀`dc\_`（代表 DAOS 客户端），而服务器端函数使用 `ds\_`前缀（代表 DAOS 服务器）。客户端和服务器组件之间使用的协议和 RPC 格式通常定义在名为`rpc.h`.

在控制平面上下文中执行的所有 Go 代码都位于 `src/control`. 管理和安全是跨控制（Go 语言）和数据（C 语言）平面并通过 dRPC 进行内部通信的服务。

暴露给最终用户（即 I/O 中间件或应用程序开发人员）的官方 DAOS API的标头位于`src/include`并使用 `daos\_`前缀。每个基础设施库都导出一个 API，该 API 在任何服务下都可用`src/include/daos`并且可以被任何服务使用。给定服务导出的客户端 API（带`dc\_`前缀）也存储在下`src/include/daos`，而服务器端接口（带`ds\_`前缀）在`src/include/daos_srv`.



### 基础设施库

GURT 和通用 DAOS（即`libdaos\_common`）库为 DAOS 服务提供日志记录、调试和通用数据结构（例如哈希表、btree...）。

本地 NVM 存储由版本控制对象存储 (VOS) 和 blob I/O (BIO) 库管理。VOS 在 SCM 中实现持久索引，而 BIO 负责根据分配策略将应用程序数据存储在 NVMe SSD 或 SCM 中。VEA 层集成到 VOS 中并管理 NVMe SSD 上的块分配。

DAOS 对象分布在性能（即分片）和弹性（即复制或纠删码）的多个目标targets上。放置库placement实现了不同的算法（例如基于环的放置、跳转一致性哈希等），以根据目标列表和对象标识符生成对象的布局。

复制服务 replicated service (RSVC) 库最终提供了一些通用代码来支持容错。这由池、容器和管理服务与 RDB 库一起使用，RDB 库在 Raft 上实现了复制的键值存储。

有关这些基础设施库的进一步阅读，请参阅：

- [公共库](https://github.com/daos-stack/daos/blob/master/src/common/README.md)
- [版本控制对象存储 (VOS)](https://github.com/daos-stack/daos/blob/master/src/vos/README.md)
- [Blob I/O (BIO) Binary Large Object 二进制大对象](https://github.com/daos-stack/daos/blob/master/src/bio/README.md)
- [对象放置算法](https://github.com/daos-stack/daos/blob/master/src/placement/README.md)
- [复制数据库 (RDB)](https://github.com/daos-stack/daos/blob/master/src/rdb/README.md)
- [复制服务框架 (RSVC)](https://github.com/daos-stack/daos/blob/master/src/rsvc/README.md)



### DAOS 服务

下图显示了 DAOS 服务的内部分层以及与上述不同库的交互。 [![DAOS 内部分层](docs/graph/layering.png)](https://github.com/daos-stack/daos/blob/master/docs/graph/layering.png)

垂直框代表 DAOS 服务，而水平框代表基础设施库。

要进一步阅读每个服务的内部结构：

- [存储池服务](https://github.com/daos-stack/daos/blob/master/src/pool/README.md)
- [容器服务](https://github.com/daos-stack/daos/blob/master/src/container/README.md)
- [键数组对象服务](https://github.com/daos-stack/daos/blob/master/src/object/README.md)
- [自我修复（又名重建）](https://github.com/daos-stack/daos/blob/master/src/rebuild/README.md)
- [安全](https://github.com/daos-stack/daos/blob/master/src/security/README.md)



## 软件兼容性

DAOS 中的互操作性是通过持久数据结构的协议和模式版本控制来处理的。



### 协议兼容性

DAOS 存储堆栈将提供有限的协议互操作性。将执行版本兼容性检查以验证：

- 同一池中的所有目标都运行相同的协议版本。
- 与应用程序链接的客户端库最多可能比目标版本早一个协议版本, 允许版本跨度为1。

如果检测到同一个存储池中的存储目标之间协议版本不匹配，整个DAOS系统将无法启动，并向控制API报告失败。同样，来自运行与目标不兼容的协议版本的客户端的连接将返回错误。



### PM Schema 持久存储模式兼容性和升级

持久数据结构的模式可能会不时演变以修复错误、添加新的优化或支持新功能。为此，持久数据结构支持模式版本控制。

升级架构版本不会自动完成，必须由管理员启动。将提供专用的升级工具来将架构版本升级到最新版本。同一池中的所有目标必须具有相同的架构版本。版本检查在系统初始化时执行以强制执行此约束。

为了限制验证矩阵，每个新的 DAOS 版本都将发布支持的架构版本列表。要使用新的 DAOS 版本运行，管理员需要将 DAOS 系统升级到支持的架构版本之一。新目标将始终使用最新版本重新格式化。此版本控制模式仅适用于存储在持久内存中的数据结构，不适用于仅存储没有元数据的用户数据的块存储。





# DAOS 控制平面daos_server

DAOS 在两个紧密集成的平面上运行，控制和数据。数据平面处理繁重的传输操作，而控制平面协调流程和存储管理，促进数据平面的操作。

DAOS 控制平面是用 Go 编写的，并作为 DAOS 服务器 ( `daos_server`) 进程运行。除了在同一主机上运行的 DAOS 数据平面（引擎Engine）进程的实例化和管理之外，它还负责网络和存储硬件的配置和分配。

## 代码组织

控制目录包含用于服务器、代理和 dmg 应用程序的“cmd”子目录。这些应用程序导入控制 API ( `src/control/lib/control`) 或服务器包以及提供给定功能所需的外围共享包 common、drpc、fault、logging 和 security。

特定的库包可以在 lib/ 中找到，它们通过语言绑定（例如 lib/spdk）或特定的格式化功能（例如 lib/hostlist 或 lib/txtfmt）提供对本机存储库的访问。

events 包提供了 RAS 框架的 golang 组件，用于通过 dRPC 从 DAOS 引擎接收事件，并将管理服务可操作事件转发给 MS 领导者。

pbin 包提供了一个框架，用于转发由特权二进制文件`daos_admin`代表`daos_server`.

提供程序包provider包含外部环境external的接口垫片，最初只是针对 Linux 操作系统。

系统包system封装了 DAOS 系统的概念及其相关的成员资格。

## 开发者文档

请参阅特定于软件包的自述文件README。

- [服务器[server](https://github.com/daos-stack/daos/blob/master/src/control/server/README.md)](https://github.com/daos-stack/daos/blob/master/src/control/server/README.md)
- [godoc 参考](https://godoc.org/github.com/daos-stack/daos/src/control)

## 用户文档

- [在线文档](https://daos-stack.github.io/)



# DAOS 数据平面（又名 daos_engine）

## 模块接口

I/O 引擎支持允许按需加载服务器端代码的模块接口。每个模块实际上都是由 I/O 引擎通过 dlopen 动态加载的库。模块和 I/O 引擎之间的接口在`dss_module`数据结构中定义。

每个模块应指定：

- 模块名称
- 来自的模块标识符`daos_module_id` identifier
- 特征位掩码 bitmask
- 模块初始化和完成函数 initialization and finalize function

此外，模块可以选择配置：

- 整个堆栈启动并运行后调用的设置setup和清理函数cleanup
- CART RPC 处理程序 handlers
- dRPC 处理程序 handlers

## 线程模型和 Argobot 集成

I/O 引擎是一个使用 Argobots 进行非阻塞处理的多线程进程。

默认情况下，每个目标会创建一个主 xstream 和无卸载 xstream。可以通过 daos_engine 命令行参数配置实际卸载 xstream 的数量。此外，还会创建一个额外的 xstream 来处理传入的元数据请求。每个 xstream 都绑定到特定的 CPU 内核。主要的 xstream 是接收来自客户端和其他服务器的传入目标请求的流。启动特定 ULT 以在网络和 NVMe I/O 操作上取得进展。

## 线程本地存储 (TLS) Thread-local Storage

每个 xstream 分配可以通过`dss_tls_get()`函数访问的私有存储。注册时，每个模块可以指定一个模块键，其数据结构大小将由 TLS 中的每个 xstream 分配。该`dss_module_key_get()`函数将为特定的已注册模块密钥返回此数据结构。

## Incast 变量集成

*Incast* 是指一种多对一的通信模式，常见于部署了大量的分布式存储或者计算服务（如Hadoop, MapReduce, HDFS, Cassandra等）的数据中心中

DAOS 使用 IV（incast 变量）在单个 IV 命名空间下的服务器之间共享值和状态，该命名空间以树的形式组织。树根称为 IV 领导者，服务器可以是叶子或非叶子。每个服务器都维护自己的 IV 缓存。在 fetch 期间，如果本地缓存不能满足请求，它会将请求转发给其父级，直到到达根（IV 领导者）。至于更新，它首先更新它的本地缓存，然后转发给它的父节点，直到它到达根节点，然后将更改传播到所有其他服务器。IV 命名空间是每个池的，在池连接期间创建，在池断开连接时销毁。要使用IV，每个用户都需要在IV命名空间下注册自己以获得一个标识，然后它会使用这个ID在IV命名空间下获取或更新自己的IV值。

## dRPC 服务器

I/O 引擎包括一个 dRPC 服务器，用于侦听给定 Unix 域套接字上的活动。有关 dRPC基础知识以及 Go 和 C 中的低级 API 的更多详细信息，请参阅[dRPC 文档。](https://github.com/daos-stack/daos/blob/master/src/control/drpc/README.md)

dRPC 服务器会定期轮询传入的客户端连接和请求。它可以通过对象处理多个同时的客户端连接，该`struct drpc_progress_context`对象管理`struct drpc`侦听套接字的对象以及任何活动的客户端连接。

服务器循环在 xstream 0 中其自己的用户级线程 (ULT) 中运行。dRPC 套接字已设置为非阻塞，轮询使用超时 0，这允许服务器在 ULT 中运行，而不是在其自己的 xstream 中运行. 预计该频道的流量相对较低。

### dRPC 进程

`drpc_progress`代表 dRPC 服务器循环的一次迭代。工作流程如下：

1. 同时轮询侦听套接字和任何打开的客户端连接超时。
2. 如果在客户端连接上看到任何活动：
   1. 如果有数据进来：调用`drpc_recv`来处理进来的数据。
   2. 如果客户端已断开连接或连接已断开：释放`struct drpc`对象并将其从`drpc_progress_context`.
3. 如果在侦听器上看到任何活动：
   1. 如果有新连接进入：调用`drpc_accept`并将新`struct drpc`对象添加到`drpc_progress_context`.
   2. 如果有错误：返回`-DER_MISC`给调用者。这会导致在 I/O 引擎中记录错误，但不会中断 dRPC 服务器循环。在侦听器上收到错误是出乎意料的。
4. 如果没有看到任何活动，请返回`-DER_TIMEDOUT`给调用者。这纯粹是出于调试目的。实际上，I/O 引擎会忽略此错误代码，因为缺少活动实际上并不是错误情况。

### dRPC 处理程序注册

单个 DAOS 模块可以通过为一个或多个 dRPC 模块 ID 注册处理函数来实现对 dRPC 消息的处理。

注册处理程序很简单。在`dss_server_module`field 中`sm_drpc_handlers`，静态分配一个数组，数组中`struct dss_drpc_handler`的最后一项归零以指示列表的结尾。将该字段设置为 NULL 表示无需注册。当 I/O 引擎加载 DAOS 模块时，它会自动注册所有的 dRPC 处理程序。

**注意：** dRPC 模块 ID**与**DAOS 模块 ID 不同。这是因为给定的 DAOS 模块可能需要注册多个 dRPC 模块 ID，具体取决于它所涵盖的功能。dRPC 模块 ID 在系统范围内必须是唯一的，并列在中央头文件中：`src/include/daos/drpc_modules.h`

dRPC 服务器使用该函数`drpc_hdlr_process_msg`来处理传入的消息。此函数检查传入消息的模块 ID，搜索处理程序，如果找到则执行处理程序，并返回`Drpc__Response`. 如果没有找到，它会生成自己的`Drpc__Response`指示模块 ID 未注册。



# dRPC

dRPC 是通过 Unix 域套接字在同一物理系统的本地进程之间进行通信的一种方式。在任何给定时间，进程都可以充当客户端、服务器或两者兼而有之，尽管每个侦听 dRPC 服务器都需要自己的 Unix 域套接字。

如果文件系统中的该位置已经存在某些内容，则服务器将无法创建套接字，即使它是套接字的旧版本。或者，您的应用程序可能希望在创建套接字之前取消链接该文件系统位置。

dRPC 调用由模块和方法标识符定义。dRPC 模块可以被认为是相关功能的一个包。dRPC 方法指示服务器要执行的特定功能。如果该方法需要输入，则应将其编组在 dRPC 调用的主体中。服务器将使用 dRPC 响应结构进行响应，该结构可能在正文中包含特定于方法的响应。

DAOS dRPC 实现依赖于协议缓冲区来定义通过 dRPC 通道传递的结构。作为调用或响应的一部分通过 dRPC 发送的任何结构都必须[在 .proto 文件中定义](https://github.com/daos-stack/daos/blob/master/src/proto)。

## Go API

在 Go 中，drpc 包括客户端和服务器功能，如下所述。有关 C API 的文档，请参阅[此处](https://github.com/daos-stack/daos/blob/master/src/common/README.md)。

dRPC 调用和响应由 Protobuf 生成的`drpc.Call`结构`drpc.Response`体表示。

### Go Client

dRPC 客户端由`drpc.ClientConnection`对象表示。

#### 基本客户工作流程

1. 使用 dRPC 服务器的 Unix 域套接字的路径创建一个新的客户端连接：

   ```
   conn := drpc.NewClientConnection("/var/run/my_socket.sock")
   ```

2. 连接到 dRPC 服务器：

   ```
   err := conn.Connect()
   ```

3. 创建您的

   ```
   drpc.Call
   ```

   并将其发送到服务器：

   ```
   call := drpc.Call{}
   // Set up the Call with module, method, and body
   resp, err := drpc.SendMsg(call)
   ```

   错误表示

   ```
   drpc.Call
   ```

   无法发送，或

   ```
   drpc.Response
   ```

   收到无效。如果没有返回错误，

   ```
   drpc.Response
   ```

   仍应检查服务器报告的错误内容。

4. 根据需要发送尽可能多的呼叫。

5. 完成后关闭连接：

   ```
   conn.Close()
   ```

### Go Server

dRPC 服务器由`drpc.DomainSocketServer`对象表示。

单个 dRPC 模块必须在服务器上注册才能处理该模块的传入 dRPC 调用。要创建 dRPC 模块，请创建一个实现`drpc.Module`接口的对象。模块 ID 必须是唯一的。

#### 基本服务器工作流程

1. 使用服务器的 Unix 域套接字创建新的 DomainSocketServer：

   ```
   drpcServer, err := drpc.NewDomainSocketServer("/var/run/my_socket.sock")
   ```

2. 注册服务器需要处理的 dRPC 模块：

   ```
   drpcServer.RegisterRPCModule(&MyExampleModule{})
   drpcServer.RegisterRPCModule(&AnotherExampleModule{})
   ```

3. 启动服务器以启动 Goroutine 以开始侦听和处理传入连接：

   ```
   err = drpc.Start()
   ```

4. 当需要关闭服务器时，关闭正在监听的 Goroutine：

   ```
   drpcServer.Shutdown()
   ```





# 集体和 RPC 运输 (CaRT)

https://github.com/daos-stack/cart

> ⚠️ **警告：** CaRT 正在大力开发中。使用风险自负。

CaRT 是用于大数据和 Exascale HPC 的开源 RPC 传输层。它支持传统的 P2P RPC 交付和集体 RPC，后者通过可扩展的基于树的消息传播在一组目标服务器上调用 RPC。

# Gurt 有用的例程和类型 (GURT) 

GURT Useful Routines and Types 是一个辅助函数和数据类型的开源库。该库使操作列表、哈希表、堆和日志记录变得容易。

所有 Gurt 有用的例程和类型都以“d”为前缀，这是字母表中的第 4 个字母，因为 gurt 有用的单词有 4 个字母。

## 执照 License

CaRT 是在 BSD 许可下分发的开源软件。请参阅[许可证](https://github.com/daos-stack/cart/blob/master/LICENSE)和[通知](https://github.com/daos-stack/cart/blob/master/NOTICE)文件以获取更多信息。

## 构建 Build

CaRT 需要支持 C99 的编译器和 scons 构建工具来构建。

CaRT 依赖于一些第三方库：

- Mercury ( https://github.com/mercury-hpc/mercury ) 用于 RPC 和底层通信 Mercury 使用 openpa ( http://git.mcs.anl.gov/radix/openpa.git ) 进行原子操作。
- PMIx ( https://github.com/pmix/master ) PMIx 使用 hwloc 库 (wget [https://www.open-mpi.org/software/hwloc/v1.11/downloads/hwloc-1.11.2.tar .gz](https://www.open-mpi.org/software/hwloc/v1.11/downloads/hwloc-1.11.2.tar.gz)）。
- Openmpi 运行环境 ompi 需要使用外部 PMIx/hwloc 进行编译（示例配置为“./configure --with-pmix=/your_pmix_install_path / --with-hwloc=/your_hwloc_install_path --with-libevent=external”） .

安装完所有依赖模块后，可以在顶级源目录中执行“scons”来构建它。





# DAOS 通用库

[DAOS 通用库](src/common/README.md)

所有 DAOS 组件共享的通用功能和基础设施在外部共享库中提供。这包括以下功能：

- 哈希和校验和例程
- 对非阻塞操作的事件和事件队列支持
- 记录和调试基础设施
- 锁定原语
- 网络传输

## 任务调度引擎 (TSE) Task Scheduler Engine

TSE 是一个通用库，用于创建具有函数回调的通用任务，可选地添加这些任务之间的依赖关系，并将它们安排在一个引擎中，该引擎按照插入它们的依赖关系图确定的顺序执行这些任务。任务依赖图是调度程序的组成部分，允许用户创建多个任务并以非阻塞方式进行。

TSE 不是 DAOS 特定的，但曾经是 DAOS 核心的一部分，后来作为独立 API 被提取到 common src 中。API 是通用的，允许在没有任何 DAOS 特定功能的引擎中创建任务。DAOS 库确实提供了一个构建在 TSE 之上的任务 API。有关这方面的更多信息，请参见[此处](https://github.com/daos-stack/daos/blob/master/src/client/api/README.md)。此外，DAOS 在内部使用 TSE 来跟踪和处理与 API 事件关联的所有 API 任务，并且在某些情况下，调度与单个 API 任务相对应的多个飞行中的“子”任务，并添加对该任务的依赖以进行跟踪所有那些飞行中的“子”任务。DAOS 库中的 Array API 和具有多个副本的对象更新就是一个例子。

### 调度程序 API

调度程序 API 允许用户创建通用调度程序并向其添加任务。在创建调度器时，用户可以注册一个完成回调，以便在调度器完成时调用。

添加到调度程序的任务不会自行进行。必须在调度程序上显式调用进度函数 (daos_sched_progress) 才能在引擎中的任务上取得进展。用户可以在他们的程序中偶尔调用这个进度函数，或者可以派生一个重复调用进度函数的线程。

### 任务 API

任务 API 允许创建具有通用主体功能的任务并将它们添加到调度程序。一旦在调度程序中创建了一个任务，如果没有用户对任务调度函数的显式调用，它就不会被实际调度运行，除非它是任务依赖关系图的一部分，在这种情况下，显式调度调用只需要图中的第一个任务。创建任务后，用户可以为任务注册任意数量的依赖项，这些依赖项需要在计划运行之前完成。此外，用户将能够在任务上注册准备和完成回调：

- 准备回调在任务准备好运行但尚未执行时执行，这意味着创建任务的依赖关系已完成并且调度程序已准备好调度任务。当要调度的任务需要在创建任务时不可用但在任务的依赖关系完成后可用的信息时，这很有用；例如为任务主体功能设置一些输入参数。
- 完成回调在任务完成执行时执行，并且当发生这种情况时用户需要做更多的工作或处理。这很有用的一个示例是设置在 TSE 之上构建的更高级别的事件或请求的完成，或者跟踪依赖项列表中多个任务的错误状态。

任务 API 上的其他几个功能可以支持：

- 在任务本身上设置一些可以查询的私有数据。
- 在没有数据复制的情况下向/从任务堆栈空间推送和弹出数据
- 通用任务列表

有关该功能的更多详细信息，请参见[此处](https://github.com/daos-stack/daos/blob/master/src/include/daos/tse.h)DAOS 代码中的 TSE 标头。

## dRPC C API

有关 dRPC 概念和相应 Go API 的一般概述，请参见 [此处](https://github.com/daos-stack/daos/blob/master/src/control/drpc/README.md)。

在 C API 中，活动的 dRPC 连接由指向上下文对象 ( `struct drpc`) 的指针表示。上下文提供了通过 Unix 域套接字进行通信所需的所有状态信息。

默认情况下，上下文以一个对其的引用开始。您可以使用 . 添加对给定上下文的引用`drpc_add_ref()`。完成上下文后，应使用 释放对象`drpc_close()`。当最后一个引用被释放时，对象被释放。

dRPC 调用和响应由 Protobuf 生成的结构 `Drpc__Call`和`Drpc__Response`.

### C 客户端

连接到有效的 Unix 域套接字会返回一个 dRPC 上下文，该上下文可用于对设置套接字的服务器执行任意数量的 dRPC 调用。

**注意：**目前同步调用（使用标志`R_SYNC`）是唯一支持的类型。异步调用会收到即时响应，但从未真正处理过。

#### 基本客户工作流程

1. 打开与服务器的 Unix Domain Socket 的连接：

   ```
   struct drpc *ctx;
   rc = drpc_connect("/var/run/my_socket.sock", &ctx);
   ```

2. 发送 dRPC 调用：

   ```
   Drpc__Call *call;
   /* Alloc and set up your Drpc__Call */
   Drpc__Response *resp = NULL; /* Response will be allocated by drpc_call */
   int result = drpc_call(ctx, R_SYNC, call, &resp);
   ```

   从返回的错误代码

   ```
   drpc_call()
   ```

   表示消息无法发送，或者没有响应。如果

   ```
   drpc_call()
   ```

   返回成功，仍然需要检查响应的内容是否有从服务器返回的错误。

3. 根据需要发送尽可能多的 dRPC 调用。

4. 完成连接后，关闭它：`drpc_close(ctx);` **注意**：`drpc_close()`调用后，dRPC 上下文指针已被释放并且不再有效。

### C 服务器

dRPC 服务器设置一个 Unix 域套接字并开始在其上侦听客户端连接。通常，这意味着创建一个侦听 dRPC 上下文来检测任何传入连接。然后，当客户端连接时，会为该特定会话生成一个新的 dRPC 上下文。会话上下文是实际发送和接收数据的上下文。可以同时打开多个会话上下文。

套接字始终设置为非阻塞，因此有必要`ctx->comm->fd`使用类似系统调用`poll()`或`select()`在符合 POSIX 的系统上轮询上下文文件描述符 ( ) 上的活动。这不仅适用于侦听 dRPC 上下文，还适用于为特定客户端会话生成的任何 dRPC 上下文。

服务器流依赖于自定义处理函数，其工作是`Drpc__Call`适当地分派传入的消息。处理程序函数应检查模块和方法 ID，确保执行所需的方法，并`Drpc__Response`根据结果填充 a。

#### 基本服务器工作流程

1. 在给定路径设置 Unix 域套接字并使用自定义处理函数创建侦听上下文：

   ```
   void my_handler(Drpc__Call *call, Drpc__Response *resp) {
       /* Handle the message based on module/method IDs */
   }
   ...
   struct drpc *listener_ctx = drpc_listen("/var/run/drpc_socket.sock",
                                           my_handler);
   ```

2. 轮询侦听器上下文的文件描述符 ( `listener_ctx->comm->fd`)。

3. 在传入活动中，接受连接：

   ```
   struct drpc *session_ctx = drpc_accept(listener_ctx);
   ```

   这会为特定客户端创建会话上下文。该客户端的所有通信都将通过会话上下文进行。

4. 轮询会话上下文的文件描述符 ( `session_ctx->comm->fd`) 以获取传入数据。

5. 在传入数据时，接收消息：

   ```
   Drpc__Call *incoming;
   int result = drpc_recv_call(session_ctx, &incoming);
   if (result != 0) {
       /* process errors */
   }
   ```

   这会将传入的数据解组为

   ```
   Drpc__Call
   ```

   . 如果数据不是 a 

   ```
   Drpc__Call
   ```

   ，则返回错误。

6. 分配一个 Drpc__Response 并将其传递给会话处理程序。

   ```
   Drpc__Response *resp = drpc_response_create(call);
   session_ctx->handler(call, resp);
   ```

   您的会话处理程序应使用任何错误或有效负载填写响应。

7. 将响应发送给调用者并清理：

   ```
   int result = drpc_send_response(session_ctx, resp);
   if (result != 0) {
       /* process errors */
   }
   drpc_response_free(resp);
   drpc_call_free(call);
   ```

8. 如果客户端已关闭连接，请关闭会话上下文以释放指针：

   ```
   drpc_close(session_ctx);
   ```

9. 当需要关闭服务器时，请关闭所有打开的会话上下文，如上所述。然后`drpc_close()`是侦听器上下文。

## 校验和

### Checksummer

“校验和”用于从分散聚集列表创建校验和。checksummer 使用一个函数表（在初始化时指定）将常见的校验和调用适配到实现校验和算法的底层库。目前 isa-l 和 isa-l_crypto 库用于支持 adler32、crc16、crc32、crc64、sha1、sha256 和 sha512。支持这些算法的所有函数表都在 [src/common/multihash_isal.c](https://github.com/daos-stack/daos/blob/master/src/common/multihash_isal.c). 这些函数表未公开，但有一个辅助函数 (daos_mhash_type2algo) 将在给定 DAOS_CSUM_TYPE 的情况下返回适当的函数表。还有另一个帮助函数 (daos_contprop2csumtype) 将容器属性值转换为适当的 DAOS_CSUM_TYPE。从容器属性校验和值到函数表的双重“查找”是为了消除校验和和容器信息的耦合。

所有校验和函数都应以 daos_csummer_* 开头，并以结构 daos_csummer 作为第一个参数。为了初始化一个新的 daos_csummer，daos_csummer_init 将地址指向一个指针（因此可以分配内存），一个函数表的地址实现了所需的校验和算法，并且因为这是一个 DAOS 校验和，用于“块”的大小（见 [VOS](https://github.com/daos-stack/daos/blob/master/src/vos/README.md)有关块和块大小的详细信息）。如果不是因为需要将传入的数据分成块，校验和就不需要块大小。完成校验和后，应调用 daos_csummer_destroy 以释放分配的资源。如果实现，大多数校验和函数都是对函数表的简单传递。主要的例外是 daos_csummer_calc，它使用其他校验和函数，从由分散聚集列表 (d_sg_list_t) 和 I/O 描述符 (daos_iod_t) 的范围 (daos_recx_t) 表示的适当内存创建校验和。校验和被放入校验和缓冲区 (daos_csum_buf_t) 的集合中，每个缓冲区包含多个校验和。daos_csum_buf_t 和校验和的内存将被分配。因此，当完成校验和时，

有一组辅助函数（以 dcb 为前缀）作用于 daos_csum_buf_t。这些功能应该是直截了当的。daos_csum_buf_t 包含一个指向校验和第一个字节的指针以及校验和的相关信息，包括计数、校验和的大小等。