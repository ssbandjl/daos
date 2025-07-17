# https://docs.daos.io/latest/QSG/build_from_scratch/?h=install#install-prerequisites
# submodule: git submodule update --init --recursive

# utils/scripts/install-ubuntu.sh
# apt install scons -y
# sudo apt install patchelf
# apt install curl -y
# apt install libboost-python-dev
# sudo apt install uuid-dev
# apt install meson ninja-build -y
# sudo pip3 install pyelftools

# download go: go1.24.4.linux-amd64.tar.gz
# rm -rf /usr/local/go && tar -C /usr/local -xzf go1.24.4.linux-amd64.tar.gz
# export PATH=$PATH:/usr/local/go/bin

# scons --help
# export PATH=$PATH:/usr/local/go/bin;export GO_BIN=/usr/local/go/bin

#release
scons --jobs $(nproc) --config=force --build-deps=yes install

cd src/control/
go clean -modcache -cache
./run_go_tests.sh
# link go
sudo ln -s /usr/local/go/bin/go /usr/bin/go
export GO111MODULE=on
#debug
scons --jobs $(nproc) --config=force --build-deps=yes BUILD_TYPE=debug install
or
export GO111MODULE=on;scons --jobs $(nproc) --config=force --build-deps=yes BUILD_TYPE=debug install


setup env to bashrc:
export daospath=/root/project/stor/daos
export CPATH=${daospath}/install/include/:$CPATH
export PATH=${daospath}/install/bin/:${daospath}/install/sbin:$PATH



build ofi:
with fabtests
cd /root/project/stor/daos/build/external/debug/ofi/fabtests
./autogen.sh
./configure --with-libfabric=/root/project/stor/daos/install/prereq/debug/ofi --prefix=/root/project/stor/daos/build/external/debug/ofi/build/fabtests && make -j 32 && sudo make install
cd /root/project/stor/daos/build/external/debug/ofi/build/fabtests
ll

https://docs.daos.io/v2.6/admin/predeployment_check/
set linux max mmap regions limit:
cp utils/rpms/10-daos_server.conf /etc/sysctl.d/10-daos_server.conf
/usr/lib/systemd/systemd-sysctl /etc/sysctl.d/10-daos_server.conf

optimize nvme ssd block size:
export daos_src=/root/project/stor/daos
# $daos_src/install/share/spdk/scripts/setup.sh
$daos_src/install/share/daos/control/setup_spdk.sh

spdk_nvme_manage
root@hpc117:~/project/stor/daos# ls -alh install/prereq/debug/spdk/bin
total 796K
drwxr-xr-x 2 root root 4.0K Jan 24 15:06 .
drwxr-xr-x 6 root root 4.0K Jan 24 15:06 ..
-rwxr-xr-x 1 root root  59K Jan 24 15:06 spdk_nvme_discovery_aer
-rwxr-xr-x 1 root root 264K Jan 24 15:06 spdk_nvme_identify
-rwxr-xr-x 1 root root  30K Jan 24 15:06 spdk_nvme_lsvmd
-rwxr-xr-x 1 root root 179K Jan 24 15:06 spdk_nvme_manage
-rwxr-xr-x 1 root root 249K Jan 24 15:06 spdk_nvme_perf

scan network:
install/bin/daos_server network scan





