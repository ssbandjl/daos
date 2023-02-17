/*
 * (C) Copyright 2018-2021 Intel Corporation.
 *
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 */

#ifndef __DAOS_DRPC_H__
#define __DAOS_DRPC_H__

#include <daos/drpc.pb-c.h>
#include <daos/common.h>

/*
 * Using a packetsocket over the unix domain socket means that we receive
 * a whole message at a time without knowing its size. So for this reason
 * we need to restrict the maximum message size so we can preallocate a
 * buffer to put all of the information in. This value is also defined in
 * the corresponding control plane file drpc_server.go. If changed here
 * it must be changed in that file as well
 * 在 unix 域套接字上使用 packetsocket 意味着我们在不知道其大小的情况下一次接收整个消息。 因此，出于这个原因，我们需要限制最大消息大小，以便我们可以预先分配一个缓冲区来放入所有信息。该值也在相应的控制平面文件 drpc_server.go 中定义。 如果在此处更改，则也必须在该文件中进行更改
 */
#define UNIXCOMM_MAXMSGSIZE (1 << 17)
/* 此补丁为 DRPC 添加了初始客户端框架。 它提供了一种创建 DRPC 连接和调用 golang DRPC 服务器的机制 */
struct unixcomm {
	int fd; /** File descriptor of the unix domain socket */
	int flags; /** Flags set on unix domain socket */
};

typedef void (*drpc_handler_t)(Drpc__Call *, Drpc__Response *);

/* Define a custom allocator so we can log and use fault injection
 * in the DRPC code.
 */

struct drpc_alloc {
	ProtobufCAllocator	alloc;
	bool			oom;
};

void *
daos_drpc_alloc(void *arg, size_t size);

void
daos_drpc_free(void *allocater_data, void *pointer);

#define PROTO_ALLOCATOR_INIT(self) {.alloc.alloc = daos_drpc_alloc,	\
				    .alloc.free = daos_drpc_free,	\
				    .alloc.allocator_data = &self}

/**
 * dRPC connection context. This includes all details needed to communicate
 * on the dRPC channel.
 */
struct drpc {
	struct unixcomm	*comm; /** unix domain socket communication context */
	int		sequence; /** sequence number of latest message sent */
	uint32_t	ref_count; /** open refs to this ctx */

	/**
	 * Handler for messages received by a listening drpc context.
	 * For client contexts, this is NULL.
	 */
	drpc_handler_t	handler;
};

enum rpcflags {
	R_SYNC = 1
};

int drpc_call_create(struct drpc *ctx, int32_t module, int32_t method,
		     Drpc__Call **callp);
void drpc_call_free(Drpc__Call *call);

Drpc__Response *drpc_response_create(Drpc__Call *call);
void drpc_response_free(Drpc__Response *resp);

int drpc_call(struct drpc *ctx, int flags, Drpc__Call *msg,
		Drpc__Response **resp);
int drpc_connect(char *sockaddr, struct drpc **);
struct drpc *drpc_listen(char *sockaddr, drpc_handler_t handler);
bool drpc_is_valid_listener(struct drpc *ctx);
struct drpc *drpc_accept(struct drpc *listener_ctx);
int drpc_recv_call(struct drpc *ctx, Drpc__Call **call);
int drpc_send_response(struct drpc *ctx, Drpc__Response *resp);
int drpc_close(struct drpc *ctx);

int drpc_add_ref(struct drpc *ctx);

#endif /* __DAOS_DRPC_H__ */
