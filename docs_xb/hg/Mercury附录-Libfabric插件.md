# 附录-Libfabric 插件

提示

本页中的实施说明仅供参考，可能略有过时。如果您只是在寻找用户文档，请参阅此[页面](https://mercury-hpc.github.io/user/na/#ofi)上的 NA 插件文档。

## OFI 能力

OFI 插件使用以下 libfabric 功能：

- `FI_EP_RDM`
- `FI_DIRECTED_RECV`
- `FI_READ`
- `FI_RECV`
- `FI_REMOTE_READ`
- `FI_REMOVE_WRITE`
- `FI_RMA`
- `FI_SEND`
- `FI_TAGGED`
- `FI_WRITE`
- `FI_SOURCE`
- `FI_SOURCE_ERR`
- `FI_ASYNC_IOV`
- `FI_CONTEXT`
- `FI_MR_BASIC`
- 可扩展端点

## 功能矩阵

- 支持的![✔](https://twemoji.maxcdn.com/v/latest/svg/2714.svg)
- 有限支持或模拟![❗](https://twemoji.maxcdn.com/v/latest/svg/2757.svg)（见脚注）
- 不支持![❌](https://twemoji.maxcdn.com/v/latest/svg/274c.svg)

| 特征         | tcp                                                          | 动词                                                         | psm2                                                         | gni                                                          |
| :----------- | :----------------------------------------------------------- | :----------------------------------------------------------- | :----------------------------------------------------------- | :----------------------------------------------------------- |
| 源寻址       | ![❗](https://twemoji.maxcdn.com/v/latest/svg/2757.svg)[1](https://mercury-hpc.github.io/user/ofi/#addressing_emul) | ![❗](https://twemoji.maxcdn.com/v/latest/svg/2757.svg)[1](https://mercury-hpc.github.io/user/ofi/#addressing_emul) | ![✔](https://twemoji.maxcdn.com/v/latest/svg/2714.svg)       | ![✔](https://twemoji.maxcdn.com/v/latest/svg/2714.svg)       |
| 手动进度     | ![❗](https://twemoji.maxcdn.com/v/latest/svg/2757.svg)[2](https://mercury-hpc.github.io/user/ofi/#progress_emul) | ![✔](https://twemoji.maxcdn.com/v/latest/svg/2714.svg)       | ![❗](https://twemoji.maxcdn.com/v/latest/svg/2757.svg)[2](https://mercury-hpc.github.io/user/ofi/#progress_emul) | ![❗](https://twemoji.maxcdn.com/v/latest/svg/2757.svg)[2](https://mercury-hpc.github.io/user/ofi/#progress_emul) |
| `FI_WAIT_FD` | ![✔](https://twemoji.maxcdn.com/v/latest/svg/2714.svg)       | ![✔](https://twemoji.maxcdn.com/v/latest/svg/2714.svg)       | ![✔](https://twemoji.maxcdn.com/v/latest/svg/2714.svg)       | ![❌](https://twemoji.maxcdn.com/v/latest/svg/274c.svg)       |
| 可扩展端点   | ![❗](https://twemoji.maxcdn.com/v/latest/svg/2757.svg)[3](https://mercury-hpc.github.io/user/ofi/#scal_emul) | ![❌](https://twemoji.maxcdn.com/v/latest/svg/274c.svg)       | ![✔](https://twemoji.maxcdn.com/v/latest/svg/2714.svg)       | ![✔](https://twemoji.maxcdn.com/v/latest/svg/2714.svg)       |

1 *模拟：*源地址编码在消息头中。

2 *模拟：*提供者正在使用线程来驱动后台进度。

3 *模拟：*提供者资源是共享的。

## 性能

下面是 libfabric 插件与可用的 libfabric 提供程序在同时使用等待和忙碌自旋机制、mercury v1.0.0 和 libfabric v1.7.0a1 时的性能比较。

### 无限频带`verbs;rxm`， InfiniBand

使用 FDR InfiniBand 接口在 ALCF 的[cooley](https://www.alcf.anl.gov/user-guides/cooley)集群上的两个节点之间测量性能`mlx5_0`。下图显示了 RPC 平均时间与`ofi+tcp`在飞行中使用单个 RPC 时的比较：

![RPC 时间 1](https://mercury-hpc.github.io/assets/ofi/verbs/rpc_time1.svg)

相同的情节，但有 16 个 RPC 在运行中：

![RPC 时间 16](https://mercury-hpc.github.io/assets/ofi/verbs/rpc_time16.svg)

下图显示了具有*拉式*批量传输性能的 RPC`ofi+tcp`与各种传输大小的比较：

![写入带宽 1](https://mercury-hpc.github.io/assets/ofi/verbs/write_bw1.svg)

相同的情节，但有 16 个 RPC 在运行中：

![写入带宽 16](https://mercury-hpc.github.io/assets/ofi/verbs/write_bw16.svg)

下图显示了具有*推送*批量传输性能的 RPC`ofi+tcp`与各种传输大小的比较：

![读取带宽 1](https://mercury-hpc.github.io/assets/ofi/verbs/read_bw1.svg)

相同的情节，但有 16 个 RPC 在运行中：

![写入带宽 16](https://mercury-hpc.github.io/assets/ofi/verbs/read_bw16.svg)

### 全方位路径`psm2`

使用 PSM2 v11.2.23 的 Omni-Path 接口在 LCRC 的[bebop](https://www.lcrc.anl.gov/systems/resources/bebop/)集群上的两个节点之间测量性能。下图显示了 RPC 平均时间与`ofi+tcp`在飞行中使用单个 RPC 时的比较：

![RPC 时间 1](https://mercury-hpc.github.io/assets/ofi/psm2/rpc_time1.svg)

相同的情节，但有 16 个 RPC 在运行中：

![RPC 时间 16](https://mercury-hpc.github.io/assets/ofi/psm2/rpc_time16.svg)

下图显示了具有*拉式*批量传输性能的 RPC`ofi+tcp`与各种传输大小的比较：

![写入带宽 1](https://mercury-hpc.github.io/assets/ofi/psm2/write_bw1.svg)

相同的情节，但有 16 个 RPC 在运行中：

![写入带宽 16](https://mercury-hpc.github.io/assets/ofi/psm2/write_bw16.svg)

下图显示了具有*推送*批量传输性能的 RPC`ofi+tcp`与各种传输大小的比较：

![读取带宽 1](https://mercury-hpc.github.io/assets/ofi/psm2/read_bw1.svg)

相同的情节，但有 16 个 RPC 在运行中：

![写入带宽 16](https://mercury-hpc.github.io/assets/ofi/psm2/read_bw16.svg)

### 白羊座`gni`

[使用带有 uGNI v6.0.14.0 的接口在 Nersc 的cori](http://www.nersc.gov/users/computational-systems/cori/)系统上的两个 Haswell 节点（排他模式下的调试队列）之间测量性能`ipogif0`。下图显示了 RPC 平均时间与`ofi+tcp`在飞行中使用单个 RPC 时的比较：

![RPC 时间 1](https://mercury-hpc.github.io/assets/ofi/gni/rpc_time1.svg)

相同的情节，但有 16 个 RPC 在运行中：

![RPC 时间 16](https://mercury-hpc.github.io/assets/ofi/gni/rpc_time16.svg)

下图显示了具有*拉式*批量传输性能的 RPC`ofi+tcp`与各种传输大小的比较：

![写入带宽 1](https://mercury-hpc.github.io/assets/ofi/gni/write_bw1.svg)

相同的情节，但有 16 个 RPC 在运行中：

![写入带宽 16](https://mercury-hpc.github.io/assets/ofi/gni/write_bw16.svg)

下图显示了具有*推送*批量传输性能的 RPC`ofi+tcp`与各种传输大小的比较：

![读取带宽 1](https://mercury-hpc.github.io/assets/ofi/gni/read_bw1.svg)

相同的情节，但有 16 个 RPC 在运行中：

![写入带宽 16](https://mercury-hpc.github.io/assets/ofi/gni/read_bw16.svg)

------

最后更新： 2021 年 12 月 4 日