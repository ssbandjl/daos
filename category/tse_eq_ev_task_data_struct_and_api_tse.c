/* 事件队列 */
typedef struct daos_eq {
	/* After event is completed, it will be moved to the eq_comp list */
	d_list_t		eq_comp;
	int			eq_n_comp;

	/** Launched events will be added to the running list */
	d_list_t		eq_running;
	int			eq_n_running;

	int			use_lock;
	int			is_polling;
	int			eq_fd;

	struct {
		uint64_t	space[72];
	}			eq_private;

} daos_eq_t;


/* EQ私有数据 */
struct daos_eq_private {
	/* link chain in the global hash list */
	struct d_hlink		eqx_hlink;
	pthread_mutex_t		eqx_lock;
	unsigned int		eqx_lock_init:1,
				eqx_finalizing:1;

	/* CRT context associated with this eq */
	crt_context_t		eqx_ctx;

	/* Scheduler associated with this EQ */
	tse_sched_t		eqx_sched;
};





/* 任务类型 */
typedef struct tse_task {
	int			dt_result;
	/** padding bytes */
	int			dt_pad32;
	/* daos schedule internal */
	struct {
		char		dt_space[TSE_PRIV_SIZE];
	}			dt_private;
} tse_task_t;


/* 任务私有数据 */
struct tse_task_private {
	struct tse_sched_private	*dtp_sched;

	/* function for the task */
	tse_task_func_t			 dtp_func;

	/* links to user task list like tse_task_list_add/_del etc APIs */
	d_list_t			 dtp_task_list;

	/* links to scheduler */
	d_list_t			 dtp_list;

	/* time to start running this task */
	uint64_t			 dtp_wakeup_time;

	/* list of tasks that depend on this task */
	d_list_t			 dtp_dep_list;

	/* daos prepare task callback list */
	d_list_t			 dtp_prep_cb_list;

	/* daos complete task callback list */
	d_list_t			 dtp_comp_cb_list;

	/* task has been completed */
	ATOMIC uint8_t			dtp_completed;
	/* task is in running state */
	ATOMIC uint8_t			dtp_running;
	/* Don't propagate err-code from dependent tasks */
	uint8_t				dtp_no_propagate;
	uint8_t				dtp_pad;
	/* number of dependent tasks */
	uint16_t			 dtp_dep_cnt;
	/* refcount of the task */
	uint16_t			 dtp_refcnt;
	/**
	 * task parameter pointer, it can be assigned while creating task,
	 * or explicitly call API tse_task_priv_set. User can just use
	 * \a dtp_buf instead of this if parameter structure is enough to
	 * fit in.
	 */
	void				*dtp_priv;
	/**
	 * DAOS internal task parameter pointer.
	 */
	void				*dtp_priv_internal;
	/**
	 * reserved buffer for user to assign embedded parameters, it also can
	 * be used as task stack space that can push/pop parameters to
	 * facilitate I/O handling. The embedded parameter uses buffer from the
	 * bottom, and the stack space grows down from top.
	 *
	 * The sum of dtp_stack_top and dtp_embed_top should not exceed
	 * TSE_TASK_ARG_LEN.
	 */
	uint16_t			 dtp_stack_top;
	uint16_t			 dtp_embed_top;
	/* generation of the task, +1 every time when task re-init or add dependent task */
	ATOMIC uint32_t			 dtp_generation;
	char				 dtp_buf[TSE_TASK_ARG_LEN];
};


/* 调度器类型 */
typedef struct {
	int		ds_result;

	/* user data associated with the scheduler (completion cb data, etc.) */
	void		*ds_udata;

	/* daos schedule internal */
	struct {
		uint64_t	ds_space[48];
	}			ds_private;
} tse_sched_t;



/* 调度器私有数据 */
struct tse_sched_private {
	/* lock to protect schedule status and sub task list */
	pthread_mutex_t dsp_lock;

	/* The task will be added to init list when it is initially
	 * added to scheduler without any delay. A task with a delay
	 * will be added to dsp_sleeping_list.
	 */
	d_list_t	dsp_init_list;

	/* The task will be moved to complete list after the
	 * complete callback is being executed
	 **/
	d_list_t	dsp_complete_list;

	/**
	 * The task running list.
	 **/
	d_list_t	dsp_running_list;

	/* list of sleeping tasks sorted by dtp_wakeup_time */
	d_list_t	dsp_sleeping_list;

	/* the list for complete callback */
	d_list_t	dsp_comp_cb_list;

	/* 目标引擎异常, task暂存到homeless队列 */
	d_list_t	dsp_homeless_list;

	int		dsp_refcount;

	/* number of tasks being executed */
	int		dsp_inflight;

	uint32_t	dsp_cancelling:1,
			dsp_completing:1;
};


/* 调度器完成回调 */ 
typedef int (*tse_sched_comp_cb_t)(void *args, int rc);

/* 任务主体函数 */ 
typedef int (*tse_task_func_t)(tse_task_t *);

/* 任务回调 */ 
typedef int (*tse_task_cb_t)(tse_task_t *, void *arg);


///////////////////////////////////////////////////////////
 
/* 创建EQ, 创建网络上下文, 初始化调度器(初始/运行/完成/休眠/完成回调等队列), 设置调度器引用计数和飞行计数, 注册调度器回调, 为调度器绑定私有数据(如网络上下文), 注册完成回调 -> 完成回调等队列  */
int daos_eq_create(daos_eq_t *eq_out, int use_lock, int is_polling, hrpc_ctx_t ctx) 
{
  struct daos_eq_private *eqx;
  struct daos_eq         *eq;
  int    rc = 0;

  D_ALLOC_PTR(eq);
  D_INIT_LIST_HEAD(&eq->eq_running);
	D_INIT_LIST_HEAD(&eq->eq_comp);
	eq->eq_n_running = 0;
	eq->eq_n_comp = 0;

  eq->use_lock = use_lock;
  eq->is_polling = is_polling;

  eqx = daos_eq2eqx(eq);
  rc = D_MUTEX_INIT(&eqx->eqx_lock, NULL);
  eqx->eqx_lock_init = 1;

	// 创建网络上下文
  rc = crt_context_create(&eqx->eqx_ctx);
  eq->eq_fd = hrpc_mod_fd_get(&eqx->eqx_ctx);
  /* 为EQ初始化调度器 */
  rc = tse_sched_init(&eqx->eqx_sched, NULL, eqx->eqx_ctx);
  *eq_out = eq;
}



/* 初始化调度器 (初始/运行/完成/休眠/完成回调等队列), 设置调度器引用计数和飞行计数, 注册调度器回调, 为调度器绑定私有数据(如网络上下文) */
int tse_sched_init(tse_sched_t *sched, tse_sched_comp_cb_t comp_cb, void *udata)
{
	struct tse_sched_private	*dsp = tse_sched2priv(sched);
	int				 rc;


	D_INIT_LIST_HEAD(&dsp->dsp_init_list);
	D_INIT_LIST_HEAD(&dsp->dsp_running_list);
	D_INIT_LIST_HEAD(&dsp->dsp_complete_list);
	D_INIT_LIST_HEAD(&dsp->dsp_sleeping_list);
	D_INIT_LIST_HEAD(&dsp->dsp_comp_cb_list);

	D_INIT_LIST_HEAD(&dsp->dsp_homeless_list);


	if (comp_cb != NULL) {
		rc = tse_sched_register_comp_cb(sched, comp_cb, udata);
		if (rc != 0)
			return rc;
	}

	sched->ds_udata = udata;
	sched->ds_result = 0;
}

/* 事件EV */ 
typedef struct daos_event {
	/** return code of non-blocking operation */
	int			ev_error;
	/** Internal use - 152 + 8 bytes pad for pthread_mutex_t size difference on __aarch64__ */
	struct {
		uint64_t	space[20];
	}			ev_private;
	/** Used for debugging */
	uint64_t		ev_debug;
} daos_event_t;

/* 事件私有数据 */ 
struct daos_event_private {
	daos_handle_t		evx_eqh;
	d_list_t		evx_link;
	/** children list */
	d_list_t		evx_child;
	unsigned int		evx_nchild;
	unsigned int		evx_nchild_running;
	unsigned int		evx_nchild_comp;
	/** flag to indicate whether event is a barrier event */
	unsigned int		is_barrier:1;
	/** flag to indicate whether to convert DER to errno */
	unsigned int		is_errno:1;

	unsigned int		evx_flags;
	ATOMIC daos_ev_status_t	evx_status;

	struct daos_event_private *evx_parent;

	crt_context_t		evx_ctx;
	struct daos_event_callback evx_callback;

	tse_sched_t		*evx_sched;
	/** Lock for events that are not in an EQ, including the thread private event */
	pthread_mutex_t		evx_lock;
};


// 事件完成回调
typedef int (*daos_event_comp_cb_t)(void *, daos_event_t *, int);

// 事件初始化
int daos_event_init(struct daos_event *ev, daos_eq_t eq, struct daos_event *parent)
{
  atomic_init(&evx->evx_status, DAOS_EVS_READY);
  evx->evx_ctx = daos_eq_ctx; // 全局网络上下文或eqx上的网络
	evx->evx_sched = &daos_sched_g; // 全局调度器或eqx上的调度器
}

// 为事件注册完成回调
int daos_event_register_comp_cb(struct daos_event *ev, daos_event_comp_cb_t cb, void *arg)
{
	ecl->op_comp_cb = cb;
  d_list_add_tail(&ecl->op_comp_list, &evx->evx_callback.evx_comp_list);
}


// 启动事件
int daos_event_launch(struct daos_event *ev)
{
  // 启动事件, 已就绪事件才能启动, 检查子事件个数, 如果运行中+已完成的子事件个数小于子事件总数, 则返回, 即所有子事件启动后才能启动父事件; 如果事件在终止中,则退出, 
  evx->evx_status = DAOS_EVS_RUNNING;

  // 将事件加入EQ运行队列, EQ运行计数+1
  d_list_add_tail(&evx->evx_link, &eq->eq_running);
  eq->eq_n_running++;
}

// 完成事件, 任务完成时一般会调用EV完成
void daos_event_complete(struct daos_event *ev, int rc)
{
  // 遍历事件回调上的完成队列并执行回调, 原子标记事件为已完成, 如有父事件, 更新父事件上子事件的运行和完成计数, 修改父事件状态, 完成被屏障拦住的父事件, 将自己设置为父事件等, 将事件从EQ运行队列移动到EQ的完成队列, EQ完成计数+1,同时EQ运行计数-1
  // 封装函数: daos_event_complete_cb
  d_list_for_each_entry_safe(ecl, tmp, &evx->evx_callback.evx_comp_list, op_comp_list) {
    d_list_del_init(&ecl->op_comp_list);
    err = ecl->op_comp_cb(ecl->op_comp_arg, daos_evx2ev(evx), rc);
    D_FREE(ecl);
  }
}

// 销毁EQ(或强制,及不检查EQ运行和完成队列为空),运行和完成队列不为空且非强制模式, 则返回设备繁忙错误(DER_BUSY), 将EQ标记为终止中防止其他线程启动事件, 下刷EQ上网络上下文中的RPC, 终止所有启动的EV(运行队列上的EV), 如果该EQ上的网络不是全局的网络上下文, 则将该网络上下文销毁,即保留全局的那个网络上下文, 将EQ的网络上下文指针置空
int daos_eq_destroy(daos_eq_t eq, int flags);



// 轮询EQ, 返回事件个数和事件列表, 内部区分中断和轮训模式
int daos_eq_poll(daos_eq_t eq, int wait_running, int64_t timeout, unsigned int n_events, struct daos_event **events)


// 终止事件, 从各种列表、parent_list、子列表和事件队列哈希列表中取消事件链接，并销毁所有子事件, EV不能是运行中, 销毁EV锁, 处理子EV, 处理父EV, 删除EV的链表, EV上的网络上下文置空, EQ引用-1
int daos_event_fini(struct daos_event *ev) // 终止事件, 从各种列表、parent_list、子列表和事件队列哈希列


// 创建任务, 依次传入: 任务函数, 调度器, 私有参数(一般是NULL), 任务二级指针(待返回的任务), 主要逻辑: 拿到调度器私有数据, 新建一个任务, 初始化该任务的链表, 任务队列, 依赖队列, 完成回调队列, 任务前置回调函数队列, 任务引用+1, 设置主体函数, 私有参数和调度器, 返回该任务
int tse_task_create(tse_task_func_t task_func, tse_sched_t *sched, void *priv, tse_task_t **taskp)
// 参考用法: tse_task_create(check_func_n, &sched, counter, &task)


// 为任务注册完成回调, 参考用法: tse_task_register_comp_cb(task, task_comp_event, NULL, 0)
int tse_task_register_comp_cb(tse_task_t *task, tse_task_cb_t comp_cb, void *arg, daos_size_t arg_size)



// 调度任务, 参数: 任务, 立即调度, 主流程: 从task拿到调度器, 如果该任务没有依赖任务且任务前置工作也做完了, 则该任务已就绪, 则将调度器飞行计数+1, 设置调度器为运行中, 设置唤醒时间为0, 将任务加入调度器的运行队列, 可立即执行主体任务(忽略任务返回值)
int tse_task_schedule(tse_task_t *task, bool instant)
// 参考用法: tse_task_schedule(task, true)



// 获取调度器进展(进度): 如果调度器是取消中, 则直接返回, 否则, 运行调度器 -> tse_sched_run(sched)
void tse_sched_progress(tse_sched_t *sched)
// 参考用法: tse_sched_progress(&sched)


// 在调度器上运行一轮任务(依次执行初始化调度器进展, 处理调度器完成, 检查完成): 完成调度器或没有任务处理时才返回 
static void tse_sched_run(tse_sched_t *sched)
// 初始化调度器进展: 将休眠中唤醒的任务加入初始队列, 遍历初始队列, 将没有依赖的主任务, 或者调度器处于取消状态时, 将该任务加入临时队列, 遍历临时队列, 如果调度器被取消, 则直接完成该任务(修改计数器, 标记任务完成, 加入调度器完成队列), 否则加入运行队列, 接着如果执行任务主体函数, 返回处理的个数
static int tse_sched_process_init(struct tse_sched_private *dsp)

// 处理调度器完成: 将任务从调度器完成队列移除, 设置任务调度器结果, 检查任务依赖, 修改计数器
tse_sched_process_complete(dsp)

// 检查调度器是否完成(所有任务执行完成, 初始和休眠队列为空,且无飞行中的任务)
bool tse_sched_check_complete(tse_sched_t *sched)


// 完成任务: 传入任务和返回值, 执行任务回调 -> tse_task_complete_callback(task), 完成任务, 更新调度器列表, 将该任务从调度器的完成列表移除
tse_task_complete(task, rc)

// 执行任务回调: 遍历任务上的完成回调列表，如果所有CB都执行完毕并且不重新初始化任务，则返回true。 如果任务被用户重新初始化，则意味着它再次处于运行状态，因此我们在重新初始化它的当前 CB 处中断，并返回 false，这意味着任务尚未完成。 所有剩余未执行的 CB 仍保持附加状态，但已执行的 CB 此时已从列表中删除,
static bool tse_task_complete_callback(tse_task_t *task)




#include<jemalloc.h> /* 普通内存 */
#include<zmem.h> /* 零拷贝 */


/* 事件队列, 可参考libfabric: struct util_eq,  */
typedef struct daos_eq {
	/* After event is completed, it will be moved to the eq_comp list */
	d_list_t		eq_comp;
	int			eq_n_comp;

	/** Launched events will be added to the running list */
	d_list_t		eq_running;
	int			eq_n_running;

	int			use_lock;
	int			is_polling;
	int			eq_fd;

	struct {
		uint64_t	space[72];
	}			eq_private;

} daos_eq_t;


/* EQ私有数据 */
struct daos_eq_private {
	/* link chain in the global hash list */
	struct d_hlink		eqx_hlink;
	pthread_mutex_t		eqx_lock;
	unsigned int		eqx_lock_init:1,
				eqx_finalizing:1;

	hrpc_ctx_t		eqx_ctx;

	/* Scheduler associated with this EQ */
	tse_sched_t		eqx_sched;

	int		use_public_ctx;
};


/* 事件 */
typedef struct daos_event {
	/** return code of non-blocking operation */
	int			ev_error;
	/** Internal use - 152 + 8 bytes pad for pthread_mutex_t size difference on __aarch64__ */
	struct {
		uint64_t	space[20];
	}			ev_private;
	/** Used for debugging */
	uint64_t		ev_debug;
} daos_event_t;



/* 任务类型 */
typedef struct tse_task {
	int			dt_result;
	/** padding bytes */
	int			dt_pad32;
	/* daos schedule internal */
	struct {
		char		dt_space[TSE_PRIV_SIZE];
	}			dt_private;
} tse_task_t;


/* 任务私有数据 */
struct tse_task_private {
	struct tse_sched_private	*dtp_sched;

	/* function for the task */
	tse_task_func_t			 dtp_func;

	/* links to user task list like tse_task_list_add/_del etc APIs */
	d_list_t			 dtp_task_list;

	/* links to scheduler */
	d_list_t			 dtp_list;

	/* time to start running this task */
	uint64_t			 dtp_wakeup_time;

	/* list of tasks that depend on this task */
	d_list_t			 dtp_dep_list;

	/* daos prepare task callback list */
	d_list_t			 dtp_prep_cb_list;

	/* daos complete task callback list */
	d_list_t			 dtp_comp_cb_list;

	/* task has been completed */
	ATOMIC uint8_t			dtp_completed;
	/* task is in running state */
	ATOMIC uint8_t			dtp_running;
	/* Don't propagate err-code from dependent tasks */
	uint8_t				dtp_no_propagate;
	uint8_t				dtp_pad;
	/* number of dependent tasks */
	uint16_t			 dtp_dep_cnt;
	/* refcount of the task */
	uint16_t			 dtp_refcnt;
	/**
	 * task parameter pointer, it can be assigned while creating task,
	 * or explicitly call API tse_task_priv_set. User can just use
	 * \a dtp_buf instead of this if parameter structure is enough to
	 * fit in.
	 */
	void				*dtp_priv;
	/**
	 * DAOS internal task parameter pointer.
	 */
	void				*dtp_priv_internal;
	/**
	 * reserved buffer for user to assign embedded parameters, it also can
	 * be used as task stack space that can push/pop parameters to
	 * facilitate I/O handling. The embedded parameter uses buffer from the
	 * bottom, and the stack space grows down from top.
	 *
	 * The sum of dtp_stack_top and dtp_embed_top should not exceed
	 * TSE_TASK_ARG_LEN.
	 */
	uint16_t			 dtp_stack_top;
	uint16_t			 dtp_embed_top;
	/* generation of the task, +1 every time when task re-init or add dependent task */
	ATOMIC uint32_t			 dtp_generation;
	char				 dtp_buf[TSE_TASK_ARG_LEN];
};


/* 调度器类型 */
typedef struct {
	int		ds_result;

	/* user data associated with the scheduler (completion cb data, etc.) */
	void		*ds_udata;

	/* daos schedule internal */
	struct {
		uint64_t	ds_space[48];
	}			ds_private;
} tse_sched_t;



/* 调度器私有数据 */
struct tse_sched_private {
	/* lock to protect schedule status and sub task list */
	pthread_mutex_t dsp_lock;

	/* The task will be added to init list when it is initially
	 * added to scheduler without any delay. A task with a delay
	 * will be added to dsp_sleeping_list.
	 */
	d_list_t	dsp_init_list;

	/* The task will be moved to complete list after the
	 * complete callback is being executed
	 **/
	d_list_t	dsp_complete_list;

	/**
	 * The task running list.
	 **/
	d_list_t	dsp_running_list;

	/* list of sleeping tasks sorted by dtp_wakeup_time */
	d_list_t	dsp_sleeping_list;

	/* the list for complete callback */
	d_list_t	dsp_comp_cb_list;

	/* 目标引擎异常, task暂存到homeless队列 */
	d_list_t	dsp_homeless_list;

	int		dsp_refcount;

	/* number of tasks being executed */
	int		dsp_inflight;

	uint32_t	dsp_cancelling:1,
			dsp_completing:1;
};


/**
 * 使用输入调度器 sched 创建一个新任务，并将该任务与输入事件 ev 关联。 如果输入调度程序为 NULL，则任务将附加到该事件对应EQ的内部调度器
 *
 * \param func [input]	 任务主体函数
 * \param sched [input]	 任务需要添加到该调度器的队列中
 * \param ev [input]		 将task与ev关联
 * \param taskp [output] 返回的任务, 由内部分配和初始化, 如果调用任务完成函数，任务将在内部释放
 *
 * \return			成功: 0, 失败: 负数错误码
 */
int dc_task_create(tse_task_func_t func, tse_sched_t *sched, daos_event_t *ev, tse_task_t **taskp)
{
	...
	rc = tse_task_create(func, sched, NULL, &task);
	args = task_ptr2args(task);
	args->ta_magic = DAOS_TASK_MAGIC;
	...
	tse_task_register_comp_cb(task, task_comp_event, NULL, 0)
	...
	*taskp = task;
}

/* 调度器完成回调 调度由 dc_task_create_ev() 创建的任务，如果关联事件
\a 任务是私有事件，该函数将等待完成
任务的结束，否则立即返回并完成
通过测试事件或轮询 EQ 发现。

如果 \a instant 为 true，则任务将立即执行。 */ 

/**
 * 调度一个任务, 
 *
 * \param func [input]	 任务主体函数
 * \param sched [input]	 任务需要添加到该调度器的队列中
 * \param ev [input]		 将task与ev关联
 * \param taskp [output] 返回的任务, 由内部分配和初始化, 如果调用任务完成函数，任务将在内部释放
 *
 * \return			成功: 0, 失败: 负数错误码
 */
int dc_task_schedule(tse_task_t *task, bool instant)


/* 调度器完成回调 */ 
typedef int (*tse_sched_comp_cb_t)(void *args, int rc);

/* 任务主体函数 */ 
typedef int (*tse_task_func_t)(tse_task_t *);

/* 任务回调 */ 
typedef int (*tse_task_cb_t)(tse_task_t *, void *arg);


///////////////////////////////////////////////////////////
 
/**
 * 创建EQ, 创建或使用网络上下文, 初始化调度器(初始/运行/完成/休眠/完成回调等队列), 设置调度器引用计数和飞行计数, 注册调度器回调, 为调度器绑定私有数据(如网络上下文), 注册完成回调放入完成回调队列
 * 
 * \param[out] eq_out	  					返回EQ
 * \param[in] use_lock						开启有锁模式
 * \param[in] is_polling					开启轮询模式
 * \param[in] ctx							    共享网络上下文, NULL表示需要创建, 一个共享网络上下文对应一个EQ(1:1)
 * \param[in] mod_ctx							业务网络上下文, NULL表示需要创建, 一个业务网络上下文对应一个EQ(1:1)
 *
 * \return		0: 成功, 负数错误码: 失败
 */
int daos_eq_create(void **eq_out, int use_lock, int is_polling, hrpc_ctx_t ctx, hrpc_mod_t mod_ctx) 
{
  struct daos_eq_private *eqx;
  struct daos_eq         *eq;
  int    rc = 0;

  D_ALLOC_PTR(eq);
  D_INIT_LIST_HEAD(&eq->eq_running);
	D_INIT_LIST_HEAD(&eq->eq_comp);
	eq->eq_n_running = 0;
	eq->eq_n_comp = 0;

  eq->use_lock = use_lock;
  eq->is_polling = is_polling;

  eqx = daos_eq2eqx(eq);
  rc = D_MUTEX_INIT(&eqx->eqx_lock, NULL);
  eqx->eqx_lock_init = 1;

	eqx->use_public_ctx = 1; // 记录业务ctx, 默认使用公用业务ctx, 共享ctx从何获取?
	if(ctx && mod_ctx) {
		eqx->eqx_ctx = ctx;
		eqx->eqx_mod_ctx = mod_ctx;
		eqx->use_public_ctx = 0;
	} else {
    // monc -> monitor得到nid(待确认-刘)
    nid = monclient.monc_get_nid();

    hrpc_init_param_t param = {
      .na_type = HRPC_NA_RDMA,
      .ip_port_list1 = "192.168.1.100", // 从配置中获取本地IP
      .core_id = 0x01,
      .core_nums = 1,
      ...
    }
		// 创建网络上下文
    hrpc_ctx_t ctx = NULL;
		rc = hrpc_context_create(nid, param, ctx);

    hrpc_git_t gid;
    hrpc_mod_t mod_ctx;
    hrpc_mod_register(ctx, &gid, &mod_ctx);
    eqx->eqx_ctx = ctx; // 记录共享ctx
    eqx->eqx_mod_ctx = mod_ctx; // 记录业务ctx
    eqx->eqx_gid = gid;
	}
	
	/* 设置网络工作模式 is_polling? */
	hrpc_mod_work_set(ctx/* hrpc_mod_ctx_t */, poling);
  // eq->eq_fd = hrpc_mod_fd_get(&eqx->eqx_ctx);
  /* 为EQ初始化调度器 */
  rc = tse_sched_init(&eqx->eqx_sched, NULL, eqx->eqx_ctx);
  *eq_out = (void*)eq;
}

/**
 * 为调度器注册回调
 *
 * \param sched [input]		调度器
 * \param comp_cb [input]	调度器完成回调
 * \param arg [input]		调度器完成回调参数
 *
 * \return			成功: 0, 失败: 负数错误码
 */
int tse_sched_register_comp_cb(tse_sched_t *sched, tse_sched_comp_cb_t comp_cb, void *arg);


/**
 * 终止调度器
 *
 * \param sched [input]		待终止的调度器
 */
void tse_sched_fini(tse_sched_t *sched);


/**
 * 等待调度程序中的所有任务完成并终止调度器。 如果另一个线程正在完成调度程序，则会立即返回
 *
 * \param sched	[input]	待终止的调度器
 * \param ret	[input]	调度器完成后的结果
 * \param cancel [input] 是否取消调度器中所有的任务
 */
void tse_sched_complete(tse_sched_t *sched, int ret, bool cancel);



/**
 *  初始化调度器 (初始/运行/完成/休眠/完成回调等队列), 设置调度器引用计数和飞行计数, 注册调度器回调, 为调度器绑定私有数据(如网络上下文),
 *  使用可选的完成回调和指向用户数据的指针初始化调度程序。 调用者负责完成或取消调度程序
 *
 * \param sched [input]		待初始化的调度器
 * \param comp_cb [input]	调度器完成时调用的回调(可选)
 * \param udata [input]		指向与调度程序关联的用户数据的可选指针。 它存储在调度程序结构中的 ds_udata 中，并在调用回调时作为参数传递给 comp_cb
 *
 * \return			成功: 0, 失败: 负数错误码
 */
int tse_sched_init(tse_sched_t *sched, tse_sched_comp_cb_t comp_cb, void *udata)
{
	struct tse_sched_private	*dsp = tse_sched2priv(sched);
	int				 rc;


	D_INIT_LIST_HEAD(&dsp->dsp_init_list);
	D_INIT_LIST_HEAD(&dsp->dsp_running_list);
	D_INIT_LIST_HEAD(&dsp->dsp_complete_list);
	D_INIT_LIST_HEAD(&dsp->dsp_sleeping_list);
	D_INIT_LIST_HEAD(&dsp->dsp_comp_cb_list);

	D_INIT_LIST_HEAD(&dsp->dsp_homeless_list);


	if (comp_cb != NULL) {
		rc = tse_sched_register_comp_cb(sched, comp_cb, udata);
		if (rc != 0)
			return rc;
	}

	sched->ds_udata = udata;
	sched->ds_result = 0;
}

/* 事件EV */ 
typedef struct daos_event {
	/** return code of non-blocking operation */
	int			ev_error;
	/** Internal use - 152 + 8 bytes pad for pthread_mutex_t size difference on __aarch64__ */
	struct {
		uint64_t	space[20];
	}			ev_private;
	/** Used for debugging */
	uint64_t		ev_debug;
} daos_event_t;

/* 事件私有数据 */ 
struct daos_event_private {
	daos_handle_t		evx_eqh;
	d_list_t		evx_link;
	/** children list */
	d_list_t		evx_child;
	unsigned int		evx_nchild;
	unsigned int		evx_nchild_running;
	unsigned int		evx_nchild_comp;
	/** flag to indicate whether event is a barrier event */
	unsigned int		is_barrier:1;
	/** flag to indicate whether to convert DER to errno */
	unsigned int		is_errno:1;

	unsigned int		evx_flags;
	ATOMIC daos_ev_status_t	evx_status;

	struct daos_event_private *evx_parent;

	crt_context_t		evx_ctx;
	struct daos_event_callback evx_callback;

	tse_sched_t		*evx_sched;
	/** Lock for events that are not in an EQ, including the thread private event */
	pthread_mutex_t		evx_lock;
};


// 事件完成回调
typedef int (*daos_event_comp_cb_t)(void *, daos_event_t *, int);


/* EV状态 */
typedef enum {
	DAOS_EVS_READY,
	DAOS_EVS_RUNNING,
	DAOS_EVS_COMPLETED,
	DAOS_EVS_ABORTED,
} daos_ev_status_t;


/**
 * 为 EQ 初始化一个新事件, 将EV置零, 设置状态为:已准备(DAOS_EVS_READY), 初始化子事件链表头, 自己的链表头, 事件回调结构上的事件完成回调链表头, 如果传入父事件, 则检查父事件状态(必须为已准备), 检查父事件的父事件(父事件不能有父事件,及多层嵌套), 将该事件插入到该父事件的子链表上, 继承父事件的EQ, 网络上下文, 调度器, 为父事件上的子事件计数+1, 如果没有传入父事件, 则继承传入EQ的网络上下文和调度器, 如果EQ无效, 则为该事件绑定全局网络上下文和全局调度器
 *
 * \param[in] ev	待初始化的事件
 * \param[in] eq	该事件排队需要的事件队列, 如果指定了父级事件，则该事件将被忽略
 * \param[in] parent	父事件，如果没有父事件，则可以为 NULL, 如果它不为 NULL，则调用者将永远不会看到此事件的完成，相反，只有当父级的所有子级完成时才会看到父级的完成, 然而，与父事件相关联的操作可以在其子事件之前启动或完成, 父事件完成只是将多个事件完成状态合并为一个的简单方法
 *
 * \return		成功: 0, 错误: 负数错误码
 */
int daos_event_init(struct daos_event *ev, daos_eq_t *eq, struct daos_event *parent)
{
	...
  atomic_init(&evx->evx_status, DAOS_EVS_READY);
	daos_eq_private *eqx = (daos_eq_private *)(eq->eq_private.space);
 	evx->evx_ctx = eqx->eqx_mod_ctx;     // 全局网络上下文或eqx上的网络
	evx->evx_sched = eqx->eqx_sched; // 全局调度器或eqx上的调度器
	...
}



/**
 * 为事件注册完成回调(用得少), 分配事件完成列表, 初始化该列表上的操作完成链表, 设置传入的参数和回调, 最后将该完成列表元素加入事件回调结构上的事件完成队列
 *
 * \param[in] ev	输入事件
 * \param[in] cb	完成回调
 * \param[in] arg	传给回调的用户参数
 *
 * \return		成功: 0, 错误: 负数错误码
 */
int daos_event_register_comp_cb(struct daos_event *ev, daos_event_comp_cb_t cb, void *arg)
{
	ecl->op_comp_cb = cb;
  d_list_add_tail(&ecl->op_comp_list, &evx->evx_callback.evx_comp_list);
}


/**
 * 启动事件, 将事件标记为运行中, 并且移动到事件队列的运行队列
 *
 * \param ev		[IN]	待启动的事件
 */
int daos_event_launch(struct daos_event *ev)
{
  // 启动事件, 已就绪事件才能启动, 检查子事件个数, 如果运行中+已完成的子事件个数小于子事件总数, 则返回, 即所有子事件启动后才能启动父事件; 如果事件在终止中,则退出, 
  evx->evx_status = DAOS_EVS_RUNNING;

  // 将事件加入EQ运行队列, EQ运行计数+1
  d_list_add_tail(&evx->evx_link, &eq->eq_running);
  eq->eq_n_running++;
}


/**
 * 完成事件, 将EV标记为完成状态, 并加入完成队列, 一般在任务完成时调用该函数
 *
 * \param ev [IN]	待完成的事件
 * \param rc [IN]	设置期望的操作返回码, 一般为0
 */
void daos_event_complete(struct daos_event *ev, int rc)
{
  // 遍历事件回调上的完成队列并执行回调, 原子标记事件为已完成, 如有父事件, 更新父事件上子事件的运行和完成计数, 修改父事件状态, 完成被屏障拦住的父事件, 将自己设置为父事件等, 将事件从EQ运行队列移动到EQ的完成队列, EQ完成计数+1,同时EQ运行计数-1
  // 封装函数: daos_event_complete_cb
  d_list_for_each_entry_safe(ecl, tmp, &evx->evx_callback.evx_comp_list, op_comp_list) {
    d_list_del_init(&ecl->op_comp_list);
    err = ecl->op_comp_cb(ecl->op_comp_arg, daos_evx2ev(evx), rc);
    D_FREE(ecl);
  }
}

/**
 * 销毁事件队列, 销毁EQ(或强制,及不检查EQ运行和完成队列为空),运行和完成队列不为空且非强制模式, 则返回设备繁忙错误(DER_BUSY), 将EQ标记为终止中防止其他线程启动事件, 下刷EQ上网络上下文中的RPC, 终止所有启动的EV(运行队列上的EV), 如果该EQ上的网络不是全局的网络上下文, 则将该网络上下文销毁,即保留全局的那个网络上下文, 将EQ的网络上下文指针置空
 *
 * \param[in] eq	待销毁的EQ
 * \param[in] flags	控制标志, 可强制销毁(DAOS_EQ_DESTROY_FORCE	1)
 *
 * \return		成功: 0, 失败: 负数错误码
 */
int daos_eq_destroy(daos_eq_t *eq, int flags);



// 轮询EQ, 返回事件个数和事件列表, 内部区分中断和轮询模式
/**
 * 从EQ中取回完成的事件
 *
 * \param[in] eqh	EQ handle
 * \param[in] wait_running EQ上有正在运行的事件时才等待(EQ运行队列中有任务)。 有些事件可能已初始化但未运行。 该参数决定仅等待正在运行的事件还是所有的事件
 * \param[in] timeout 等待多少毫秒(MS)
 *
 * \param[in] nevents	事件数组的大小, 返回的事件数小于或等于该数组大小
 * \param[out] events	事件数组
 *
 * \return		成功: 0, 失败: 负数错误码
 */
int daos_eq_poll(daos_eq_t eq, int wait_running, int64_t timeout, unsigned int n_events, struct daos_event **events)


/**
 * 终止事件, 从各种列表、parent_list、子列表和事件队列哈希列表中取消事件链接，并销毁所有子事件, EV不能是运行中, 销毁EV锁, 处理子EV, 处理父EV, 删除EV的链表, EV上的网络上下文置空, EQ引用-1
 * \param[in] ev	待终止的事件
 * \return		成功: 0, 失败: 负数错误码
 */ 
int daos_event_fini(struct daos_event *ev) // 终止事件, 从各种列表、parent_list、子列表和事件队列哈希列



 /**
	* 创建任务, 依次传入: 任务函数, 调度器, 私有参数(一般是NULL), 任务二级指针(待返回的任务), 主要逻辑: 拿到调度器私有数据, 新建一个任务, 初始化该任务的链表, 任务队列, 依赖队列, 完成回调队列, 任务前置回调函数队列, 任务引用+1, 设置主体函数, 私有参数和调度器, 返回该任务

 * \param task_func [input]	任务主体函数
 * \param sched [input]		该任务待附加的调度器
 * \param priv [input]		任务私有数据, 可通过tse_task_get_priv获取参数
 * \param taskp [output]	返回初始化后的任务
 *
 * \return		成功: 0, 失败: 负数错误码
 */
int tse_task_create(tse_task_func_t task_func, tse_sched_t *sched, void *priv, tse_task_t **taskp)
// 参考用法: tse_task_create(check_func_n, &sched, counter, &task)


/**
 * 为任务注册完成回调, 参考用法: tse_task_register_comp_cb(task, task_comp_event, NULL, 0)
 *
 * \param task [input]		待设置任务完成回调的任务
 * \param comp_cb [input]	任务回调
 * \param arg [input]		回调参数
 * \param top [input]		指示回调是否插入到回调堆栈的顶部（true）或底部（false）
 *
 * \return		成功: 0, 失败: 负数错误码
 */
int tse_task_register_comp_cb(tse_task_t *task, tse_task_cb_t comp_cb, void *arg, daos_size_t arg_size)



/**
 * 为任务注册依赖任务(支持多个), 在主任务被调度执行前, 需要完成的依赖任务
 *
 * \param task	[IN]	为哪个任务添加依赖
 * \param num_deps [IN]	依赖任务的个数
 * \param dep_tasks [IN] 依赖任务数组
 *
 * \return		成功: 0, 失败: 负数错误码
 */
int tse_task_register_deps(tse_task_t *task, int num_deps, tse_task_t *dep_tasks[]);


/**
 * 调度任务, 参数: 任务, 立即调度, 主流程: 从task拿到调度器, 如果该任务没有依赖任务且任务前置工作也做完了, 则该任务已就绪, 则将调度器飞行计数+1, 设置调度器为运行中, 设置唤醒时间为0, 将任务加入调度器的运行队列, 可立即执行主体任务(忽略任务返回值)
 * 将任务添加到初始化的调度程序中。 如果任务主体函数应作为该函数的一部分立即调用，则 instant 应设置为 true； 否则，如果错误任务将出现在调度程序初始化列表中，并在调度程序进行时进行
 *
 * \param task [input]		待调度的任务
 * \param instant [input]	控制标签, 立即调度或延迟调度
 *
 * \return		成功: 0, 失败: 负数错误码
 */
int tse_task_schedule(tse_task_t *task, bool instant)



/** 获取调度器进展(进度): 如果调度器是取消中, 则直接返回, 否则, 运行调度器 -> tse_sched_run(sched)
 * 
 * \param sched	[IN]	待获取进展的调度器
*/
void tse_sched_progress(tse_sched_t *sched)
// 参考用法: tse_sched_progress(&sched)


// 在调度器上运行一轮任务(依次执行初始化调度器进展, 处理调度器完成, 检查完成): 完成调度器或没有任务处理时才返回 
static void tse_sched_run(tse_sched_t *sched)
// 初始化调度器进展: 将休眠中唤醒的任务加入初始队列, 遍历初始队列, 将没有依赖的主任务, 或者调度器处于取消状态时, 将该任务加入临时队列, 遍历临时队列, 如果调度器被取消, 则直接完成该任务(修改计数器, 标记任务完成, 加入调度器完成队列), 否则加入运行队列, 接着如果执行任务主体函数, 返回处理的个数
static int tse_sched_process_init(struct tse_sched_private *dsp)

// 处理调度器完成: 将任务从调度器完成队列移除, 设置任务调度器结果, 检查任务依赖, 修改计数器
tse_sched_process_complete(dsp)


/**
 * 检查调度器是否完成(所有任务执行完成, 初始和休眠队列为空,且无飞行中的任务)
 *
 * \param sched	[IN]	待检查的调度器
 *
 * \return		已完成: true, 其他:false
 */
bool tse_sched_check_complete(tse_sched_t *sched)


// 完成任务: 传入任务和返回值, 执行任务回调 -> tse_task_complete_callback(task), 完成任务, 更新调度器列表, 将该任务从调度器的完成列表移除
tse_task_complete(task, rc) /* 一般被 crt_req_send(rpc, daos_rpc_cb, task) 调用 */


/**
 * 将任务标记为完成, 执行任务回调, 更新调度器列表
 *
 * \param task [input]	待完成的任务
 * \param ret [input]	  期望的返回值
 **/
void tse_task_complete(tse_task_t *task, int ret);

// 执行任务回调: 遍历任务上的完成回调列表，如果所有CB都执行完毕并且不重新初始化任务，则返回true。 如果任务被用户重新初始化，则意味着它再次处于运行状态，因此我们在重新初始化它的当前 CB 处中断，并返回 false，这意味着任务尚未完成。 所有剩余未执行的 CB 仍保持附加状态，但已执行的 CB 此时已从列表中删除,
static bool tse_task_complete_callback(tse_task_t *task)


/* 检查完成列表中的任务、依赖任务状态检查、计划状态更新等。此后任务将移至完成列表 */
// 用法: tse_task_post_process(task)
static int tse_task_post_process(tse_task_t *task)
{
	done = tse_task_complete_callback(task_tmp) // 执行任务回调 -> dtc->dtc_cb(task, dtc->dtc_arg)
}



/* 外层事件 */
struct external_event {
	daos_event_t                  ev;
	size_t                        de_len; /**< The size returned by daos */
	d_iov_t                       de_iov;
	d_sg_list_t                   de_sgl;
	d_list_t                      de_list;
	daos_eq_t                    *eq;
	struct dfuse_obj_hdl         *de_oh;
	void (*complete_cb)(struct external_event *ev);
} external_event_t;



/**
 * 获取EQ上的文件描述符, 用于中断模式
 *
 * \param eq [input]		输入EQ
 *
 * \return		返回EQ的fd
 */
int eq_get_fd(void *eq /* struct daos_eq */)
{
	struct daos_eq *eq_tmp = NULL;
	eq_tmp = (struct daos_eq*)eq;
	return eq_tmp.fd;
}



/** 返回 dc_task_create 创建的任务的嵌入参数 */
void * dc_task_get_args(tse_task_t *task);


#ifndef __DAOS_OBJ_INTENRAL_H__
#define __DAOS_OBJ_INTENRAL_H__
#include <daos_event.h>
#include <daos/tse.h>
/* tgt_main 写, example 参考: dfuse_cb_write */
int main ()
{

	/* tgt zmem_pool */
	rc = zmem_pool_init(socketid, zmem_conf);

	/* 分配sgl */
	d_sg_list_t	sgl;
	zmem_sgl_t *sgl = zmem_sgl_alloc(mid, size);

	/* 设置buf */
	d_iov_t	iov;
	d_iov_set(&iov, *io_buf, size);
	sgl.sg_nr = 1;
	sgl.sg_nr_out = 0;
	sgl.sg_iovs = &iov;


	rados_t rados = NULL;
	ret = rados_connect(rados);



  /* EV */
	external_event_t *external_ev; // 外层EV
	external_ev->eq = eq;
	rc = daos_event_init(&external_ev->ev, external_ev->eq, NULL/*parent_ev*/);
	
	external_ev->complete_cb = complete_cb; /* 业务回调 */


	/* 创建写task */
	tse_task_t	*task;
	dc_task_create(objecter_write, NULL, external_ev->ev, &task);
	args = dc_task_get_args(task);
	/* 准备OP参数(oid,pool_id等) */ 
	args->oid = oid;
	args->pool_id = pool_id;
	args->snapc = snapc;
	args->sgl.sg_nr       = 1;
	args->sgl	= sgl;
	args->target = NULL;
	args.dst_gid = NULL;

	// tse_task_schedule(task, instant) // 立即调度任务, 执行:objecter_write
	dc_task_schedule(task, true)
}



// 参考: class Objecter : public md_config_obs_t, public Dispatcher, crush 依赖 bucket_t，bucket_pool_t，object_t
typedef struct objecter {
  char      			        *obj_name;
  daos_eq_t			          *eq; // Messenger *messenger
  d_list_t		            homeless;
  monclient               *monclient;
  pthread_t               finisher_tid; // Finisher *finisher;
  std::unique_ptr<OSDMap> osdmap;
  using Dispatcher::cct;
  PerfCounters *logger = nullptr;

  // sclient
  struct engine_op_target {
    uint64_t flags = 0;
    epoch_t enginemap_epoch = 0;
    uint32_t pool;     // pool id
    object_t base_oid;
    engine_t engineid;
    bucket_t bucketid;
    d_list_t		op_target_link; // 可以挂到objecter的homeless队列
    ...
  } engine_op_target_t;

  // pclient
  struct osd_op_target{
    epoch_t epoch = 0;  ///< latest epoch we calculated the mapping
    epoch_t enginemap_epoch = 0;
    uint32_t pool;     ///< pool id
    pg_t pgid; ///< last (raw) pg we mapped to
    uint16_t up[]; ///< set of up osds for last pg we mapped to
    uint16_t acting[]; ///< set of acting osds for last pg we mapped to
    int up_primary = -1; ///< last up_primary we mapped to
    int acting_primary = -1;  ///< last acting_primary we mapped to
  } osd_op_target_t;

   /**
	 * 对象更新OSD_MAP
	 *
	 * @param osd_map 分发器分发已解码的osd_map
	 */
  void (*handle_osd_map) (osd_map_t *osd_map);
} objecter_t;



/**
 * 执行写任务
 *
 * \param task [input]		待执行的任务
 *
 * \return		成功: 0, 失败: 负数错误码
 */
int objecter_write(tse_task_t *task)
{
	args = dc_task_get_args(task);

	/* 拿到objecter */
	objecter = args->objecter;


	engine_op_target_t engine_op_target=NULL;
	engine_op_target.oid = args->oid;
	/* crush */
	_calc_target(&engine_op_target, nullptr);
	if(!engine_op_target.engineid.valid())
	{
		d_list_move_tail(&engine_op_target.op_target_link, &objecter->homeless)
		return 0;
	}

	/* 8k拆分, ec满条带, bulk 待定 */
	if (pool_is_ec(args.pool_id))
	{
		pool_rw_req_reassemb(args.sgl)
	}

	/* 发送RPC请求 */
  ctx = args->ev.evx.ctx;
	rpc = hrpc_alloc(ctx, gid) // 分配RPC请求
	req = hrpc_set(rpc, 1, &req, 10 /* timeout */) // 设置请求参数, req -> rpc
  rc = req_send(args, ctx, rpc, daos_rpc_cb);
	...
}


int req_send(args, ctx, rpc, daos_rpc_cb);
{
	int ret = hrpc_req_send(rpc, args.dst_gid, daos_rpc_cb, rpc_args); // 下游拿到rpc后, 如果rpc大于16K, 需要执行(hrpc_bulk_transfer)拉取数据
	/* 如果连接未建立,则建连 */
	if (ret == HRPC_NO_CONN) {
		monclient = monc_get();
		dst_url = monclient.monc_get_url(args.dst_gid); // monitor提供接口?
    rc = hrpc_connect(ctx, args.dst_gid, dst_url); // 需要检查是否建立连接
		int ret = hrpc_req_send(rpc, args.dst_gid, daos_rpc_cb, rpc);
		return 0;
	}
	int ret = hrpc_req_send(rpc, args.dst_gid, daos_rpc_cb, rpc); // 需要检查错误码
}




int _calc_target(&op->target, nullptr)
{
	get_bucket_pool(t->base_oloc.pool);
	object_locator_to_bucket(t->base_oid, t->base_oloc, bucketid); // 计算对象的bucket
	crush_buffer_map_dse.find(bucketid.m_pool) // 优先查缓存
	bucket_to_raw_engines(cct, bucketid, &engineid) // 未命中,计算引擎ID
}

/* 在rados_connect中开启finisher线程 */
finisher(void *objecter)
{
	daos_eq_t *eq = objecter->eq;
	/* 中断 */
	if(!eq->polling)
	{
		struct epoll_event events[10];
		struct epoll_event ev;
		fd = hrpc_mod_fd_get(mod_ctx) // 获取网络上下文fd
		nfds = epoll_wait(fd, events, num, -1) // 等待网络回复事件, 业务可通过eq_get_fd获取EQ上的fd
		...
	}

	rc = daos_eq_poll(eq, 1, wait_time, num, &ev_array) //
	for (i = 0; i < rc; i++)
	{
		external_ev = container_of(ev_array[i], struct daos_event, ev);
		external_ev->complete_cb(external_ev); 
		/* 1. 执行业务回调 */

		/* 2. mon -> monc map_update */
		decode(map);
		switch(map.type)
		case: OSD_MAP
			dispatcher(handle_osd_map, map)
			{
				objecter.handle_osd_map(map);
			}
	}
	
	
	
}


void handle_osd_map(osd_map_t *osd_map)
{
	if (m->get_last() <= osdmap->get_epoch());// 忽略旧map
	...
	d_list_for_each_entry_safe(objecter, engine_op_target, &objecter->homeless, op_target_link)
	{
		if (engine_op_target.engine_id) 
		{
			resend_rpc ...
		}
	}

}


daos_eq_poll(daos_eq *eq, 1, wait_time, num, ev_array)
{
	num = hrpc_mod_trigger(mod_ctx, count)
	req_recv(void *arg) // 网络调用事件完成回调 ev_progress_cb  
}



/**
 * RPC发送完成后的回调, 将任务完成, 触发事件完成
 *
 * \param cb_info [input]		回调参数
 *
 * \return		无
 */
static void daos_rpc_cb(const struct crt_cb_info *cb_info)
{
	tse_task_t	*task = cb_info->cci_arg;
	int		rc = cb_info->cci_rc;

	tse_task_complete(task, rc);
}


set_pmtime_cb(rpc){
	pmtime = rpc.pmtime
	arg.pmtime = pmtime;
}


extern "C" int _rados_stat(rados_ioctx_t io, const char *o, uint64_t *psize, time_t *pmtime){

 rados_completion_t completion;
 completion.cb = set_pmtime_cb
 completion.ev = ev

}


---------- rados_aio_write ----------
r = rados_aio_create_completion(fri, complete_callback, NULL, &fri->completion);
r = rados_aio_write(rados->io_ctx, object, fri->completion, io_u->xfer_buf, io_u->xfer_buflen, io_u->offset);
extern "C" int _rados_aio_write(rados_ioctx_t io, const char *o, rados_completion_t completion, const char *buf, size_t len, uint64_t off)
{
	// 如果业务没带EQ, 则使用公用EQ, io.rados_client.objecter.eq
	void        *eq = io.rados_client.objecter.eq;

	daos_event_t   *ev;
	rc = daos_event_init(ev, eq, NULL/*parent_ev*/);

	completion.ev = ev // rados_completion_t completion 对象内嵌了一个ev

	// 或通过 rados_completion_t completion 拿到回调和参数
	// external_ev->complete_cb = complete_cb;


	/* 创建写task */
	tse_task_t	*task;
	daos_aio_write_t	*args
	dc_task_create(objecter_write, NULL, completion.ev, &task);
	args->oid = oid;
	args->sgl	= sgl;
	args->pmtime	= pmtime;
	args->objecter = io.rados_client.objecter;
	...
	dc_task_schedule(task, true); // -> objecter_write
}

---------- rados_aio_write ----------



---------- rados_connect ----------
extern "C" int _rados_connect(rados_t cluster)
{
	librados::RadosClient *client = (librados::RadosClient *)cluster;
	int retval = client->connect();
}

int librados::RadosClient::connect()
{
	monclient = monc_create(cct); //或者获取 monc_get()

	/* map_sub */
	monclient.monc_set_want_keys(monclient, OSD | ENGINE) // 设置需要的KEY
	monclient.monc_sub_want(monclient, "osdmap", 0, 0, handle_osd_map) // 传总控回调更合适?
	monclient.monc_sub_want(monclient, "engine_map", 0, 0, handle_engine_map)
	monc_renew_subs(monclient) // 发起订阅


	// nid = monclient.get_nid();
	// hrpc_context_create(nid, param, ctx)
	// hrpc_mod_register(ctx, &gid, &mod_ctx)
	// hrpc_mod_work_mode_set(mod_ctx, poling) // 设置网络工作模式(轮询|中断), 默认轮询
	
	void        *eq = NULL;
  /* mod_ctx 可为NULL, 或传入 */
	rc = daos_eq_create(&eq, 1/* use_lock */, 1/*is_polling*/, NULL/* ctx */, NULL/* mod_ctx */);
	
	// 参考: objecter = new (std::nothrow) Objecter(cct, messenger, &monclient, &finisher)
	objecter_t 		*objecter = NULL;
	rc = objecter_init(&objecter, monclient, eq);
	pthread_t *finisher_tid;
	rc = pthread_create(finisher_tid, NULL, finisher, objecter /* args */);
	objecter->finisher_tid = finisher_tid;
	...
	rados_client->objecter = objecter;
	...
}

int objecter_init(objecter_t objecter, monclient_t monclient, daos_eq_t *eq)
{
	objecter->eq = eq;
	objecter->monclient = monclient;
	objecter.handle_osd_map = handle_osd_map;
}
---------- rados_aio_write ----------




#endif /* __DAOS_OBJ_INTENRAL_H__ */





TSE要点:
参数大小840字节+互斥锁

参数大小:
struct daos_task_args
任务参数 880
TSE_TASK_ARG_LEN
任务参数长度=840+互斥锁(40B) = 880

struct tse_task_private
任务私有参数 1016
TSE_PRIV_SIZE
私有大小=任务参数长度+136
1016

typedef struct tse_task
任务大小 1024
TSE_TASK_SIZE
任务长度=私有+8=1024


/**
 * 给任务携带小参数, 指定参数大小, 获取指定task嵌入的参数, 返回参数地址, 先对输入大小按8字节向上对齐, 计算可用大小(预留内存总大小-栈顶大小), 嵌入大小不能超过可用大小(超过限制返回NULL), 修改嵌入参数大小为指定的大小, 将当前缓冲区地址转为void指针并返回
 *
 *
 * \param task [in] 指定的任务
 * \param size [in] 需要携带参数的字节大小
 *
 * \return	成功返回参数指针, 失败返回NULL
 **/
void * tse_task_buf_embedded(tse_task_t *task, int size);

/**
 * 使用新的任务函数重置任务。 该任务必须已经完成或尚未开始，并且必须具有 > 0 的有效引用计数（未释放）。 这允许用户重用具有不同主体函数的任务，而不必为不同的操作重新创建任务
 *
 *
 * \param task [in] 指定的任务
 * \param task_func [in] 新任务函数
 * \param priv [in] 新任务函数参数
 *
 * \return	成功返回0, 失败返回负数错误码
 **/
int tse_task_reset(tse_task_t *task, tse_task_func_t task_func, void *priv);


/**
 * 将参数, 推入任务堆栈空间。 该API仅在任务堆栈上保留空间，不涉及数据拷贝, 获取可用大小, 输入大小按8字节向上对齐, 将参数大小累加到栈顶指针, 计算新的参数地址(固定地址+总大小-栈顶大小)并返回. 如: 在更新任务中将任务参数压栈 obj_req_valid(task, args, DAOS_OBJ_RPC_UPDATE ...
 *  
 * --------------- dtp_buf_addr 低地址(高地址-低地址是总大小)
 *
 * 
 * --------------- top_new, 返回这个位置的地址(下次从这个位置往下读size大小)
 * size, 这中间放置新参数
 * --------------- top_old
 * 
 * ---------------  高地址
 * 
 * \param task [in] 指定的任务
 * \param size [in] 需要推入参数的字节大小
 *
 * \return	成功返回参数指针, 失败返回NULL
 **/
void * tse_task_stack_push(tse_task_t *task, uint32_t size);


/**
 * 弹出任务参数, 对齐并检测输入的大小, 计算参数地址, 比如在 obj_comp_cb 中拿回携带的任务参数
 *
 * --------------- dtp_buf_addr 低地址(高地址-低地址是总大小)
 *
 * 
 * --------------- top_old, 返回这个位置的地址
 * size, 这中间放置新参数
 * --------------- top_new, top最终的大小在这个位置
 * 
 * --------------- 高地址
 * \param task [in] 指定的任务
 * \param size [in] 需要弹出参数的字节大小
 *
 * \return	成功返回参数指针, 失败返回NULL
 **/
void * tse_task_stack_pop(tse_task_t *task, uint32_t size);


修复: 不安排尚未准备好的即时任务



参数: src/include/daos/tse.h
/* tse_task arguments max length (pthread_mutex_t is of different size between x86 and aarch64). */
#define TSE_TASK_ARG_LEN	(840 + sizeof(pthread_mutex_t))
/* internal tse private data size (struct tse_task_private) */
#define TSE_PRIV_SIZE		(TSE_TASK_ARG_LEN + 136)
/* tse_task is used to track single asynchronous operation (8 bytes used for public members). */
#define TSE_TASK_SIZE		(TSE_PRIV_SIZE + 8)




[root@node215 build]# ./pahole -C tse_task_private  /opt/h3c/lib/libdaos.so.2
die__process_function: tag not supported (INVALID)!
struct tse_task_private {
        struct tse_sched_private * dtp_sched;            /*     0     8 */
        tse_task_func_t            dtp_func;             /*     8     8 */
        d_list_t                   dtp_task_list;        /*    16    16 */
        d_list_t                   dtp_list;             /*    32    16 */
        uint64_t                   dtp_wakeup_time;      /*    48     8 */
        uint64_t                   dtp_create_time;      /*    56     8 */
        /* --- cacheline 1 boundary (64 bytes) --- */
        d_list_t                   dtp_dep_list;         /*    64    16 */
        d_list_t                   dtp_prep_cb_list;     /*    80    16 */
        d_list_t                   dtp_comp_cb_list;     /*    96    16 */
        uint32_t                   dtp_completed:1;      /*   112:31  4 */
        uint32_t                   dtp_running:1;        /*   112:30  4 */
        uint32_t                   dtp_dep_cnt:29;       /*   112: 1  4 */

        /* XXX 1 bit hole, try to pack */

        uint32_t                   dtp_refcnt;           /*   116     4 */
        void *                     dtp_priv;             /*   120     8 */
        /* --- cacheline 2 boundary (128 bytes) --- */
        void *                     dtp_priv_internal;    /*   128     8 */
        uint16_t                   dtp_stack_top;        /*   136     2 */
        uint16_t                   dtp_embed_top;        /*   138     2 */
        uint32_t                   dtp_generation;       /*   140     4 */
        char                       dtp_buf[880];         /*   144   880 */
        /* --- cacheline 16 boundary (1024 bytes) --- */

        /* size: 1024, cachelines: 16, members: 19 */
        /* bit holes: 1, sum bit holes: 1 bits */
}; 




[root@node215 build]#  ./pahole -C daos_task_args  /opt/h3c/lib/libdaos.so.2
die__process_function: tag not supported (INVALID)!
struct daos_task_args {
	uint32_t                   ta_magic;             /*     0     4 */
	uint32_t                   ta_opc;               /*     4     4 */
	union {
		daos_set_params_t  mgmt_set_params;      /*          32 */
		daos_pool_replicas_t pool_add_replicas;  /*          40 */
		daos_pool_replicas_t pool_remove_replicas; /*          40 */
		daos_mgmt_get_bs_state_t mgmt_get_bs_state; /*          32 */
		daos_pool_connect_t pool_connect;        /*          56 */
		daos_pool_disconnect_t pool_disconnect;  /*           8 */
		daos_pool_update_t pool_update;          /*          48 */
		daos_pool_query_t  pool_query;           /*          32 */
		daos_pool_query_target_t pool_query_info; /*          24 */
		daos_pool_list_attr_t pool_list_attr;    /*          24 */
		daos_pool_get_attr_t pool_get_attr;      /*          40 */
		daos_pool_set_attr_t pool_set_attr;      /*          40 */
		daos_pool_stop_svc_t pool_stop_svc;      /*           8 */
		daos_pool_list_cont_t pool_list_cont;    /*          24 */
		daos_cont_create_t cont_create;          /*          40 */
		daos_cont_open_t   cont_open;            /*          56 */
		daos_cont_close_t  cont_close;           /*           8 */
		daos_cont_destroy_t cont_destroy;        /*          40 */
		daos_cont_query_t  cont_query;           /*          24 */
		daos_cont_aggregate_t cont_aggregate;    /*          16 */
		daos_cont_rollback_t cont_rollback;      /*          16 */
		daos_cont_subscribe_t cont_subscribe;    /*          16 */
		daos_cont_list_attr_t cont_list_attr;    /*          24 */
		daos_cont_get_attr_t cont_get_attr;      /*          40 */
		daos_cont_set_attr_t cont_set_attr;      /*          40 */
		daos_cont_alloc_oids_t cont_alloc_oids;  /*          24 */
		daos_cont_list_snap_t cont_list_snap;    /*          40 */
		daos_cont_create_snap_t cont_create_snap; /*          32 */
		daos_cont_destroy_snap_t cont_destroy_snap; /*          24 */
		daos_tx_open_t     tx_open;              /*          24 */
		daos_tx_commit_t   tx_commit;            /*          16 */
		daos_tx_abort_t    tx_abort;             /*           8 */
		daos_tx_close_t    tx_close;             /*           8 */
		daos_tx_restart_t  tx_restart;           /*           8 */
		struct daos_obj_register_class_t obj_reg_class; /*          24 */
		daos_obj_query_class_t obj_query_class;  /*          24 */
		daos_obj_list_class_t obj_list_class;    /*          24 */
		daos_obj_open_t    obj_open;             /*          40 */
		daos_obj_close_t   obj_close;            /*           8 */
		daos_obj_punch_t   obj_punch;            /*          48 */
		daos_obj_query_t   obj_query;            /*          32 */
		daos_obj_query_key_t obj_query_key;      /*          48 */
		struct daos_obj_sync_args obj_sync;      /*          32 */
		daos_obj_fetch_t   obj_fetch;            /*          96 */
		daos_obj_update_t  obj_update;           /*          96 */
		daos_obj_list_dkey_t obj_list_dkey;      /*         136 */
		daos_obj_list_akey_t obj_list_akey;      /*         136 */
		daos_obj_list_recx_t obj_list_recx;      /*         136 */
		daos_obj_list_obj_t obj_list_obj;        /*         136 */
		daos_array_create_t array_create;        /*          56 */
		daos_array_open_t  array_open;           /*          64 */
		daos_array_close_t array_close;          /*           8 */
		daos_array_destroy_t array_destroy;      /*          16 */
		daos_array_io_t    array_io;             /*          40 */
		daos_array_get_size_t array_get_size;    /*          24 */
		daos_array_set_size_t array_set_size;    /*          24 */
		block_io_t         block_io;             /*          48 */
		daos_kv_open_t     kv_open;              /*          40 */
		daos_kv_close_t    kv_close;             /*           8 */
		daos_kv_destroy_t  kv_destroy;           /*          16 */
		daos_kv_get_t      kv_get;               /*          48 */
		daos_kv_put_t      kv_put;               /*          48 */
		daos_kv_remove_t   kv_remove;            /*          32 */
		daos_kv_list_t     kv_list;              /*          48 */
	} ta_u;                                          /*     8   136 */
	/* --- cacheline 2 boundary (128 bytes) was 16 bytes ago --- */
	daos_event_t *             ta_ev;                /*   144     8 */

	/* size: 152, cachelines: 3, members: 4 */
	/* last cacheline: 24 bytes */
};

参数最大880



176B
typedef struct daos_event {
	/** return code of non-blocking operation 非阻塞操作设置的操作码 */
	int			ev_error;
	/** Internal use - 152 + 8 bytes pad for pthread_mutex_t size difference on __aarch64__ */
	struct {
		uint64_t	space[20];
	}			ev_private;
	/** Used for debugging */
	uint64_t		ev_debug;
} daos_event_t;


[root@node215 build]#  ./pahole -C daos_event  /opt/h3c/lib/libdaos.so.2
struct daos_event {
	int                        ev_error;             /*     0     4 */

	/* XXX 4 bytes hole, try to pack */

	struct {
		uint64_t           space[19];            /*     8   152 */
		/* --- cacheline 2 boundary (128 bytes) was 24 bytes ago --- */
	} ev_private;                                    /*     8   152 */
	/* --- cacheline 2 boundary (128 bytes) was 32 bytes ago --- */
	uint64_t                   ev_debug;             /*   160     8 */

	/* size: 168, cachelines: 3, members: 3 */
	/* sum members: 164, holes: 1, sum holes: 4 */
	/* last cacheline: 40 bytes */
};




struct daos_event_private {
	daos_handle_t              evx_eqh;              /*     0     8 */
	d_list_t                   evx_link;             /*     8    16 */
	d_list_t                   evx_child;            /*    24    16 */
	unsigned int               evx_nchild;           /*    40     4 */
	unsigned int               evx_nchild_running;   /*    44     4 */
	unsigned int               evx_nchild_comp;      /*    48     4 */
	unsigned int               is_barrier:1;         /*    52:31  4 */
	unsigned int               is_errno:1;           /*    52:30  4 */

	/* XXX 30 bits hole, try to pack */

	unsigned int               evx_flags;            /*    56     4 */
	daos_ev_status_t           evx_status;           /*    60     4 */
	/* --- cacheline 1 boundary (64 bytes) --- */
	struct daos_event_private * evx_parent;          /*    64     8 */
	crt_context_t              evx_ctx;              /*    72     8 */
	struct daos_event_callback evx_callback;         /*    80    24 */
	tse_sched_t *              evx_sched;            /*   104     8 */

	/* size: 112, cachelines: 2, members: 14 */
	/* bit holes: 1, sum bit holes: 30 bits */
	/* last cacheline: 48 bytes */
};

struct daos_eq {
	d_list_t                   eq_comp;              /*     0    16 */
	int                        eq_n_comp;            /*    16     4 */

	/* XXX 4 bytes hole, try to pack */

	d_list_t                   eq_running;           /*    24    16 */
	int                        eq_n_running;         /*    40     4 */

	/* XXX 4 bytes hole, try to pack */

	struct {
		uint64_t           space[72];            /*    48   576 */
		/* --- cacheline 9 boundary (576 bytes) --- */
	} eq_private;                                    /*    48   576 */
	/* --- cacheline 9 boundary (576 bytes) was 48 bytes ago --- */

	/* size: 624, cachelines: 10, members: 5 */
	/* sum members: 616, holes: 2, sum holes: 8 */
	/* last cacheline: 48 bytes */
};




tas# 任务调度引擎(TSE)

|   Short   | Long     |  CN    |
| ---- | ---- | ---- |
| tse      | task schedule engine     | 任务调度引擎,源自DAOS, 结合任务,调度器, 事件, 事件队列, 网络上下文等完成异步任务调度的一套通用API    |
|  task    |  task    | 任务和任务私有数据的抽象, 包含任务函数, 链接到调度器或依赖任务的链表, 任务回调, 预回调, 完成或运行状态, 任务参数,计数器等     |
|  sched    |  schedule    | 调度器及调度器私有数据的抽象, 在一个调度器下管理多个任务, 包含初始队列, 运行队列, 休眠队列, 调度器完成回调队列, 任务完成队列, 飞行计数器, 取消或完成等状态机      |
|  ev    |  event    | 事件,父子事件, 事件状态机, 事件回调, 关联的调度器, 网络上下文,事件队列, 屏障, 标记等      |
|  eq    |   event queue   | 事件队列,存放事件的各种队列的抽象, 包含存放事件完成的队列, 事件运行队列, 关联的网络上下文, 调度器, 终止状态等     |
|  ctx    |   network context   | 网络上下文   |




