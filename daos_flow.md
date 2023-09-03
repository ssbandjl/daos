'参考: https://plantuml.com/zh/sequence-diagram'

@startuml

title DAOS用户态文件系统写流程

note right

dmg pool create sxb -z 4g; dmg pool list --verbose
daos container create sxb --type POSIX sxb; daos container query sxb sxb --verbose; daos cont get-prop sxb sxb
mkdir -p /tmp/sxb; dfuse --mountpoint=/tmp/sxb --pool=sxb --cont=sxb; df -h
cd /tmp/sxb
for i in {0..5};do
  echo "$i, `date`"
  dd if=/dev/zero of=$i bs=1M count=100 oflag=direct
  sleep 3
done

end note


APP -> FS: write(写10字节数据到/tmp/sxb/file)

FS -> libfuse3: write_buf

dfuse -> dfuse: dfuse_do_work 循环处理 \n fuse_session_process_buf_int \n se->op.write_buf \n dfuse_cb_write \n \
fuse_buf_copy(&ibuf, bufv, 0) 拷贝用户数据


dfuse -> dfs: dfs_write(oh->doh_dfs, oh->doh_obj, &ev->de_sgl, position, &ev->de_ev) \n\
daos_array_write(obj->oh, DAOS_TX_NONE, &iod, sgl, ev) \n\
dc_task_create(dc_array_write, NULL, ev, &task) 创建任务 \n\
dc_task_schedule(task, true) 调度客户端写数组任务

dfs -> array: dc_array_write 写数组(IO)
array -> array: dc_array_io 写数组(IO)
array -> array: compute_dkey 计算分布式key\n\
create_sgl 创建分散聚集列表 \n\
daos_task_create DAOS_OPC_OBJ_UPDATE 创建对象更新任务 \n\
tse_task_register_deps(task, 1, &io_task) 注册依赖任务 \n\
tse_task_list_sched(&io_task_list, false) 批量任务调度执行
note left: 在给定此范围的数组索引的情况下计算 dkey

array -> obj: dc_obj_update_task 执行对象更新任务

obj -> obj: obj_req_valid 校验请求\n\
tse_task_stack_push 任务压栈 \n\
dc_io_epoch_set 设置epoch \n\
tse_task_stack_pop 任务出栈 \n\

obj -> obj: dc_obj_update 提交对象更新 \n\
obj_task_init(task, DAOS_OBJ_RPC_UPDATE 初始化对象更新任务  \n\
obj_shards_2_fwtgts 根据分片查找转发的目标 \n\
obj_rw_bulk_prep 准备读写用的大块内存 \n\
obj_req_fanout 扇出对象写请求(准备读写和执行对象分片读写)


obj -> obj: dc_obj_shard_rw 对象分片读写\n操作码: DAOS_OBJ_RPC_UPDATE
note right: 设置目标(组,TAG,RANK)

obj -> daos_engine: crt_req_send(rpc, daos_rpc_cb, task)\n发送对象分片读写RPC给engine

daos_engine -> srv_obj: ds_obj_rw_handler 服务端对象读写处理器
srv_obj -> srv_obj: obj_ioc_begin 访问VOS前的检查 \n\
process_epoch 处理纪元\n\
dtx_leader_begin \n\
dtx_leader_exec_ops obj_tgt_update \n\
在所有目标上执行对象更新操作 \n\
obj_local_rw 执行一次本地对象读写 \n\
vos_update_begin 准备更新VOS \n\
bio_iod_prep 准备块IO(IO描述) \n\
bio_iod_post 提交块IO(IO描述) \n\
dma_rw 内存直接访问 \n\
nvme_rw 执行nvme读写

srv_obj -> spdk: spdk_blob_io_write 通过SPDK接口写blob

spdk -> nvme_disk: spdk_bdev_write_blocks SPDK写NVME(落盘)
nvme_disk -> spdk: callback
spdk -> srv_obj: rw_completion 读写回调 \n\
iod_dma_completion -> 完成DMA \n\
biod->bd_completion -> wal_completion | data_completion


srv_obj -> srv_obj: obj_rw_complete 完成对象读写请求 \n\
对象读写完成, 更新延迟计数器, 发送回复, 释放资源等

srv_obj -> obj: obj_rw_reply 发送回复

obj -> obj: dc_rw_cb 读写回调,释放资源, 逐层往上回调

obj -> dfuse: 回复
dfuse -> app: 回复





@enduml
