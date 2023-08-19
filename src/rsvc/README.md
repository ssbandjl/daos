# Replicated Services

Module `rsvc` implements a generic replicated service framework. This file covers service replication in general, before focusing specifically on module `rsvc`.

模块“rsvc”实现了一个通用的复制服务框架。 在特别关注模块 `rsvc` 之前，该文件一般涵盖服务复制。

## Introduction

Certain DAOS RPC services, such as Pool Service (`pool_svc`), and Container Service (`cont_svc`), are replicated using the state machine approach with Raft. Each of these services tolerates the failure of any minority of its replicas. By spreading its replicas across different fault domains, the service can be highly available. Since this replication approach is self-contained in the sense that it requires only local persistent storage and point to point unreliable messaging, but not any external configuration management service, these services are mandatory for bootstrapping DAOS systems as well as managing the configuration of the lighter-weight I/O replication protocol.

某些 DAOS RPC 服务，例如池服务 (`pool_svc`) 和容器服务 (`cont_svc`)，是使用 Raft 的状态机方法进行复制的。 这些服务中的每一个都容忍其任何少数副本的故障。 通过将其副本分布在不同的故障域中，该服务可以实现高可用性。 由于这种复制方法是自包含的，因为它只需要本地持久存储和点对点不可靠的消息传递，而不需要任何外部配置管理服务，因此这些服务对于引导 DAOS 系统以及管理轻量级的配置是必需的 重量 I/O 复制协议。

### Architecture

An RPC service handles incoming _service requests_ (or just _requests_) based on its current _service state_ (or just _state_). To replicate a service is, therefore, to replicate its state so that each request is handled based on the state reached through all prior requests.

RPC 服务根据其当前的_服务状态_（或只是_状态_）处理传入的_服务请求_（或只是_请求_）。 因此，复制服务就是复制其状态，以便根据所有先前请求所达到的状态来处理每个请求。

The state of a service is replicated with a Raft log. The service transforms requests into state queries and deterministic state updates. All state updates are committed to the Raft log first, before being applied to the state. Since Raft guarantees the consistency among the log replicas, the service replicas end up applying the same set of state updates in the same order and go through identical state histories.

服务的状态通过 Raft 日志进行复制。 该服务将请求转换为状态查询和确定性状态更新。 在应用到状态之前，所有状态更新都首先提交到 Raft 日志。 由于 Raft 保证日志副本之间的一致性，服务副本最终以相同的顺序应用相同的状态更新集并经历相同的状态历史。

Raft adopts a strong leadership design, so does each replicated service. A service leader of a term is the Raft leader of the same Raft term. Among the replicas of a service, only the leader of the highest term can handle requests. For the server side, the service code is similar to that of a non-replicated RPC service, except for the handling of leadership change events. For the client side, the service requests must be sent to the current leader, which must be searched for if not known already.

Raft 采用强领导设计，每个复制的服务也是如此。 一个term的service leader就是同一个Raft term的Raft leader。 在服务的副本中，只有最高任期的领导才能处理请求。 对于服务器端，服务代码类似于非复制的 RPC 服务，除了领导变更事件的处理。 对于客户端，服务请求必须发送给当前的领导者，如果不知道，则必须搜索。

A replicated service is implemented using a stack of modules: 复制服务是使用一堆模块实现的

	[ pool_svc, cont_svc, ... ]
	[ ds_rsvc ]
	[                rdb                ]
	[ raft ]
	[                vos                ]

`pool_svc`, and `cont_svc` implement the request handlers and the leadership change event handlers of the respective services. They define their respective service state in terms of the RDB data model provided by `rdb`, implement state queries and updates using RDB transactions, and register their leadership change event handlers into the framework `rsvc` offers.

`pool_svc` 和 `cont_svc` 实现了各自服务的请求处理程序和领导层变更事件处理程序。 他们根据 rdb 提供的 RDB 数据模型定义各自的服务状态，使用 RDB 事务实现状态查询和更新，并将他们的领导变更事件处理程序注册到 rsvc 提供的框架中。

`rdb` (`daos_srv/rdb`) implements a hierarchical key-value store data model with transactions, replicated using Raft. It delivers Raft leadership change events to `ds_rsvc`, implements transactions using the Raft log, and stores a service's data model and its own internal metadata using the VOS data model. `rdb` on the leader replica, interfaces with VOS to monitor available persistent storage and, when free space drops below a threshold, rejects new transactions before appending entries to the Raft log that could otherwise result in the service becoming unavailable (due to a mix of successful and "out of space" failures in applying the entries). It also interfaces with VOS to periodically compact storage by triggering aggregation of older versions (epochs) of the log.

`rdb` (`daos_srv/rdb`) 实现了一个带有事务的分层键值存储数据模型，使用 Raft 进行复制。 它将 Raft 领导层变更事件传递给 ds_rsvc，使用 Raft 日志实现事务，并使用 VOS 数据模型存储服务的数据模型和它自己的内部元数据。 领导副本上的 rdb 与 VOS 接口以监控可用的持久存储，并且当可用空间低于阈值时，在将条目附加到 Raft 日志之前拒绝新事务，否则可能导致服务变得不可用（由于混合 应用条目的成功和“空间不足”失败）。 它还与 VOS 接口，通过触发日志的旧版本（时代）的聚合来定期压缩存储。

`raft` (`rdb/raft/include/raft.h`) implements the Raft core protocol in a library. Its integration with VOS and CaRT is done inside `rdb` via callback functions.

`raft` (`rdb/raft/include/raft.h`) 在库中实现了 Raft 核心协议。 它与 VOS 和 CaRT 的集成是通过回调函数在 rdb 内部完成的。

A replicated service client (e.g., `dc_pool`) uses `dc_rsvc` to search for the current service leader:

复制的服务客户端（例如，`dc_pool`）使用`dc_rsvc` 来搜索当前的服务领导者：

	[ dc_pool ]
	[ dc_rsvc ]

The search is accomplished with a combination of a client maintained list of candidate service replicas and server RPC error responses in some cases containing a hint where the current leader may be found. A server not running the service responds with an error a client uses to remove that server from its list. A server acting as a non-leader replica responds with a different error, including a hint a client uses to (if necessary) add to its list and alter its search for the leader. And, `dc_rsvc` both at client startup and later when the client's list of candidate service replicas may become empty (e.g., due to membership changes in the service), contacts one of the DAOS servers running on a management service node to get an up-to-date list of service replicas for the pool.

搜索是通过客户端维护的候选服务副本列表和服务器 RPC 错误响应的组合来完成的，在某些情况下包含可能找到当前领导者的提示。 未运行该服务的服务器响应一个错误，客户端使用该错误从其列表中删除该服务器。 作为非领导者副本的服务器以不同的错误响应，包括客户端用来（如有必要）添加到其列表并更改其对领导者的搜索的提示。 并且，`dc_rsvc` 在客户端启动时和稍后当客户端的候选服务副本列表可能变空时（例如，由于服务中的成员更改），联系在管理服务节点上运行的 DAOS 服务器之一以启动 池的最新服务副本列表。

## Module `rsvc`

The main purpose of `rsvc` is to avoid code duplication among different replicated service implementations. The callback-intensive API follows from the attempt to extract as much common code as possible, even at the expense of API simplicity. This is a key difference from how other module APIs are designed.

`rsvc` 的主要目的是避免不同复制服务实现之间的代码重复。 回调密集型 API 遵循尽可能多地提取通用代码的尝试，即使以 API 简单性为代价。 这是与其他模块 API 设计方式的主要区别。

`rsvc` has two parts:

* `ds_rsvc` (`daos_srv/rsvc.h`): server-side framework.
* `dc_rsvc` (`daos/rsvc.h`): client-side library.

`dc_rsvc` is currently still called `rsvc`. A rename will be done in a future patch.

`dc_rsvc` 目前仍称为 `rsvc`。 重命名将在未来的补丁中完成。
