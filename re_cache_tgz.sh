# ➜  main pwd
# /root/project/stor/daos/main
# ➜  main tree -L 1
# .
# ├── cache
# ├── cache_tgz
# └── daos

# 2 directories, 1 file
# ➜  main tree -L 2
# .
# ├── cache
# │   ├── argobots
# │   ├── libfabric
# │   └── ucx
# ├── cache_tgz
# └── daos
#     ├── build_docker.sh
#     ├── build.sh
#     ├── cache_tgz
# ...


rm -f cache_tgz
cd ../ && tar -zcvf cache_tgz cache
cp cache_tgz daos/

