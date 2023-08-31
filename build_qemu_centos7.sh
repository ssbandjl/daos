# cp -r cache/* build/external/debug; rm -rf /opt/daos/
# sudo yum install centos-release-scl
# sudo yum install devtoolset-8-gcc*
# scl enable devtoolset-8 bash; scl enable devtoolset-9 bash
# yum install devtoolset-8-gcc*; scl enable devtoolset-7 bash
# cd /opt/rh/xxx; ./enable
# scons-3 --warn=no-all --jobs 1 install PREFIX=/opt/daos BUILD_TYPE=debug TARGET_TYPE=debug --build-deps=yes
# env.Append(CCFLAGS=['-g', '-Wextra', '-Wshadow', '-Wall', '-fpic', '-Wno-maybe-uninitialized'])
# scl enable devtoolset-9 bash
scons-3 --jobs 1 install PREFIX=/opt/daos BUILD_TYPE=debug TARGET_TYPE=debug --build-deps=yes
