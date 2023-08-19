# Replicated Data Base (RDB) 复制数据库

Pool and container services are made highly available by replicating their internal metadata using Raft-based consensus and strong leadership. A service replicated in this generic approach tolerates the failure of any minority of its replicas. By spreading replicas of each service across the fault domains, pool and container services can therefore tolerate a reasonable number of target failures.

池和容器服务通过使用基于 Raft 的共识和强大的领导力复制其内部元数据来实现高可用性。 以这种通用方法复制的服务可以容忍其任何少数副本的故障。 通过跨故障域分布每个服务的副本，池和容器服务因此可以容忍合理数量的目标故障

<a id="8.3.1"></a>
## Architecture

A replicated service is built around a Raft replicated log. The service transforms RPCs into state queries and deterministic state updates. All state updates are committed to the replicated log first, before being applied by any of the service replicas. Since Raft guarantees consistency among log replicas, the service replicas end up applying the same set of state updates in the same order and go through identical state histories.

复制服务是围绕 Raft 复制日志构建的。 该服务将 RPC 转换为状态查询和确定性状态更新。 在被任何服务副本应用之前，所有状态更新都首先提交到复制日志。 由于 Raft 保证日志副本之间的一致性，服务副本最终以相同的顺序应用相同的状态更新集并经历相同的状态历史。

Among all replicas of a replicated service, only the current leader can handle service RPCs. The leader of a service is the current Raft leader (i.e., a Raft leader with the highest term number at the moment). Non-leader replicas reject all service RPCs and try to redirect the clients to the current leader to the best of their knowledge. Clients cache the addresses of the replicas as well as who current leader is. Occasionally, a client may not get any meaningful redirection hints and can find current leader by communicating to a random replicas.

在复制服务的所有副本中，只有当前领导者可以处理服务 RPC。 服务的领导者是当前的 Raft 领导者（即当前任期数最高的 Raft 领导者）。 非领导者副本拒绝所有服务 RPC，并尽其所知尝试将客户端重定向到当前领导者。 客户端缓存副本的地址以及当前领导者是谁。 有时，客户端可能得不到任何有意义的重定向提示，并且可以通过与随机副本通信来找到当前领导者。

The <a href="#f8.1">figure</a> below shows the modules constituting a service replica. The service module handles RPCs by transforming them into state queries and deterministic state updates. The Raft module implements the replicated log following the Raft protocol, by communicating with Raft modules on other replicas. It provides methods for the service module to perform the queries and the updates. The storage module, which in this case is the persistent memory and the file system, stores the service and Raft state. It uses VOS to update the state stored in persistent memory atomically.

下面的<a href="#f8.1">图</a>显示了构成服务副本的模块。 服务模块通过将 RPC 转换为状态查询和确定性状态更新来处理 RPC。 Raft 模块通过与其他副本上的 Raft 模块通信，按照 Raft 协议实现复制日志。 它为服务模块提供了执行查询和更新的方法。 存储模块，在本例中是持久内存和文件系统，存储服务和 Raft 状态。 它使用 VOS 以原子方式更新存储在持久内存中的状态。

<a id="f8.1"></a>
**Service replication modules**

![../../docs/graph/Fig_041.png](../../docs/graph/Fig_041.png "Service replication modules")

<a id="8.3.2"></a>
## RPC Handling RPC处理

When an RPC request arrives at the leader, a service thread of the service module picks up the request and handles it by executing a handler function designed for this type of request. As far as service replication is concerned, a handler comprises state queries (e.g., reading the pool properties), state updates (e.g., writing a new version of the pool map), and RPCs to other services (e.g., CONT_TGT_DESTROY RPCs sent to target services). Some handlers involve only queries, some involve updates as well as queries, and others involve all three kinds of actions; rarely, if ever, do handlers involve only updates but no queries.

当 RPC 请求到达领导者时，服务模块的服务线程将拾取请求并通过执行为此类请求设计的处理函数来处理它。 就服务复制而言，处理程序包括状态查询（例如，读取池属性）、状态更新（例如，写入池映射的新版本）和到其他服务的 RPC（例如，CONT_TGT_DESTROY RPC 发送到目标 服务）。 一些处理程序只涉及查询，一些涉及更新和查询，还有一些涉及所有三种操作； 处理程序很少（如果有的话）只涉及更新而不涉及查询。

A handler must assemble all its updates into a single log entry, commit the log entry, and wait for the log entry to become applicable before applying the updates to the service state. Using a single log entry per update RPC easily makes each update RPC atomic with regard to leader crashes and leadership changes. If RPCs that cannot satisfy this requirement are introduced in the future, additional transaction recovery mechanisms will be required. A leader's service state therefore always represents the effects of all completed update RPCs this leader has handled so far.

处理程序必须将其所有更新组装到一个日志条目中，提交日志条目，并等待日志条目变得可用，然后再将更新应用到服务状态。 每个更新 RPC 使用单个日志条目很容易使每个更新 RPC 在领导者崩溃和领导者变更方面具有原子性。 如果以后引入不能满足这个要求的RPC，就需要额外的事务恢复机制。 因此，领导者的服务状态始终代表该领导者迄今为止处理的所有已完成更新 RPC 的影响。

Queries, on the other hand, can read directly from the service state, without going through the replicated log. To make sure a request sees the effects of all completed update RPCs handled by all leaders ever elected, however, the handler must ask the Raft module whether there has been any leadership changes. If there has been none, all queries made for this request so far are not stale. If the leader has lost its leadership, the handler aborts the request with an error redirecting the client to the new leader.

另一方面，查询可以直接从服务状态读取，而无需通过复制的日志。 如果没有，则到目前为止为此请求所做的所有查询都不是陈旧的。 如果领导者失去了领导地位，处理程序将中止请求，并将客户端重定向到新的领导者。

RPCs to other services, if they update state of destination services, must be idempotent. In case of a leadership change, the new leader may send them again, if the client resent the service request in question.

其他服务的 RPC，如果它们更新目标服务的状态，则必须是幂等的。 如果领导层发生变化，如果客户对相关服务请求表示不满，新的领导层可能会再次发送它们。

Handlers need to cope with reasonable concurrent executions. Conventional local locking on the leader is sufficient to make RPC executions linearizable. Once a leadership change happens, the old leader can no longer perform any updates or leadership verifications with-out noticing the leadership change, which causes all RPCs in execution to abort. The RPCs on the new leader are thus not in conflict with those still left on the old leader. The locks therefore do not need to be replicated as part of the service state.

处理程序需要处理合理的并发执行。 领导者上的传统本地锁定足以使 RPC 执行可线性化。 一旦领导层发生变化，旧的领导层将无法在不注意到领导层变化的情况下执行任何更新或领导层验证，这会导致所有正在执行的 RPC 中止。 因此，新领导者上的 RPC 不会与旧领导者上仍然存在的 RPC 冲突。 因此，不需要将锁作为服务状态的一部分进行复制。
