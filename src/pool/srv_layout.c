/*
 * (C) Copyright 2017-2022 Intel Corporation.
 *
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 */
/**
 * ds_pool: Pool Server Storage Layout Definitions
 */

#define D_LOGFAC DD_FAC(pool)

#include <daos_srv/rdb.h>
#include <daos_srv/security.h>
#include <daos/pool.h>
#include "srv_layout.h"

/** Root KVS */
RDB_STRING_KEY(ds_pool_prop_, map_version);
RDB_STRING_KEY(ds_pool_prop_, map_buffer);
RDB_STRING_KEY(ds_pool_prop_, label);
RDB_STRING_KEY(ds_pool_prop_, acl);
RDB_STRING_KEY(ds_pool_prop_, space_rb);
RDB_STRING_KEY(ds_pool_prop_, self_heal);
RDB_STRING_KEY(ds_pool_prop_, reclaim);
RDB_STRING_KEY(ds_pool_prop_, owner);
RDB_STRING_KEY(ds_pool_prop_, owner_group);
RDB_STRING_KEY(ds_pool_prop_, connectable);
RDB_STRING_KEY(ds_pool_prop_, nhandles);
RDB_STRING_KEY(ds_pool_prop_, handles);
RDB_STRING_KEY(ds_pool_prop_, ec_cell_sz);
RDB_STRING_KEY(ds_pool_prop_, redun_fac);
RDB_STRING_KEY(ds_pool_prop_, ec_pda);
RDB_STRING_KEY(ds_pool_prop_, rp_pda);
RDB_STRING_KEY(ds_pool_attr_, user);
RDB_STRING_KEY(ds_pool_prop_, policy);
RDB_STRING_KEY(ds_pool_prop_, global_version);
RDB_STRING_KEY(ds_pool_prop_, upgrade_status);
RDB_STRING_KEY(ds_pool_prop_, upgrade_global_version);
RDB_STRING_KEY(ds_pool_prop_, scrub_sched);
RDB_STRING_KEY(ds_pool_prop_, scrub_freq);
RDB_STRING_KEY(ds_pool_prop_, scrub_thresh);
RDB_STRING_KEY(ds_pool_prop_, svc_redun_fac);

/** default properties, should cover all optional pool properties */
struct daos_prop_entry pool_prop_entries_default[DAOS_PROP_PO_NUM] = {
	{
		.dpe_type	= DAOS_PROP_PO_LABEL,
		.dpe_str	= "pool_label_not_set",
	}, {
		.dpe_type	= DAOS_PROP_PO_SPACE_RB,
		.dpe_val	= 0,
	}, {
		.dpe_type	= DAOS_PROP_PO_SELF_HEAL,
		.dpe_val	= DAOS_SELF_HEAL_AUTO_EXCLUDE |
				  DAOS_SELF_HEAL_AUTO_REBUILD,
	}, {
		.dpe_type	= DAOS_PROP_PO_RECLAIM,
		.dpe_val	= DAOS_RECLAIM_LAZY,
	}, {
		.dpe_type	= DAOS_PROP_PO_ACL,
		.dpe_val_ptr	= NULL, /* generated dynamically */
	}, {
		.dpe_type	= DAOS_PROP_PO_OWNER,
		.dpe_str	= "NOBODY@",
	}, {
		.dpe_type	= DAOS_PROP_PO_OWNER_GROUP,
		.dpe_str	= "NOBODY@",
	}, {
		.dpe_type	= DAOS_PROP_PO_SVC_LIST,
		.dpe_val_ptr	= NULL,
	}, {
		.dpe_type	= DAOS_PROP_PO_EC_CELL_SZ,
		.dpe_val	= DAOS_EC_CELL_DEF,
	}, {
		.dpe_type	= DAOS_PROP_PO_REDUN_FAC,
		.dpe_val	= DAOS_PROP_PO_REDUN_FAC_DEFAULT,
	}, {
		.dpe_type	= DAOS_PROP_PO_EC_PDA,
		.dpe_val	= DAOS_PROP_PO_EC_PDA_DEFAULT,

	}, {
		.dpe_type	= DAOS_PROP_PO_RP_PDA,
		.dpe_val	= DAOS_PROP_PO_RP_PDA_DEFAULT,
	}, {
		.dpe_type	= DAOS_PROP_PO_POLICY,
		.dpe_str	= DAOS_PROP_POLICYSTR_DEFAULT,
	}, {
		.dpe_type	= DAOS_PROP_PO_GLOBAL_VERSION,
		.dpe_val	= DAOS_POOL_GLOBAL_VERSION,
	}, {
		.dpe_type	= DAOS_PROP_PO_UPGRADE_STATUS,
		.dpe_val	= DAOS_UPGRADE_STATUS_NOT_STARTED,
	}, {
		.dpe_type	= DAOS_PROP_PO_SCRUB_MODE,
		.dpe_val	= DAOS_SCRUB_MODE_OFF,
	}, {
		.dpe_type	= DAOS_PROP_PO_SCRUB_FREQ,
		.dpe_val	= 604800, /* 1 week in seconds */
	}, {
		.dpe_type	= DAOS_PROP_PO_SCRUB_THRESH,
		.dpe_val	= 0,
	}, {
		.dpe_type	= DAOS_PROP_PO_SVC_REDUN_FAC,
		.dpe_val	= DAOS_PROP_PO_SVC_REDUN_FAC_DEFAULT,
	}
};

daos_prop_t pool_prop_default = {
	.dpp_nr		= DAOS_PROP_PO_NUM,
	.dpp_entries	= pool_prop_entries_default,
};

int
ds_pool_prop_default_init(void)
{
	struct daos_prop_entry	*entry;

	entry = daos_prop_entry_get(&pool_prop_default, DAOS_PROP_PO_ACL);
	if (entry != NULL) {
		D_DEBUG(DB_MGMT,
			"Initializing default ACL pool prop\n");
		entry->dpe_val_ptr = ds_sec_alloc_default_daos_pool_acl();
		/* - 实施容器 ACL 道具。
- 将所有者和所有者组道具添加到容器中。
- 为容器初始化默认 ACL。
- 将默认所有者更改为与真实用户不匹配的特殊字符串 (NOBODY@)。 在 UNIX 系统中，某些情况下使用小写的“nobody@”。
- 将容器相关权限添加到 daos_acl_dump()。
- 清理 ACL 验证逻辑和到/从 str 逻辑。
- 更新 daos_test 套件中默认所有者/组/ACL 道具的容器查询测试。
- 清理池和容器道具中的一些共享逻辑。 */
		if (entry->dpe_val_ptr == NULL)
			return -DER_NOMEM;
	}
	return 0;
}

void
ds_pool_prop_default_fini(void)
{
	struct daos_prop_entry	*entry;

	entry = daos_prop_entry_get(&pool_prop_default, DAOS_PROP_PO_ACL);
	if (entry != NULL) {
		D_DEBUG(DB_MGMT, "Freeing default ACL pool prop\n");
		D_FREE(entry->dpe_val_ptr);
	}
}
