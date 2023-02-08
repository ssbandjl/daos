# 概述

Mercury由三个主要层组成：

1. [网络抽象层](https://mercury-hpc.github.io/user/na/)，它在较低级别的网络结构之上提供高性能通信接口。
2. [RPC 层](https://mercury-hpc.github.io/user/hg/)，它为用户提供发送和接收 RPC 元数据（小消息）的必要组件。这包括函数参数的序列化和反序列化；
3. [批量层](https://mercury-hpc.github.io/user/hg_bulk/)，它提供了处理大型参数的必要组件——这意味着通过 RMA 传输大量数据；
4. （*可选*）[高级 RPC 层](https://mercury-hpc.github.io/user/hg_macros/)，旨在提供方便的 API，构建在较低层之上，并提供用于生成 RPC 存根以及序列化和反序列化功能的宏。

这三个主要层可以总结在下图中：

![概述](https://mercury-hpc.github.io/assets/doc/overview.svg)

根据定义，一个 RPC 调用由一个进程发起，称为 *origin*，然后转发到另一个进程，该进程将执行该调用，称为*target*。每一方，起点和目标，都使用一个 RPC *处理器*来序列化和反序列化通过接口发送的参数。使用相对较小的参数调用函数会导致使用网络抽象层公开的短消息机制，而包含大数据参数的函数额外使用远程内存访问 (RMA) 机制。请注意，当批量数据足够小时，Mercury 会自动将其与元数据一起嵌入（如果它适合的话）。