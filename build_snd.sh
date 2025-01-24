# scons --help
export PATH=$PATH:/usr/local/go/bin;export GO_BIN=/usr/local/go/bin;scons --jobs $(nproc) --config=force --build-deps=yes install

export daospath=/root/project/stor/daos
export CPATH=${daospath}/install/include/:$CPATH
export PATH=${daospath}/install/bin/:${daospath}/install/sbin:$PATH

