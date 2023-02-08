# DAOS 性能调优 [clickme](https://docs.daos.io/v2.0/admin/performance_tuning/#daos-performance-tuning)

本节介绍如何验证 DAOS 系统（即网络和存储）中的基准构建块的性能，然后是整个堆栈。

## 网络性能[¶](https://docs.daos.io/v2.0/admin/performance_tuning/#network-performance)

DAOS [CarRT](https://github.com/daos-stack/daos/tree/release/2.0/src/cart/) （集体和 RPC 传输）层可以在与应用程序相同的上下文中验证和基准测试网络通信，并使用与常规 DAOS 相同的网络/调整选项。

CaRT`self_test`可以在生产环境中以非破坏性方式在 DAOS 服务器上运行。唯一的要求是在运行 self_test 的客户端节点上运行格式化的 DAOS 系统和 DAOS 代理。

### 参数[¶](https://docs.daos.io/v2.0/admin/performance_tuning/#parameters)

`self_test`支持不同的消息大小、批量传输、多个目标以及以下测试场景：

- 自测客户端到服务器 - 将`self_test`RPC 直接发送到服务器列表。
- 跨服务器 -`self_test`向将发出跨服务器 RPC 的不同服务器发送指令。该模型支持多对多通信模型。

通过`--master-endpoint`选项选择模式。如果在命令行上通知了此选项，那么我们处于第一种模式，并且 self_test 二进制文件本身发出 RPC。如果指定了一个或多个主端点，那么我们处于跨服务器模式。

端点由以冒号分隔的两个值对组成。第一个值是与 中显示的引擎排名相匹配的排名`dmg system query`。第二个值称为标记，用于标识引擎中的服务线程。DAOS 引擎使用以下映射：

- 标签 0 由元数据服务处理池和容器操作使用。
- 标签 1 用于跨服务器监控 (SWIM)。
- DAOS 目标使用标签 2 到 [#targets + 1]（每个目标一个标签）。
- 标签 [#targets + 2] 到 [#targets + #helpers + 1] 由助手服务线程使用。

例如，具有`targets: 16`并且`nr_xs_helpers: 4`将使用以下标签分布的引擎：

- 标签 0：元数据服务
- 标签一：监控服务
- 标记 2-17：目标 0 到 15（总共 16 个目标）
- 标签 18-21：助手服务

该引擎总共导出了 21 个端点。

通过网络发送的 RPC 流可以通过`--message-sizes` 采用大小元组列表的选项进行配置。将为每个元组单独报告性能。每个大小整数都可以在前面加上一个字符来指定底层传输机制。可用类型有：

- 'e' - 空（无负载）
- 'i' - I/O 向量 (IOV)
- 'b' - 批量传输

例如，(b1000) 将通过批量双向传输 1000 个字节。同样， (i100 b1000) 将使用 IOV 发送和批量回复。

`--repetitions-per-size`可用于定义每个端点每个消息大小的样本数，默认值为 10000。`--max-inflight-rpcs` 确定同时发出的并发 RPC 数。

`self_test`有更多的选择。有关`self_test`用法的完整说明，请运行：

```bash
$ self_test --help
```

### 示例：客户端到服务器[¶](https://docs.daos.io/v2.0/admin/performance_tuning/#example-client-to-servers)

在客户端到服务器模式下运行 self_test：

```bash
self_test -u --group-name daos_server --endpoint 0:2 --message-size '(0 b1048578)' --max-inflight-rpcs 16 --repetitions 100000
```

这将发送 100k RPC 和一个空请求，一个 1MB 的批量放置，然后是一个从运行 self_test 应用程序的节点到引擎 rank 0 的第一个目标的空回复。这个工作负载有效地模拟了一个 1MB 的 DAOS 上的获取/读取 RPC .

将使用以下命令模拟 1MB 的更新/写入 RPC：

```bash
self_test -u --group-name daos_server --endpoint 0:2 --message-size "(b1048578 0)" --max-inflight-rpcs 16 --repetitions 100000
```

空请求和回复的 RPC 速率通常也有助于评估网络的最大能力。这可以通过以下命令行来实现：

```bash
self_test -u --group-name daos_server --endpoint 0:2 --message-size "(0 0)" --max-inflight-rpcs 16 --repetitions 100000
```

例如，0 可以替换为 i2048 以发送 2Kb 的有效负载。

所有这 3 个测试都可以组合在一个独特的运行中：

```bash
self_test -u --group-name daos_server --endpoint 0:2 --message-size "(0 0) (b1048578 0) (0 b1048578)" --max-inflight-rpcs 16 --repetitions 100000
```

RPC 也可以发送到一系列引擎等级和标签，如下所示：

```bash
self_test -u --group-name daos_server --endpoint 0-<MAX_RANK>:0-<MAX_TAG> --message-size "(0 0) (b1048578 0) (0 b1048578)" --max-inflight-rpcs 16 --repetitions 100000
```

笔记

默认情况下，self_test 将使用代理选择的网络接口。这可以通过手动设置 OFI_INTERFACE 和 OFI_DOMAIN 环境变量来强制执行。例如导出 OFI_INTERFACE=eth0；导出 OFI_DOMAIN=eth0 或导出 OFI_INTERFACE=ib0；导出 OFI_DOMAIN=mlx5_0

笔记

根据硬件配置，代理可能会根据调度进程的 NUMA 节点为 self_test 应用程序分配不同的网络接口。因此建议使用 taskset 将 self_test 进程绑定到特定的核心。例如taskset -c 1 self_test ...

### 示例：跨服务器[¶](https://docs.daos.io/v2.0/admin/performance_tuning/#example-cross-servers)

在跨服务器模式下运行 self_test：

```bash
$ self_test -u --group-name daos_server --endpoint 0-<MAX_SERVER-1>:0 \
  --master-endpoint 0-<MAX_RANK>:0-<MAX_TAG> \
  --message-sizes "b1048576,b1048576 0,0 b1048576,i2048,i2048 0,0 i2048" \
  --max-inflight-rpcs 16 --repetitions 100
```

`self_test`上面的命令将使用以下消息大小运行基准测试：

```bash
b1048576     1Mb bulk transfer Get and Put
b1048576 0   1Mb bulk transfer Get only
0 b1048576   1Mb bulk transfer Put only
I2048        2Kb iovec Input and Output
i2048 0      2Kb iovec Input only
0 i2048      2Kb iovec Output only
```

笔记

重复次数、最大飞行 RPC、消息大小可以根据特定的测试/实验进行调整。

## 存储性能[¶](https://docs.daos.io/v2.0/admin/performance_tuning/#storage-performance)

### 单片机[¶](https://docs.daos.io/v2.0/admin/performance_tuning/#scm)

DAOS 提供了一个工具`vos_perf`，用于在存储类内存上对版本控制对象存储进行基准测试。有关`vos_perf`用法的完整说明，请运行：

```bash
$ vos_perf --help
```

该`-R`选项用于定义要执行的操作：

- `U`for `update`（即写）操作
- `F`for `fetch`（即读）操作
- `P`用于`punch`（即截断）操作
- `p`显示上一个操作的性能结果。

例如，-R "U;p F;p" 表示更新密钥，打印更新速率/带宽，获取密钥，然后打印获取速率/带宽。object/dkey/akey/value 的数量可以分别通过 -o、-d、-a 和 -n 选项传递。值大小通过 -s 参数指定（例如 -s 4K 表示 4K 值）。

例如，要在 VOS 模式下测量 10M 更新和获取操作的速率，请挂载 pmem 设备，然后运行：

```bash
$ cd /mnt/daos0
$ df .
Filesystem      1K-blocks  Used  Available Use% Mounted on
/dev/pmem0     4185374720 49152 4118216704   1% /mnt/daos0
$ taskset -c 1 vos_perf -f ./vos -P 100G -d 10000000 -a 1 -n 1 -s 4K -z -R "U;p F;p"
Test :
        VOS (storage only)
Pool :
        a3b7ff28-56ff-4974-9283-62990dd770ad
Parameters :
        pool size     : SCM: 102400 MB, NVMe: 0 MB
        credits       : -1 (sync I/O for -ve)
        obj_per_cont  : 1 x 1 (procs)
        dkey_per_obj  : 10000000 (buf)
        akey_per_dkey : 1
        recx_per_akey : 1
        value type    : single
        stride size   : 4096
        zero copy     : yes
        VOS file      : ./vos0
Running test=UPDATE
Running UPDATE test (iteration=1)
UPDATE successfully completed:
        duration  : 124.478568 sec
        bandwidth : 313.809    MB/sec
        rate      : 80335.11   IO/sec
        latency   : 12.448     us (nonsense if credits > 1)
Duration across processes:
        MAX duration : 124.478568 sec
        MIN duration : 124.478568 sec
        Average duration : 124.478568 sec
Completed test=UPDATE
Running test=FETCH
Running FETCH test (iteration=1)
FETCH successfully completed:
        duration  : 23.884087  sec
        bandwidth : 1635.503   MB/sec
        rate      : 418688.81  IO/sec
        latency   : 2.388      us (nonsense if credits > 1)
Duration across processes:
        MAX duration : 23.884087  sec
        MIN duration : 23.884087  sec
        Average duration : 23.884087  sec
```

Taskset 用于改变 daos_perf 进程的 CPU 亲和性。

警告

持久内存的性能可能会受到 NUMA 亲和性的影响。因此，建议将 daos_perf 的亲和性设置为本地连接到持久内存设备的 CPU 内核。

可以在第二个 pmem 设备上执行相同的测试来比较性能。

```
$ cd /mnt/daos1/
$ df .
Filesystem      1K-blocks   Used  Available Use% Mounted on
/dev/pmem1     4185374720 262144 4118003712   1% /mnt/daos1
$ taskset -c 36 vos_perf -f ./vos -P 100G -d 10000000 -a 1 -n 1 -s 4K -z -R "U;p F;p"
Test :
        VOS (storage only)
Pool :
        9d6c3fbd-a4f1-47d2-92a9-6112feb52e74
Parameters :
        pool size     : SCM: 102400 MB, NVMe: 0 MB
        credits       : -1 (sync I/O for -ve)
        obj_per_cont  : 1 x 1 (procs)
        dkey_per_obj  : 10000000 (buf)
        akey_per_dkey : 1
        recx_per_akey : 1
        value type    : single
        stride size   : 4096
        zero copy     : yes
        VOS file      : ./vos0
Running test=UPDATE
Running UPDATE test (iteration=1)
UPDATE successfully completed:
        duration  : 123.389467 sec
        bandwidth : 316.579    MB/sec
        rate      : 81044.19   IO/sec
        latency   : 12.339     us (nonsense if credits > 1)
Duration across processes:
        MAX duration : 123.389467 sec
        MIN duration : 123.389467 sec
        Average duration : 123.389467 sec
Completed test=UPDATE
Running test=FETCH
Running FETCH test (iteration=1)
FETCH successfully completed:
        duration  : 24.114830  sec
        bandwidth : 1619.854   MB/sec
        rate      : 414682.58  IO/sec
        latency   : 2.411      us (nonsense if credits > 1)
Duration across processes:
        MAX duration : 24.114830  sec
        MIN duration : 24.114830  sec
        Average duration : 24.114830  sec
Completed test=FETCH
```

可以通过使用更大的记录大小（即 -s 选项）来测试带宽。例如：

```
$ taskset -c 36 vos_perf -f ./vos -P 100G -d 40000 -a 1 -n 1 -s 1M -z -R "U;p F;p"
Test :
        VOS (storage only)
Pool :
        dc44f0dd-930e-43b1-b599-5cc141c868d9
Parameters :
        pool size     : SCM: 102400 MB, NVMe: 0 MB
        credits       : -1 (sync I/O for -ve)
        obj_per_cont  : 1 x 1 (procs)
        dkey_per_obj  : 40000 (buf)
        akey_per_dkey : 1
        recx_per_akey : 1
        value type    : single
        stride size   : 1048576
        zero copy     : yes
        VOS file      : ./vos0
Running test=UPDATE
Running UPDATE test (iteration=1)
UPDATE successfully completed:
        duration  : 21.247287  sec
        bandwidth : 1882.593   MB/sec
        rate      : 1882.59    IO/sec
        latency   : 531.182    us (nonsense if credits > 1)
Duration across processes:
        MAX duration : 21.247287  sec
        MIN duration : 21.247287  sec
        Average duration : 21.247287  sec
Completed test=UPDATE
Running test=FETCH
Running FETCH test (iteration=1)
FETCH successfully completed:
        duration  : 10.133850  sec
        bandwidth : 3947.167   MB/sec
        rate      : 3947.17    IO/sec
        latency   : 253.346    us (nonsense if credits > 1)
Duration across processes:
        MAX duration : 10.133850  sec
        MIN duration : 10.133850  sec
        Average duration : 10.133850  sec
Completed test=FETCH
```

笔记

使用第三代英特尔® 至强® 可扩展处理器 (ICX)，PMEM_NO_FLUSH 环境变量可以设置为 1，以利用扩展异步 DRAM 刷新 (eADR) 功能

一个名为 daos_perf 的工具与 vos_perf 具有相同的语法，也可用于从具有完整 DAOS 堆栈的计算节点运行测试。请参阅下一节了解更多信息。

### 固态硬盘[¶](https://docs.daos.io/v2.0/admin/performance_tuning/#ssds)

SSD 的性能可以通过 spdk_nvme_perf 工具直接使用 SPDK 进行测量。可以运行它以非破坏性方式测试带宽，如下所示：

```bash
spdk_nvme_perf -q 16 -o 1048576 -w read -c 0xff -t 60
```

可以使用以下命令测量 IOPS：

```bash
spdk_nvme_perf -q 16 -o 4096 -w read -c 0xff -t 60
```

`-q`用于控制队列深度，`-o`对于 I/O 大小，`-w`是操作，可以是 (rand)read、(rand)write 或 (rand)rw。测试持续时间（以分钟为单位）由`-t`参数定义。

警告

*write 和*rw 选项具有破坏性。

此命令使用所有可用的 SSD。可以通过 `--allowed-pci-addr`选项指定特定的 SSD，后跟感兴趣的 SSD 的 PCIe 地址。

该`-c`选项用于指定用于以 core mash 的形式提交 I/O 的 CPU 内核。`-c 0xff`使用前 8 个内核。

笔记

在使用 Intel VMD 的存储节点上，`--enable-vmd`必须指定该选项。

还有更多选择。请运行`spdk_nvme_perf`以查看可以调整的参数列表。

## 端到端性能[¶](https://docs.daos.io/v2.0/admin/performance_tuning/#end-to-end-performance)

DAOS 可以使用几个广泛使用的 IO 基准测试，如 IOR、mdtest 和 FIO。有几个后端可以与这些基准一起使用。

### 伊尔[¶](https://docs.daos.io/v2.0/admin/performance_tuning/#ior)

具有以下后端的IOR ( [https://github.com/hpc/ior )：](https://github.com/hpc/ior)

- IOR API POSIX、MPIIO 和 HDF5 可以与通过 dfuse 访问的 DAOS POSIX 容器一起使用。这可以在没有或使用 I/O 拦截库 ( `libioil`) 的情况下工作。使用时性能明显更好`libioil`。有关 IO 拦截库使用 dfuse 的详细信息，请参阅 [POSIX DFUSE 部分][7]。
- DAOS 的自定义 DFS（DAOS 文件系统）插件可以通过构建具有 DAOS 支持的 IOR 并选择 API=DFS 来使用。这将 IOR 直接与 DAOS 文件系统 ( `libdfs`) 集成，无需 FUSE 或拦截库。有关如何使用 DFS 驱动程序的一些基本说明，请参阅 hpc/ior 存储库中的 [DAOS README][10]。
- 当使用 IOR API=MPIIO 时，可以通过为文件名提供`daos://`前缀来使用 DAOS 的 ROMIO ADIO 驱动程序。此 ADIO 驱动程序绕过`dfuse` 并直接`libdfs`调用对 DAOS POSIX 容器执行 I/O 的调用。支持 DAOS 的 MPIIO 驱动程序在上游 MPICH 存储库中可用，并包含在英特尔 MPI 中。请参考 [MPI-IO 文档][8]。
- 用于 DAOS 的 HDF5 VOL 连接器正在开发中。这会将 HDF5 数据模型直接映射到 DAOS 数据模型，并与 DAOS 容器一起工作 （与用于其他 IOR API 的 DAOS 容器相反）`--type=HDF5`。`--type=POSIX`请参考 [HDF5 with DAOS 文档][9]。

IOR 有几个参数来表征性能。使用的主要参数包括：

- 传输大小 (-t)
- 块大小 (-b)
- 段大小 (-s)

对于更多用例，IO-500 工作负载是衡量系统性能的良好起点：https://github.com/IO500/io500

### 测试[¶](https://docs.daos.io/v2.0/admin/performance_tuning/#mdtest)

mdtest 发布在与 IOR 相同的存储库中。上面列出的相应后端支持 mdtest，但仅设计用于支持 IOR 的 MPI-IO 和 HDF5 后端除外。hpc/ior 存储库中的 [DAOS README][10] 包含一些使用 DAOS 运行 mdtest 的示例。

mdtest 的 IO-500 工作负载为性能测量提供了一些良好的标准。

### FIO[¶](https://docs.daos.io/v2.0/admin/performance_tuning/#fio)

DAOS 引擎集成到 FIO 并在上游可用。要构建它，只需运行：

```bash
$ git clone http://git.kernel.dk/fio.git
$ cd fio
$ ./configure
$ make install
```

如果 DAOS 是通过包安装的，它应该会被自动检测到。如果没有，请指定DAOS库和头文件的路径配置如下：

```
$ CFLAGS="-I/path/to/daos/install/include" LDFLAGS="-L/path/to/daos/install/lib64" ./configure
```

一旦成功构建，一次可以运行默认示例：

```bash
$ export POOL= # your pool UUID
$ export CONT= # your container UUID
$ fio ./examples/dfs.fio
```

请注意，在读取稀疏 POSIX 文件中的漏洞时，DAOS 不会通过网络传输数据（即零）。因此，如果 fio 读取文件中未分配的范围，则可以报告非常高的读取带宽。因此，从第一个写入阶段开始 fio 是一个很好的做法。

FIO 也可用于使用 dfuse 和拦截库以及所有基于 POSIX 的引擎（如 sync 和 libaio）对 DAOS 性能进行基准测试。

### daos_perf[¶](https://docs.daos.io/v2.0/admin/performance_tuning/#daos_perf)

最后，DAOS 提供了一个名为的工具`daos_perf`，允许直接对 DAOS 对象 API 进行基准测试。它具有与`vos_perf`IOR 类似的语法，并且可以作为 MPI 应用程序运行。有关`daos_perf`用法的完整说明，请运行：

```bash
$ daos_perf --help
```

像`vos_perf`，该`-R`选项用于定义要执行的操作：

- `U`for `update`（即写）操作
- `F`for `fetch`（即读）操作
- `P`用于`punch`（即截断）操作
- `p`显示上一个操作的性能结果。

例如，-R "U;p F;p" 表示更新密钥，打印更新速率/带宽，获取密钥，然后打印获取速率/带宽。object/dkey/akey/value 的数量可以分别通过 -o、-d、-a 和 -n 选项传递。值大小通过 -s 参数指定（例如 -s 4K 表示 4K 值）。

## 客户端调优[¶](https://docs.daos.io/v2.0/admin/performance_tuning/#client-tuning)

为获得最佳性能，DAOS 客户端应专门将自身绑定到 NUMA 节点，而不是让核心分配和内存绑定碰运气。这允许 DAOS 代理从其 PID 检测客户端的 NUMA 亲和性，并自动分配具有匹配 NUMA 节点的网络接口。GetAttachInfo 响应中提供的网络接口用于初始化 CaRT。

要覆盖自动分配的接口，客户端应设置环境变量`OFI_INTERFACE`以匹配所需的网络接口。

DAOS 代理在第一个 GetAttachInfo 请求上扫描客户端计算机，以确定支持 DAOS 服务器的 OFI 提供程序的可用网络接口集。该请求作为 `libdaos daos_init()`每个客户端执行的初始化序列的一部分发生。

收到后，代理会填充由 NUMA 亲和性索引的响应缓存。如果客户端应用程序已将自己绑定到特定的 NUMA 节点，并且该 NUMA 节点具有与之关联的网络设备，则 DAOS 代理将提供 GetAttachInfo 响应，其中包含与客户端的 NUMA 节点对应的网络接口。

当每个 NUMA 节点存在多个适当的网络接口时，代理使用循环资源分配方案来负载平衡该 NUMA 节点的响应。

如果客户端绑定到没有匹配网络接口的 NUMA 节点，则使用默认 NUMA 节点来选择响应。如果 DAOS 代理可以检测到任何 NUMA 节点上的任何有效网络设备，则默认响应将包含客户端的有效网络接口。当提供默认响应时，会在代理的日志中发出一条消息：

```
No network devices bound to client NUMA node X.  Using response from NUMA Y
```

为了提高性能，值得弄清楚客户端是否将自己绑定到错误的 NUMA 节点，或者代理的结构扫描中是否缺少该 NUMA 节点的预期网络设备。

在某些情况下，代理可能检测不到网络设备，响应缓存将为空。在这种情况下，GetAttachInfo 响应将不包含接口分配，并且将在代理的日志中找到以下信息消息：

```
No network devices detected in fabric scan; default AttachInfo response may be incorrect
```

在任何一种情况下，管理员都可以 `daos_agent net-scan`使用适当的调试标志执行命令，以更深入地了解配置问题。

**禁用 GetAttachInfo 缓存：**

默认配置启用代理 GetAttachInfo 缓存。如果需要，可以通过设置代理的环境变量在 DAOS 代理启动之前禁用缓存`DAOS_AGENT_DISABLE_CACHE=true`。缓存仅在代理启动时加载。将在代理的日志中找到以下调试消息：

```
GetAttachInfo agent caching has been disabled
```

如果在代理运行时网络配置发生更改，则必须重新启动它才能看到这些更改。有关其他信息，请参阅 [系统部署：代理启动](https://docs.daos.io/v2.0/admin/deployment/#disable-agent-cache-optional) 文档部分。