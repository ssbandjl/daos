# Versioned Block Allocator

The Versioned Block Allocator is used by VOS for managing blocks on NVMe SSD, it's basically an extent based block allocator specially designed for DAOS.

## Allocation metadata

The blocks allocated by VEA are used to store single value or array value in VOS. Since the address and size from each allocation is tracked in VOS index trees, the VEA allocation metadata tracks only free extents in a btree, more importantly, this allocation metadata is stored on SCM along with the VOS index trees, so that block allocation and index updating could be easily made into single PMDK transaction, at the same time, the performance would be much better than storing the metadata on NVMe SSD.

## Scalable concurrency

Thanks to the shared-nothing architecture of DAOS server, scalable concurrency isn't a design cosideration for VEA, which means VEA doesn't have to split space into zones for mitigating the contention problem.

## Delayed atomicity action

VOS update is executed in a 'delayed atomicity' manner, which consists of three steps:
<ol>
<li>Reserve space for update and track the reservation transiently in DRAM;</li>
<li>Start RMDA transfer to land data from client to reserved space;</li>
<li>Turn the reservation into a persistent allocation and update the allocated address in VOS index within single PMDK transaction;</li>
Obviously, the advantage of this manner is that the atomicity of allocation and index updating can be guaranteed without an undo log to revert the actions in first step.

To support this delayed atomicity action, VEA maintains two sets of allocation metadata, one in DRAM for transient reservation, the other on SCM for persistent allocation.
</ol>

## Allocation hint

VEA assumes a predictable workload pattern: All the block allocate and free calls are from different 'IO streams', and the blocks allocated within the same IO stream are likely to be freed at the same time, so a straightforward conclusion is that external fragmentations could be reduced by making the per IO stream allocations contiguous.

The IO stream model perfectly matches DAOS storage architecture, there are two IO streams per VOS container, one is the regular updates from client or rebuild, the other one is the updates from background VOS aggregation. VEA provides a set of hint API for caller to keep a sequential locality for each IO stream, that requires each caller IO stream to track its own last allocated address and pass it to the VEA as a hint on next allocation.


版本化块分配器
VOS 使用版本化块分配器来管理 NVMe SSD 上的块，它基本上是专为 DAOS 设计的基于范围的块分配器。

分配元数据
VEA 分配的块用于在 VOS 中存储单个值或数组值。 由于每个分配的地址和大小都在 VOS 索引树中进行跟踪，因此 VEA 分配元数据仅跟踪 btree 中的空闲盘区，更重要的是，此分配元数据与 VOS 索引树一起存储在 SCM 上，以便块分配和索引 更新可以很容易地变成单个 PMDK 事务，同时，性能会比将元数据存储在 NVMe SSD 上要好得多。

可扩展并发
由于 DAOS 服务器的无共享架构，可扩展并发性不是 VEA 的设计考虑因素，这意味着 VEA 不必将空间划分为多个区域来缓解争用问题。

延迟原子性动作
VOS更新以“延迟原子性”的方式执行，包括三个步骤：

预留更新空间并在 DRAM 中瞬时跟踪预留空间；
启动 RMDA 将土地数据从客户端传输到预留空间；
将预留转为持久分配，并在单个 PMDK 事务内更新 VOS 索引中的分配地址；
显然，这种方式的优点是可以保证分配和索引更新的原子性，而无需撤消日志来恢复第一步的操作。
为了支持这种延迟原子性操作，VEA 维护两组分配元数据，一组在 DRAM 中用于临时保留，另一组在 SCM 中用于持久分配。

分配提示
VEA 假设一种可预测的工作负载模式：所有块分配和释放调用都来自不同的“IO 流”，并且同一 IO 流中分配的块可能会同时释放，因此一个简单的结论是，外部碎片可能会 通过使每个 IO 流分配连续来减少。

IO流模型完美匹配DAOS存储架构，每个VOS容器有两个IO流，一个是来自客户端或重建的定期更新，另一个是来自后台VOS聚合的更新。 VEA 为调用者提供了一组提示 API，以保持每个 IO 流的顺序局部性，这要求每个调用者 IO 流跟踪其自己的最后分配的地址并将其传递给 VEA 作为下次分配的提示。
