# 批量层Mercury Bulk Layer

除了上一层，一些 RPC 可能需要传输更大量的数据。对于这些 RPC，可以使用bulk层。*它建立在网络抽象层中定义的 RMA 协议之上，并防止中间内存复制。

## HG 批量接口

该接口允许原始进程通过创建一个批量描述符（其中包含虚拟内存地址信息、正在公开的内存区域的大小以及依赖于底层网络实现的其他参数）来向目标公开一个内存区域。批量描述符可以与 RPC 请求参数一起序列化并发送到目标（使用 RPC 层）。当目标获取输入参数时，它可以反序列化批量描述符，获取必须传输的内存缓冲区的大小，并启动传输。只有目标应该启动单向传输，以便它们能够在控制数据流的同时保护它们的内存免受并发访问。

由于在传输完成时没有发送明确的 ack 消息，因此源进程只能假设一旦它收到来自目标的 RPC 响应，就完成了对其本地内存的访问。因此，在 RPC 没有响应的情况下，在启动批量传输时应格外小心，以确保在可以安全释放和访问其暴露的内存时通知源。

### 描述符

该接口使用 HG RPC 层定义的类和执行上下文。要启动批量传输，需要在源和目标上创建一个批量描述符，稍后将传递给`HG_Bulk_transfer()` 调用。

```
hg_return_t
HG_Bulk_create(hg_class_t *hg_class, hg_uint32_t count,
               void **buf_ptrs, const hg_size_t *buf_sizes,
               hg_uint8_t flags, hg_bulk_t *handle);
```

可以使用以下方法释放批量描述符：

```
hg_return_t HG_Bulk_free(hg_bulk_t handle);
```

为方便起见，现有批量描述符中的内存指针可以通过以下方式访问：

```
hg_return_t
HG_Bulk_access(hg_bulk_t handle, hg_size_t offset, hg_size_t size,
               hg_uint8_t flags, hg_uint32_t max_count, void **buf_ptrs,
               hg_size_t *buf_sizes, hg_uint32_t *actual_count);
```

此外，还可以通过`HG_Bulk_bind()`函数将源地址绑定到批量句柄，但代价是序列化和反序列化寻址信息的额外开销。仅当从`HG_Get_info()`调用中检索到的源地址与必须用于传输的源地址不同（例如，多个源）时，才需要这样做。

```
hg_return_t
HG_Bulk_bind(hg_bulk_t handle, hg_context_t *context);
```

在这种特殊情况下，可以使用以下方法直接检索地址信息：

```
hg_addr_t
HG_Bulk_get_addr(hg_bulk_t handle);
```

### 序列化

批量句柄的序列化和反序列化永远不应该由用户明确地完成，我们鼓励使用提供例程的mercury proc 例程`hg_proc_hg_bulk_t`：

```
hg_return_t
hg_proc_hg_bulk_t(hg_proc_t proc, void *data);
```

有关 RPC 参数序列化的更多详细信息，请参阅本[节](https://mercury-hpc.github.io/user/hg_macros/)。

### 批量传输

当接收到来自源的批量描述符时，目标可以启动到/从其自己的批量描述符的批量传输。虚拟偏移可用于透明地从非连续块传输数据块。呼叫是非阻塞的。当操作完成时，用户回调被放置到上下文的完成队列中。

```
hg_return_t
HG_Bulk_transfer(hg_context_t *context, hg_bulk_cb_t callback, void *arg,
                 hg_bulk_op_t op, hg_addr_t origin_addr,
                 hg_bulk_t origin_handle, hg_size_t origin_offset,
                 hg_bulk_t local_handle, hg_size_t local_offset,
                 hg_size_t size, hg_op_id_t *op_id);
```

请注意，为方便起见，由于需要在 RPC 目标上的 RPC 回调中实现传输，因此该例程`HG_Get_info()`可以轻松检索类、上下文和源地址：

```
struct hg_info {
    hg_class_t *hg_class;               /* HG class */
    hg_context_t *context;              /* HG context */
    hg_addr_t addr;                     /* HG address */
    hg_id_t id;                         /* RPC ID */
};

struct hg_info *
HG_Get_info(hg_handle_t handle);
```

------

最后更新： 2021 年 12 月 4 日