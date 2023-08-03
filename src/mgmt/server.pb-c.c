/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: server.proto */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C__NO_DEPRECATED
#define PROTOBUF_C__NO_DEPRECATED
#endif

#include "server.pb-c.h"
void   ctl__set_log_masks_req__init
                     (Ctl__SetLogMasksReq         *message)
{
  static const Ctl__SetLogMasksReq init_value = CTL__SET_LOG_MASKS_REQ__INIT;
  *message = init_value;
}
size_t ctl__set_log_masks_req__get_packed_size
                     (const Ctl__SetLogMasksReq *message)
{
  assert(message->base.descriptor == &ctl__set_log_masks_req__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t ctl__set_log_masks_req__pack
                     (const Ctl__SetLogMasksReq *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &ctl__set_log_masks_req__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t ctl__set_log_masks_req__pack_to_buffer
                     (const Ctl__SetLogMasksReq *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &ctl__set_log_masks_req__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
Ctl__SetLogMasksReq *
       ctl__set_log_masks_req__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (Ctl__SetLogMasksReq *)
     protobuf_c_message_unpack (&ctl__set_log_masks_req__descriptor,
                                allocator, len, data);
}
void   ctl__set_log_masks_req__free_unpacked
                     (Ctl__SetLogMasksReq *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &ctl__set_log_masks_req__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   ctl__set_log_masks_resp__init
                     (Ctl__SetLogMasksResp         *message)
{
  static const Ctl__SetLogMasksResp init_value = CTL__SET_LOG_MASKS_RESP__INIT;
  *message = init_value;
}
size_t ctl__set_log_masks_resp__get_packed_size
                     (const Ctl__SetLogMasksResp *message)
{
  assert(message->base.descriptor == &ctl__set_log_masks_resp__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t ctl__set_log_masks_resp__pack
                     (const Ctl__SetLogMasksResp *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &ctl__set_log_masks_resp__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t ctl__set_log_masks_resp__pack_to_buffer
                     (const Ctl__SetLogMasksResp *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &ctl__set_log_masks_resp__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
Ctl__SetLogMasksResp *
       ctl__set_log_masks_resp__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (Ctl__SetLogMasksResp *)
     protobuf_c_message_unpack (&ctl__set_log_masks_resp__descriptor,
                                allocator, len, data);
}
void   ctl__set_log_masks_resp__free_unpacked
                     (Ctl__SetLogMasksResp *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &ctl__set_log_masks_resp__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor ctl__set_log_masks_req__field_descriptors[7] =
{
  {
    "sys",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    offsetof(Ctl__SetLogMasksReq, sys),
    NULL,
    &protobuf_c_empty_string,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "masks",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    offsetof(Ctl__SetLogMasksReq, masks),
    NULL,
    &protobuf_c_empty_string,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "streams",
    3,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    offsetof(Ctl__SetLogMasksReq, streams),
    NULL,
    &protobuf_c_empty_string,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "subsystems",
    4,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    offsetof(Ctl__SetLogMasksReq, subsystems),
    NULL,
    &protobuf_c_empty_string,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "reset_masks",
    5,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_BOOL,
    0,   /* quantifier_offset */
    offsetof(Ctl__SetLogMasksReq, reset_masks),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "reset_streams",
    6,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_BOOL,
    0,   /* quantifier_offset */
    offsetof(Ctl__SetLogMasksReq, reset_streams),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "reset_subsystems",
    7,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_BOOL,
    0,   /* quantifier_offset */
    offsetof(Ctl__SetLogMasksReq, reset_subsystems),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned ctl__set_log_masks_req__field_indices_by_name[] = {
  1,   /* field[1] = masks */
  4,   /* field[4] = reset_masks */
  5,   /* field[5] = reset_streams */
  6,   /* field[6] = reset_subsystems */
  2,   /* field[2] = streams */
  3,   /* field[3] = subsystems */
  0,   /* field[0] = sys */
};
static const ProtobufCIntRange ctl__set_log_masks_req__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 7 }
};
const ProtobufCMessageDescriptor ctl__set_log_masks_req__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "ctl.SetLogMasksReq",
  "SetLogMasksReq",
  "Ctl__SetLogMasksReq",
  "ctl",
  sizeof(Ctl__SetLogMasksReq),
  7,
  ctl__set_log_masks_req__field_descriptors,
  ctl__set_log_masks_req__field_indices_by_name,
  1,  ctl__set_log_masks_req__number_ranges,
  (ProtobufCMessageInit) ctl__set_log_masks_req__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor ctl__set_log_masks_resp__field_descriptors[2] =
{
  {
    "status",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(Ctl__SetLogMasksResp, status),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "errors",
    2,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_STRING,
    offsetof(Ctl__SetLogMasksResp, n_errors),
    offsetof(Ctl__SetLogMasksResp, errors),
    NULL,
    &protobuf_c_empty_string,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned ctl__set_log_masks_resp__field_indices_by_name[] = {
  1,   /* field[1] = errors */
  0,   /* field[0] = status */
};
static const ProtobufCIntRange ctl__set_log_masks_resp__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor ctl__set_log_masks_resp__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "ctl.SetLogMasksResp",
  "SetLogMasksResp",
  "Ctl__SetLogMasksResp",
  "ctl",
  sizeof(Ctl__SetLogMasksResp),
  2,
  ctl__set_log_masks_resp__field_descriptors,
  ctl__set_log_masks_resp__field_indices_by_name,
  1,  ctl__set_log_masks_resp__number_ranges,
  (ProtobufCMessageInit) ctl__set_log_masks_resp__init,
  NULL,NULL,NULL    /* reserved[123] */
};
