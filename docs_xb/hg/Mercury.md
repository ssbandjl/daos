# Mercury

[![构建状态](https://github.com/mercury-hpc/mercury/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/mercury-hpc/mercury/actions/workflows/ci.yml) [![最新版本](https://camo.githubusercontent.com/75530950e23377defce47c1ff3ffde4b7eede318b37a75d7cd856bde0f7caebe/68747470733a2f2f696d672e736869656c64732e696f2f6769746875622f72656c656173652f6d6572637572792d6870632f6d6572637572792f616c6c2e737667)](https://github.com/mercury-hpc/mercury/releases) [![Spack 版本](https://camo.githubusercontent.com/deee2036a70d06aeab2d2949494ce206fb8f9673c4e93c0d084e7e59433f178e/68747470733a2f2f696d672e736869656c64732e696f2f737061636b2f762f6d6572637572792e737667)](https://spack.readthedocs.io/en/latest/package_list.html#mercury)

Mercury 是一个专门设计用于 HPC 系统的 RPC 框架，它允许异步传输参数和执行请求，以及直接支持大数据参数。网络实现是抽象的，允许轻松移植到未来的系统并有效地使用现有的本地传输机制。Mercury 的接口是通用的，允许对任何函数调用进行序列化。Mercury 是[Mochi](https://github.com/mochi-hpc/)微服务生态系统的核心组件。

请参阅随附的 LICENSE.txt 文件以获取许可证详细信息。

欢迎贡献和补丁，但需要填写贡献者许可协议 (CLA)。如果您有兴趣通过订阅 [邮件列表](http://mercury-hpc.github.io/help#mailing-lists)为 Mercury 做出贡献，请联系我们。

# 支持的架构

MPI 实现支持的架构通常由网络抽象层支持。

OFI libfabric 插件和 SM 插件是稳定的，在大多数工作负载中提供最佳性能。当前支持的 Libfabric 提供程序有：`tcp`, `verbs`, `gni`, `cxi`.

UCX 插件也可用作 libfabric 不可用或不建议使用的平台上的替代传输，当前支持的协议是 tcp 和动词。

MPI 和 BMI (tcp) 插件仍然受支持，但逐渐被弃用，因此只能用作后备方法。CCI 插件已弃用且不再受支持。

有关插件要求的详细信息，请参阅[插件要求](https://github.com/mercury-hpc/mercury#plugin-requirements)部分。

# 文档

有关Mercury 的快速介绍，请参阅水星[网站上提供的文档。](http://mercury-hpc.github.io/documentation/)

# 软件要求

编译和运行 Mercury 需要各种软件包的最新版本。请注意，使用这些软件包的过旧版本可能会导致难以追踪的间接错误。

## 插件要求

要使用 OFI libfabric 插件，请参阅此[页面](https://github.com/ofiwg/libfabric)上提供的 libfabric 构建说明。

要使用 UCX 插件，请参阅此[页面](https://openucx.readthedocs.io/en/master/running.html#ucx-build-and-install)上提供的 UCX 构建说明。

要在 Linux 上使用本机 NA SM（共享内存）插件，需要内核 v3.2 中引入的跨内存附加（CMA）功能。yama 安全模块还必须配置为允许访问远程进程内存（请参阅此[页面](https://www.kernel.org/doc/Documentation/security/Yama.txt)）。在 MacOS 上，当前需要在二进制文件中包含 na_sm.plist 文件的代码签名以允许访问进程内存。

要使用 BMI 插件，最方便的方法是通过 spack 安装它，也可以这样做：

```
git clone https://github.com/radix-io/bmi.git && cd bmi
./prepare && ./configure --enable-shared --enable-bmi-only
make && make install
```

为了使用 MPI 插件，Mercury 需要一个*配置良好的*MPI 实现（MPICH2 v1.4.1 或更高版本/OpenMPI v1.6 或更高版本）， `MPI_THREAD_MULTIPLE`并在接受远程连接的目标上可用。*不*接受传入连接的 进程*不需要*具有多线程执行级别。

## 可选要求

对于可选的自动代码生成功能（用于生成序列化和反序列化例程），必须包含 BOOST 库的预处理器子集（建议使用 Boost v1.48 或更高版本）。因此，库本身不是必需的，因为只使用了标头。如果没有安装 BOOST 并且想要使用此功能，Mercury 会包含这些标头。

# 编译

如果您安装完整的源代码，请将 tarball 放在您有权限的目录中（例如，您的主目录）并解压缩：

```
bzip2 -dc mercury-X.tar.bz2 | tar xvf -
```

替换`'X'`为包的版本号。

（可选）如果您使用 git（不带选项）签出源代码`--recursive` 并想要构建测试套件（需要 kwsys 子模块）或使用校验和（需要 mchecksum 子模块），则需要从根目录发出源目录下面的命令：

```
git submodule update --init
```

Mercury 使用 CMake 构建系统并要求您进行源外构建。为此，您必须创建一个新的构建目录并`ccmake`从中运行命令：

```
cd mercury-X
mkdir build
cd build
ccmake .. (where ".." is the relative path to the mercury-X directory)
```

`'c'`多次键入并选择合适的选项。推荐的选项是：

```
BUILD_SHARED_LIBS                ON (or OFF if the library you link
                                 against requires static libraries) 如果您链接的库需要静态库选OFF
BUILD_TESTING                    ON/OFF
Boost_INCLUDE_DIR                /path/to/include/directory
CMAKE_INSTALL_PREFIX             /path/to/install/directory
MERCURY_ENABLE_DEBUG             ON/OFF
MERCURY_TESTING_ENABLE_PARALLEL  ON/OFF
MERCURY_USE_BOOST_PP             ON
MERCURY_USE_CHECKSUMS            ON/OFF
MERCURY_USE_SYSTEM_BOOST         ON/OFF
MERCURY_USE_SYSTEM_MCHECKSUM     ON/OFF
MERCURY_USE_XDR                  OFF
NA_USE_BMI                       ON/OFF
NA_USE_MPI                       ON/OFF
NA_USE_OFI                       ON/OFF
NA_USE_PSM                       ON/OFF
NA_USE_PSM2                      ON/OFF
NA_USE_SM                        ON/OFF
NA_USE_UCX                       ON/OFF
BUILD_EXAMPLES									 OFF/ON
```

设置包含目录和库路径可能需要您通过键入切换到高级模式`'t'`。完成并且没有看到任何错误后，键入`'g'`以生成 makefile。退出 CMake 配置屏幕并准备好构建目标后，请执行以下操作：

```
make
```

（可选）详细编译/构建输出：

这是通过插入命令来完成`VERBOSE=1`的。`make`例如：

```
make VERBOSE=1
```

# 安装

假设`CMAKE_INSTALL_PREFIX`已设置（参见上一步）并且您对目标目录具有写入权限，请从构建目录执行：

```
 make install
```

# 测试

可以运行测试来检查基本的 RPC 功能（请求和批量数据传输）是否正常工作。CTest 用于运行测试，只需从构建目录运行：

```
ctest .
```

（可选）详细测试：

这是通过插入命令来完成`-V`的。`ctest`例如：

```
ctest -V .
```

可以通过插入来显示额外的详细信息`-VV`。例如：

```
ctest -VV .
```

一些测试使用一个服务器进程和 X 客户端进程运行。要更改正在使用的客户端进程的数量，`MPIEXEC_MAX_NUMPROCS` 需要修改变量（如果看不到，请切换到高级模式）。默认值由 CMake 根据可用的内核数自动检测。请注意，您需要`make`在生成生成文件后再次运行才能使用新值。

# 常问问题

以下是最常见问题的列表。

- *问：为什么我会得到对 libfabric 符号的未定义引用？*

  答：在极少数情况下，libfabric 库的多个副本会安装在同一系统上。要确保您使用的是 libfabric 库的正确副本，请执行以下操作：

  ```
  ldconfig -p | grep libfabric
  ```

  如果返回的库不是您所期望的，请确保在您的 目录中设置`LD_LIBRARY_PATH`或添加一个条目。`/etc/ld.so.conf.d`

- *问：是否有任何日志记录机制？*

  A: 要打开错误/警告/调试日志，`HG_LOG_LEVEL`环境变量可以设置为`error`,`warning`或`debug`值。请注意，要打印调试输出，`MERCURY_ENABLE_DEBUG` 还必须在编译时设置 CMake 变量。可以使用`HG_LOG_SUBSYS`环境变量选择特定的子系统。