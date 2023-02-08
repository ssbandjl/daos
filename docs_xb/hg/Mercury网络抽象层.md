# 网络抽象层 Network Abstraction Layer NA

*网络抽象*(NA) 层由 RPC 层和批量层Bulk在内部使用。NA 层使用插件机制，因此可以在运行时轻松添加和选择对各种网络协议的支持。

信息

如果您打算使用 Mercury 的 RPC 层（HG 调用），则不应直接使用 NA 接口。在这种情况下，请直接跳转到[可用插件](https://mercury-hpc.github.io/user/na/#available-plugins) 部分以获取在初始化 Mercury 时可以使用的插件列表——然后在 [RPC 层](https://mercury-hpc.github.io/user/hg/)部分中进一步描述 Mercury 的初始化。

## 网络抽象接口（NA）

NA 提供了一组最小的函数调用，这些函数调用抽象了底层网络结构，可用于提供： *目标地址查找*、带有意外和预期消息*的点对点消息、**远程内存访问 (RMA)*、*进度*和*取消*。API 是非阻塞的并使用回调机制，以便上层可以更轻松地提供异步执行：当取得进展（内部或调用后`NA_Progress()`）并且操作完成时，用户回调被放置到完成队列中。然后可以在调用`NA_Trigger()`.

### 初始化

使用 NA 时，程序的第一步应该包括初始化 NA 接口和选择将要使用的底层插件。用指定初始化 NA 接口`info_string` 会导致创建新`na_class_t`对象(网络抽象类)。有关info_string格式的更多信息，请参阅[可用插件](https://mercury-hpc.github.io/user/na/#available-plugins)部分 。此外，可以指定 `na_class_t`对象是否正在侦听listening——这是*唯一*定义“服务器”特定行为的时候，所有后续调用都不区分“客户端”和“服务器”，并且而是只使用*origin*和*target*的概念。然而，值得注意的是，`listen` 标识 可能对分配的资源产生影响，并且通过的地址`info_string`将用于创建远程对等点可以访问的端点。

```c
na_class_t *
NA_Initialize(const char *info_string, na_bool_t listen);
初始化
```

如果需要更具体的行为，也可以使用以下NA_Initialize_opt带选项的网络抽象初始化调用来传递特定的初始化选项。

| 选项                  | 描述                                                         |
| :-------------------- | :----------------------------------------------------------- |
| `auth_key`            | 可用于通信的授权密钥。所有进程都应该使用相同的密钥进行通信。 |
| `ip_subnet`           | 要使用的首选 IP 子网。                                       |
| `max_contexts`        | 预期创建的最大上下文数。                                     |
| `max_expected_size`   | 可以传递的最大预期大小提示来控制意外消息的大小。             |
| `max_unexpected_size` | 可以传递的最大意外大小提示来控制意外消息的大小。             |
| `progress_mode`       | 进度模式标志。设置`NA_NO_BLOCK`将强制忙于进度并删除任何等待/通知调用。 |
| `thread_mode`         | 线程模式标志。设置`NA_THREAD_MODE_SINGLE`将放宽线程安全要求。 |

```c
struct na_init_info {
    const char *auth_key;
    const char *ip_subnet;
    na_uint8_t max_contexts;
    na_size_t max_unexpected_size;
    na_size_t max_expected_size;
    na_uint32_t progress_mode;
    na_uint8_t thread_mode;
};

na_class_t *
NA_Initialize_opt(const char *info_string, na_bool_t listen,
                  const struct na_init_info *na_init_info);
```

从这些初始化调用创建的`na_class_t`对象稍后应该通过调用NA_Finalize来释放：

```c
na_return_t
NA_Finalize(na_class_t *na_class);
```

初始化接口后，必须在此插件中创建上下文，该上下文在内部创建， 并关联操作的完成队列CQ：

```c
na_context_t *
NA_Context_create(na_class_t *na_class);
```

然后可以使用以下方法将其销毁：

```c
na_return_t
NA_Context_destroy(na_class_t *na_class, na_context_t *context);
```

### 目标地址查找 Target Address Lookup

要与目标通信，必须首先获取其地址。最方便和最安全的方法是在目标上调用NA_Addr_self：

```c
na_return_t
NA_Addr_self(na_class_t *na_class, na_addr_t *addr);
```

然后使用以下命令将该地址转换为字符串：

```c
na_return_t
NA_Addr_to_string(na_class_t *na_class, char *buf, na_size_t buf_size, na_addr_t addr);
```

然后可以通过带外机制（例如，使用文件等）将字符串交换给其他进程，然后可以使用以下函数查找目标：

```c
na_return_t
NA_Addr_lookup(na_class_t *na_class, const char *name, na_addr_t *addr);
```

然后必须使用以下方法释放所有地址：

```
na_return_t
NA_Addr_free(na_class_t *na_class, na_addr_t addr);
```

### 点对点消息 Point-to-point Messaging

NA 中的点对点消息传递始终是非阻塞的，在调用后执行完成回调`NA_Trigger()`（一旦操作完成并放入完成队列）。NA 支持两种不同的发送和接收消息模式： *意外*或*预期*。预期的消息应始终预先发布其接收receive pre-posted.。尽管如果不是这种情况，消息可能会在不通知的情况下被丢弃，但它们通常仍在排队等待稍后处理。另一个句柄上的意外消息从不需要预先发布接收消息，并且还允许删除消息（尽管插件通常再次将它们排队）。两种类型的消息都是标记消息tagged，它们采用相同的参数进行发送：

```c
na_return_t
NA_Msg_send_unexpected(na_class_t *na_class, na_context_t *context,
    na_cb_t callback, void *arg, const void *buf, na_size_t buf_size,
    void *plugin_data, na_addr_t dest_addr, na_uint8_t dest_id, na_tag_t tag,
    na_op_id_t *op_id);

na_return_t
NA_Msg_send_expected(na_class_t *na_class, na_context_t *context,
    na_cb_t callback, void *arg, const void *buf, na_size_t buf_size,
    void *plugin_data, na_addr_t dest_addr, na_uint8_t dest_id, na_tag_t tag,
    na_op_id_t *op_id);
```



仅仅是它们的接收操作不同：

```c
na_return_t
NA_Msg_recv_unexpected(na_class_t *na_class, na_context_t *context,
    na_cb_t callback, void *arg, void *buf, na_size_t buf_size,
    void *plugin_data, na_op_id_t *op_id);

na_return_t
NA_Msg_recv_expected(na_class_t *na_class, na_context_t *context,
    na_cb_t callback, void *arg, void *buf, na_size_t buf_size,
    void *plugin_data, na_addr_t source_addr, na_uint8_t source_id,
    na_tag_t tag, na_op_id_t *op_id);
```

一个只匹配特定的`source_addr`，`tag`， 而另一个匹配*任何源*和标签，然后可以从回调信息中检索。

```c
struct na_cb_info_recv_unexpected {
    na_size_t actual_buf_size;
    na_addr_t source;
    na_tag_t tag;
};
```

请注意，为了获得最佳性能，`NA_Msg_buf_alloc()`可能`NA_Msg_buf_free()` 会用于分配发送和接收缓冲区。

### 远程内存访问 RMA（Remote Memory Access）

远程内存访问需要首先向 NA 层注册希望访问的主机内存。这分两步完成，首先创建一个句柄控制器来描述要注册的内存缓冲区， 然后调用`NA_Mem_register()`注册它。

```c
na_return_t
NA_Mem_handle_create(na_class_t *na_class, void *buf, na_size_t buf_size,
                     unsigned long flags, na_mem_handle_t *mem_handle);

na_return_t
NA_Mem_register(na_class_t *na_class, na_mem_handle_t mem_handle);
```

同样，必须调用 NA_Mem_deregister()` 和 `NA_Mem_handle_free() 才能释放资源

一旦内存被注册，目标的控制器必须被序列化并与将初始化RMA 操作的对等方交换。这是通过调用NA_Mem_handle_serialize()：

```c
na_return_t
NA_Mem_handle_serialize(na_class_t *na_class, void *buf, na_size_t buf_size,
                        na_mem_handle_t mem_handle);
```

然后，对等方可以使用以下方法反序列化：

```c
na_return_t
NA_Mem_handle_deserialize(na_class_t *na_class, na_mem_handle_t *mem_handle,
                          const void *buf, na_size_t buf_size);
```

并使用描述其远程内存的目标句柄和描述其本地内存的本地句柄启动 RMA 操作：

```c
na_return_t
NA_Put(na_class_t *na_class, na_context_t *context, na_cb_t callback, void *arg,
    na_mem_handle_t local_mem_handle, na_offset_t local_offset,
    na_mem_handle_t remote_mem_handle, na_offset_t remote_offset,
    na_size_t data_size, na_addr_t remote_addr, na_uint8_t remote_id, na_op_id_t *op_id);

na_return_t
NA_Get(na_class_t *na_class, na_context_t *context, na_cb_t callback, void *arg,
    na_mem_handle_t local_mem_handle, na_offset_t local_offset,
    na_mem_handle_t remote_mem_handle, na_offset_t remote_offset,
    na_size_t data_size, na_addr_t remote_addr, na_uint8_t remote_id, na_op_id_t *op_id);
```

与点对点操作类似，RMA 操作是非阻塞的，并且使用基于回调的模型，该模型在调用`NA_Trigger()` 操作完成后触发。

### 进度和取消

NA 进度模型始终是明确的，用户应该先调用 `NA_Progress()`，然后调用`NA_Trigger()`：

```
na_return_t
NA_Progress(na_class_t *na_class, na_context_t *context, unsigned int timeout);

na_return_t
NA_Trigger(na_context_t *context, unsigned int timeout, unsigned int max_count,
           int callback_ret[], unsigned int *actual_count);
```

`NA_Trigger()`总是在单个上下文上运行，而`NA_Progress()`可以同时在类和上下文上运行。当调用 progress 时，一旦操作完成或已经在完成队列中，它就会返回，以便可以调用`NA_Trigger()`以清空队列并执行用户回调。

当必须取消操作时，用户应该调用`NA_Cancel()`该操作：

```
na_return_t
NA_Cancel(na_class_t *na_class, na_context_t *context, na_op_id_t *op_id);
```

取消总是异步的。当/如果操作成功取消，它将被推送到完成队列并`NA_CANCELED`返回错误代码。

## 可用插件

NA 支持不同的后端实现。然而，在大多数情况下，OFI/libfabric 是推荐用于节点间通信的插件，而 SM（共享内存）推荐用于节点内通信intra-node。

### 概括

下表总结了当前插件列表以及我们当前支持这些插件的传输。

| 插件class/传输protocal | `tcp`                                                  | `verbs`                                                | `shm`                                                  | `psm2`                                                 | `gni`                                                  |
| :-------- | :----------------------------------------------------- | :----------------------------------------------------- | :----------------------------------------------------- | :----------------------------------------------------- | :----------------------------------------------------- |
| `ofi`     | ![✔](https://twemoji.maxcdn.com/v/latest/svg/2714.svg) | ![✔](https://twemoji.maxcdn.com/v/latest/svg/2714.svg) | ![❔](https://twemoji.maxcdn.com/v/latest/svg/2754.svg) | ![❗](https://twemoji.maxcdn.com/v/latest/svg/2757.svg) | ![✔](https://twemoji.maxcdn.com/v/latest/svg/2714.svg) |
| `ucx`     | ![✔](https://twemoji.maxcdn.com/v/latest/svg/2714.svg) | ![✔](https://twemoji.maxcdn.com/v/latest/svg/2714.svg) | ![❔](https://twemoji.maxcdn.com/v/latest/svg/2754.svg) | ![❌](https://twemoji.maxcdn.com/v/latest/svg/274c.svg) | ![❔](https://twemoji.maxcdn.com/v/latest/svg/2754.svg) |
| `sm|na`      | ![❌](https://twemoji.maxcdn.com/v/latest/svg/274c.svg) | ![❌](https://twemoji.maxcdn.com/v/latest/svg/274c.svg) | ![✔](https://twemoji.maxcdn.com/v/latest/svg/2714.svg) | ![❌](https://twemoji.maxcdn.com/v/latest/svg/274c.svg) | ![❌](https://twemoji.maxcdn.com/v/latest/svg/274c.svg) |
| `bmi`     | ![✔](https://twemoji.maxcdn.com/v/latest/svg/2714.svg) | ![❌](https://twemoji.maxcdn.com/v/latest/svg/274c.svg) | ![❌](https://twemoji.maxcdn.com/v/latest/svg/274c.svg) | ![❌](https://twemoji.maxcdn.com/v/latest/svg/274c.svg) | ![❌](https://twemoji.maxcdn.com/v/latest/svg/274c.svg) |

警告

每个插件可能支持额外的传输，但我们不建议使用它们，除非在上表中明确提及，因为它们要么不稳定，要么未经测试。所选插件无法使用带有的传输。不支持传输，但将来可能会提供。带有已知问题的传输。

### 初始化字符串格式

下表总结了每个插件的协议和预期格式（`[ ]`表示可选，在这种情况下插件将选择要使用的默认主机名和端口）。

| 插件 | 协议                                                         | 初始化格式[1](https://mercury-hpc.github.io/user/na/#init_format) |
| :--- | :----------------------------------------------------------- | :----------------------------------------------------------- |
| ofi  | tcp<br/>verbs<br/>psm2<br/>gni                               | `ofi+tcp[://<hostname,IP,interface name>:<port>]`<br/>`ofi+verbs[://[domain/]<hostname,IP,interface name>:<port>]`[2](https://mercury-hpc.github.io/user/na/#ofi_verbs_config)<br/>`ofi+psm2`[3](https://mercury-hpc.github.io/user/na/#ofi_psm2_config)<br/>`ofi+gni[://<hostname,IP,interface name>]` [4](https://mercury-hpc.github.io/user/na/#ofi_gni_config) |
| ucx  | all<br/>tcp<br/>rc,ud [5](https://mercury-hpc.github.io/user/na/#ucx_tls) | `ucx+all[://[net_device/]<hostname,IP,interface name>:<port>]` |
| na   | sm                                                           | `na+sm[://<shm_prefix>]`                                     |
| bmi  | tcp                                                          | `bmi+tcp[://<hostname,IP>:<port>]`                           |
| mpi  | dynamic, static[6](https://mercury-hpc.github.io/user/na/#mpi_static) | `mpi+<dynamic, static>`                                      |

提示： 

传递的无效端口号可能会被底层实现静默忽略，在这种情况下，将自动选择一个新端口。

1在不监听的情况下初始化时，可以省略端口规范。

2也可以直接传递libfabric域名来选择合适的适配器使用。`fi_info`查看提供者名称的命令生成的输出`verbs;ofi_rxm`（例如，`mlx5_0`）。

3任何正在传递的主机名或端口都将被忽略。

4不需要传递端口信息，最常见的接口名称是`ipogif0`，如果不传递主机名，则默认使用。

5有关可用传输的完整列表，请参阅 UCX[文档。](https://openucx.readthedocs.io/en/master/faq.html#which-transports-does-ucx-use)

6 MPI 静态模式要求在同一个 mpirun 调用中启动所有汞进程。

### OFI 开放互联接口

（*从 v1.0.0 开始*）NA OFI/libfabric 插件可用于一般用途，但某些提供程序（libfabric 传输插件）可能仍处于早期开发状态。该插件目前支持 tcp、verbs、psm2 和 gni 传输。有关其他实施和性能详细信息，请参阅此[页面](https://mercury-hpc.github.io/user/ofi/)。

*技术说明：*

- 所有 libfabric 提供程序都支持低 CPU 消耗（即空闲而不忙旋转）。目前`sockets`，`psm2`和`gni`提供者通过使用内部进度线程来实现这一点。
- 无连接并使用可靠的数据报。
- RMA（用于 Mercury 批量操作）在支持它的传输（即动词、psm2 和 gni）上本地实现。
- ofi/tcp ( `tcp`provider) 使用 RxM 层来模拟无连接端点。它还模拟 RMA 操作。
- ofi/verbs ( `verbs`provider) 使用 RxM 层来模拟无连接端点（发送的第一条消息可能更慢）。
- ofi/psm2（`psm2`提供者）在多线程工作流中存在问题。它可用于英特尔® Omni-Path 互连。
- ofi/gni ( `gni`provider) 可用于带有 Gemini/Aries 互连的 Cray ®系统。请注意，当需要在单独的作业之间进行通信时，它需要使用 Cray ® DRC 来交换凭据（请参阅[DRC 凭据](https://mercury-hpc.github.io/user/drc/)部分）。

*影响变量：*

- `RDMAV_HUGEPAGES_SAFE`: 结合提供者使用大页面时必须设置`verbs`。
- `FI_UNIVERSE_SIZE`: 必须在超过 256 个与`tcp`或`verbs`提供者的对等点时设置。

有关每种传输的更多详细信息，请参阅 libfabric 联机帮助页。

### UCX

（*从 v2.1.0 开始*）UCX 插件可用于一般用途。默认情况下，与其他插件相反，UCX 插件能够自动确定最适合使用哪种传输方式。这是通过传递`all`关键字代替特定传输来实现的。但是请注意，我们只测试 UCX 的 `tcp`和`verbs`协议。

*技术说明：*

- 当前在连接的端点之上模拟无连接。因此，预计发送到目标的第一条消息将比后续消息慢。
- 需要使用启用线程安全的 UCX 库，除非用户使用`thread_mode`init 选项（见[上文](https://mercury-hpc.github.io/user/na/#initialization)）明确告诉 NA，他们不会访问具有多个线程的类和上下文。
- `NA_Addr_to_string()`不能用于非侦听进程将自身地址转换为字符串。这是因为 UCX 在连接之前不公开端点。

*影响变量：*

- `ucx_info -c -f`将显示默认配置。这些变量中的每一个都可以被用户覆盖。但是请注意，`UCX_TLS` 和`UCX_NET_DEVICES`当前已被 NA UCX 插件覆盖。
- NA UCX 插件当前`UCX_UNIFIED_MODE`默认设置为 true 以进行性能优化，因为我们希望给定系统的所有节点都具有相同的配置。

### SM

（*从 v0.9.0 开始*）这是集成的共享内存 NA 插件。插件稳定，为本地节点通信提供了明显更好的性能。这个插件的目标是为其他 NA 插件使用`auto_sm`初始化选项连接到本地服务时提供一个透明的快捷方式（有关更多详细信息，请参阅[RPC 部分](https://mercury-hpc.github.io/user/hg/)），但它也可用作单节点服务的主要传输。

*技术说明：*

- 使用完全无连接的通信。
- 低 CPU 消耗（即空闲时不忙于旋转或使用线程）。
- RMA（用于 Mercury 批量操作）是通过 Linux 上的跨内存附加 ( [CMA](https://lwn.net/Articles/405284/) ) 本地实现的，其他平台也有回退方法。有关其他实施和性能详细信息，请参阅此[页面](https://mercury-hpc.github.io/user/sm/) 。

### BMI

BMI 库本身不再处于基本维护之外的积极功能开发中，但 NA BMI 插件在与 BMI 的 TCP 方法一起使用时为 IP 网络提供了一个非常稳定且性能合理的选项。

*技术说明：*

- 低 CPU 消耗（即空闲时不忙于旋转或使用线程）。
- 支持动态客户端连接和断开。
- RMA（用于 Mercury 批量操作）通过点对点消息传递进行模拟。
- 不支持*同时*初始化多个实例。
- 不支持除 TCP 之外的其他 BMI 方法。
- 有关一般 BMI 信息，请参阅[本文](http://ieeexplore.ieee.org/abstract/document/1420118/)。

## 已弃用的插件

### CCI

（*已弃用*）此 NA 插件不再可用于一般用途，现在已弃用，因为 CCI 本身不再积极维护。

### MPI

MPI 实现几乎可用于任何平台，NA MPI 插件为原型设计和功能测试提供了方便的选项。但是，它并未针对性能进行优化，并且在用于持久服务时存在一些实际限制。

*技术说明：*

- 只有当底层 MPI 实现支持时，客户端才能动态连接到服务器`MPI_Comm_connect()`。
- RMA（用于 Mercury 批量操作）通过点对点消息传递进行模拟（注意：MPI 窗口创建需要集体协调，不适合 RPC 使用）。
- 显着的 CPU 消耗（progress 函数迭代地轮询挂起的操作以完成）。

------

最后更新： 2021 年 12 月 4 日