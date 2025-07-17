# source /root/.bashrc, not work
scons --clean
rm -rf build/debug/gcc/src
scons --jobs $(nproc) --config=force --build-deps=yes BUILD_TYPE=debug install
