# Mercury RPC 层

https://mercury-hpc.github.io/user/hg/

RPC 层用于发送和接收 RPC。RPC 参数通常很小。为了处理更大的参数，可以使用下一节中介绍的批量接口bulk。

https://mercury-hpc.github.io/user/hg_bulk/

## HG接口

HG 接口直接构建在 NA（网络抽象层[Network Abstraction Layer](https://mercury-hpc.github.io/user/na/)） 接口之上。发送 RPC 通常会导致发布两个 NA 操作：响应的预期接收和 RPC 请求的意外发送。因此，整个接口是非阻塞的，目的是在给定目标上异步执行 RPC。为了实现这一点，HG 接口提供了以下原语：*目标地址查找*、*RPC 注册*、 *RPC 执行*、*进度*和*取消*。

### 初始化

要初始化 HG RPC 接口，有两个选项可用，或者使用默认`HG_Init()`函数并指定初始化信息字符串，如 NA 插件[部分](https://mercury-hpc.github.io/user/na/#available-plugins)中所述：

```c
hg_class_t *
HG_Init(const char *info_string, hg_bool_t listen);
```

或者通过使用`HG_Init_opt()`允许传递额外选项的函数。

| 选项                | 描述                                                         |
| :------------------ | :----------------------------------------------------------- |
| `na_init_info`      | 请参阅 NA 初始化[选项](https://mercury-hpc.github.io/user/na/#na-interface)。 |
| `na_class`          | 从现有的 NA 类启用初始化。                                   |
| `request_post_init` | 控制在创建上下文时发布的请求的初始数量。 默认值为：256       |
| `request_post_incr` | 控制在初始请求数用完时增量发布的请求数。 默认值为：256       |
| `auto_sm`           | 控制当源和目标共享同一节点时是否应自动使用共享内存。         |
| `sm_info_string`    | 覆盖 NA SM 插件的默认初始化字符串。                          |
| `no_bulk_eager`     | 防止小批量数据与 RPC 请求一起自动嵌入。                      |
| `no_loopback`       | 禁用允许将 RPC 转发到自身地址的内部环回接口。                |

```c
struct hg_init_info {
    struct na_init_info na_init_info;
    na_class_t *na_class;
    hg_uint32_t request_post_init;
    hg_uint32_t request_post_incr;
    hg_bool_t auto_sm;
    const char *sm_info_string;
    hg_bool_t no_bulk_eager;
    hg_bool_t no_loopback;

struct hg_init_info {
    struct na_init_info na_init_info;   /* NA Init Info */
    na_class_t *na_class;               /* NA class */
    hg_bool_t auto_sm;                  /* Use NA SM plugin with local addrs */
    hg_bool_t stats;                    /* (Debug) Print stats at exit */
};

hg_class_t *
HG_Init_opt(const char *na_info_string, hg_bool_t na_listen,
            const struct hg_init_info *hg_init_info);
```

与 NA 层类似，`HG_Init()`调用会导致创建新`hg_class_t`对象。`hg_class_t`稍后可以在调用以下对象后释放该对象：

```c
hg_return_t
HG_Finalize(hg_class_t *hg_class);
```

接口初始化后，必须创建一个执行上下文，它（类似于 NA 层）在内部将特定队列与将完成的操作相关联：

```
hg_context_t *
HG_Context_create(hg_class_t *hg_class);
```

然后可以使用以下方法将其销毁：

```
hg_return_t
HG_Context_destroy(hg_context_t *context);
```

### 注册

在发送 RPC 之前，HG 类需要一种识别它的方法，以便可以在目标上执行与该 RPC 对应的回调。此外，必须提供序列化和反序列化与该 RPC 关联的函数参数的函数。这是通过`HG_Register_name()`函数完成的。请注意，使用宏MERCURY_REGISTER可以稍微简化此步骤。必须在具有相同`func_name`标识符的源和目标上进行注册。或者`HG_Register()`，可用于传递用户定义的唯一标识符并避免提供的函数名称的内部散列hashing。

```
typedef hg_return_t (*hg_proc_cb_t)(hg_proc_t proc, void *data);
typedef hg_return_t (*hg_rpc_cb_t)(hg_handle_t handle);

hg_id_t
HG_Register_name(hg_class_t *hg_class, const char *func_name,
                 hg_proc_cb_t in_proc_cb, hg_proc_cb_t out_proc_cb,
                 hg_rpc_cb_t rpc_cb);
```

在不再接收 RPC 的情况下，也可以（但不是必须）取消注册现有的 RPC ID，方法是使用：

```
hg_return_t
HG_Deregister(hg_class_t *hg_class, hg_id_t id);
```

在 RPC 不需要响应的情况下，可以通过使用以下调用（在已注册的 RPC 上）指示没有响应（因此避免等待消息被发送回）：

```
hg_return_t
HG_Registered_disable_response(hg_class_t *hg_class, hg_id_t id, hg_bool_t disable);
```

### 目标地址查找 Target Address Lookup

如[概述](https://mercury-hpc.github.io/user/overview/)部分所述，客户端和服务器之间没有真正的区别，因为可能希望客户端也充当其他进程的服务器。*因此，接口只使用了origin*和*target*的区别。

与 NA 的 API 类似，第一步是检索目标的地址，这可以通过调用目标来完成：

```
hg_return_t
HG_Addr_self(hg_class_t *hg_class, hg_addr_t *addr);
```

然后使用以下方法将其转换为字符串：

```
hg_return_t
HG_Addr_to_string(hg_class_t *hg_class, char *buf, hg_size_t *buf_size, hg_addr_t addr);
```

然后可以通过带外机制（例如，使用文件等）将字符串与源交换回来，然后可以使用以下函数查找目标：

```
hg_return_t
HG_Addr_lookup(hg_class_t *hg_class, const char *name, hg_addr_t *addr);
```

然后必须使用以下方法释放所有地址：

```
hg_return_t
HG_Addr_free(hg_class_t *hg_class, hg_addr_t addr);
```

### RPC 执行

执行一个 RPC 一般由两部分组成，一个在源端，它将发送 RPC 请求并接收响应，一个在目标端，它将接收请求，执行它并返回一个响应。

#### 源 Origin

使用调用后定义的 RPC ID `HG_Register()`，可以使用该`HG_Create()`调用定义一个新`hg_handle_t`对象，该对象可用于（稍后在不重新分配资源的情况下重新使用）设置/获取输入/输出参数。

```
hg_return_t
HG_Create(hg_context_t *context, hg_addr_t addr, hg_id_t id, hg_handle_t *handle);
```

这个句柄可以被销毁`HG_Destroy()`——并且引用计数可以防止在句柄仍在使用时释放资源。

```
hg_return_t
HG_Destroy(hg_handle_t handle);
```

第二步是将输入参数打包到一个结构中，`HG_Register()`调用时提供了一个序列化函数。`HG_Forward()`然后可以使用该 函数发送该结构（描述输入参数）。这个函数是非阻塞的。完成后，可以通过调用来执行关联的回调`HG_Trigger()`。

```
typedef hg_return_t (*hg_cb_t)(const struct hg_cb_info *callback_info);

hg_return_t
HG_Forward(hg_handle_t handle, hg_cb_t callback, void *arg, void *in_struct);
```

当`HG_Forward()`完成时（即，当用户回调可以被触发时），RPC 已经被远程执行并且带有输出结果的响应已经被发回。然后可以使用以下函数检索此输出（通常在回调中）：

```
hg_return_t
HG_Get_output(hg_handle_t handle, void *out_struct);
```

检索输出可能会导致创建内存对象，然后必须通过调用来释放：

```
hg_return_t
HG_Free_output(hg_handle_t handle, void *out_struct);
```

为安全起见，如有必要，必须在调用`HG_Free_output()`. 请注意，在 RPC 没有响应的情况下，RPC 成功发送后完成（即没有输出可检索）。

#### 目标 Target

在目标上，`HG_Register()`必须定义传递给调用的 RPC 回调函数。

```
typedef hg_return_t (*hg_rpc_cb_t)(hg_handle_t handle);
```

每当收到新的 RPC 时，就会调用该回调。输入参数可以通过以下方式检索：

```
hg_return_t
HG_Get_input(hg_handle_t handle, void *in_struct);
```

检索输入可能会导致创建内存对象，然后必须通过调用来释放：

```
hg_return_t
HG_Free_input(hg_handle_t handle, void *in_struct);
```

检索输入后，可以将输入结构中包含的参数传递给实际的函数调用。执行完成后，可以用函数的返回值和/或输出参数填充输出结构。然后可以使用以下方法将其发回：

```
typedef hg_return_t (*hg_cb_t)(const struct hg_cb_info *callback_info);

hg_return_t
HG_Respond(hg_handle_t handle, hg_cb_t callback, void *arg, void *out_struct);
```

这个调用也是非阻塞的。当它完成时，相关的回调被放置到一个完成队列中。然后可以在调用 后触发它 `HG_Trigger()`。请注意，在 RPC 没有响应的情况下，调用 `HG_Respond()`将返回错误。

### 进度和取消

Mercury 使用回调模型。回调被传递给非阻塞函数，并在操作完成时被推送到上下文的完成队列。通过调用`HG_Progress()`. `HG_Progress()`当操作完成、在完成队列中或`timeout`到达时返回。

```
hg_return_t
HG_Progress(hg_context_t *context, unsigned int timeout);
```

当一个操作完成时，调用`HG_Trigger()`允许回调执行与主进度循环分开控制。

警告

如果在提交操作后发生故障，则操作可能会以`HG_SUCCESS`返回代码或错误返回代码完成。因此，应始终检查结构中的`ret`字段是否存在潜在错误。`hg_cb_info`

```
hg_return_t
HG_Trigger(hg_context_t *context, unsigned int timeout,
           unsigned int max_count, unsigned int *actual_count);
```

在某些情况下，可能希望调用`HG_Progress()`then`HG_Trigger()`或让它们通过使用单独的线程并行执行。

当需要取消 HG 操作时，可以调用`HG_Cancel()`HG 句柄来取消正在进行的 `HG_Forward()`或`HG_Respond()`操作。有关取消操作和处理超时的更多详细信息，请参阅此 [页面](https://mercury-hpc.github.io/user/cancel/)。

```
hg_return_t
HG_Cancel(hg_handle_t handle);
```

取消总是异步的。当/如果操作成功取消，它将被推送到完成队列，并且回调`ret`值将设置一个`HG_CANCELED`错误返回码。

------

最后更新： 2021 年 12 月 4 日