/**
 * (C) Copyright 2016-2022 Intel Corporation.
 *
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 */
/**
 * ds_pool: Pool Server
 *
 * This is part of daos_server. It exports the pool RPC handlers and implements
 * Pool Server API.
 */
#define D_LOGFAC	DD_FAC(pool)

#include <daos_srv/pool.h>
#include <daos/rpc.h>
#include <daos_srv/daos_engine.h>
#include <daos_srv/bio.h>
#include "rpc.h"
#include "srv_internal.h"
#include "srv_layout.h"
bool ec_agg_disabled;
/* CORDAOSM-1 dsm：初始化 dsm 代码结构 这个补丁列出了初始的 DSM 代码结构。 因为池创建和存储部分仍在处理中，所以它们的内容已从此补丁中删除 */
static int
init(void)
{
	int rc;

	rc = ds_pool_cache_init();
	if (rc != 0)
		D_GOTO(err, rc);

	rc = ds_pool_hdl_hash_init();
	if (rc != 0)
		D_GOTO(err_pool_cache, rc);

	rc = ds_pool_iv_init();
	if (rc)
		D_GOTO(err_hdl_hash, rc);

	rc = ds_pool_prop_default_init();
	if (rc)
		D_GOTO(err_pool_iv, rc);

	ec_agg_disabled = false;
	/* DAOS-7254 聚合：将 EC 聚合与 VOS 聚合分开 (#5667)
将 EC agg 和 VOS aggregate 分离成两个 ULT，避免 iv fetch，dsc_pool/container open 每次回调，同时保证 EC aggregation 可以在删除快照时触发。
添加 DAOS_EC_AGG 环境以禁用 EC 聚合以进行测试 */
	d_getenv_bool("DAOS_EC_AGG_DISABLE", &ec_agg_disabled);
	if (unlikely(ec_agg_disabled))
		D_WARN("EC aggregation is disabled.\n");

	ds_pool_rsvc_class_register();
	/* DAOS-3106 池：NVMe 错误反应操作 (#1250)
实现NVMe故障反应操作，增加nvme_recovery测试套件。
限制：
- 由于缺乏离线获取pool map的基础设施，不支持离线NVMe故障反应。 一旦池缓存和池映射 IV 清理完成，它就会完成。
- nvme_recovery 测试只能手动运行，因为尚不支持 NVMe reint，失败的 VOS 目标无法重新集成，每次运行后需要重新启动 DAOS 服务器 */
	bio_register_ract_ops(&nvme_reaction_ops);
	return 0;

err_pool_iv:
	ds_pool_iv_fini();
err_hdl_hash:
	ds_pool_hdl_hash_fini();
err_pool_cache:
	ds_pool_cache_fini();
err:
	return rc;
}

static int
fini(void)
{
	ds_pool_rsvc_class_unregister();
	ds_pool_hdl_hash_fini();
	ds_pool_iv_fini();
	ds_pool_cache_fini();
	ds_pool_prop_default_fini();
	return 0;
}

static int
setup(void)
{
	bool start = true;

	d_getenv_bool("DAOS_START_POOL_SVC", &start);
	if (start)
		return ds_pool_start_all();
	return 0;
}

static int
cleanup(void)
{
	int rc;

	rc = ds_pool_stop_all();
	if (rc)
		D_ERROR("Stop pools failed. "DF_RC"\n", DP_RC(rc));

	return rc;
}

static struct crt_corpc_ops ds_pool_tgt_disconnect_co_ops = {
	.co_aggregate	= ds_pool_tgt_disconnect_aggregator,
	.co_pre_forward	= NULL,
};

static struct crt_corpc_ops ds_pool_tgt_query_co_ops = {
	.co_aggregate	= ds_pool_tgt_query_aggregator,
	.co_pre_forward	= NULL,
};

/* Define for cont_rpcs[] array population below.
 * See POOL_PROTO_*_RPC_LIST macro definition
 */
#define X(a, b, c, d, e)	\
{				\
	.dr_opc       = a,	\
	.dr_hdlr      = d,	\
	.dr_corpc_ops = e,	\
}

static struct daos_rpc_handler pool_handlers_v4[] = {
	POOL_PROTO_CLI_RPC_LIST(4),
	POOL_PROTO_SRV_RPC_LIST,
};

static struct daos_rpc_handler pool_handlers_v5[] = {
	POOL_PROTO_CLI_RPC_LIST(5),
	POOL_PROTO_SRV_RPC_LIST,
};

#undef X

static void *
pool_tls_init(int xs_id, int tgt_id)
{
	struct pool_tls *tls;

	D_ALLOC_PTR(tls);
	if (tls == NULL)
		return NULL;

	D_INIT_LIST_HEAD(&tls->dt_pool_list);
	return tls;
}

static void
pool_tls_fini(void *data)
{
	struct pool_tls		*tls = data;
	struct ds_pool_child	*child;

	D_ASSERT(tls != NULL);

	/* pool child cache should be empty now */
	d_list_for_each_entry(child, &tls->dt_pool_list, spc_list) {
		D_ERROR(DF_UUID": ref: %d\n",
			DP_UUID(child->spc_uuid), child->spc_ref);
	}

	if (!d_list_empty(&tls->dt_pool_list)) {
		bool strict = false;

		d_getenv_bool("DAOS_STRICT_SHUTDOWN", &strict);
		if (strict)
			D_ASSERTF(false, "dt_pool_list not empty\n");
		else
			D_ERROR("dt_pool_list not empty\n");
	}

	D_FREE(tls);
}

struct dss_module_key pool_module_key = {
	.dmk_tags = DAOS_SERVER_TAG,
	.dmk_index = -1,
	.dmk_init = pool_tls_init,
	.dmk_fini = pool_tls_fini,
};

struct dss_module_metrics pool_metrics = {
	.dmm_tags = DAOS_SYS_TAG,
	.dmm_init = ds_pool_metrics_alloc,
	.dmm_fini = ds_pool_metrics_free,
	.dmm_nr_metrics = ds_pool_metrics_count,
};
/* DAOS-147 构建：目录重命名和适应构建系统
开始源码重构：
- 重命名目录
- 更改构建系统以采用新布局
- 建立一个单一的 libdaos 库
- 启动 DSM 拆分为池和容器。 重复代码
尚未删除。
- 将 dtp 代码移到 common 中并将生成的库重命名为 libcart 以便于转换 */
struct dss_module pool_module =  {
	.sm_name	= "pool",
	.sm_mod_id	= DAOS_POOL_MODULE,
	.sm_ver		= DAOS_POOL_VERSION,
	.sm_proto_count	= 2,
	.sm_init	= init,
	.sm_fini	= fini,
	.sm_setup	= setup,
	.sm_cleanup	= cleanup,
	.sm_proto_fmt	= {&pool_proto_fmt_v4, &pool_proto_fmt_v5},
	.sm_cli_count	= {POOL_PROTO_CLI_COUNT, POOL_PROTO_CLI_COUNT},
	.sm_handlers	= {pool_handlers_v4, pool_handlers_v5},
	.sm_key		= &pool_module_key,
	.sm_metrics	= &pool_metrics,
};
