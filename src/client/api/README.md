# DAOS Client Library

The DAOS API is divided along several functionalities to address the different
features that DAOS exposes:
- Management API: pool and target management
- Pool Client API: pool access
- Container API: container management and access, container snapshots
- Transaction API: transaction model and concurrency control
- Object, Array and KV APIs: object and data management and access
- Event, Event Queue, and Task API: non-blocking operations
- Addons API: array and KV operations built over the DAOS object API
- DFS API: DAOS file system API to emulate a POSIX namespace over DAOS
- DUNS API: DAOS unified namespace API for integration with an existing system
  namespace.

Each of those components have associated README.md files that provide more
details about the functionality they support except for APIs to support
non-blocking operations which is discussed here.

The libdaos API is available under [/src/include/daos\_\*](/src/include/) and
associated man pages under [/docs/man/man3/](/docs/man/man3/).

## Event & Event Queue

DAOS API functions can be used in either blocking or non-blocking mode. This is
determined through a pointer to a DAOS event passed to each API call that:

- if NULL indicates that the operation is to be blocking. The operation will
  return after completing the operation. The error codes for all failure cases
  will be returned through the return code of the API function itself.

- if a valid event is used, the operation will run in non-blocking mode and
  return immediately after scheduling the operation in the internal scheduler
  and after RPCs are submitted to the underlying stack. The return value of the
  operation is success if the scheduling succeeds, but does not indicate that
  the actual operation succeeds. The errors that can be caught on return are
  either invalid parameters or scheduling problems. The actual return code of
  the operation will be available in the event error code (event.ev_error) when
  the event completes.

A valid event to be used must be created first with a separate API call. To allow users to track
multiple events at a time, an event can be created as part of an event queue, which is basically a
collection of events that can be progressed and polled together. An event queue also creates a
separate task scheduler internally for all DAOS tasks as well as a new network context. The network
context creation is an expensive operation on some network providers, and thus users should try to
limit the number of event queue being created in their applications or IO middleware libraries on
top of DAOS. Alternatively, an event can be created without an event queue, and be tracked
individually. In this case, and for blocking operations, an internal global task scheduler and
network context is used instead for the independent ones that would be created for an event
queue. Once an event is completed, it can re-used for another DAOS API call to minimize the need for
event creation and allocations inside the DAOS library.

事件和事件队列

DAOS API 函数可以在阻塞或非阻塞模式下使用。 这是通过传递给每个 API 调用的指向 DAOS 事件的指针来确定的：如果 NULL 表示操作将被阻塞。 操作完成后会返回。 所有失败情况的错误码都将通过API函数本身的返回码返回。 如果使用有效的事件，则该操作将以非阻塞模式运行，并在内部调度程序中调度该操作以及将 RPC 提交到底层堆栈后立即返回。 如果调度成功，则操作的返回值为success，但并不表示实际操作成功。 返回时可以捕获的错误要么是无效参数，要么是调度问题。 当事件完成时，操作的实际返回代码将在事件错误代码 (event.ev_error) 中提供。 必须首先通过单独的 API 调用创建要使用的有效事件。 为了允许用户一次跟踪多个事件，可以将事件创建为事件队列的一部分，事件队列基本上是可以一起进行和轮询的事件的集合。 事件队列还在内部为所有 DAOS 任务创建一个单独的任务调度程序以及一个新的网络上下文。 在某些网络提供商上，网络上下文创建是一项昂贵的操作，因此用户应尝试限制在 DAOS 之上的应用程序或 IO 中间件库中创建的事件队列的数量。 或者，可以在没有事件队列的情况下创建事件，并单独跟踪。 在这种情况下，对于阻塞操作，将使用内部全局任务调度程序和网络上下文来代替为事件队列创建的独立任务调度程序和网络上下文。 事件完成后，它可以重新用于另一个 DAOS API 调用，以最大限度地减少 DAOS 库内事件创建和分配的需要


## Task Engine Integration

The DAOS Task API provides an alternative way to use the DAOS API in a
non-blocking manner and at the same time build a task dependency tree between
DAOS API operation. This is useful for applications and middleware libraries
using DAOS and needing to build a schedule of DAOS operations with dependencies
between each other (N-1, 1-N, N-N).

To leverage the task API, the user would need to create a scheduler where DAOS
tasks can be created as a part of. The task API is generic enough to allow the
user to mix DAOS specific tasks (through the DAOS task API) and other user
defined tasks and add dependencies between those.

For more details on how TSE is used in client library, see [TSE internals
documentation](/src/common/README.md) for more details.
