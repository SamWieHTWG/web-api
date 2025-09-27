/*
Copyright (C)
ISG Industrielle Steuerungstechnik GmbH
https://www.isg-stuttgart.de
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#include "cnc_os_ifc.h"
#include "cnc-wrapper.h"

static void log_access(const char* function_name, uint32_t thread_id, uint32_t group_id, uint32_t offset_id, int32_t result) {
    FILE *log_file = fopen("object-access.txt", "a");
    if (log_file != NULL) {
        time_t now = time(NULL);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

        fprintf(log_file, "[%s] %s(thread=%u, group=%u, offset=%u) -> result=%d",
                timestamp, function_name, thread_id, group_id, offset_id, result);

        if (result != 0) {
            fprintf(log_file, " ERROR");
        }

        fprintf(log_file, "\n");
        fclose(log_file);
    }
}

static void log_data_type_access(const char* function_name, uint32_t thread_id, uint32_t group_id, uint32_t offset_id, CNC_DATA_TYPE result) {
    FILE *log_file = fopen("object-access.txt", "a");
    if (log_file != NULL) {
        time_t now = time(NULL);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

        fprintf(log_file, "[%s] %s(thread=%u, group=%u, offset=%u) -> type=%s(%d)\n",
                timestamp, function_name, thread_id, group_id, offset_id, cnc_get_type_name(result), (int)result);
        fclose(log_file);
    }
}

int32_t cnc_read_value_wrapper(uint32_t thread_id, uint32_t group_id, uint32_t offset_id, void* value, uint32_t length) {
    CNC_OBJECT_ID cnc_id = {thread_id, group_id, offset_id};
    int32_t result = cnc_read_value(cnc_id, value, length);
    log_access("cnc_read_value", thread_id, group_id, offset_id, result);
    return result;
}

int32_t cnc_write_value_wrapper(uint32_t thread_id, uint32_t group_id, uint32_t offset_id, void* value, uint32_t length) {
    CNC_OBJECT_ID cnc_id = {thread_id, group_id, offset_id};
    int32_t result = cnc_write_value(cnc_id, value, length);
    log_access("cnc_write_value", thread_id, group_id, offset_id, result);
    return result;
}

int32_t cnc_read_write_value_wrapper(uint32_t thread_id, uint32_t group_id, uint32_t offset_id, void* value, uint32_t read_length, uint32_t write_length) {
    CNC_OBJECT_ID cnc_id = {thread_id, group_id, offset_id};
    int32_t result = cnc_read_write_value(cnc_id, value, read_length, write_length);
    log_access("cnc_read_write_value", thread_id, group_id, offset_id, result);
    return result;
}

CNC_DATA_TYPE cnc_get_object_data_type_wrapper(uint32_t thread_id, uint32_t group_id, uint32_t offset_id) {
    CNC_OBJECT_ID cnc_id = {thread_id, group_id, offset_id};
    E_CNC_OBJECT_DATA_TYPE sdk_type = cnc_get_object_data_type(cnc_id);
    CNC_DATA_TYPE result = (CNC_DATA_TYPE)sdk_type;
    log_data_type_access("cnc_get_object_data_type", thread_id, group_id, offset_id, result);
    return result;
}

uint32_t cnc_get_type_size(CNC_DATA_TYPE type) {
    switch (type) {
        case CNC_TYPE_BOOLEAN:
        case CNC_TYPE_UNS08:
        case CNC_TYPE_SGN08:
        case CNC_TYPE_CHAR:
            return 1;
        case CNC_TYPE_UNS16:
        case CNC_TYPE_SGN16:
            return 2;
        case CNC_TYPE_UNS32:
        case CNC_TYPE_SGN32:
        case CNC_TYPE_REAL32:
            return 4;
        case CNC_TYPE_UNS64:
        case CNC_TYPE_SGN64:
        case CNC_TYPE_REAL64:
            return 8;
        case CNC_TYPE_STRING:
        case CNC_TYPE_STRUCT:
        default:
            return 0; // Variable length
    }
}

const char* cnc_get_type_name(CNC_DATA_TYPE type) {
    switch (type) {
        case CNC_TYPE_NONE:     return "NONE";
        case CNC_TYPE_BOOLEAN:  return "BOOLEAN";
        case CNC_TYPE_UNS08:    return "UNS08";
        case CNC_TYPE_SGN08:    return "SGN08";
        case CNC_TYPE_UNS16:    return "UNS16";
        case CNC_TYPE_SGN16:    return "SGN16";
        case CNC_TYPE_UNS32:    return "UNS32";
        case CNC_TYPE_SGN32:    return "SGN32";
        case CNC_TYPE_UNS64:    return "UNS64";
        case CNC_TYPE_SGN64:    return "SGN64";
        case CNC_TYPE_REAL64:   return "REAL64";
        case CNC_TYPE_REAL32:   return "REAL32";
        case CNC_TYPE_CHAR:     return "CHAR";
        case CNC_TYPE_STRING:   return "STRING";
        case CNC_TYPE_STRUCT:   return "STRUCT";
        default:                return "ERROR";
    }
}