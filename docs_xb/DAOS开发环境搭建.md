# 开发环境[¶](https://docs.daos.io/v2.0/dev/development/#development-environment)

本节介绍了创建对开发人员友好的环境以促进 DAOS 开发的具体说明。这包括如何重新生成 protobuf 文件或添加新的 Go 包依赖项，这仅用于开发目的。

## DAOS开发环境构建 [¶](https://docs.daos.io/v2.0/dev/development/#building-daos-for-development)

DAOS 存储库托管在[GitHub 上](https://github.com/daos-stack/daos)。要检查当前的开发版本，只需运行：

```bash
$ git clone --recurse-submodules https://github.com/daos-stack/daos.git
ubix:
git clone --recurse-submodules https://github.com/daos-stack/daos.git -b v2.0.1
```

对于特定的分支或标签（例如 v1.2），添加`-b v1.2`到上面的命令行。

运行以下`scons`命令, 参数--build-deps, 使构建时的依赖包, 安装在 PREFIX/prereq/$TARGET_TYPE 下的组件特定目录中。

```bash
export daos_prefix_path=xxx
$ scons PREFIX=${daos_prefix_path}
      install
      --build-deps=yes
      --config=force
默认:
scons install --build-deps=yes --config=force -j8
```

将组件安装到单独的目录中允许通过替换为单独升级`--build-deps=yes`组件 `--update-prereq={component\_name}`。这需要对以前的环境配置进行更改。对于自动化环境设置， source `utils/sl/setup_local.sh`。

安装路径应该是可重定位的，但`daos_admin` 无法找到 daos 和依赖项的新位置的例外情况。由于相对路径，所有其他库和二进制文件都应该可以正常工作。编辑`.build-vars.sh`文件以用新文件替换旧文件可以恢复 setup_local.sh 自动设置路径的能力。

要运行 daos_server，要么需要将 daos_admin 中的 rpath 修补到新的安装位置，`spdk`要么`isal`需要`LD_LIBRARY_PATH`设置。这可以在采购时使用`SL_SPDK_PREFIX`和设置来完成。这也可以通过以下命令来完成：`SL_ISAL_PREFIX``setup_local.sh`

```
source utils/sl/setup_local.sh
sudo -E utils/setup_daos_admin.sh [path to new location of daos]
```

此脚本仅适用于`daos_admin`.

通过这种方法，DAOS 使用 中的预构建依赖项构建 `${daos_prefix_path}/prereq`，并保存所需的选项以供将来编译。因此，在第一次之后，在开发过程中，只有“ `scons --config=force`”和“ `scons --config=force install`”就足以编译对 DAOS 源代码的更改。

如果您希望使用 clang 而不是gcc编译 DAOS ，请在 scons 命令行上进行设置。`COMPILER=clang`此选项也会保存以供将来编译。

此外，用户可以指定`BUILD_TYPE=[dev|release|debug]`并且 scons 将保存各种`BUILD_TYPE`、`COMPILER`和`TARGET_TYPE` 选项的中间构建，因此用户可以在选项之间切换而无需完全重新构建，因此成本最低。默认情况下，`TARGET_TYPE`设置为`'default'`这意味着它使用该`BUILD_TYPE`设置。`BUILD_TYPE`为避免为每个设置重建先决条件 ，`TARGET_TYPE`可以显式设置`BUILD_TYPE`为始终使用该组先决条件的设置。这些设置存储在 daos.conf 中，因此无需在后续构建中设置这些值。

如果需要，`ALT_PREFIX`可以设置为以冒号分隔的前缀路径，以在其中查找已构建的组件。如果设置，构建将在继续构建之前检查这些路径中的组件。

### 自定义构建目标[¶](https://docs.daos.io/v2.0/dev/development/#custom-build-targets)

DAOS 构建还支持构建目标以自定义构建 DAOS 的哪些部分。目前，只定义了三个这样的目标`client`，`server`、 和 `test`。

要仅构建客户端库和工具，请使用以下命令：

```bash
$ scons [args] client install
```

`server`要改为构建服务器，请`client`在上述命令中替换。

请注意，每次构建时都需要指定此类目标，因为默认值相当于`client server test`在命令行上指定。目前 `test`，目标是依赖于`client`并且`server`也是。

### 堆栈分析器[¶](https://docs.daos.io/v2.0/dev/development/#stack-analyzer)

使用 gcc 编译器时，DAOS 构建包含一个为每个函数生成堆栈使用报告的工具。它报告 DAOS 中每个函数添加的堆栈帧的大小（以字节为单位）。

使用该`--analyze-stack="[arg] ..."`选项启用报告。

要获取此选项的使用信息，请运行

```bash
$ scons COMPILER=gcc --analyze-stack="-h"
```

该工具通常在构建后运行，但`-e`可以添加选项以立即运行并退出，如下例所示：

```bash
$ scons COMPILER=gcc --analyze-stack="-e -c 1024 -x tests" -Q
```

仅当使用 gcc 执行了先前的构建时，才应使用此选项。scons`-Q`选项减少了编译器设置的混乱。

此外，该工具支持按目录和文件名过滤并指定要报告的下限值的选项。

### 构建可选组件[¶](https://docs.daos.io/v2.0/dev/development/#building-optional-components)

有一些可选组件可以包含在 DAOS 构建中。例如，包括`psm2`提供者。运行以下`scons` 命令：

```bash
$ scons PREFIX=${daos_prefix_path}
      INCLUDE=psm2
      install
      --build-deps=yes
      --config=force
```

请参阅内置帮助命令以获取可选参数`scons`下所有可选组件的完整列表。`INCLUDE`

```bash
$ scons -h
scons: Reading SConscript files ...

INCLUDE: Optional components to build
    (all|none|comma-separated list of names)
    allowed names: psm2 psm3
    default: none
    actual:
```

可以通过编辑 [utils/build.config](https://github.com/daos-stack/daos/blob/release/2.0/utils/build.config) 文件来更改组件的版本。

> ***笔记\***
>
> 不保证对可选组件的支持，可以在不另行通知的情况下删除。

## go依赖[¶](https://docs.daos.io/v2.0/dev/development/#go-dependencies)

贡献 Go 代码的开发人员可能需要更改`src/control/vendor`目录中的外部依赖项。DAOS 代码库使用 [Go Modules](https://github.com/golang/go/wiki/Modules)来管理这些依赖项。由于此功能从 1.11 版开始内置于 Go 发行版中，因此无需其他工具来管理依赖项。

除了其他好处之外，使用 Go Modules 的主要优点之一是它消除了在`$GOPATH`.

虽然可以在不将供应商目录检查到 SCM 的情况下使用 Go Modules，但 DAOS 项目继续使用供应商依赖项vendored，以使我们的构建系统免受瞬态网络问题和与非供应商构建相关的其他问题的影响。

以下是示例工作流的简短列表。有关更多详细信息，请参阅在线[提供的](https://engineering.kablamo.com.au/posts/2018/just-tell-me-how-to-use-go-modules/)众多 [资源](https://blog.golang.org/migrating-to-go-modules)之一[。](https://github.com/golang/go/wiki/Modules#quick-start)

```bash
# add a new dependency
$ cd ~/daos/src/control # or wherever your daos clone lives
$ go get github.com/awesome/thing
# make sure that github.com/awesome/thing is imported somewhere in the codebase
$ ./run_go_tests.sh
# note that go.mod and go.sum have been updated automatically
#
# when ready to commit and push for review:
$ go mod vendor
$ git commit -a # should pick up go.mod, go.sum, vendor/*, etc.
# update an existing dependency
$ cd ~/daos/src/control # or wherever your daos clone lives
$ go get -u github.com/awesome/thing
# make sure that github.com/awesome/thing is imported somewhere in the codebase
$ ./run_go_tests.sh
# note that go.mod and go.sum have been updated automatically
#
# when ready to commit and push for review:
$ go mod vendor
$ git commit -a # should pick up go.mod, go.sum, vendor/*, etc.
# replace/remove an existing dependency
$ cd ~/daos/src/control # or wherever your daos clone lives
$ go get github.com/other/thing
# make sure that github.com/other/thing is imported somewhere in the codebase,
# and that github.com/awesome/thing is no longer imported
$ ./run_go_tests.sh
# note that go.mod and go.sum have been updated automatically
#
# when ready to commit and push for review:
$ go mod tidy
$ go mod vendor
$ git commit -a # should pick up go.mod, go.sum, vendor/*, etc.
```

在所有情况下，更新供应商目录后，最好验证您的更改是否按预期应用。为了做到这一点，一个简单的工作流程是清除缓存以强制进行干净的构建，然后运行测试脚本，该脚本是供应商vendor感知的，不会尝试下载缺失的模块：

```bash
$ cd ~/daos/src/control # or wherever your daos clone lives
$ go clean -modcache -cache
$ ./run_go_tests.sh
$ ls ~/go/pkg/mod # ~/go/pkg/mod should either not exist or be empty
```

## Protobuf 编译器[¶](https://docs.daos.io/v2.0/dev/development/#protobuf-compiler)

DAOS 控制平面基础设施使用[协议缓冲区](https://github.com/protocolbuffers/protobuf) 作为其 RPC 请求的数据序列化格式。并非所有开发人员都需要编译`\*.proto`文件，但如果需要更改 Protobuf，开发人员必须使用与 proto3 语法兼容的 Protobuf 编译器重新生成相应的 C 和 Go 源文件。

### 推荐版本[¶](https://docs.daos.io/v2.0/dev/development/#recommended-versions)

推荐的安装方法是克隆 git 存储库，查看下面提到的标记版本，然后从源代码安装。更高版本可能有效，但不能保证。从源代码构建时，您可能会遇到与权限不足有关的安装错误。如果发生这种情况，您可以尝试将 repo 重新定位到`/var/tmp/`以便从那里构建和安装。

- [协议缓冲区](https://github.com/protocolbuffers/protobuf)v3.11.4。[安装说明](https://github.com/protocolbuffers/protobuf/blob/master/src/README.md)。
- [Protobuf-C](https://github.com/protobuf-c/protobuf-c) v1.3.3。[安装说明](https://github.com/protobuf-c/protobuf-c/blob/master/README.md)。
- gRPC 插件： protoc [-gen-go](https://github.com/golang/protobuf)是[go.mod](https://github.com/daos-stack/daos/blob/master/src/control/go.mod)中指定的版本。该插件由 $DAOSREPO/src/proto 中的 Makefile 自动安装。

### 编译 Protobuf 文件[¶](https://docs.daos.io/v2.0/dev/development/#compiling-protobuf-files)

源 ( `.proto`) 文件位于`$DAOSREPO/src/proto`. 生成编译的 C/Go protobuf 定义的首选机制是使用此目录中的 Makefile。在添加或删除源文件或更新生成的文件目标时，应注意保持 Makefile 更新。

请注意，生成的文件会检入 SCM，并且不会作为正常 DAOS 构建过程的一部分生成。这允许开发人员确保在对源文件进行任何更改后生成的文件是正确的。

```bash
$ cd ~/daos/src/proto # or wherever your daos clone lives
$ make
protoc -I /home/foo/daos/src/proto/mgmt/ --go_out=plugins=grpc:/home/foo/daos/src/control/common/proto/mgmt/ acl.proto
protoc -I /home/foo/daos/src/proto/mgmt/ --go_out=plugins=grpc:/home/foo/daos/src/control/common/proto/mgmt/ mgmt.proto
...
$ git status
...
#       modified:   ../control/common/proto/mgmt/acl.pb.go
#       modified:   ../control/common/proto/mgmt/mgmt.pb.go
...
```

验证生成的 C/Go 文件是否正确后，像添加任何其他文件一样添加并提交它们。

## Docker 中的 DAOS 开发[¶](https://docs.daos.io/v2.0/dev/development/#daos-development-in-docker)

本节介绍如何在 Docker 容器中构建和运行 DAOS 服务。至少需要 5GB 的 DRAM 和 16GB 的磁盘空间。在 Mac 上，请确保“Preferences/{Disk, Memory}”下的 Docker 设置已相应配置。

### 构建 Docker 镜像[¶](https://docs.daos.io/v2.0/dev/development/#building-a-docker-image)

要直接从 GitHub 构建 Docker 映像，请运行以下命令：

```bash
$ docker build https://github.com/daos-stack/daos.git#master \
        -f utils/docker/Dockerfile.centos.7 -t daos
```

或从本地树：

```bash
$ docker build  . -f utils/docker/Dockerfile.centos.7 -t daos
```

这将创建一个 CentOS 7 映像，从 GitHub 获取最新的 DAOS 版本，构建它，并将其安装在映像中。对于 Ubuntu 和其他 Linux 发行版，将 Dockerfile.centos.7 替换为 Dockerfile.ubuntu.20.04 和感兴趣的适当版本。

### 简单的 Docker 设置[¶](https://docs.daos.io/v2.0/dev/development/#simple-docker-setup)

一旦创建了镜像，就可以启动一个最终运行 DAOS 服务的容器：

```bash
$ docker run -it -d --privileged --cap-add=ALL --name server -v /dev:/dev daos
```

笔记

如果您想对导出到容器的设备更有选择性，则应通过 -v 选项列出各个设备并将其导出为卷。在这种情况下，还应通过 -v /dev/hugepages:/dev/hugepages 和 -v /dev/hugepages-1G:/dev/hugepages-1G 将大页面设备添加到命令行

警告

如果 Docker 在非 Linux 系统（例如 OSX）上运行，应从命令行中删除 -v /dev:/dev。

`daos_server_local.yml`配置文件设置一个简单的本地 DAOS 系统，在容器中运行一个服务器实例。默认情况下，它使用 4GB 的 DRAM 来模拟持久内存和 /tmp 下的 16GB 大容量存储。如有必要，可以在 yaml 文件中更改存储大小。

可以在 docker 容器中启动 DAOS 服务，如下所示：

```bash
$ docker exec server daos_server start \
        -o /home/daos/daos/utils/config/examples/daos_server_local.yml
```

笔记

请确保 uio_pci_generic 模块已加载到主机上。

启动后，DAOS 服务器等待管理员格式化系统。这可以使用以下命令在不同的 shell 中触发：

```bash
$ docker exec server dmg -i storage format
```

成功完成格式化后，将启动存储引擎，并可以使用 daos 管理工具创建池（请参阅下一节）。

有关 SCM、SSD 或真实结构的更高级配置，请参阅下一节。