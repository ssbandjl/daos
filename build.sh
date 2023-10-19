# src/control/server/storage/scm.go
#// DefaultSysMemRsvd    = humanize.GiByte * 16  // per-system
#	DefaultSysMemRsvd    = humanize.GiByte * 1   // per-system 适配调试环境, 内存资源较少的情况

# scons --jobs 1 install  PREFIX=/opt/daos TARGET_TYPE=debug
# rm -rf /root/project/stor/daos/main/daos/build/external/debug/argobots
# 删除缓存: rm -rf build .sconf_temp .sconsign.dblite
# change DefaultSysMemRsvd to 1

#centos7:
# scl enable devtoolset-9 bash
echo "scl enable devtoolset-9 bash"
scons-3.6 --jobs 32 install PREFIX=/opt/daos BUILD_TYPE=debug TARGET_TYPE=debug --build-deps=yes   



# for qemu: scons-3 --jobs 1 install PREFIX=/opt/daos BUILD_TYPE=debug TARGET_TYPE=debug --build-deps=yes 
# scons --jobs 32 install PREFIX=/opt/daos BUILD_TYPE=debug TARGET_TYPE=debug --build-deps=yes   # for qemu: scons-3 --jobs 1 install PREFIX=/opt/daos BUILD_TYPE=debug TARGET_TYPE=debug --build-deps=yes 
# scons --jobs 1 install PREFIX=/opt/daos BUILD_TYPE=debug TARGET_TYPE=debug --build-deps=yes --implicit-deps-changed --config=force
# scons --jobs 1 install PREFIX=/opt/daos BUILD_TYPE=debug TARGET_TYPE=debug --build-deps=no --implicit-deps-changed --config=force

#gcc -Wno-coverage-mismatch, By default, this warning is enabled and is treated as an error. -Wno-coverage-mismatch can be used to disable the warning or -Wno-error=coverage-mismatch can be used to disable the error


