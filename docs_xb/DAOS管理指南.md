

# 管理指南





# 故障排除[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#troubleshooting)

## DAOS 错误[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#daos-errors)

DAOS 错误编号从 1000 开始。最常见的错误记录在下表中。

| DAOS 错误          | 价值   | 描述                                     |
| :----------------- | :----- | :--------------------------------------- |
| DER_NO_PERM        | 1001   | 没有权限                                 |
| DER_NO_HDL         | 1002   | 无效的句柄                               |
| DER_INVAL          | 1003   | 无效参数                                 |
| DER_EXIST          | 1004   | 实体已存在                               |
| DER_NONEXIST       | 1005   | 指定的实体不存在                         |
| DER_UNREACH        | 1006   | 无法访问的节点                           |
| DER_NOSPACE        | 1007   | 存储目标上没有剩余空间                   |
| DER_ALREADY        | 1008   | 已执行操作                               |
| DER_NOMEM          | 1009   | 记不清                                   |
| DER_NOSYS          | 1010   | 功能未实现                               |
| DER_TIMEDOUT       | 1011   | 暂停                                     |
| DER_BUSY           | 1012   | 设备或资源繁忙                           |
| DER_AGAIN          | 1013   | 再试一次                                 |
| DER_PROTO          | 1014   | 不兼容的协议                             |
| DER_UNINIT         | 1015   | 设备或资源未初始化                       |
| DER_TRUNC          | 1016   | 缓冲区太短                               |
| DER_OVERFLOW       | 1017   | 对于定义的数据类型或缓冲区大小，数据太长 |
| DER_CANCELED       | 1018   | 操作已取消                               |
| DER_OOG            | 1019   | 超出组或成员列表                         |
| DER_HG             | 1020   | 传输层水银误差                           |
| DER_MISC           | 1025   | 杂项错误                                 |
| DER_BADPATH        | 1026   | 错误的路径名                             |
| DER_NOTDIR         | 1027   | 不是目录                                 |
| DER_EVICTED        | 1032   | 等级已被驱逐                             |
| DER_DOS            | 1034   | 拒绝服务                                 |
| DER_BAD_TARGET     | 1035   | RPC 的目标不正确                         |
| DER_HLC_SYNC       | 1037   | HLC 同步错误                             |
| DER_IO             | 2001年 | 通用 I/O 错误                            |
| DER_ENOENT         | 2003年 | 未找到条目                               |
| DER_NOTYPE         | 2004年 | 未知对象类型                             |
| DER_NOSCHEMA       | 2005年 | 未知对象架构                             |
| DER_KEY2BIG        | 2012   | 密钥太大                                 |
| DER_REC2BIG        | 2013   | 记录太大                                 |
| DER_IO_INVAL       | 2014   | IO 缓冲区无法匹配对象范围                |
| DER_EQ_BUSY        | 2015   | 事件队列正忙                             |
| DER_SHUTDOWN       | 2017   | 服务应该关闭                             |
| DER_INPROGRESS     | 2018   | 操作正在进行中                           |
| DER_NOTREPLICA     | 2020   | 不是服务副本                             |
| DER_CSUM           | 2021   | 校验和错误                               |
| DER_REC_SIZE       | 2024   | 记录大小错误                             |
| DER_TX_RESTART     | 2025   | 交易应该重新开始                         |
| DER_DATA_LOSS      | 2026   | 数据丢失或无法恢复                       |
| DER_TX_BUSY        | 2028   | TX 未提交                                |
| DER_AGENT_INCOMPAT | 2029   | 代理与 libdaos 不兼容                    |

当操作失败时，DAOS 会返回一个否定的 DER 错误。有关错误的完整列表，请检查 [daos_errno.h](https://github.com/daos-stack/daos/blob/release/2.0/src/include/daos_errno.h) （`DER_ERR_GURT_BASE`等于 1000，`DER_ERR_DAOS_BASE`等于 2000）。

API 中提供了函数 d_errstr()，用于将错误编号转换为错误消息。

## 日志文件[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#log-files)

在服务器端，作为正常服务器操作的一部分创建了三个日志文件：

| 零件     | 配置参数        | 示例配置值             |
| :------- | :-------------- | :--------------------- |
| 控制平面 | 控制日志文件    | /tmp/daos_server.log   |
| 数据平面 | 日志文件        | /tmp/daos_engine.*.log |
| 特权助手 | helper_log_file | /tmp/daos_admin.log    |
| 代理人   | 日志文件        | /tmp/daos_agent.log    |

### 控制平面日志[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#control-plane-log)

控制平面的默认日志级别是 INFO。可以使用`control_log_mask`config 参数设置以下级别：

- 调试
- 信息
- 错误

### 数据平面日志[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#data-plane-log)

数据平面 ( `daos_engine`) 日志记录是基于每个实例配置的。换句话说，该部分下的每个部分`servers:`都必须有自己的日志记录配置。`log_file`config 参数被转换为 D_LOG_FILE 环境变量值。有关更多详细信息，请参阅本文档的[调试系统](https://docs.daos.io/v2.0/admin/troubleshooting/#debugging-system) 部分。

### 特权助手日志[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#privileged-helper-log)

默认情况下，特权助手仅发出错误级别的日志记录，这些日志记录由控制平面捕获并包含在该日志中。如果 `helper_log_file`在服务器配置中设置了该参数，则 DEBUG 级别的日志记录将被发送到指定的文件。

### Daos 代理日志[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#daos-agent-log)

如果`log_file`在代理配置中设置了 config 参数，则 DEBUG 级别的日志记录将被发送到指定的文件。

## 调试系统[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#debugging-system)

DAOS 使用 [CarRT](https://github.com/daos-stack/daos/tree/release/2.0/src/cart/)中定义的调试系统，特别是 GURT 库。服务器和客户端的默认日志都是`stdout`，除非`D_LOG_FILE`环境变量（客户端）或 `log_file`配置参数（服务器）另外设置。

### 注册子系统/设施[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#registered-subsystemsfacilities)

调试日志系统包括一系列子系统或工具，它们为相关日志消息定义组（根据源文件定义）。有在 GURT 中定义的通用设施，以及可以在每个项目的基础上定义的其他设施（例如用于 CaRT 和 DAOS 的设施）。DD_SUBSYS 可用于设置启用日志记录的子系统。默认情况下启用所有子系统（“DD_SUBSYS=all”）。

- DAOS Facilities: common, tree, vos, client, server, rdb, pool, container, object, placement, rebuild, tier, mgmt, bio, tests
- Common Facilities (GURT): MISC, MEM
- CaRT Facilities: RPC, BULK, CORPC, GRP, LM, HG, ST, IV

### 优先记录[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#priority-logging)

输出到 stderr 的优先级由 DD_STDERR 设置。在 DAOS（特定于项目）中默认设置为 CRIT（“DD_STDERR=CRIT”），这意味着所有 CRIT 和更严重的日志消息都将转储到 stderr。但是，这与记录到“/tmp/daos.log”的优先级是分开的。可以使用 D_LOG_MASK 设置日志记录的优先级，默认情况下设置为 INFO ("D_LOG_MASK=INFO")，这将导致除 DEBUG 消息之外的所有消息都被记录。D_LOG_MASK 也可用于指定基于每个子系统的日志记录级别（“D_LOG_MASK=DEBUG,MEM=ERR”）。

### 调试掩码/流：[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#debug-masksstreams)

DEBUG 消息占日志消息的大部分，可能需要更精细的粒度。掩码位设置为在 D_DEBUG(mask, ...) 中传递的第一个参数。为此，可以设置 DD_MASK 以启用不同的调试流。与设施类似，在 GURT 中定义了常见的调试流，以及可以在每个项目的基础上定义的其他流（CaRT 和 DAOS）。默认情况下启用所有调试流（“DD_MASK=all”）。

- DAOS 调试掩码：
  - md = 元数据操作
  - pl = 放置操作
  - mgmt = 池管理
  - epc = 纪元系统
  - df = 持久格式
  - 重建 = 重建过程
  - daos_default =（组掩码）io、md、pl 和重建操作
- 通用调试掩码 (GURT)：
  - any = 通用消息，无分类
  - 跟踪 = 函数跟踪、树/哈希/lru 操作
  - mem = 内存操作
  - net = 网络操作
  - io = 对象 I/Otest = 测试程序

### 常见用例[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#common-use-cases)

- 所有消息的通用设置（默认设置）

  ```
  D_LOG_MASK=DEBUG
  DD_SUBSYS=all
  DD_MASK=all
  ```

- 禁用所有日志以进行性能调整

  ```
  D_LOG_MASK=ERR -> will only log error messages from all facilities
  D_LOG_MASK=FATAL -> will only log system fatal messages
  ```

- 禁用嘈杂的调试日志记录子系统

  ```
  D_LOG_MASK=DEBUG,MEM=ERR -> disables MEM facility by
  restricting all logs from that facility to ERROR or higher priority only
  ```

- 启用感兴趣的设施子集

  ```
  DD_SUBSYS=rpc,tests
  D_LOG_MASK=DEBUG -> required to see logs for RPC and TESTS
  less severe than INFO (the majority of log messages)
  ```

- 通过设置调试掩码微调调试消息

  ```
  D_LOG_MASK=DEBUG
  DD_MASK=mgmt -> only logs DEBUG messages related to pool
  management
  ```

有关调试系统环境的更多信息，请参阅 DAOS 环境变量文档。

## 常见的 DAOS 问题[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#common-daos-problems)

### 不兼容的代理[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#incompatible-agent)

当收到 DER_AGENT_INCOMPAT 时，意味着客户端库 libdaos.so 可能与 DAOS 代理不匹配。libdaos.so、DAOS Agent 和 DAOS Server 必须从兼容的源构建，以便每个组件之间的 GetAttachInfo 协议相同。根据您的情况，您需要将 DAOS 代理或 libdaos.so 更新到较新的版本，以保持彼此的兼容性。

### HLC 同步[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#hlc-sync)

当收到 DER_HLC_SYNC 时，这意味着发送方和接收方 HLC 时间戳的偏差超过了最大允许的系统时钟偏移量（默认为 1 秒）。

为了纠正这种情况，使用 NTP 等服务将所有服务器时钟同步到相同的参考时间。

### 共享内存错误[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#shared-memory-errors)

当收到 DER_SHMEM_PERMS 时，这意味着此 I/O 引擎缺乏访问同一机器上先前运行 I/O 引擎留下的共享内存段的权限。当 I/O 引擎在关闭时无法删除共享内存段时，就会发生这种情况，并且在这些连续运行之间用于启动 I/O 引擎的用户/组之间存在不匹配。要解决此问题，请手动识别共享内存段并将其删除。

发出`ipcs`查看共享内存段的问题。输出将显示由 组织的段列表`key`。

```
ipcs

------ Message Queues --------
key        msqid      owner      perms      used-bytes   messages

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status
0xffffffff 49938432   root       666        40         0
0x10242048 98598913   jbrosenz   660        1048576    0
0x10242049 98631682   jbrosenz   660        1048576    0

------ Semaphore Arrays --------
key        semid      owner      perms      nsems
```

带有键 [0x10242048 .. (0x10242048 + 运行的 I/O 引擎数)] 的共享内存段是必须删除的段。用于`ipcrm`删除段。

例如，要删除 I/O 引擎实例 0 留下的共享内存段，请发出：

```
sudo ipcrm -M 0x10242048
```

要删除 I/O 引擎实例 1 留下的共享内存段，请发出：

```
sudo ipcrm -M 0x10242049
```

### 服务器启动问题[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#server-start-issues)

1. 阅读位于`control_log_file`.

2. 验证该`daos_server`进程当前未在运行。

3. 检查 /dev 中的 SCM 设备路径。

4. 使用 验证 PCI 地址`dmg storage scan`。

   笔记

   必须以最低设置启动服务器。您也可以使用 获取地址`daos_server storage scan`。

5. 格式化配置文件中定义的 SCM。

6. 使用`dmg config generate`. 将填充各种要求而不会出现语法错误。

7. 尝试从`allow_insecure: true`. 这将排除凭证证书问题。

8. 验证`access_points`主机是否可访问且端口未使用。

9. 检查`provider`条目。请参阅管理员指南的“网络扫描和配置”部分，以确定要使用的正确提供商。

10. 入住。`fabric_iface`_ `engines`它们应该可用并启用。

11. 检查`socket_dir`是否可由 daos_server 写入。

### 创建池时出错[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#errors-creating-a-pool)

1. 检查您想在哪个引擎等级中创建一个池`dmg system query --verbose`并验证其状态是否已加入。
2. `DER_NOSPACE(-1007)`出现：检查NVMe和PMEM的大小。接下来，检查现有池的大小。然后检查正在创建的这个新池是否适合剩余的磁盘空间。

### 创建容器的问题[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#problems-creating-a-container)

1. 检查 daos 的路径是否是您想要的二进制文件。通常是`/usr/bin/daos`。
2. 更改服务器配置时，需要重新启动代理。
3. `DER_UNREACH(-1006)`：检查 PMEM 和 NVMe 之间的套接字 ID 一致性。首先，确定您使用的是哪个套接字`daos_server network scan -p all`。例如，如果您在引擎部分使用的接口是 eth0，则查找它属于哪个 NUMA Socket。`daos_server storage scan`接下来，通过调用或确定您可以与此套接字一起使用的磁盘`dmg storage scan`。例如，如果 eth0 属于 NUMA Socket 0，则仅使用 Socket ID 列中为 0 的磁盘。
4. 检查服务端config( `fabric_iface`)中使用的接口是否也存在于客户端，并且可以与服务端通信。
5. 检查代理配置的 access_points 指向正确的服务器主机。
6. 调用`daos pool query`并检查池是否存在并且有可用空间。

### 应用程序运行缓慢[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#applications-run-slow)

验证您是否`fabric_iface`在服务器配置中将 Infiniband 用于 :。使用以太网时 IO 会明显变慢。

## 常见错误和解决方法[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#common-errors-and-workarounds)

### 使用没有 daos_admin 权限的 dmg 命令[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#use-dmg-command-without-daos_admin-privilege)

```
# Error message or timeout after dmg system query
$ dmg system query
ERROR: dmg: Unable to load Certificate Data: could not load cert: stat /etc/daos/certs/admin.crt: no such file or directory

# Workaround

# 1. Make sure the admin-host /etc/daos/daos_control.yml is correctly configured.
    # including:
        # hostlist: <daos_server_lists>
        # port: <port_num>
        # transport\config:
            # allow_insecure: <true/false>
            # ca\cert: /etc/daos/certs/daosCA.crt
            # cert: /etc/daos/certs/admin.crt
            # key: /etc/daos/certs/admin.key

# 2. Make sure the admin-host allow_insecure mode matches the applicable servers.
```

### 在 daos_agent 启动之前使用 daos 命令[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#use-daos-command-before-daos_agent-started)

```
$ daos cont create $DAOS_POOL
daos ERR  src/common/drpc.c:217 unixcomm_connect() Failed to connect to /var/run/daos_agent/daos_agent.sock, errno=2(No such file or directory)
mgmt ERR  src/mgmt/cli_mgmt.c:222 get_attach_info() failed to connect to /var/run/daos_agent/daos_agent.sock DER_MISC(-1025): 'Miscellaneous error'
failed to initialize daos: Miscellaneous error (-1025)


# Work around to check for daos_agent certification and start daos_agent
    #check for /etc/daos/certs/daosCA.crt, agent.crt and agent.key
    $ sudo systemctl enable daos_agent.service
    $ sudo systemctl start daos_agent.service
```

### 使用带有无效或错误参数的 daos 命令[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#use-daos-command-with-invalid-or-wrong-parameters)

```
# Lack of providing daos pool_uuid
$ daos pool list-cont
pool UUID required
rc: 2
daos command (v1.2), libdaos 1.2.0
usage: daos RESOURCE COMMAND [OPTIONS]
resources:
          pool             pool
          container (cont) container
          filesystem (fs)  copy to and from a POSIX filesystem
          object (obj)     object
          shell            Interactive obj ctl shell for DAOS
          version          print command version
          help             print this message and exit
use 'daos help RESOURCE' for resource specifics

# Invalid sub-command cont-list
$ daos pool cont-list --pool=$DAOS_POOL
invalid pool command: cont-list
error parsing command line arguments
daos command (v1.2), libdaos 1.2.0
usage: daos RESOURCE COMMAND [OPTIONS]
resources:
          pool             pool
          container (cont) container
          filesystem (fs)  copy to and from a POSIX filesystem
          object (obj)     object
          shell            Interactive obj ctl shell for DAOS
          version          print command version
          help             print this message and exit
use 'daos help RESOURCE' for resource specifics

# Working daos pool command
$ daos pool list-cont --pool=$DAOS_POOL
bc4fe707-7470-4b7d-83bf-face75cc98fc
```

## 由于没有空间，dmg 池创建失败[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#dmg-pool-create-failed-due-to-no-space)

```
$ dmg pool create --size=50G mypool
Creating DAOS pool with automatic storage allocation: 50 GB NVMe + 6.00% SCM
ERROR: dmg: pool create failed: DER_NOSPACE(-1007): No space on storage target

# Workaround: dmg storage query scan to find current available storage
    dmg storage query usage
    Hosts  SCM-Total SCM-Free SCM-Used NVMe-Total NVMe-Free NVMe-Used
    -----  --------- -------- -------- ---------- --------- ---------
    boro-8 17 GB     6.0 GB   65 %     0 B        0 B       N/A

    $ dmg pool create --size=2G mypool
    Creating DAOS pool with automatic storage allocation: 2.0 GB NVMe + 6.00% SCM
    Pool created with 100.00% SCM/NVMe ratio
    -----------------------------------------
      UUID          : b5ce2954-3f3e-4519-be04-ea298d776132
      Service Ranks : 0
      Storage Ranks : 0
      Total Size    : 2.0 GB
      SCM           : 2.0 GB (2.0 GB / rank)
      NVMe          : 0 B (0 B / rank)

    $ dmg storage query usage
    Hosts  SCM-Total SCM-Free SCM-Used NVMe-Total NVMe-Free NVMe-Used
    -----  --------- -------- -------- ---------- --------- ---------
    boro-8 17 GB     2.9 GB   83 %     0 B        0 B       N/A
```

### dmg 池销毁超时[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#dmg-pool-destroy-timeout)

```
# dmg pool destroy Timeout or failed due to pool has active container(s)
# Workaround pool destroy --force option

    $ dmg pool destroy --pool=$DAOS_POOL --force
    Pool-destroy command succeeded
```

## 错误报告[¶](https://docs.daos.io/v2.0/admin/troubleshooting/#bug-report)

应通过我们的[问题跟踪器](https://jira.daos.io/)报告错误 ，并使用测试用例重现问题（如果适用）和调试日志。

创建票证后，应从本文档“[日志文件”](https://docs.daos.io/v2.0/admin/troubleshooting/#log-files)部分中描述的位置收集日志并将其附加到票证。

为避免附加大文件时出现问题，请以压缩容器格式附加日志，例如 .zip 或 .tar.bz2。