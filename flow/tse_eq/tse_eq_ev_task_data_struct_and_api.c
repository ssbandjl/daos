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
int daos_eq_create(daos_eq_t *eq_out, int use_lock, int is_polling) 
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

