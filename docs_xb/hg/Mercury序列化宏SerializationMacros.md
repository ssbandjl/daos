# Mercury 序列化宏 Serialization Macros

为方便起见，Mercury 提供了可以减少发送 RPC 调用所需的代码量的宏。Mercury 没有使用繁琐的 RPC 存根和代码生成器(tedious RPC stubs and code generators)，而是使用[Boost 预处理器库](http://www.boost.org/doc/libs/release/libs/preprocessor/)，以便用户可以生成序列化和反序列化函数参数所需的所有样板代码。



介绍
Boost Preprocessing 库是一个宏库，支持预处理器元编程。该库支持 C++ 和 C 编译。它不依赖于任何其他 Boost 库，因此可以用作独立库。
该库要求编译器支持可变参数宏[variadic macros](https://www.boost.org/doc/libs/1_80_0/libs/preprocessor/doc/topics/variadic_macros.html).。由于可变参数宏是 C99 或 C++11 功能，因此该库表面上至少需要在那些 C 或 C++ 标准级别进行编译。许多编译器在较低级别支持可变参数宏，因此只要不强制执行严格的 C++98/C++03 合规性，仍然可以在该编译级别使用该库。
Dave Abrahams 和 Aleksey Gurtovoy 的 C++ Template Metaprogramming: Concepts, Tools, and Techniques from Boost and Beyond 的摘录已经可用。这段摘录包含对预处理器库和预处理器元编程的基本介绍，它可以帮助刚接触该库的用户和有兴趣了解该库提供的一些设施的用户。





## RPC 注册

当通过 RPC 层注册一个新的 RPC 时（参见 [上一](https://mercury-hpc.github.io/user/hg/#registration)节），用户应该告诉 Mercury 如何序列化和反序列化输入和输出参数。为了促进这一点，并结合以下宏，`MERCURY_REGISTER()`宏通过将类型映射到生成的 proc 函数，使注册 RPC 调用更加方便。

```
MERCURY_REGISTER(hg_class, func_name, in_struct_type_name, out_struct_type_name, rpc_cb);
```

例子

```
int rpc_open(const char *path, rpc_handle_t handle, int *event_id);
```

可以使用`MERCURY_REGISTER`宏并直接传递输入/输出结构的类型。在不存在输入或输出参数的情况下，`void`可以将类型传递给宏。

```
rpc_id = MERCURY_REGISTER(hg_class, "rpc_open", rpc_open_in_t, rpc_open_out_t, rpc_open_cb);
```

### 预定义类型

Mercury 已经定义了一些类型并使用标准类型，因此在序列化和反序列化时，类型的大小在平台之间是固定的。为方便起见，HG 类型也可用于序列化批量句柄，例如，也可用于字符串等。

| 预定义类型 | 类型名称                                                     |
| :--------- | :----------------------------------------------------------- |
| 标准类型   | `int8_t`, `uint8_t` `int16_t`, `uint16_t` `int32_t`, `uint32_t` `int64_t`,`uint64_t` |
| 字符串     | `hg_string_t`,`hg_const_string_t`                            |
| 批量描述符 | `hg_bulk_t`                                                  |
| 汞类型     | `hg_bool_t`, `hg_id_t`, `hg_size_t`,`hg_ptr_t`               |

## 新类型说明

宏`MERCURY_GEN_PROC()`可用于描述通常由原始类型组成的新类型。该宏生成一个新结构和一个可用于序列化参数的 proc 函数。结构字段包含输入参数或输出参数。生成的 proc 例程使用来自预先存在的类型的 proc 例程来序列化和反序列化每个字段。

```
MERCURY_GEN_PROC(struct_type_name, fields)
```

例子

下面的函数有两个输入参数，一个输出参数和一个返回值。

```
int rpc_open(const char *path, rpc_handle_t handle, int *event_id);
```

以下宏可用于为输入参数生成样板代码（再次，请参阅[预定义类型](https://mercury-hpc.github.io/user/hg_macros/#predefined-types)部分以获取可传递给此宏的预先存在的类型的列表）：

```
MERCURY_GEN_PROC( rpc_open_in_t, ((hg_const_string_t)(path))
                                ((rpc_handle_t)(handle)) )

/* Will generate an rpc_open_in_t struct */
typedef struct {
    hg_const_string_t path;
    rpc_handle_t handle;
} rpc_open_in_t;

/* and an hg_proc_rpc_open_in_t proc function */
hg_return_t
hg_proc_rpc_open_in_t(hg_proc_t proc, void *data)
{
    hg_return_t ret;
    rpc_open_in_t *struct_data = (rpc_open_in_t *) data;

    ret = hg_proc_hg_const_string_t(proc, &struct_data->path);
    if (ret != HG_SUCCESS) {
      /* error */
    }
    ret = hg_proc_rpc_handle_t(proc, &struct_data->handle);
    if (ret != HG_SUCCESS) {
      /* error */
    }
    return ret;
}
```

请注意分隔字段名称及其类型的括号。然后每个字段由另一对括号分隔。这遵循 Boost 预处理器库的序列数据类型。

## 现有`struct`说明

然而，在某些情况下，Mercury 不知道参数类型，前面的示例就是这种`rpc_handle_t`类型。`MERCURY_GEN_STRUCT_PROC`对于这些情况，可以使用另一个名为MERCURY_GEN_STRUCT_PROC的宏。它为现有的结构或类型定义了一个序列化函数——这假设该类型可以映射到现有的类型；如果没有，用户可以创建自己的 proc 函数并使用`hg_proc_raw`获取字节流的例程。

```
MERCURY_GEN_STRUCT_PROC(struct_type_name, fields)
```

例子

以下函数具有一种非标准类型，`rpc_handle_t`.

```
int rpc_open(const char *path, rpc_handle_t handle, int *event_id);

/* pre-defined struct */
typedef struct {
    hg_uint64_t cookie;
} rpc_handle_t;
```

然后可以使用以下宏通过定义其字段来为该类型生成样板代码。

```
MERCURY_GEN_STRUCT_PROC( rpc_handle_t, ((hg_uint64_t)(cookie)) )

/* Will generate an hg_proc_rpc_handle_t function */
static hg_return_t
hg_proc_rpc_handle_t(hg_proc_t proc, void *data)
{
    hg_return_t ret;
    rpc_handle_t *struct_data = (rpc_handle_t *) data;

    ret = hg_proc_hg_uint64_t(proc, &struct_data->cookie);
    if (ret != HG_SUCCESS) {
      /* error */
    }
    return ret;
}
```

------

最后更新： 2021 年 12 月 4 日