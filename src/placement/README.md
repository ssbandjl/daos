<a id="10.2"></a>
# Algorithmic object placement 对象放置算法

DAOS uses the pool map to create a set of placement maps that are used to compute algorithmic object layouts and to drive consensus data distribution. This approach uses consistent hash based algorithms to generate object layout based on object ID, object schema, and one of the placement maps. DAOS uses a modular approach that allows different placement maps to be used by different objects to obtain the performance characteristics required by the application.

DAOS 使用池映射创建一组放置映射，用于计算算法对象布局和驱动共识数据分发。 这种方法使用基于一致散列的算法来生成基于对象 ID、对象模式和放置映射之一的对象布局。 DAOS 使用模块化的方法，允许不同的对象使用不同的放置图来获得应用程序所需的性能特征。

<a id="10.2.1"></a>
## Placement Map

A placement map is essentially an abstracted and permuted pool map; it does not necessarily include all details of the pool map. Instead it only retains component relationships that can be used to distribute object shards for the resilience and performance requirements of the application.

放置映射本质上是一个抽象和置换的池映射； 它不一定包括池映射的所有详细信息。 相反，它只保留可用于分发对象分片以满足应用程序的弹性和性能要求的组件关系。

<a id="f10.2"></a>

**Pool-map and placement maps**

![../../docs/graph/Fig_043.png](../../docs/graph/Fig_043.png "Pool-map and placement maps")

A placement map does not maintain a copy of status or any characteristics of the corresponding pool map components, but only references pool map components. Each time DAOS computes an object distribution based on a placement map, it also needs to check the corresponding component status and attributes from the pool map. This adds an extra step for indirect memory access, but can significantly reduce cache pollution and memory consumption when there are many placement maps but only one pool map in a DAOS pool.

放置映射不维护状态副本或相应池映射组件的任何特征，而仅引用池映射组件。 DAOS 每次根据放置映射计算对象分布时，还需要从池映射中检查相应的组件状态和属性。 这为间接内存访问增加了一个额外的步骤，但是当 DAOS 池中有许多放置映射但只有一个池映射时，可以显着减少缓存污染和内存消耗。

As shown in the <a href="#f10.2">figure</a>, a storage pool may have multiple types of placement maps because different applications can have various fault tolerance and performance requirements. In addition, there can be many instances of the same placement map in order to accelerate rebuild and rebalance by workload declustering.

如<a href="#f10.2">图</a>所示，一个存储池可能有多种类型的放置映射，因为不同的应用程序可能有不同的容错和性能要求。 此外，可以有多个相同放置图的实例，以便通过工作负载去集群来加速重建和重新平衡。

DAOS today includes two placement map algorithms:

### [Jump Placement Map](JUMP_MAP.md)

The Jump Placement Map is the default placement map in DAOS. It utilizes the Jump Consistent Hashing algorithm in order to pseudorandomly distribute objects amongst different fault domains. This distributes them across fault domains as far apart from one another as possible in order to avoid data loss in the event of a failure affecting an entire fault domain. It was designed to efficiently move data between systems when the physical configuration of the system changes (i.e. more capacity is added).

Jump Placement Map 是 DAOS 中默认的放置映射。 它利用 Jump Consistent Hashing 算法在不同的故障域之间伪随机分布对象。 这会将它们分布在尽可能远离彼此的故障域中，以避免在发生影响整个故障域的故障时丢失数据。 它旨在在系统的物理配置发生变化（即添加更多容量）时有效地在系统之间移动数据。

### [Ring Placement Map](RING_MAP.md)

The Ring Placement Map was the original placement map developed for DAOS. It utilizes a ring memory structure that puts targets on the ring in a pattern such that given any random location on the ring, that location and its neighbors will be physically in separate fault domains. This makes it extremely fast to compute placement locations, but also makes it difficult to modify dynamically. It can not currently be used as it does not support several of the newer API methods required by DAOS - specifically those for server reintegration, drain, and addition.

Ring Placement Map 是为 DAOS 开发的原始放置图。 它利用环形内存结构，将目标以某种模式放置在环上，这样给定环上的任意随机位置，该位置及其邻居将在物理上处于不同的故障域中。 这使得计算放置位置的速度非常快，但也使得动态修改变得困难。 它目前无法使用，因为它不支持 DAOS 所需的几种较新的 API 方法——特别是那些用于服务器重新集成reintegration、耗尽drain和添加addition的方法。
