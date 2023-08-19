# DAOS Pool

A pool is a set of targets spread across different storage nodes over which data and metadata are distributed to achieve horizontal scalability, and replicated or erasure-coded to ensure durability and availability (see: <a href="/docs/overview/storage.md#daos-pool">Storage Model: DAOS Pool</a>).

池是一组分布在不同存储节点上的目标，数据和元数据分布在这些节点上以实现水平可伸缩性，并进行复制或擦除编码以确保持久性和可用性

<a id="9.1"></a>



## Pool Service

The Pool Service (`pool_svc`) stores the metadata for pools, and provides an API to query and update the pool configuration. Pool metadata are organized as a hierarchy of key-value stores (KVS) that is replicated over a number of servers backed by Raft consensus protocol which uses strong leadership; client requests can only be serviced by the service leader while non-leader replicas merely respond with a hint pointing to the current leader for the client to retry. `pool_svc` derives from a generic replicated service module `rsvc` (see: <a href="/src/rsvc/README.md#architecture">Replicated Services: Architecture</a>) whose implementation facilitates the client search for the current leader.

池服务 (pool_svc) 存储池的元数据，并提供 API 来查询和更新池配置。 池元数据被组织为键值存储（KVS）的层次结构，该层次结构在多个服务器上复制，该服务器由使用强大领导力的 Raft 共识协议支持； 客户端请求只能由服务领导者提供服务，而非领导者副本仅响应指向当前领导者的提示，以便客户端重试。 pool_svc 派生自通用复制服务模块 rsvc（请参阅：复制服务：体系结构），其实现有助于客户端搜索当前领导者。

<a id="9.1.1"></a>

#### Metadata Layout

![Pool Service Layout](/docs/graph/Fig_072.png "Pool Service Layout")

The top-level KVS stores the pool map, security attributes such as the UID, GID and mode, information related to space management and self-healing (see: <a href="/src/rebuild/README.md">Rebuild</a>) as well as a second-level KVS containing user-defined attributes (see: <a href="/src/container/README.md#metadata-layout">Container Service: Metadata Layout</a>). In addition, it also stores information on pool connections, represented by a pool handle and identified by a client-generated handle UUID. The terms "pool connection" and "pool handle" may be used interchangeably.

顶层 KVS 存储池映射、UID、GID 和模式等安全属性、与空间管理和自我修复相关的信息（请参阅：Rebuild）以及包含用户定义属性的二级 KVS（请参阅 ：容器服务：元数据布局）。 此外，它还存储有关池连接的信息，由池句柄表示并由客户端生成的句柄 UUID 标识。 术语“池连接”和“池手柄”可以互换使用。

<a id="9.3"></a>

## Pool Operations

<a id="9.3.1"></a>

#### Pool / Pool Service Creation

Pool creation is driven entirely by the Management Service since it requires special privileges for steps related to allocation of storage and querying of fault domains. After formatting all the targets, the management module passes the control to the pool module by calling the`ds_pool_svc_dist_create`, which initializes service replication on the selected subset of nodes for the combined Pool and Container Service. The Pool module now sends a `POOL_CREATE` request to the service leader which creates the service database; the list of targets and their fault domains are then converted into the initial version of the pool map and stored in the pool service, along with other initial pool metadata.

池创建完全由管理服务驱动，因为它需要特殊权限才能执行与存储分配和故障域查询相关的步骤。 格式化所有目标后，管理模块通过调用 ds_pool_svc_dist_create 将控制权传递给池模块，该模块在选定的节点子集上为合并的池和容器服务初始化服务复制。 Pool 模块现在向创建服务数据库的服务负责人发送 POOL_CREATE 请求； 然后将目标列表及其故障域转换为池映射的初始版本，并与其他初始池元数据一起存储在池服务中。

<a id="9.3.2"></a>

#### Pool Connection

To establish a pool connection, a client process calls the `daos_pool_connect` method in the client library with the pool UUID, connection information (such as group name and list of service ranks) and connection flags; this initiates a `POOL_CONNECT` request to the Pool Service. The Pool Service tries to authenticate the request according to the security model in use (e.g., UID/GID in a POSIX-like model), and to authorize the requested capabilities to the client-generated pool handle UUID.  Before proceeding, the pool map is transferred to the client; if there are errors from this point onwards, the server can simply ask the client to discard the pool map.

要建立池连接，客户端进程使用池 UUID、连接信息（如组名和服务等级列表）和连接标志调用客户端库中的 daos_pool_connect 方法； 这会向池服务发起 POOL_CONNECT 请求。 池服务尝试根据使用的安全模型（例如，类 POSIX 模型中的 UID/GID）对请求进行身份验证，并将请求的功能授权给客户端生成的池句柄 UUID。 在继续之前，池映射被传输到客户端； 如果从这一点开始出现错误，服务器可以简单地要求客户端丢弃池映射。

At this point, the Pool Service checks for existing pool handles:

- If a pool handle with the same UUID already exists, a pool connection has already been established and nothing else needs to be done.
- If another pool handle exists such that either the currently requested or the existing one has exclusive access, the connection request is rejected with a busy status code. 如果存在另一个池句柄，使得当前请求的或现有的池句柄具有独占访问权，则连接请求将被拒绝，并显示忙状态代码。

If everything goes well, the pool service sends a collective `POOL_TGT_CONNECT` request to all targets in the pool with the pool handle UUID. The Target Service creates and caches the local pool objects and opens the local VOS pool for access. 如果一切顺利，池服务将使用池句柄 UUID 向池中的所有目标发送一个集体 POOL_TGT_CONNECT 请求。 Target Service创建并缓存本地池对象，并打开本地VOS池进行访问

A group of peer application processes may share a single pool connection handle 一组对等应用程序进程可以共享一个池连接句柄 (see: <a href="/docs/overview/storage.md#daos-pool">Storage Model: DAOS Pool</a> and <a href="/docs/overview/use_cases.md#storage-management--workflow-integration">Use Cases: Storage Management and Workflow Integration</a>).

To close a pool connection, a client process calls the `daos_pool_disconnect` method in the client library with the pool handle, triggering a `POOL_DISCONNECT` request to the Pool Service, which sends a collective `POOL_TGT_DISCONNECT` request to all targets in the pool. These steps destroy all state associated with the connection, including all container handles. Other client processes sharing this connection should destroy their copies of the pool handle locally, preferably before the disconnect method is called on behalf of everyone. If a group of client processes terminate prematurely, before having a chance to call the pool disconnect method, their pool connection will eventually be evicted once the pool service learns about the event from the run-time environment.

要关闭池连接，客户端进程使用池句柄调用客户端库中的“daos_pool_disconnect”方法，触发对池服务的“POOL_DISCONNECT”请求，池服务向池中的所有目标发送集体“POOL_TGT_DISCONNECT”请求。 这些步骤会破坏与连接关联的所有状态，包括所有容器句柄。 共享此连接的其他客户端进程应在本地销毁池句柄的副本，最好是在代表每个人调用 disconnect 方法之前。 如果一组客户端进程在有机会调用池断开连接方法之前过早终止，一旦池服务从运行时环境获悉该事件，它们的池连接最终将被逐出。
