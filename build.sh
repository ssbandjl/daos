# scons --jobs 1 install  PREFIX=/opt/daos TARGET_TYPE=debug
# rm -rf /root/project/stor/daos/main/daos/build/external/debug/argobots
scons --jobs 32 install PREFIX=/opt/daos BUILD_TYPE=debug TARGET_TYPE=debug --build-deps=yes
# scons --jobs 1 install PREFIX=/opt/daos BUILD_TYPE=debug TARGET_TYPE=debug --build-deps=yes --implicit-deps-changed --config=force
# scons --jobs 1 install PREFIX=/opt/daos BUILD_TYPE=debug TARGET_TYPE=debug --build-deps=no --implicit-deps-changed --config=force