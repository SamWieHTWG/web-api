/*
Copyright (C)
ISG Industrielle Steuerungstechnik GmbH
https://www.isg-stuttgart.de
*/

#ifndef CNC_WRAPPER_H
#define CNC_WRAPPER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t iThread;
    uint32_t iGroup;
    uint32_t iOffset;
} CNC_OBJECT_ID_SIMPLE;

// Data type enumeration matching CNC SDK
typedef enum {
    CNC_TYPE_NONE = 0,
    CNC_TYPE_BOOLEAN,
    CNC_TYPE_UNS08,
    CNC_TYPE_SGN08,
    CNC_TYPE_UNS16,
    CNC_TYPE_SGN16,
    CNC_TYPE_UNS32,
    CNC_TYPE_SGN32,
    CNC_TYPE_UNS64,
    CNC_TYPE_SGN64,
    CNC_TYPE_REAL64,
    CNC_TYPE_STRUCT,
    CNC_TYPE_REAL32,
    CNC_TYPE_CHAR,
    CNC_TYPE_STRING,
    CNC_TYPE_ERROR = 99
} CNC_DATA_TYPE;

// Basic wrapper functions
int32_t cnc_read_value_wrapper(uint32_t thread_id, uint32_t group_id, uint32_t offset_id, void* value, uint32_t length);
int32_t cnc_write_value_wrapper(uint32_t thread_id, uint32_t group_id, uint32_t offset_id, void* value, uint32_t length);
int32_t cnc_read_write_value_wrapper(uint32_t thread_id, uint32_t group_id, uint32_t offset_id, void* value, uint32_t read_length, uint32_t write_length);

// Type-aware functions
CNC_DATA_TYPE cnc_get_object_data_type_wrapper(uint32_t thread_id, uint32_t group_id, uint32_t offset_id);
uint32_t cnc_get_type_size(CNC_DATA_TYPE type);
const char* cnc_get_type_name(CNC_DATA_TYPE type);

#endif /* CNC_WRAPPER_H */