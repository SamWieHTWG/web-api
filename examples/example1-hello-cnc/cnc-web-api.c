/*
Copyright (C)
ISG Industrielle Steuerungstechnik GmbH
https://www.isg-stuttgart.de
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#pragma comment(lib, "ws2_32.lib")
#define close closesocket
#define sleep(x) Sleep((x) * 1000)
#define usleep(x) Sleep((x) / 1000)
typedef int socklen_t;
typedef HANDLE pthread_t;
typedef unsigned (__stdcall *PTHREAD_START) (void *);
#define pthread_create(thhandle,attr,thfunc,tharg) (int)((*thhandle=(HANDLE)_beginthreadex(NULL,0,(PTHREAD_START)thfunc,tharg,0,NULL))==NULL)
#define pthread_join(thread, result) ((WaitForSingleObject((thread),INFINITE)!=WAIT_OBJECT_0) || !CloseHandle(thread))
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#endif

#include "cnc-wrapper.h"
#include "cnc-web-api.h"
#include "cJSON.h"

#ifdef _WIN32
#include <wincrypt.h>
#pragma comment(lib, "crypt32.lib")
#endif

#define HTTP_PORT 8080
#define BUFFER_SIZE 4096
#define MAX_RESPONSE_SIZE 8192
#define WS_MAGIC_STRING "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

// WebSocket opcodes
#define WS_OPCODE_TEXT 0x1
#define WS_OPCODE_CLOSE 0x8
#define WS_OPCODE_PING 0x9
#define WS_OPCODE_PONG 0xA

// Windows compatibility
#ifdef _WIN32
#define strdup _strdup
#endif

extern bool f_cnc_shutdown;

// Function prototypes
static char* handle_websocket_read_request(cJSON* request);
static char* handle_websocket_write_request(cJSON* request);

#ifdef _WIN32
static SOCKET api_server_socket = INVALID_SOCKET;
#else
static int api_server_socket = -1;
#endif
static pthread_t cnc_web_api_thread;

typedef struct {
    char* data;
    size_t size;
} HttpResponse;

// Helper function to convert C value to cJSON based on type
static cJSON* value_to_json(const void* value, CNC_DATA_TYPE type, uint32_t length) {
    switch (type) {
        case CNC_TYPE_BOOLEAN:
            return cJSON_CreateBool(*(bool*)value);
        case CNC_TYPE_UNS08:
            return cJSON_CreateNumber(*(uint8_t*)value);
        case CNC_TYPE_SGN08:
            return cJSON_CreateNumber(*(int8_t*)value);
        case CNC_TYPE_UNS16:
            return cJSON_CreateNumber(*(uint16_t*)value);
        case CNC_TYPE_SGN16:
            return cJSON_CreateNumber(*(int16_t*)value);
        case CNC_TYPE_UNS32:
            return cJSON_CreateNumber(*(uint32_t*)value);
        case CNC_TYPE_SGN32:
            return cJSON_CreateNumber(*(int32_t*)value);
        case CNC_TYPE_UNS64:
            return cJSON_CreateNumber((double)*(uint64_t*)value);
        case CNC_TYPE_SGN64:
            return cJSON_CreateNumber((double)*(int64_t*)value);
        case CNC_TYPE_REAL32:
            return cJSON_CreateNumber(*(float*)value);
        case CNC_TYPE_REAL64:
            {
                double val = *(double*)value;
                printf("[CNC-WEB-API] Converting REAL64: %f\n", val);
                return cJSON_CreateNumber(val);
            }
        case CNC_TYPE_CHAR:
            {
                char str[2] = {*(char*)value, '\0'};
                return cJSON_CreateString(str);
            }
        case CNC_TYPE_STRING:
            // Ensure null-terminated string for JSON
            {
                char* str_value = (char*)value;
                return cJSON_CreateString(str_value ? str_value : "");
            }
        case CNC_TYPE_STRUCT:
        case CNC_TYPE_NONE:
        default:
            // For STRUCT, NONE, or unknown types, return as hex string
            printf("[CNC-WEB-API] WARNING: Converting type %d to hex string (not handled in switch)\n", (int)type);
            {
                char* hex_str = malloc(length * 2 + 1);
                if (hex_str) {
                    for (uint32_t i = 0; i < length; i++) {
                        sprintf(hex_str + i * 2, "%02X", ((uint8_t*)value)[i]);
                    }
                    hex_str[length * 2] = '\0';
                    cJSON* result = cJSON_CreateString(hex_str);
                    free(hex_str);
                    return result;
                }
            }
            return cJSON_CreateNull();
    }
}

// Helper function to convert cJSON value to C value based on type
static bool json_to_value(cJSON* json, void* value, CNC_DATA_TYPE type) {
    if (!json || !value) return false;

    switch (type) {
        case CNC_TYPE_BOOLEAN:
            if (cJSON_IsBool(json)) {
                *(bool*)value = cJSON_IsTrue(json);
                return true;
            }
            // Also accept numbers for boolean (0 = false, anything else = true)
            if (cJSON_IsNumber(json)) {
                *(bool*)value = (cJSON_GetNumberValue(json) != 0.0);
                printf("[CNC-WEB-API] Converting number %.0f to boolean %s\n",
                       cJSON_GetNumberValue(json), (*(bool*)value) ? "true" : "false");
                return true;
            }
            break;
        case CNC_TYPE_UNS08:
            if (cJSON_IsNumber(json)) {
                *(uint8_t*)value = (uint8_t)cJSON_GetNumberValue(json);
                return true;
            }
            break;
        case CNC_TYPE_SGN08:
            if (cJSON_IsNumber(json)) {
                *(int8_t*)value = (int8_t)cJSON_GetNumberValue(json);
                return true;
            }
            break;
        case CNC_TYPE_UNS16:
            if (cJSON_IsNumber(json)) {
                *(uint16_t*)value = (uint16_t)cJSON_GetNumberValue(json);
                return true;
            }
            break;
        case CNC_TYPE_SGN16:
            if (cJSON_IsNumber(json)) {
                *(int16_t*)value = (int16_t)cJSON_GetNumberValue(json);
                return true;
            }
            break;
        case CNC_TYPE_UNS32:
            if (cJSON_IsNumber(json)) {
                *(uint32_t*)value = (uint32_t)cJSON_GetNumberValue(json);
                return true;
            }
            break;
        case CNC_TYPE_SGN32:
            if (cJSON_IsNumber(json)) {
                *(int32_t*)value = (int32_t)cJSON_GetNumberValue(json);
                return true;
            }
            break;
        case CNC_TYPE_UNS64:
            if (cJSON_IsNumber(json)) {
                *(uint64_t*)value = (uint64_t)cJSON_GetNumberValue(json);
                return true;
            }
            break;
        case CNC_TYPE_SGN64:
            if (cJSON_IsNumber(json)) {
                *(int64_t*)value = (int64_t)cJSON_GetNumberValue(json);
                return true;
            }
            break;
        case CNC_TYPE_REAL32:
            if (cJSON_IsNumber(json)) {
                *(float*)value = (float)cJSON_GetNumberValue(json);
                return true;
            }
            break;
        case CNC_TYPE_REAL64:
            if (cJSON_IsNumber(json)) {
                *(double*)value = cJSON_GetNumberValue(json);
                return true;
            }
            break;
        case CNC_TYPE_CHAR:
            if (cJSON_IsString(json)) {
                const char* str = cJSON_GetStringValue(json);
                if (str && strlen(str) > 0) {
                    *(char*)value = str[0];
                    return true;
                }
            }
            break;
        case CNC_TYPE_STRING:
            if (cJSON_IsString(json)) {
                const char* str = cJSON_GetStringValue(json);
                if (str) {
                    strcpy((char*)value, str);
                    return true;
                }
            }
            break;
        default:
            return false;
    }
    return false;
}

#ifdef _WIN32
static void send_http_response(SOCKET client_socket, int status_code, const char* status_text,
                              const char* content_type, const char* body) {
#else
static void send_http_response(int client_socket, int status_code, const char* status_text,
                              const char* content_type, const char* body) {
#endif
    char response[MAX_RESPONSE_SIZE];
    int body_length = body ? strlen(body) : 0;

    snprintf(response, sizeof(response),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "\r\n"
        "%s",
        status_code, status_text, content_type, body_length, body ? body : "");

    send(client_socket, response, strlen(response), 0);
}

#ifdef _WIN32
static void send_error_response(SOCKET client_socket, int status_code, const char* message) {
#else
static void send_error_response(int client_socket, int status_code, const char* message) {
#endif
    cJSON* response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "error", message);
    cJSON_AddNumberToObject(response, "status", status_code);

    char* json_string = cJSON_Print(response);
    if (json_string) {
        send_http_response(client_socket, status_code, "Error", "application/json", json_string);
        free(json_string);
    }
    cJSON_Delete(response);
}

// WebSocket utility functions
static void base64_encode(const unsigned char* input, int length, char* output) {
    const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i, j;
    for (i = 0, j = 0; i < length; i += 3, j += 4) {
        uint32_t octet_a = i < length ? input[i] : 0;
        uint32_t octet_b = i + 1 < length ? input[i + 1] : 0;
        uint32_t octet_c = i + 2 < length ? input[i + 2] : 0;
        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        output[j] = base64_chars[(triple >> 3 * 6) & 0x3F];
        output[j + 1] = base64_chars[(triple >> 2 * 6) & 0x3F];
        output[j + 2] = base64_chars[(triple >> 1 * 6) & 0x3F];
        output[j + 3] = base64_chars[(triple >> 0 * 6) & 0x3F];
    }

    int padding = length % 3;
    if (padding) {
        for (int k = 0; k < 3 - padding; k++) {
            output[j - 1 - k] = '=';
        }
    }
    output[j] = '\0';
}

static void sha1_hash(const char* input, unsigned char* output) {
#ifdef _WIN32
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    DWORD cbHash = 20;

    if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        if (CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash)) {
            if (CryptHashData(hHash, (BYTE*)input, strlen(input), 0)) {
                CryptGetHashParam(hHash, HP_HASHVAL, output, &cbHash, 0);
            }
            CryptDestroyHash(hHash);
        }
        CryptReleaseContext(hProv, 0);
    }
#else
    // For Linux, would need to link with OpenSSL or implement simple SHA1
    // For now, just zero out (will need proper implementation)
    memset(output, 0, 20);
#endif
}

#ifdef _WIN32
static bool handle_websocket_handshake(SOCKET client_socket, const char* key) {
#else
static bool handle_websocket_handshake(int client_socket, const char* key) {
#endif
    char concat_key[256];
    snprintf(concat_key, sizeof(concat_key), "%s%s", key, WS_MAGIC_STRING);

    unsigned char sha1_result[20];
    sha1_hash(concat_key, sha1_result);

    char accept_key[32];
    base64_encode(sha1_result, 20, accept_key);

    char response[512];
    snprintf(response, sizeof(response),
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s\r\n"
        "\r\n", accept_key);

    send(client_socket, response, strlen(response), 0);
    printf("[CNC-WEB-API] WebSocket handshake completed\n");
    return true;
}

#ifdef _WIN32
static int websocket_send_text(SOCKET client_socket, const char* message) {
#else
static int websocket_send_text(int client_socket, const char* message) {
#endif
    int msg_len = strlen(message);
    unsigned char* frame = malloc(msg_len + 10);
    if (!frame) return -1;
    int frame_len = 0;

    // Frame format: FIN(1) + RSV(3) + OPCODE(4) + MASK(1) + PAYLOAD_LEN(7/16/64) + PAYLOAD
    frame[0] = 0x80 | WS_OPCODE_TEXT; // FIN=1, OPCODE=TEXT

    if (msg_len < 126) {
        frame[1] = msg_len;
        frame_len = 2;
    } else if (msg_len < 65536) {
        frame[1] = 126;
        frame[2] = (msg_len >> 8) & 0xFF;
        frame[3] = msg_len & 0xFF;
        frame_len = 4;
    } else {
        // For simplicity, not implementing 64-bit length
        return -1;
    }

    memcpy(frame + frame_len, message, msg_len);
    frame_len += msg_len;

    int result = send(client_socket, (char*)frame, frame_len, 0);
    free(frame);
    return result;
}

#ifdef _WIN32
static int websocket_receive_text(SOCKET client_socket, char* buffer, int buffer_size) {
#else
static int websocket_receive_text(int client_socket, char* buffer, int buffer_size) {
#endif
    unsigned char header[14];
    int bytes_received = recv(client_socket, (char*)header, 2, 0);
    if (bytes_received != 2) return -1;

    bool fin = (header[0] & 0x80) != 0;
    int opcode = header[0] & 0x0F;
    bool masked = (header[1] & 0x80) != 0;
    int payload_len = header[1] & 0x7F;

    if (opcode == WS_OPCODE_CLOSE) return 0;
    if (opcode != WS_OPCODE_TEXT) return -1;

    int header_len = 2;
    if (payload_len == 126) {
        bytes_received = recv(client_socket, (char*)header + 2, 2, 0);
        if (bytes_received != 2) return -1;
        payload_len = (header[2] << 8) | header[3];
        header_len = 4;
    }

    unsigned char mask[4];
    if (masked) {
        bytes_received = recv(client_socket, (char*)mask, 4, 0);
        if (bytes_received != 4) return -1;
    }

    if (payload_len >= buffer_size) return -1;

    bytes_received = recv(client_socket, buffer, payload_len, 0);
    if (bytes_received != payload_len) return -1;

    if (masked) {
        for (int i = 0; i < payload_len; i++) {
            buffer[i] ^= mask[i % 4];
        }
    }

    buffer[payload_len] = '\0';
    return payload_len;
}

#ifdef _WIN32
static void handle_websocket_session(SOCKET client_socket) {
#else
static void handle_websocket_session(int client_socket) {
#endif
    printf("[CNC-WEB-API] WebSocket session started\n");

    char message_buffer[BUFFER_SIZE];

    while (!f_cnc_shutdown) {
        int message_len = websocket_receive_text(client_socket, message_buffer, sizeof(message_buffer));

        if (message_len <= 0) {
            printf("[CNC-WEB-API] WebSocket connection closed or error\n");
            break;
        }

        printf("[CNC-WEB-API] WebSocket message received: %s\n", message_buffer);

        // Parse JSON message
        cJSON* request = cJSON_Parse(message_buffer);
        if (!request) {
            websocket_send_text(client_socket, "{\"error\":\"Invalid JSON\",\"success\":false}");
            continue;
        }

        cJSON* type_json = cJSON_GetObjectItem(request, "type");
        if (!type_json || !cJSON_IsString(type_json)) {
            websocket_send_text(client_socket, "{\"error\":\"Missing type field\",\"success\":false}");
            cJSON_Delete(request);
            continue;
        }

        char* response_json = NULL;
        const char* type = cJSON_GetStringValue(type_json);

        if (strcmp(type, "read") == 0) {
            // Handle read request
            response_json = handle_websocket_read_request(request);
        } else if (strcmp(type, "write") == 0) {
            // Handle write request
            response_json = handle_websocket_write_request(request);
        } else {
            websocket_send_text(client_socket, "{\"error\":\"Unknown request type\",\"success\":false}");
            cJSON_Delete(request);
            continue;
        }

        if (response_json) {
            websocket_send_text(client_socket, response_json);
            free(response_json);
        }

        cJSON_Delete(request);
    }

    printf("[CNC-WEB-API] WebSocket session ended\n");
}

static char* handle_websocket_read_request(cJSON* request) {
    // Start timing
    clock_t start_time = clock();
    time_t start_wall_time = time(NULL);

    // Extract request ID for response correlation
    cJSON* id_json = cJSON_GetObjectItem(request, "id");
    int request_id = cJSON_IsNumber(id_json) ? (int)cJSON_GetNumberValue(id_json) : 0;

    cJSON* thread_json = cJSON_GetObjectItem(request, "thread");
    cJSON* group_json = cJSON_GetObjectItem(request, "group");
    cJSON* offset_json = cJSON_GetObjectItem(request, "offset");

    if (!cJSON_IsNumber(thread_json) || !cJSON_IsNumber(group_json) || !cJSON_IsNumber(offset_json)) {
        char error_response[256];
        snprintf(error_response, sizeof(error_response),
                "{\"id\":%d,\"error\":\"Missing required fields\",\"success\":false}", request_id);
        return strdup(error_response);
    }

    uint32_t thread = (uint32_t)cJSON_GetNumberValue(thread_json);
    uint32_t group = (uint32_t)cJSON_GetNumberValue(group_json);
    uint32_t offset = (uint32_t)cJSON_GetNumberValue(offset_json);

    // Get optional parameters
    cJSON* length_json = cJSON_GetObjectItem(request, "length");
    cJSON* datatype_json = cJSON_GetObjectItem(request, "datatype");

    uint32_t length = 0;
    CNC_DATA_TYPE datatype = CNC_TYPE_NONE;

    if (cJSON_IsNumber(length_json)) {
        length = (uint32_t)cJSON_GetNumberValue(length_json);
    }

    if (cJSON_IsNumber(datatype_json)) {
        datatype = (CNC_DATA_TYPE)cJSON_GetNumberValue(datatype_json);
    }

    // If datatype not provided, get it from CNC
    if (datatype == CNC_TYPE_NONE) {
        datatype = cnc_get_object_data_type_wrapper(thread, group, offset);
    }

    // If length not provided, determine from datatype
    if (length == 0) {
        length = cnc_get_type_size(datatype);
        if (length == 0) length = 32; // Default for variable-length types
    }

    // Allocate buffer and perform read
    void* buffer = malloc(length);
    if (!buffer) {
        char error_response[256];
        snprintf(error_response, sizeof(error_response),
                "{\"id\":%d,\"error\":\"Memory allocation failed\",\"success\":false}", request_id);
        return strdup(error_response);
    }

    printf("[CNC-WEB-API] WS READ: thread=%u, group=0x%x(%u), offset=0x%x(%u), type=%s(%d), length=%u\n",
           thread, group, group, offset, offset, cnc_get_type_name(datatype), (int)datatype, length);

    int32_t result = cnc_read_value_wrapper(thread, group, offset, buffer, length);

    // Create response
    cJSON* response = cJSON_CreateObject();
    cJSON_AddNumberToObject(response, "id", request_id);
    cJSON_AddNumberToObject(response, "thread", thread);
    cJSON_AddNumberToObject(response, "group", group);
    cJSON_AddNumberToObject(response, "offset", offset);
    cJSON_AddStringToObject(response, "type", cnc_get_type_name(datatype));
    cJSON_AddNumberToObject(response, "length", length);
    cJSON_AddNumberToObject(response, "result", result);

    if (result == 0) {
        // Success - add value
        printf("[CNC-WEB-API] Converting value to JSON with type %s(%d)\n", cnc_get_type_name(datatype), (int)datatype);
        cJSON* value_json = value_to_json(buffer, datatype, length);
        cJSON_AddItemToObject(response, "value", value_json);
        cJSON_AddBoolToObject(response, "success", true);
        printf("[CNC-WEB-API] WS READ RESULT: 0 (SUCCESS)\n");
    } else {
        // Error
        cJSON_AddBoolToObject(response, "success", false);
        printf("[CNC-WEB-API] WS READ RESULT: %d (ERROR)\n", result);
    }

    // End timing
    clock_t end_time = clock();
    time_t end_wall_time = time(NULL);
    double cpu_time = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;
    long wall_time = end_wall_time - start_wall_time;

    printf("[CNC-WEB-API] WS READ REQUEST END: %ld (CPU: %.2fms, Wall: %ldms)\n",
           (long)end_wall_time, cpu_time, wall_time * 1000);

    char* json_string = cJSON_Print(response);
    cJSON_Delete(response);
    free(buffer);

    return json_string;
}

static char* handle_websocket_write_request(cJSON* request) {
    // Start timing
    clock_t start_time = clock();
    time_t start_wall_time = time(NULL);

    // Extract request ID for response correlation
    cJSON* id_json = cJSON_GetObjectItem(request, "id");
    int request_id = cJSON_IsNumber(id_json) ? (int)cJSON_GetNumberValue(id_json) : 0;

    cJSON* thread_json = cJSON_GetObjectItem(request, "thread");
    cJSON* group_json = cJSON_GetObjectItem(request, "group");
    cJSON* offset_json = cJSON_GetObjectItem(request, "offset");
    cJSON* value_json = cJSON_GetObjectItem(request, "value");

    if (!cJSON_IsNumber(thread_json) || !cJSON_IsNumber(group_json) ||
        !cJSON_IsNumber(offset_json) || !value_json) {
        char error_response[256];
        snprintf(error_response, sizeof(error_response),
                "{\"id\":%d,\"error\":\"Missing required fields\",\"success\":false}", request_id);
        return strdup(error_response);
    }

    uint32_t thread = (uint32_t)cJSON_GetNumberValue(thread_json);
    uint32_t group = (uint32_t)cJSON_GetNumberValue(group_json);
    uint32_t offset = (uint32_t)cJSON_GetNumberValue(offset_json);

    // Get optional datatype parameter
    cJSON* datatype_json = cJSON_GetObjectItem(request, "datatype");
    CNC_DATA_TYPE datatype = CNC_TYPE_NONE;

    if (cJSON_IsNumber(datatype_json)) {
        datatype = (CNC_DATA_TYPE)cJSON_GetNumberValue(datatype_json);
    }

    // If datatype not provided, get it from CNC
    if (datatype == CNC_TYPE_NONE) {
        datatype = cnc_get_object_data_type_wrapper(thread, group, offset);
    }

    // Get type size and allocate buffer
    uint32_t length = cnc_get_type_size(datatype);
    if (length == 0) length = 32; // Default for variable-length types

    void* buffer = malloc(length);
    if (!buffer) {
        char error_response[256];
        snprintf(error_response, sizeof(error_response),
                "{\"id\":%d,\"error\":\"Memory allocation failed\",\"success\":false}", request_id);
        return strdup(error_response);
    }

    // Convert JSON value to C value based on datatype
    if (!json_to_value(value_json, buffer, datatype)) {
        free(buffer);
        char error_response[256];
        snprintf(error_response, sizeof(error_response),
                "{\"id\":%d,\"error\":\"Invalid value for datatype\",\"success\":false}", request_id);
        return strdup(error_response);
    }

    printf("[CNC-WEB-API] WS WRITE: thread=%u, group=0x%x(%u), offset=0x%x(%u), type=%s, length=%u\n",
           thread, group, group, offset, offset, cnc_get_type_name(datatype), length);

    int32_t result = cnc_write_value_wrapper(thread, group, offset, buffer, length);

    // Create response
    cJSON* response = cJSON_CreateObject();
    cJSON_AddNumberToObject(response, "id", request_id);
    cJSON_AddNumberToObject(response, "thread", thread);
    cJSON_AddNumberToObject(response, "group", group);
    cJSON_AddNumberToObject(response, "offset", offset);
    cJSON_AddStringToObject(response, "type", cnc_get_type_name(datatype));
    cJSON_AddNumberToObject(response, "length", length);
    cJSON_AddNumberToObject(response, "result", result);

    if (result == 0) {
        // Success
        cJSON_AddBoolToObject(response, "success", true);
        printf("[CNC-WEB-API] WS WRITE RESULT: 0 (SUCCESS)\n");
    } else {
        // Error
        cJSON_AddBoolToObject(response, "success", false);
        printf("[CNC-WEB-API] WS WRITE RESULT: %d (ERROR)\n", result);
    }

    // End timing
    clock_t end_time = clock();
    time_t end_wall_time = time(NULL);
    double cpu_time = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;
    long wall_time = end_wall_time - start_wall_time;

    printf("[CNC-WEB-API] WS WRITE REQUEST END: %ld (CPU: %.2fms, Wall: %ldms)\n",
           (long)end_wall_time, cpu_time, wall_time * 1000);

    char* json_string = cJSON_Print(response);
    cJSON_Delete(response);
    free(buffer);

    return json_string;
}

#ifdef _WIN32
static void handle_read_request(SOCKET client_socket, const char* body) {
#else
static void handle_read_request(int client_socket, const char* body) {
#endif
    // Start timing
    clock_t start_time = clock();
    time_t start_wall_time = time(NULL);

    printf("[CNC-WEB-API] READ REQUEST START: %ld\n", (long)start_wall_time);

    cJSON* request = cJSON_Parse(body);
    if (!request) {
        send_error_response(client_socket, 400, "Invalid JSON");
        return;
    }

    cJSON* thread_json = cJSON_GetObjectItem(request, "thread");
    cJSON* group_json = cJSON_GetObjectItem(request, "group");
    cJSON* offset_json = cJSON_GetObjectItem(request, "offset");

    if (!cJSON_IsNumber(thread_json) || !cJSON_IsNumber(group_json) || !cJSON_IsNumber(offset_json)) {
        cJSON_Delete(request);
        send_error_response(client_socket, 400, "Missing required fields: thread, group, offset");
        return;
    }

    uint32_t thread_id = (uint32_t)cJSON_GetNumberValue(thread_json);
    uint32_t group_id = (uint32_t)cJSON_GetNumberValue(group_json);
    uint32_t offset_id = (uint32_t)cJSON_GetNumberValue(offset_json);

    // Check for optional datatype parameter
    cJSON* datatype_json = cJSON_GetObjectItem(request, "datatype");
    CNC_DATA_TYPE data_type;
    uint32_t type_size;

    if (cJSON_IsNumber(datatype_json)) {
        // Use provided datatype
        data_type = (CNC_DATA_TYPE)cJSON_GetNumberValue(datatype_json);
        type_size = cnc_get_type_size(data_type);
    } else {
        // Get the data type automatically (fallback)
        data_type = cnc_get_object_data_type_wrapper(thread_id, group_id, offset_id);
        type_size = cnc_get_type_size(data_type);
    }

    // Check for optional length parameter
    cJSON* length_json = cJSON_GetObjectItem(request, "length");
    uint32_t length;

    if (cJSON_IsNumber(length_json)) {
        // Use provided length
        length = (uint32_t)cJSON_GetNumberValue(length_json);
    } else {
        // Use type size or default for variable length types
        length = type_size;
        if (type_size == 0) {
            length = 256; // Default for variable length types
        }
    }

    cJSON_Delete(request);

    if (length > 4096) {
        send_error_response(client_socket, 400, "Length too large (max 4096)");
        return;
    }

    void* value = malloc(length);
    if (!value) {
        send_error_response(client_socket, 500, "Memory allocation failed");
        return;
    }

    memset(value, 0, length); // Initialize memory

    // Log the read operation
    printf("[CNC-WEB-API] READ: thread=%u, group=0x%x(%u), offset=0x%x(%u), type=%s, length=%u\n",
           thread_id, group_id, group_id, offset_id, offset_id, cnc_get_type_name(data_type), length);

    int32_t result = cnc_read_value_wrapper(thread_id, group_id, offset_id, value, length);

    printf("[CNC-WEB-API] READ RESULT: %d (%s)\n", result, (result == 0) ? "SUCCESS" : "ERROR");

    // Create JSON response
    cJSON* response = cJSON_CreateObject();
    cJSON_AddNumberToObject(response, "thread", thread_id);
    cJSON_AddNumberToObject(response, "group", group_id);
    cJSON_AddNumberToObject(response, "offset", offset_id);
    cJSON_AddStringToObject(response, "type", cnc_get_type_name(data_type));
    cJSON_AddNumberToObject(response, "length", length);
    cJSON_AddNumberToObject(response, "result", result);

    if (result == 0) {
        cJSON* value_json = value_to_json(value, data_type, length);
        cJSON_AddItemToObject(response, "value", value_json);
        cJSON_AddBoolToObject(response, "success", true);
    } else {
        cJSON_AddNullToObject(response, "value");
        cJSON_AddBoolToObject(response, "success", false);
        cJSON_AddStringToObject(response, "error", "Read operation failed");
    }

    char* json_string = cJSON_Print(response);
    if (json_string) {
        send_http_response(client_socket, 200, "OK", "application/json", json_string);
        free(json_string);
    }

    cJSON_Delete(response);
    free(value);

    // End timing
    clock_t end_time = clock();
    time_t end_wall_time = time(NULL);

    double duration_ms = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000.0;
    long wall_duration_ms = (long)(end_wall_time - start_wall_time) * 1000;

    printf("[CNC-WEB-API] READ REQUEST END: %ld (CPU: %.2fms, Wall: %ldms)\n",
           (long)end_wall_time, duration_ms, wall_duration_ms);
}

#ifdef _WIN32
static void handle_write_request(SOCKET client_socket, const char* body) {
#else
static void handle_write_request(int client_socket, const char* body) {
#endif
    // Start timing
    clock_t start_time = clock();
    time_t start_wall_time = time(NULL);

    printf("[CNC-WEB-API] WRITE REQUEST START: %ld\n", (long)start_wall_time);

    cJSON* request = cJSON_Parse(body);
    if (!request) {
        send_error_response(client_socket, 400, "Invalid JSON");
        return;
    }

    cJSON* thread_json = cJSON_GetObjectItem(request, "thread");
    cJSON* group_json = cJSON_GetObjectItem(request, "group");
    cJSON* offset_json = cJSON_GetObjectItem(request, "offset");
    cJSON* value_json = cJSON_GetObjectItem(request, "value");

    if (!cJSON_IsNumber(thread_json) || !cJSON_IsNumber(group_json) ||
        !cJSON_IsNumber(offset_json) || !value_json) {
        printf("[CNC-WEB-API] WRITE ERROR: Missing required fields\n");
        cJSON_Delete(request);
        send_error_response(client_socket, 400, "Missing required fields: thread, group, offset, value");
        return;
    }

    uint32_t thread_id = (uint32_t)cJSON_GetNumberValue(thread_json);
    uint32_t group_id = (uint32_t)cJSON_GetNumberValue(group_json);
    uint32_t offset_id = (uint32_t)cJSON_GetNumberValue(offset_json);

    // Check for optional datatype parameter
    cJSON* datatype_json = cJSON_GetObjectItem(request, "datatype");
    CNC_DATA_TYPE data_type;
    uint32_t type_size;

    if (cJSON_IsNumber(datatype_json)) {
        // Use provided datatype
        data_type = (CNC_DATA_TYPE)cJSON_GetNumberValue(datatype_json);
        type_size = cnc_get_type_size(data_type);
    } else {
        // Get the data type automatically (fallback)
        data_type = cnc_get_object_data_type_wrapper(thread_id, group_id, offset_id);
        type_size = cnc_get_type_size(data_type);
    }

    // Log the write operation request
    printf("[CNC-WEB-API] WRITE: thread=%u, group=0x%x(%u), offset=0x%x(%u), type=%s, size=%u\n",
           thread_id, group_id, group_id, offset_id, offset_id, cnc_get_type_name(data_type), type_size);

    if (type_size == 0 && data_type != CNC_TYPE_STRING) {
        printf("[CNC-WEB-API] WRITE ERROR: Cannot write to variable-length or struct types (type=%s)\n", cnc_get_type_name(data_type));
        cJSON_Delete(request);
        send_error_response(client_socket, 400, "Cannot write to variable-length or struct types");
        return;
    }

    // Handle string allocation specially
    uint32_t actual_size = type_size;
    if (data_type == CNC_TYPE_STRING) {
        if (cJSON_IsString(value_json)) {
            const char* str_value = cJSON_GetStringValue(value_json);
            actual_size = (uint32_t)strlen(str_value) + 1;
            printf("[CNC-WEB-API] STRING: Allocating %u bytes for string: \"%s\"\n", actual_size, str_value);
        } else {
            printf("[CNC-WEB-API] WRITE ERROR: Expected string value for STRING type\n");
            cJSON_Delete(request);
            send_error_response(client_socket, 400, "Expected string value for STRING type");
            return;
        }
    }

    void* value = malloc(actual_size);
    if (!value) {
        printf("[CNC-WEB-API] WRITE ERROR: Memory allocation failed for %u bytes\n", actual_size);
        cJSON_Delete(request);
        send_error_response(client_socket, 500, "Memory allocation failed");
        return;
    }

    // Convert JSON value to C value based on type
    if (!json_to_value(value_json, value, data_type)) {
        printf("[CNC-WEB-API] WRITE ERROR: Invalid value for data type %s\n", cnc_get_type_name(data_type));
        free(value);
        cJSON_Delete(request);
        send_error_response(client_socket, 400, "Invalid value for data type");
        return;
    }

    // Log the value being written
    if (data_type == CNC_TYPE_STRING || data_type == CNC_TYPE_CHAR) {
        printf("[CNC-WEB-API] WRITE VALUE: \"%s\"\n", (char*)value);
    } else {
        printf("[CNC-WEB-API] WRITE VALUE: ");
        for (uint32_t i = 0; i < type_size; i++) {
            printf("%02X ", ((uint8_t*)value)[i]);
        }
        printf("\n");
    }

    cJSON_Delete(request);

    int32_t result = cnc_write_value_wrapper(thread_id, group_id, offset_id, value, actual_size);

    printf("[CNC-WEB-API] WRITE RESULT: %d (%s)\n", result, (result == 0) ? "SUCCESS" : "ERROR");

    // Create JSON response
    cJSON* response = cJSON_CreateObject();
    cJSON_AddNumberToObject(response, "thread", thread_id);
    cJSON_AddNumberToObject(response, "group", group_id);
    cJSON_AddNumberToObject(response, "offset", offset_id);
    cJSON_AddStringToObject(response, "type", cnc_get_type_name(data_type));
    cJSON_AddNumberToObject(response, "result", result);
    cJSON_AddBoolToObject(response, "success", result == 0);

    if (result != 0) {
        cJSON_AddStringToObject(response, "error", "Write operation failed");
    }

    char* json_string = cJSON_Print(response);
    if (json_string) {
        send_http_response(client_socket, 200, "OK", "application/json", json_string);
        free(json_string);
    }

    cJSON_Delete(response);
    free(value);

    // End timing
    clock_t end_time = clock();
    time_t end_wall_time = time(NULL);

    double duration_ms = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000.0;
    long wall_duration_ms = (long)(end_wall_time - start_wall_time) * 1000;

    printf("[CNC-WEB-API] WRITE REQUEST END: %ld (CPU: %.2fms, Wall: %ldms)\n",
           (long)end_wall_time, duration_ms, wall_duration_ms);
}

#ifdef _WIN32
static void handle_read_write_request(SOCKET client_socket, const char* body) {
#else
static void handle_read_write_request(int client_socket, const char* body) {
#endif
    unsigned int object_id_val = 0;
    uint32_t read_length = 0;
    uint32_t write_length = 0;
    char hex_data[1024] = {0};
    char response_body[1024];

    if (!body) {
        send_error_response(client_socket, 400, "Missing request body");
        return;
    }

    int parsed = sscanf(body, "{\"object_id\": %u, \"read_length\": %u, \"write_length\": %u, \"data\": \"%1023s\"",
                       &object_id_val, &read_length, &write_length, hex_data);

    if (parsed < 4) {
        send_error_response(client_socket, 400, "Invalid JSON format");
        return;
    }

    char* quote_pos = strrchr(hex_data, '"');
    if (quote_pos) *quote_pos = '\0';

    if (read_length > 512 || write_length > 512) {
        send_error_response(client_socket, 400, "Length too large");
        return;
    }

    uint32_t max_length = (read_length > write_length) ? read_length : write_length;
    uint8_t* value = malloc(max_length);
    if (!value) {
        send_error_response(client_socket, 500, "Memory allocation failed");
        return;
    }

    for (uint32_t i = 0; i < write_length; i++) {
        char hex_byte[3] = {hex_data[i*2], hex_data[i*2+1], '\0'};
        value[i] = (uint8_t)strtol(hex_byte, NULL, 16);
    }

    int32_t result = cnc_read_write_value_wrapper(1, object_id_val, 0, value, read_length, write_length);

    char hex_response[1024] = {0};
    char* ptr = hex_response;
    for (uint32_t i = 0; i < read_length && (ptr - hex_response) < sizeof(hex_response) - 3; i++) {
        sprintf(ptr, "%02X", value[i]);
        ptr += 2;
    }

    snprintf(response_body, sizeof(response_body),
        "{\"object_id\": %u, \"read_length\": %u, \"write_length\": %u, \"read_data\": \"%s\", \"result\": %d, \"success\": %s}",
        object_id_val, read_length, write_length, hex_response, result, (result == 0) ? "true" : "false");

    free(value);
    send_http_response(client_socket, 200, "OK", "application/json", response_body);
}

#ifdef _WIN32
static void handle_options_request(SOCKET client_socket) {
#else
static void handle_options_request(int client_socket) {
#endif
    send_http_response(client_socket, 200, "OK", "text/plain", "");
}

#ifdef _WIN32
static void handle_client_request(SOCKET client_socket) {
#else
static void handle_client_request(int client_socket) {
#endif
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received <= 0) {
        close(client_socket);
        return;
    }

    buffer[bytes_received] = '\0';

    char method[16], path[256], version[16];
    sscanf(buffer, "%15s %255s %15s", method, path, version);

    // Log incoming request
    printf("[CNC-WEB-API] REQUEST: %s %s %s\n", method, path, version);

    char* query_string = strchr(path, '?');
    if (query_string) {
        *query_string = '\0';
        query_string++;
    }

    char* body = strstr(buffer, "\r\n\r\n");
    if (body) {
        body += 4;
    }

    // Check for WebSocket upgrade
    if (strcmp(method, "GET") == 0 && strstr(buffer, "Upgrade: websocket")) {
        printf("[CNC-WEB-API] WebSocket upgrade request detected\n");

        // Extract the Sec-WebSocket-Key
        char* key_header = strstr(buffer, "Sec-WebSocket-Key: ");
        if (key_header) {
            key_header += 19; // Skip "Sec-WebSocket-Key: "
            char* key_end = strstr(key_header, "\r\n");
            if (key_end) {
                char ws_key[32];
                int key_len = key_end - key_header;
                if (key_len < 32) {
                    strncpy(ws_key, key_header, key_len);
                    ws_key[key_len] = '\0';

                    if (handle_websocket_handshake(client_socket, ws_key)) {
                        // Handle WebSocket communication
                        handle_websocket_session(client_socket);
                    }
                }
            }
        }
        close(client_socket);
        return;
    }

    if (strcmp(method, "OPTIONS") == 0) {
        printf("[CNC-WEB-API] Handling OPTIONS request\n");
        handle_options_request(client_socket);
    }
    else if (strcmp(method, "POST") == 0 && strcmp(path, "/read") == 0) {
        printf("[CNC-WEB-API] Handling READ request\n");
        if (body) printf("[CNC-WEB-API] Request body: %s\n", body);
        handle_read_request(client_socket, body);
    }
    else if (strcmp(method, "POST") == 0 && strcmp(path, "/write") == 0) {
        printf("[CNC-WEB-API] Handling WRITE request\n");
        if (body) printf("[CNC-WEB-API] Request body: %s\n", body);
        handle_write_request(client_socket, body);
    }
    else if (strcmp(method, "POST") == 0 && strcmp(path, "/read_write") == 0) {
        printf("[CNC-WEB-API] Handling READ_WRITE request\n");
        if (body) printf("[CNC-WEB-API] Request body: %s\n", body);
        handle_read_write_request(client_socket, body);
    }
    else if (strcmp(method, "GET") == 0 && strcmp(path, "/") == 0) {
        const char* html = "<!DOCTYPE html><html><head><title>CNC Web API</title></head><body>"
                          "<h1>CNC Web API - JSON with Automatic Type Conversion</h1>"
                          "<p>All endpoints use JSON and automatically detect CNC object types.</p>"
                          "<h3>Available endpoints:</h3>"
                          "<ul>"
                          "<li><strong>POST /read</strong> - Read CNC object value<br/>"
                          "Body: {\"thread\": 1, \"group\": 131840, \"offset\": 7}<br/>"
                          "Returns: {\"value\": 12345, \"type\": \"UNS32\", \"success\": true, ...}</li>"
                          "<li><strong>POST /write</strong> - Write CNC object value<br/>"
                          "Body: {\"thread\": 1, \"group\": 131840, \"offset\": 7, \"value\": 42}</li>"
                          "<li><strong>POST /read_write</strong> - Combined read/write operation</li>"
                          "</ul>"
                          "<p><em>Types are automatically detected and values properly converted!</em></p>"
                          "</body></html>";
        send_http_response(client_socket, 200, "OK", "text/html", html);
    }
    else {
        send_error_response(client_socket, 404, "Not Found");
    }

    close(client_socket);
}

static void* cnc_web_api_server_thread(void* arg) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("CNC Web API: WSAStartup failed\n");
        return NULL;
    }

    api_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (api_server_socket == INVALID_SOCKET) {
        printf("CNC Web API: Failed to create socket\n");
        WSACleanup();
        return NULL;
    }
#else
    api_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (api_server_socket < 0) {
        printf("CNC Web API: Failed to create socket\n");
        return NULL;
    }
#endif

    int opt = 1;
    setsockopt(api_server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(HTTP_PORT);

    if (bind(api_server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("[CNC-WEB-API] ERROR: Failed to bind socket to port %d\n", HTTP_PORT);
        close(api_server_socket);
#ifdef _WIN32
        WSACleanup();
#endif
        return NULL;
    }

    printf("[CNC-WEB-API] Socket bound to port %d\n", HTTP_PORT);

    if (listen(api_server_socket, 5) < 0) {
        printf("[CNC-WEB-API] ERROR: Failed to listen on socket\n");
        close(api_server_socket);
#ifdef _WIN32
        WSACleanup();
#endif
        return NULL;
    }

    printf("[CNC-WEB-API] Server listening and ready to accept connections on port %d\n", HTTP_PORT);

    while (!f_cnc_shutdown) {
#ifdef _WIN32
        SOCKET client_socket = accept(api_server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == INVALID_SOCKET) {
            if (!f_cnc_shutdown) {
                printf("CNC Web API: Accept failed\n");
            }
            continue;
        }
#else
        int client_socket = accept(api_server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            if (!f_cnc_shutdown) {
                printf("CNC Web API: Accept failed\n");
            }
            continue;
        }
#endif

        printf("[CNC-WEB-API] Client connected, handling request...\n");
        handle_client_request(client_socket);
        printf("[CNC-WEB-API] Request completed, client disconnected\n");
    }

    close(api_server_socket);
#ifdef _WIN32
    WSACleanup();
#endif
    printf("CNC Web API: Stopped\n");
    return NULL;
}

int cnc_web_api_start(void) {
    printf("[CNC-WEB-API] Starting CNC Web API server...\n");
    if (pthread_create(&cnc_web_api_thread, NULL, cnc_web_api_server_thread, NULL) != 0) {
        printf("[CNC-WEB-API] ERROR: Failed to create thread\n");
        return -1;
    }

    printf("[CNC-WEB-API] Server thread created successfully\n");
    return 0;
}

void cnc_web_api_stop(void) {
#ifdef _WIN32
    if (api_server_socket != INVALID_SOCKET) {
        close(api_server_socket);
        api_server_socket = INVALID_SOCKET;
    }
#else
    if (api_server_socket >= 0) {
        close(api_server_socket);
        api_server_socket = -1;
    }
#endif

    pthread_join(cnc_web_api_thread, NULL);
}