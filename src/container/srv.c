/**
 * (C) Copyright 2016-2022 Intel Corporation.
 *
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 */
/**
 * ds_cont: Container Server
 *
 * This is part of daos_server. It exports the container RPC handlers and
 * Container Server API.
 */
#define D_LOGFAC	DD_FAC(container)

#include <daos_srv/daos_engine.h>
#include <daos/rpc.h>
#include "rpc.h"
#include "srv_internal.h"

static int
init(void)
/* 这个补丁列出了初始的 DSM 代码结构。 因为池创建和存储部分仍在处理中，所以它们的内容已从此补丁中删除 */
{
	int rc;

	rc = ds_oid_iv_init();
	if (rc)
		D_GOTO(err, rc);
	/* DAOS-2185 容器：将 iv 添加到容器（#297）
	将 iv 添加到容器领导者，因此快照列表可以在所有服务器上共享，并且重建可能需要它。
	注意：容器将与池共享相同的 iv 名称空间，即池下的所有容器将共享相同的 iv 名称空间。
	将原点 ds_cont 重命名为 ds_cont_child 以将其标识为每个 xstream 结构，与 ds_pool_child 对齐。
	为每个节点添加 ds_cont 以加载 iv 命名空间。
	一些清理和修复。 */
	rc = ds_cont_iv_init();
	if (rc)
		D_GOTO(err_oid_iv, rc);

	rc = ds_cont_prop_default_init();
	if (rc)
		D_GOTO(err_cont_iv, rc);

	return 0;

err_cont_iv:
	ds_cont_iv_fini();
err_oid_iv:
	ds_oid_iv_fini();
err:
	return rc;
}

static int
fini(void)
{
	ds_cont_iv_fini();
	ds_oid_iv_fini();
	ds_cont_prop_default_fini();

	return 0;
}

static struct crt_corpc_ops ds_cont_tgt_destroy_co_ops = {
	.co_aggregate   = ds_cont_tgt_destroy_aggregator,
	.co_pre_forward = NULL,
};

static struct crt_corpc_ops ds_cont_tgt_query_co_ops = {
	.co_aggregate   = ds_cont_tgt_query_aggregator,
	.co_pre_forward = NULL,
};

static struct crt_corpc_ops ds_cont_tgt_epoch_aggregate_co_ops = {
	.co_aggregate   = ds_cont_tgt_epoch_aggregate_aggregator,
	.co_pre_forward = NULL,
};

static struct crt_corpc_ops ds_cont_tgt_snapshot_notify_co_ops = {
	.co_aggregate   = ds_cont_tgt_snapshot_notify_aggregator,
	.co_pre_forward = NULL,
};

/* Define for cont_rpcs[] array population below.
 * See CONT_PROTO_*_RPC_LIST macro definition
 */
#define X(a, b, c, d, e)	\
{				\
	.dr_opc       = a,	\
	.dr_hdlr      = d,	\
	.dr_corpc_ops = e,	\
}

static struct daos_rpc_handler cont_handlers[] = {
	CONT_PROTO_CLI_RPC_LIST,
	CONT_PROTO_SRV_RPC_LIST,
};

#undef X

static void *
dsm_tls_init(int xs_id, int tgt_id)
{
	struct dsm_tls *tls;
	int		rc;

	D_ALLOC_PTR(tls);
	if (tls == NULL)
		return NULL;

	rc = ds_cont_child_cache_create(&tls->dt_cont_cache);
	if (rc != 0) {
		D_ERROR("failed to create thread-local container cache: %d\n",
			rc);
		D_FREE(tls);
		return NULL;
	}

	rc = ds_cont_hdl_hash_create(&tls->dt_cont_hdl_hash);
	if (rc != 0) {
		D_ERROR("failed to create thread-local container handle cache:"
			" "DF_RC"\n", DP_RC(rc));
		ds_cont_child_cache_destroy(tls->dt_cont_cache);
		D_FREE(tls);
		return NULL;
	}

	return tls;
}

static void
dsm_tls_fini(void *data)
{
	struct dsm_tls *tls = data;

	ds_cont_hdl_hash_destroy(&tls->dt_cont_hdl_hash);
	ds_cont_child_cache_destroy(tls->dt_cont_cache);
	D_FREE(tls);
}

struct dss_module_key cont_module_key = {
	.dmk_tags = DAOS_SERVER_TAG,
	.dmk_index = -1,
	.dmk_init = dsm_tls_init,
	.dmk_fini = dsm_tls_fini,
};

struct dss_module_metrics cont_metrics = {
	.dmm_tags = DAOS_SYS_TAG,
	.dmm_init = ds_cont_metrics_alloc,
	.dmm_fini = ds_cont_metrics_free,
	.dmm_nr_metrics = ds_cont_metrics_count,
};

struct dss_module cont_module =  {
	.sm_name	= "cont",
	.sm_mod_id	= DAOS_CONT_MODULE,
	.sm_ver		= DAOS_CONT_VERSION,
	.sm_proto_count	= 1,
	.sm_init	= init,
	.sm_fini	= fini,
	.sm_proto_fmt	= &cont_proto_fmt,
	.sm_cli_count	= CONT_PROTO_CLI_COUNT,
	.sm_handlers	= cont_handlers,
	.sm_key		= &cont_module_key,
	.sm_metrics	= &cont_metrics,
};
