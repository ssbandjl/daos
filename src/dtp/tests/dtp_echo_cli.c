/**
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * GOVERNMENT LICENSE RIGHTS-OPEN SOURCE SOFTWARE
 * The Government's rights to use, modify, reproduce, release, perform, display,
 * or disclose this software are subject to the terms of the LGPL License as
 * provided in Contract No. B609815.
 * Any reproduction of computer software, computer software documentation, or
 * portions thereof marked with this legend must also reproduce the markings.
 *
 * (C) Copyright 2015 Intel Corporation.
 */
/**
 * This is a simple example of dtp_echo rpc client based on dtp APIs.
 */

#include <dtp_echo.h>

struct gecho gecho;

static int client_wait(int num_retries, unsigned int wait_len_ms,
		       int *complete_flag)
{
	int retry, rc;
	for (retry = 0; retry < num_retries; retry++) {
		rc = dtp_progress(gecho.dtp_ctx, wait_len_ms, NULL, NULL,
				  NULL);
		if (rc != 0 && rc != -ETIMEDOUT) {
			printf("dtp_progress failed rc: %d.\n", rc);
			break;
		}
		if (*complete_flag)
			return 0;
	}
	return -ETIMEDOUT;
}

static int client_cb_common(const struct dtp_cb_info *cb_info)
{
	dtp_rpc_t		*rpc_req;
	echo_checkin_in_t	*checkin_input;
	echo_checkin_out_t	*checkin_output;

	rpc_req = cb_info->dci_rpc;

	/* set complete flag */
	printf("in client_cb_common, opc: 0x%x, dci_rc: %d.\n",
	       rpc_req->dr_opc, cb_info->dci_rc);
	*(int *) cb_info->dci_arg = 1;

	switch (cb_info->dci_rpc->dr_opc) {
	case ECHO_OPC_CHECKIN:
		checkin_input = rpc_req->dr_input;
		checkin_output = rpc_req->dr_output;
		assert(checkin_input != NULL && checkin_output != NULL);
		printf("%s checkin result - ret: %d, room_no: %d.\n",
		       checkin_input->name, checkin_output->ret,
		       checkin_output->room_no);
		break;
	case ECHO_OPC_SHUTDOWN:
		break;
	default:
		break;
	}

	return 0;
}

struct bulk_test_cli_cbinfo {
	dtp_bulk_t	bulk_hdl;
	int		*complete_flag;
};

static int bulk_test_req_cb(const struct dtp_cb_info *cb_info)
{
	echo_bulk_test_in_t		*bulk_test_input = NULL;
	echo_bulk_test_out_t		*bulk_test_output = NULL;
	struct bulk_test_cli_cbinfo	*bulk_test_cbinfo;
	dtp_rpc_t			*rpc_req;
	int				rc;

	rpc_req = cb_info->dci_rpc;
	bulk_test_cbinfo = (struct bulk_test_cli_cbinfo *)cb_info->dci_arg;

	bulk_test_input = (echo_bulk_test_in_t *)rpc_req->dr_input;
	assert(bulk_test_input != NULL);
	bulk_test_output = (echo_bulk_test_out_t *)rpc_req->dr_output;
	assert(bulk_test_output != NULL);

	printf("in bulk_test_req_cb, opc: 0x%x, dci_rc: %d.\n",
	       rpc_req->dr_opc, cb_info->dci_rc);
	printf("bulk_test_output->bulk_echo_msg: %s.\n",
	       bulk_test_output->bulk_echo_msg);

	rc = dtp_bulk_free(bulk_test_cbinfo->bulk_hdl);
	assert(rc == 0);
	/* set complete flag */
	*(bulk_test_cbinfo->complete_flag) = 1;

	free(bulk_test_cbinfo);
	return 0;
}

static void run_client(void)
{
	dtp_endpoint_t			svr_ep;
	dtp_rpc_t			*rpc_req = NULL;
	echo_checkin_in_t		*checkin_input = NULL;
	echo_checkin_out_t		*checkin_output = NULL;
	echo_bulk_test_in_t		*bulk_test_input = NULL;
	echo_bulk_test_out_t		*bulk_test_output = NULL;
	daos_sg_list_t			sgl;
	daos_iov_t			*iovs = NULL;
	dtp_bulk_t			bulk_hdl;
	struct bulk_test_cli_cbinfo	*bulk_req_cbinfo;
	char				*pchar;
	int				rc = 0, i;

	/* ============= test-1 ============
	 * send checkin RPC: "32" years old "Tom" checkin, want "3" days */
	rc = dtp_req_create(gecho.dtp_ctx, svr_ep, ECHO_OPC_CHECKIN, &rpc_req);
	assert(rc == 0 && rpc_req != NULL);

	/*
	 * The dtp_req_create already allocated the input/output buffer
	 * based on the input_size/output_size per the opcode
	 */
	checkin_input = (echo_checkin_in_t *)rpc_req->dr_input;
	assert(checkin_input != NULL);
	checkin_output = (echo_checkin_out_t *)rpc_req->dr_output;
	assert(checkin_output != NULL);

	/*
	 * No strdup will cause mercury crash when HG_Free_input
	 * in dtp_hg_reply_send_cb
	 */
	checkin_input->name = strdup("Tom@IBM");
	checkin_input->age = 32;
	checkin_input->days = 3;

	printf("sending checkin request, name: %s, age: %d, days: %d.\n",
	       checkin_input->name, checkin_input->age, checkin_input->days);

	gecho.complete = 0;
	rc = dtp_req_send(rpc_req, client_cb_common, &gecho.complete);
	assert(rc == 0);
	/* wait two minutes (in case of manually starting up clients) */
	rc = client_wait(120, 1000, &gecho.complete);
	assert(rc == 0);

	printf("client checkin request sent.\n");

	/* ============= test-2 ============
	 * simple bulk transferring */
	rpc_req = NULL;
	rc = dtp_req_create(gecho.dtp_ctx, svr_ep, ECHO_OPC_BULK_TEST,
			    &rpc_req);
	assert(rc == 0 && rpc_req != NULL);

	bulk_test_input = (echo_bulk_test_in_t *)rpc_req->dr_input;
	assert(bulk_test_input != NULL);
	bulk_test_output = (echo_bulk_test_out_t *)rpc_req->dr_output;
	assert(bulk_test_output != NULL);

	iovs = (daos_iov_t *)malloc(2 * sizeof(daos_iov_t));
	iovs[0].iov_buf_len = 4097;
	iovs[0].iov_buf = malloc(iovs[0].iov_buf_len);
	pchar = iovs[0].iov_buf;
	for (i = 0; i < iovs[0].iov_buf_len; i++)
		*(pchar++) = i + 17;
	iovs[1].iov_buf_len = 1*1024*1024 + 11;
	iovs[1].iov_buf = malloc(iovs[1].iov_buf_len);
	pchar = iovs[1].iov_buf;
	for (i = 0; i < iovs[1].iov_buf_len; i++)
		*(pchar++) = random();
	sgl.sg_llen = 2;
	sgl.sg_iovn = 2;
	sgl.sg_iovs = iovs;
	sgl.el_csums = NULL;

	/* calculate md5 checksum */
	MD5_CTX md5_ctx;
	unsigned char md5[16];
	dtp_string_t md5_str = (dtp_string_t)malloc(33);
	memset(md5_str, 0, 33);

	rc = MD5_Init(&md5_ctx);
	assert(rc == 1);
	rc = MD5_Update(&md5_ctx, iovs[0].iov_buf, iovs[0].iov_buf_len);
	assert(rc == 1);
	rc = MD5_Update(&md5_ctx, iovs[1].iov_buf, iovs[1].iov_buf_len);
	assert(rc == 1);
	rc = MD5_Final(md5, &md5_ctx);
	assert(rc == 1);
	echo_md5_to_string(md5, md5_str);

	rc = dtp_bulk_create(gecho.dtp_ctx, &sgl, DTP_BULK_RW, &bulk_hdl);
	assert(rc == 0);

	bulk_test_input->bulk_intro_msg = strdup("simple bulk testing...");
	bulk_test_input->bulk_hdl = bulk_hdl;
	bulk_test_input->bulk_md5_str = md5_str;

	printf("client sending bulk_test request, md5_str: %s.\n", md5_str);
	gecho.complete = 0;

	bulk_req_cbinfo = (struct bulk_test_cli_cbinfo *)malloc(
				sizeof(*bulk_req_cbinfo));
	assert(bulk_req_cbinfo != NULL);
	bulk_req_cbinfo->bulk_hdl = bulk_hdl;
	bulk_req_cbinfo->complete_flag = &gecho.complete;

	rc = dtp_req_send(rpc_req, bulk_test_req_cb, bulk_req_cbinfo);
	assert(rc == 0);

	rc = client_wait(100, 100, &gecho.complete);
	assert(rc == 0);
	free(iovs[0].iov_buf);
	free(iovs[1].iov_buf);
	free(iovs);


	/* ====================== */
	/* send an RPC to kill the server */
	printf("press enter to send shutdown request to server...\n");
	getchar();
	printf("client sending shutdown request...\n");
	gecho.complete = 0;
	rpc_req = NULL;
	rc = dtp_req_create(gecho.dtp_ctx, svr_ep, ECHO_OPC_SHUTDOWN, &rpc_req);
	assert(rc == 0 && rpc_req != NULL);

	assert(rpc_req->dr_input == NULL);
	assert(rpc_req->dr_output == NULL);

	rc = dtp_req_send(rpc_req, client_cb_common, &gecho.complete);
	assert(rc == 0);

	rc = client_wait(100, 100, &gecho.complete);
	assert(rc == 0);

	printf("client shuting down...\n");
}

int main(int argc, char *argv[])
{
	echo_init(0);

	run_client();

	echo_fini();

	return 0;
}
