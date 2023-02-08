# 池操作[¶](https://docs.daos.io/v2.0/admin/pool_operations/#pool-operations)

DAOS 池是一种存储预留，可以跨越 DAOS 系统中的任意数量的存储引擎。池由管理员管理。分配给池的空间量是在创建时使用`dmg pool create`命令决定的。DAOS 管理 API 也提供了这种能力。

## 池基础知识[¶](https://docs.daos.io/v2.0/admin/pool_operations/#pool-basics)

该`dmg pool`命令是管理池的主要管理工具。它的子命令可以分为以下几个方面：

- 创建、列出、查询、扩展和销毁池的命令。这些是管理池中存储分配的基本功能。
- 列出和设置池属性的命令，以及设置和列出池的访问控制列表 (ACL) 的命令。
- 用于管理故障和其他非标准场景的命令。这包括排空、排除和重新集成目标，以及将客户端连接逐出池。

### 创建池[¶](https://docs.daos.io/v2.0/admin/pool_operations/#creating-a-pool)

可以通过`dmg pool create`命令创建 DAOS 池。创建池所需的强制性参数是池标签和存储分配大小的规范。

池标签必须由字母数字字符、冒号 ( `:`)、句点 ( `.`)、连字符 ( `-`) 或下划线 ( `_`) 组成。池标签的最大长度为 127 个字符。禁止使用可以解析为 UUID 的标签（例如 123e4567-e89b-12d3-a456-426614174000）。池标签在 DAOS 系统中必须是唯一的。

池的大小由两个因素决定：有多少存储引擎参与存储分配，以及每个存储层中分配了多少容量（后者可以在每个引擎的基础上指定，也可以作为所有参与引擎的池）。将在每个参与的存储引擎上分配相同数量的存储。如果其中一个或多个引擎没有足够的可用空间来满足请求的容量，则池创建将失败。

如果既不使用`--nranks`也不使用`--ranks`选项，则池将跨越 DAOS 系统的所有存储引擎。要将池限制为仅引擎的子集，这两个选项可用于指定所需的引擎数量，或用于此池的引擎等级的显式列表。

可以通过两种不同的方式指定池的容量：

1. 该`--size`选项可用于以**字节为单位指定***总*池容量。该值表示 SCM 和 NVMe 容量的总和。SCM 和 NVMe 存储层对总池大小的相对贡献由 参数确定。默认情况下，该比率为，因此对于大小为 100TB 的池，将有 6TB 的 SCM 和 94TB 的 NVMe 存储。可以使用.`--tier-ratio``6,94``--tier-ratio 100,0`
2. `--scm-size`参数（和可选的`--nvme-size`）可用于指定*每个存储引擎*的 SCM 容量（以及可选的 NVMe 容量），以**Bytes**为单位。**每个目标**的最小 SCM 大小为 16 MiB ，因此对于具有 16 个目标的存储引擎，最小值为`--scm-size=256MiB`. NVMe 大小可以为零。如果它不为零，则最小 NVMe 大小为 1 GiB 每个**目标**，因此对于具有 16 个目标的存储引擎，最小非零 NVMe 大小为 `--nvme-size=16GiB`。为了得出总池容量，这些每个引擎的容量必须乘以参与引擎的数量。

笔记

后缀“M”、“MB”、“G”、“GB”、“T”或“TB”表示 base-10 容量，而“MiB”、“GiB”或“TiB”表示 base-2。因此，在上面的第一个示例中，指定`--scm-size=256GB` 将失败，因为 256 GB 小于最小 256 GiB。

例子：

要创建一个标记为 的池`tank`：

```bash
$ dmg pool create --size=${N}TB tank
```

此命令创建一个标记为`tank`分布在 DAOS 服务器上的池，每个服务器上的目标大小由 $N * 0.94 TB 的 NVMe 存储和 $N * 0.06 TB 的 SCM 存储组成。默认 SCM:NVMe 比率可以在池创建时进行调整，如上所述。

分配给新创建的池的 UUID 打印到标准输出以及池服务副本等级。

```bash
$ dmg pool create --help
...

[create command options]
      -g, --group=      DAOS pool to be owned by given group, format name@domain
      -u, --user=       DAOS pool to be owned by given user, format name@domain
      -p, --label=      Unique label for pool (deprecated, use positional argument)
      -P, --properties= Pool properties to be set
      -a, --acl-file=   Access Control List file path for DAOS pool
      -z, --size=       Total size of DAOS pool (auto)
      -t, --tier-ratio= Distribution of pool storage allocation over storage tiers (auto) (default: 6)
      -k, --nranks=     Number of ranks to use (auto)
      -v, --nsvc=       Number of pool service replicas
      -s, --scm-size=   Per-server SCM allocation for DAOS pool (manual)
      -n, --nvme-size=  Per-server NVMe allocation for DAOS pool (manual)
      -r, --ranks=      Storage server unique identifiers (ranks) for DAOS pool
```

该命令的典型输出如下：

```bash
$ dmg pool create --size 50GB tank
Creating DAOS pool with automatic storage allocation: 50 GB total, 6,94 tier ratio
Pool created with 6.00% SCM/NVMe ratio
-----------------------------------------
  UUID          : 8a05bf3a-a088-4a77-bb9f-df989fce7cc8
  Replica Ranks : [1-3]
  Target Ranks  : [0-15]
  Size          : 50 GB
  SCM           : 3.0 GB (188 MB / rank)
  NVMe          : 47 GB (3.0 GB / rank)
```

这创建了一个 UUID 为 8a05bf3a-a088-4a77-bb9f-df989fce7cc8 的池，默认启用冗余（1-3 级池服务副本）。

如果不需要冗余，请使用 --nsvc=1 以指定仅应创建单个池服务副本。

请注意，用户很难确定可用空间，目前我们无法提供准确的值。可用空间不仅取决于池大小，还取决于目标数量、目标大小、对象类别、存储冗余因子等。

### 池[¶](https://docs.daos.io/v2.0/admin/pool_operations/#listing-pools)

要查看 DAOS 系统中的池列表：

```bash
$ dmg pool list
Pool     Size   Used Imbalance Disabled
----     ----   ---- --------- --------
tank     47 GB  0%   0%        0/32
```

这将返回一个池标签表（如果未指定标签，则返回 UUID），其中包含每个池的以下信息： - 总池大小 - 已用空间的百分比（即 100 * 已用空间/总空间） - 不平衡百分比指示跨不同存储目标的数据分布是否均衡。0% 表示不存在不平衡，100% 表示某些存储目标可能返回空间不足错误，而其他存储目标仍有可用空间。- 禁用目标的数量（此处为 0）和池最初配置的目标数量（总数）。

--verbose 选项提供更详细的信息，包括服务复制的数量、完整的 UUID 以及每个池的 SCM 和 NVMe 之间的空间分配：

```bash
$ dmg pool list --verbose
Label UUID                                 SvcReps SCM Size SCM Used SCM Imbalance NVME Size NVME Used NVME Imbalance Disabled
----- ----                                 ------- -------- -------- ------------- --------- --------- -------------- --------
tank  8a05bf3a-a088-4a77-bb9f-df989fce7cc8 1-3      3 GB    10 kB    0%            47 GB     0 B       0%             0/32
```

### 破坏池[¶](https://docs.daos.io/v2.0/admin/pool_operations/#destroying-a-pool)

要销毁标记为 的池`tank`：

```bash
$ dmg pool destroy tank
Pool-destroy command succeeded
```

可以使用池的 UUID 代替池标签。

### 查询池[¶](https://docs.daos.io/v2.0/admin/pool_operations/#querying-a-pool)

池查询操作检索有关现有池的信息（即目标数量、空间使用情况、重建状态、属性列表等）。

要查询标记为 的池`tank`：

```bash
$ dmg pool query tank
```

可以使用池的 UUID 代替池标签。以下是仅使用 SCM 空间创建的池的输出。

```bash
    pool=47293abe-aa6f-4147-97f6-42a9f796d64a
    Pool 47293abe-aa6f-4147-97f6-42a9f796d64a, ntarget=64, disabled=8
    Pool space info:
    - Target(VOS) count:56
    - SCM:
        Total size: 28GB
        Free: 28GB, min:505MB, max:512MB, mean:512MB
    - NVMe:
        Total size: 0
        Free: 0, min:0, max:0, mean:0
    Rebuild done, 10 objs, 1026 recs
```

总大小和自由大小是所有目标的总和，而 min/max/mean 提供有关单个目标的信息。接近 0 的最小值意味着一个目标空间不足。

注意：版本控制对象存储 (VOS) 可以保留 SCM 和 NVMe 分配的一部分，以减轻碎片和后台操作（例如，聚合、垃圾收集）。预留的存储量取决于目标的大小，可能会占用 2+ GB。因此，即使池查询可能未显示接近零的最小可用空间，也可能会出现空间不足的情况。

下面的示例显示了正在进行的重建和分配的 NVMe 空间。

```bash
    pool=95886b8b-7eb8-454d-845c-fc0ae0ba5671
    Pool 95886b8b-7eb8-454d-845c-fc0ae0ba5671, ntarget=64, disabled=8
    Pool space info:
    - Target(VOS) count:56
    - SCM:
        Total size: 28GB
        Free: 28GB, min:470MB, max:512MB, mean:509MB
    - NVMe:
        Total size: 56GB
        Free: 28GB, min:470MB, max:512MB, mean:509MB
    Rebuild busy, 75 objs, 9722 recs
```

计划通过管理工具导出其他状态和遥测数据，一旦可用，将在此处记录。

### 驱逐用户[¶](https://docs.daos.io/v2.0/admin/pool_operations/#evicting-users)

将句柄/连接逐出到标记为 的池`tank`：

```bash
$ dmg pool evict tank
Pool-evict command succeeded
```

可以使用池的 UUID 代替池标签。

## 池属性[¶](https://docs.daos.io/v2.0/admin/pool_operations/#pool-properties)

属性是预定义的参数，管理员可以调整这些参数来控制池的行为。

### 物业管理[¶](https://docs.daos.io/v2.0/admin/pool_operations/#properties-management)

`dmg pool get-prop`可以通过命令行检索现有池的当前属性。

```bash
$ dmg pool get-prop tank
Pool 8a05bf3a-a088-4a77-bb9f-df989fce7cc8 properties:
Name                            Value
----                            -----
EC cell size (ec_cell_sz)       1.0 MiB
Pool label (label)              tank
Reclaim strategy (reclaim)      lazy
Self-healing policy (self_heal) exclude
Rebuild space ratio (space_rb)  0%
```

创建池时可以指定所有属性。

```bash
$ dmg pool create --size 50GB --properties reclaim:disabled tank2
Creating DAOS pool with automatic storage allocation: 50 GB NVMe + 6.00% SCM
Pool created with 100.00% SCM/NVMe ratio
-----------------------------------------
  UUID          : 1f265216-5877-4302-ad29-aa0f90df3f86
  Service Ranks : 0
  Storage Ranks : [0-1]
  Total Size    : 50 GB
  SCM           : 50 GB (25 GB / rank)
  NVMe          : 0 B (0 B / rank)

$ dmg pool get-prop tank2
Pool 1f265216-5877-4302-ad29-aa0f90df3f86 properties:
Name                            Value
----                            -----
EC cell size (ec_cell_sz)       1.0 MiB
Pool label (label)              tank2
Reclaim strategy (reclaim)      disabled
Self-healing policy (self_heal) exclude
Rebuild space ratio (space_rb)  0%
```

某些属性可以在创建池后通过该`set-prop`选项进行修改。

```bash
$ dmg pool set-prop tank2 reclaim:lazy
pool set-prop succeeded

$ dmg pool get-prop tank2 reclaim
Pool 1f265216-5877-4302-ad29-aa0f90df3f86 properties:
Name                       Value
----                       -----
Reclaim strategy (reclaim) lazy
```

### 回收策略（reclaim）[¶](https://docs.daos.io/v2.0/admin/pool_operations/#reclaim-strategy-reclaim)

DAOS 是一个版本化的对象存储，它用一个纪元号标记每个 I/O。这种版本控制机制是多版本并发控制和快照支持的基线。随着时间的推移，需要回收未使用的版本以释放存储空间并简化元数据索引。这个过程称为聚合。

reclaim 属性定义了使用什么策略来回收未使用的版本。支持三个选项：

- "lazy" ：仅在没有 IO 活动或 SCM 可用空间有压力时触发聚合（默认策略）
- "time" ：尽管有 IO 活动，但仍会定期触发聚合。
- "disabled" ：从不触发聚合。即使正在删除数据，系统最终也会耗尽空间。

### 自我修复策略 (self_heal)[¶](https://docs.daos.io/v2.0/admin/pool_operations/#self-healing-policy-self_heal)

此属性定义是否自动将失败的引擎从池中逐出。排除后，将触发自愈机制，以在幸存的存储引擎上恢复池数据冗余。支持两个选项：“排除”（默认策略）和“重建”。

### 预留空间 (space_rb)[¶](https://docs.daos.io/v2.0/admin/pool_operations/#reserve-space-space_rb)

此属性定义每个存储引擎上为自我修复目的而保留的总空间百分比。保留的空间不能被应用程序占用。有效值为 0% 到 100%，默认值为 0%。设置此属性时，指定百分比符号是可选的： `space_rb:2%`并且`space_rb:2`都指定存储容量的 2%。

### EC 单元大小 (ec_cell_sz)[¶](https://docs.daos.io/v2.0/admin/pool_operations/#ec-cell-size-ec_cell_sz)

此属性定义继承到 DAOS 容器的默认纠删码单元大小。该值通常介于 32K 和 1MB 之间。

## 访问控制列表[¶](https://docs.daos.io/v2.0/admin/pool_operations/#access-control-lists)

池的客户端用户和组访问由 [访问控制列表 (ACL) 控制](https://docs.daos.io/v2.0/overview/security/#access-control-lists)。大多数与池相关的任务都是使用 DMG 管理工具执行的，该工具由管理证书而不是用户特定的凭据进行身份验证。

访问控制的客户端池访问包括：

- 连接到游泳池。
- 查询池。
- 在池中创建容器。
- 删除池中的容器。

这反映在支持的 [池权限集中](https://docs.daos.io/v2.0/overview/security/#permissions)。

用户必须能够连接到池才能访问其中的任何容器，无论他们对这些容器的权限如何。

### 所有权[¶](https://docs.daos.io/v2.0/admin/pool_operations/#ownership)

默认情况下，该`dmg pool create`命令将使用当前用户和当前组来设置池的所有者用户和所有者组。可以使用`--user`和`--group`选项更改此默认值。

池所有权没有为访问控制决策传递特殊特权。`OWNER@`所有者用户 ( ) 和所有者组 ( )的所有所需权限`GROUP@`必须由管理员在池 ACL 中明确定义。

### 池创建时的 ACL[¶](https://docs.daos.io/v2.0/admin/pool_operations/#acl-at-pool-creation)

要使用自定义 ACL 创建池：

```bash
$ dmg pool create --size <size> --acl-file <path> <pool_label>
```

ACL 文件格式在[此处](https://docs.daos.io/v2.0/overview/security/#acl-file)详细介绍。

### 显示 ACL[¶](https://docs.daos.io/v2.0/admin/pool_operations/#displaying-acl)

查看池的 ACL：

```bash
$ dmg pool get-acl --acl-file <path> <pool_label>
```

输出与创建期间 ACL 文件中使用的字符串格式相同，每行有一个访问控制条目（即 ACE）。

示例输出如下所示：

```bash
$ dmg pool get-acl tank
# Owner: jlombard@
# Owner Group: jlombard@
# Entries:
A::OWNER@:rw
A::bob@:r
A:G:GROUP@:rw
```

### 修改 ACL[¶](https://docs.daos.io/v2.0/admin/pool_operations/#modifying-acl)

对于使用 ACL 文件的所有这些命令，ACL 文件必须采用上述用于创建容器的格式。

#### 覆盖 ACL[¶](https://docs.daos.io/v2.0/admin/pool_operations/#overwriting-acl)

用新的 ACL 替换池的 ACL：

```bash
$ dmg pool overwrite-acl --acl-file <path> <pool_label>
```

#### 添加和更新 ACE[¶](https://docs.daos.io/v2.0/admin/pool_operations/#adding-and-updating-aces)

要在现有池 ACL 中添加或更新多个条目：

```bash
$ dmg pool update-acl --acl-file <path> <pool_label>
```

要在现有池 ACL 中添加或更新单个条目：

```bash
$ dmg pool update-acl --entry <ACE> <pool_label>
```

如果 ACL 中没有主体的现有条目，则将新条目添加到 ACL。如果主体已经有一个条目，则该条目将替换为新条目。

例如：

```bash
$ dmg pool update-acl -e A::kelsey@:r tank
Pool-update-ACL command succeeded, UUID: 8a05bf3a-a088-4a77-bb9f-df989fce7cc8
# Owner: jlombard@
# Owner Group: jlombard@
# Entries:
A::OWNER@:rw
A::bob@:r
A::kelsey@:r
A:G:GROUP@:rw
```

#### 删除 ACE[¶](https://docs.daos.io/v2.0/admin/pool_operations/#removing-an-ace)

要删除现有池 ACL 中给定主体的条目：

```bash
$ dmg pool delete-acl --principal <principal> <pool_label>
```

主体对应于在池创建或之前的池 ACL 操作期间设置的 ACE 的主体部分。对于删除操作，主体参数的格式必须如下：

- 指定用户：`u:username@`
- 命名组：`g:groupname@`
- 特殊主体：`OWNER@`、`GROUP@`和`EVERYONE@`

该主体的条目将被完全删除。这并不总是意味着主体将无权访问。相反，他们对池的访问将根据剩余的 ACL 规则来决定。

## 池修改[¶](https://docs.daos.io/v2.0/admin/pool_operations/#pool-modifications)

### 自动排除[¶](https://docs.daos.io/v2.0/admin/pool_operations/#automatic-exclusion)

默认情况下，被 SWIM 监控协议检测为死机的引擎将自动从使用该引擎的所有池中排除。因此，引擎不仅会被系统标记为已排除（即，在 中`dmg system query`），而且还会在`dmg pool query`所有受影响的池的池查询输出中报告为禁用（即，）。

排除后，将自动触发集体重建过程（即，也称为自我修复）以恢复幸存引擎上的数据冗余。

笔记

重建过程可能会消耗每个引擎上的许多资源，因此会受到限制以减少对应用程序性能的影响。此当前逻辑依赖于存储节点上的 CPU 周期。默认情况下，重建过程配置为消耗多达 30% 的 CPU 周期，而将其他 70% 用于常规 I/O 操作。

### 手动排除[¶](https://docs.daos.io/v2.0/admin/pool_operations/#manual-exclusion)

操作员可以使用目标所在的等级以及该等级上的目标 idx 从特定 DAOS 池中排除一个或多个引擎或目标。如果未提供目标 idx 列表，则将排除引擎排名上的所有目标。

要从池中排除目标：

```bash
$ dmg pool exclude --rank=${rank} --target-idx=${idx1},${idx2},${idx3} <pool_label>
```

pool target exclude 命令接受 2 个参数：

- 要排除的目标的引擎等级。
- 要从该引擎排名中排除的目标的目标索引（可选）。

成功手动排除后，将触发自愈机制以恢复剩余引擎上的冗余。

### 流走[¶](https://docs.daos.io/v2.0/admin/pool_operations/#drain)

或者，当操作员想要移除一个或多个引擎或目标而不使系统以降级模式运行时，可以使用排放操作。池排空操作会在不排除指定引擎或目标的情况下启动重建，直到重建完成。这允许耗尽的实体在重建操作正在进行时继续执行 I/O。Drain 还允许将非复制数据重新构建到另一个目标上，而在传统的故障情况下，非复制数据不会被集成到重建中并且会丢失。

要从池中排出目标：

```bash
$ dmg pool drain --rank=${rank} --target-idx=${idx1},${idx2},${idx3} $DAOS_POOL
```

pool target drain 命令接受 2 个参数：

- 要排放的目标的引擎等级。
- 要从该引擎等级中排出的目标的目标索引（可选）。

### 重新融合[¶](https://docs.daos.io/v2.0/admin/pool_operations/#reintegration)

在引擎发生故障和排除后，操作员可以修复潜在问题并重新集成受影响的引擎或目标，以将池恢复到其原始状态。操作员可以通过提供目标 idx 列表重新整合引擎等级的特定目标，或者通过省略列表重新整合整个引擎等级。

```
$ dmg pool reintegrate $DAOS_POOL --rank=${rank} --target-idx=${idx1},${idx2},${idx3}
```

pool reintegrate 命令接受 3 个参数：

- 目标将重新集成到的池的标签或 UUID。
- 受影响目标的引擎等级。
- 要在该引擎等级上重新集成的目标的目标索引（可选）。

触发重建时，它将按排名 ID 和目标索引列出操作及其相关目标。

```
Target (rank 5 idx 0) is down.
Target (rank 5 idx 1) is down.
...
(rank 5 idx 0) is excluded.
(rank 5 idx 1) is excluded.
```

这些值应该与重新集成目标时使用的值相同。

```
$ dmg pool reintegrate $DAOS_POOL --rank=5 --target-idx=0,1
```

警告

虽然 dmg 池查询和列表显示每个池禁用了多少目标，但目前无法列出实际已禁用的目标。因此，目前建议尝试通过`for i in seq $NR_RANKs; do dmg pool reintegrate --rank=$i; done`. 此限制将在下一个版本中解决。

## 游泳池扩展[¶](https://docs.daos.io/v2.0/admin/pool_operations/#pool-extension)

### 加法和空间再平衡[¶](https://docs.daos.io/v2.0/admin/pool_operations/#addition-space-rebalancing)

计划在未来的版本中全面支持在线目标添加和自动空间重新平衡，一旦可用，将在此处记录。

在此之前，以下命令是占位符，并提供与在线服务器添加/重新平衡操作相关的有限功能。

操作员可以选择扩展池以包括当前不在池中的等级。这将自动触发服务器重新平衡操作，其中扩展池中的对象将在新存储中重新平衡。

```
$ dmg pool extend $DAOS_POOL --ranks=${rank1},${rank2}...
```

pool extend 命令接受一个必需参数，该参数是以逗号分隔的要包含在池中的服务器等级列表。

当池在单个操作中扩展到其所需大小时，池重新平衡操作将最有效地工作，而不是多个小型扩展。

### 调整大小[¶](https://docs.daos.io/v2.0/admin/pool_operations/#resize)

目前不支持静态池调整大小（在不添加新节点的情况下更改每个存储节点上使用的容量），并且正在考虑中。

## 池灾难性恢复[¶](https://docs.daos.io/v2.0/admin/pool_operations/#pool-catastrophic-recovery)

DAOS 池由 SSD 上的 PMDK 和 SPDK blob 管理的一组 pmemobj 文件在每个目标上实例化。用于验证和修复此持久数据的工具计划用于 DAOS v2.4，一旦可用，将在此处记录。

同时，PMDK 提供了一个恢复工具（即 pmempool check）来验证和可能修复 pmemobj 文件。如上一节所述，重建状态可以通过池查询来查询，并将扩展更多信息。

## 恢复容器所有权[¶](https://docs.daos.io/v2.0/admin/pool_operations/#recovering-container-ownership)

通常，用户需要管理他们的容器。但是，如果容器是孤立的并且没有用户有权更改所有权，管理员可以将容器的所有权转移给新用户和/或组。

要更改所有者用户：

```bash
$ dmg cont set-owner --pool <UUID> --cont <UUID> --user <owner-user>
```

要更改所有者组：

```bash
$ dmg cont set-owner --pool <UUID> --cont <UUID> --group <owner-group>
```

用户名和组名区分大小写，并且必须格式化为 [DAOS ACL user/group principals](https://docs.daos.io/v2.0/overview/security/#principal)。

由于这是一项管理操作，因此不需要管理员在容器 ACL 中分配任何权限。