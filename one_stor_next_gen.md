'参考: https://plantuml.com/zh/sequence-diagram'

@startuml

client -> librados: rados_connect
librados -> librados: create_cct
librados -> librados: crt_context_create(&daos_eq_ctx)
librados -> librados: tse_sched_init(&daos_sched_g, NULL, daos_eq_ctx) 初始化全局调度器
librados -> librados: daos_eq_create(daos_handle_t *eqh, nolock, poll)
librados -> librados: monclient.init()
librados -> librados: monclient.sub_want("mgrmap", 0, 0)
librados -> librados: monclient.renew_subs()


client -> librados: OP(如:rados_aio_write)
librados -> librados: 准备OP
librados -> librados: tse_sched_init(&sched, NULL, 0)
librados -> librados: daos_event_init(&ev->de_ev, ev->de_eqt->de_eq, NULL)
librados -> librados: 拷贝数据: copy_user_buf_to_sgl(user_buf, sgl.iov_buf)
librados -> librados: dc_task_create(op_submit, NULL, ev, &task)\n  tse_task_create(func, sched, NULL, &task) 通过ev上的调度器将ev和task关联起来\n  tse_task_register_comp_cb(task, task_comp_event, NULL, 0) 注册任务完成回调(在其中完成EV)

librados -> librados: args = dc_task_get_args(task) \n\
args = args->iod	= iod  \n\
args->sgl	= sgl \n\
args->op = op
note right: 将OP封装到task参数中

 


librados -> objecter: object_write(*task)

objecter -> objecter: _calc_target(&op->target, nullptr)  \n  \
enginemap->get_bucket_pool(t->base_oloc.pool) \n  \
enginemap->object_locator_to_bucket(t->base_oid, t->base_oloc, bucketid) 计算对象的bucket  \n  \
crush_buffer_map_dse.find(bucketid.m_pool) 先查引擎缓存   \n  \
bucket_to_raw_engines(cct, bucketid, &engineid) 未命中,计算引擎ID



objecter -> objecter: !engineid.valid() 引擎ID无效,加入homeless

alt 大IO
objecter -> objecter: big_io 大IO分片(>1MB)
objecter -> objecter: tse_task_register_deps(task, n, &io_split_task)
else 小IO
end

objecter -> rpc: hrpc_create_req
objecter -> objecter: tse_task_schedule(task, instant) 立即调度任务 \n\t\
op_submit(*task) 提交OP

objecter -> rpc: hrpc_send/bulk_transfer hrpc_cb 发送RPC/BULK
rpc -> objecter: hrpc_reply_send 接收端底层处理完成后, 发送回复给发送端(cr_output)


client -> librados: daos_eq_poll(eq, 1, wait_time, num, ev_arr)
librados -> librados: check_homess_head_timeout

alt 轮训
librados -> librados: num = hrpc_trigger(ctx, timeout, eq_progress_cb, &ev_args) \n\t\
eq_progress_cb(void *arg) \n\t\
tse_sched_progress(&epa->eqx->eqx_sched)


else 中断
end




@enduml
