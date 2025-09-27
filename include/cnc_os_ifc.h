/*
Copyright (C)
ISG Industrielle Steuerungstechnik GmbH
https://www.isg-stuttgart.de
*/

/** \file
 * @brief TODO-DOC: Module description missing
*/
#ifndef CNC_OS_IFC_H_DEFINED
#define CNC_OS_IFC_H_DEFINED

/*
+----------------------------------------------------------------------------+
| cnc_os_ifc.h                                          | Copyright    ISG   |
|                                                       |                    |
|                                                       | Gropiusplatz 10    |
|                                                       | 70563 Stuttgart    |
+-------------------------------------------------------+--------------------+
| CNC SDK API functions.                                                     |
+----------------------------------------------------------------------------+
*/

/*
+-----------------------------------------------------------------------------+
| Defines                                                                     |
+-----------------------------------------------------------------------------+
*/
#define STACK_SIZE_STD                    500000

#define PATH_FILENAME_LENGTH                 256

/*
+-----------------------------------------------------------------------------+
| Error codes of CNC-OS-interface                                             |
+-----------------------------------------------------------------------------+
*/
typedef enum _e_cnc_osifc_error_codes
{
  ERR_CNC_TCP_OBJ_ACCESS               = -56,        // Error during tcp object access
  ERR_CNC_DELETE_SHM_HLI_AXIS          = -55,        // Error while deleting axis shared memory.
  ERR_CNC_DELETE_SHM_HLI_CHANNEL       = -54,        // Error while deleting channel shared memory.
  ERR_CNC_DELETE_SHM_HLI_PLATFORM      = -53,        // Error while delete platform shared memory.
  ERR_CNC_CREATE_SHM_HLI_AXIS          = -52,        // Error while creating/getting axis shared memory.
  ERR_CNC_CREATE_SHM_HLI_CHANNEL       = -51,        // Error while creating/getting channel shared memory.
  ERR_CNC_CREATE_SHM_HLI_PLATFORM      = -50,        // Error while creating/getting platform shared memory.
  ERR_CNC_PATH_TOO_LONG                = -41,        // Path is too long.
  ERR_CNC_READ_TCPIP                   = -40,        // Remote call via TCP/IP not possible
  ERR_CNC_INVALID_AX_NUMBER            = -35,        // Invalid axis number
  ERR_CNC_INVALID_CH_NUMBER            = -34,        // Invalid channel number
  ERR_CNC_STARTUP_TIMEOUT              = -33,        // Timeout during CNC startup
  ERR_CNC_STARTUP_FILE_NOT_FOUND       = -32,        // CNC startup file not found
  ERR_CNC_TOOL_DATA_PTR_INVALID        = -31,        // Pointer to external tool data is invalid
  ERR_CNC_LICENSE_CHECK_FAILED         = -30,        // License check failed
  ERR_CNC_HLI_SHM_PTR_INVALID          = -29,        // Pointer to HLI is invalid
  ERR_CNC_HLI_SHM_ALLOCATE             = -28,        // Allocation of shared memory for HLI failed
  ERR_CNC_DRIVE_FEEDBACK_PTR_INVALID   = -27,        // Pointer to drive feedback memory is invalid
  ERR_CNC_DRIVE_CMD_PTR_INVALID        = -26,        // Pointer to drive command memory is invalid
  ERR_CNC_EXT_VAR_PTR_INVALID          = -25,        // Pointer to external variables memory is invalid
  ERR_CNC_HLI_AXIS_PTR_INVALID         = -24,        // Pointer to HLI axis memory is invalid
  ERR_CNC_HLI_CHANNEL_PTR_INVALID      = -23,        // Pointer to HLI channel memory is invalid
  ERR_CNC_HLI_PLATFORM_PTR_INVALID     = -22,        // Pointer to HLI platform memory is invalid
  ERR_CNC_ERROR_INITIALISING_HLI       = -21,        // Error while initializing HLI
  ERR_CNC_ERROR_UPDATING_PARAMETERS    = -20,        // Parameter update failed
  ERR_CNC_INVALID_TOOL_NR              = -19,        // Tool number not valid or does not exist
  ERR_CNC_INVALID_AX_INDEX             = -18,        // Axis index not valid or does not exist
  ERR_CNC_INVALID_CH_INDEX             = -17,        // Channel index not valid or does not exist
  ERR_CNC_EXT_TOOL_MGR_INIT_FAILED     = -16,        // Initialization of external tool management failed
  ERR_CNC_MEMORY_REG_FAILED            = -15,        // Registration of CNC memories failed.
  ERR_CNC_PTR_UEBER_NOT_AVAILABLE      = -14,        // Pointer to global CNC shared memory invalid
  ERR_CNC_NOT_SUFFICIENT_MEMORY        = -13,        // Not sufficient memory
  ERR_CNC_INVALID_DRIVE_IFC            = -12,        // Drive interface not available
  ERR_CNC_SCHEDULER_UNKNOWN_FCT        = -11,        // Unknown function for CNC scheduler 
  ERR_CNC_TOOL_FCT_NOT_INIT            = -10,        // External tool function not available
  ERR_CNC_MSG_FCT_NOT_INIT             = -9,         // External message function not available
  ERR_CNC_WRONG_OBJECT_DATA_TYPE       = -8,         // Object data type wrong
  ERR_CNC_WRONG_OBJECT_INDEX           = -7,         // Object index wrong
  ERR_CNC_INVALID_OBJECT_ID            = -6,         // Object ID wrong
  ERR_CNC_OBJECT_NO_FOUND              = -5,         // Object not found
  ERR_CNC_READ_WRITE_SIZE_TO_BIG       = -4,         // Size of object too big
  ERR_CNC_OBJECT_DOMAIN_NOT_AVAILABLE  = -3,         // Object domain not available
  ERR_CNC_INVALID_INDEX_GROUP          = -2,         // Object group index not valid
  ERR_CNC_INVALID_TASK_ID              = -1,         // Object task ID not valid
  ERR_CNC_NOERROR                      =  0          // No error
} E_CNC_OSIFC_ERROR_CODES;

/*
+-----------------------------------------------------------------------------+
| Enum definitions                                                            |
+-----------------------------------------------------------------------------+
*/
typedef enum _e_cnc_task_id
{
  CNC_TASK_UNDEFINED = 0,
  CNC_TASK_IPO = 1,
  CNC_TASK_MIN = CNC_TASK_IPO,
  CNC_TASK_DEC = 2,
  CNC_TASK_HMI = 3,
  CNC_TASK_MAX = CNC_TASK_HMI

} E_CNC_TASK_ID;

typedef enum _cnc_object_data_type
{
  CNC_OBJECT_TYPE_NONE = 0,
  CNC_OBJECT_TYPE_BOOLEAN,
  CNC_OBJECT_TYPE_UNS08,
  CNC_OBJECT_TYPE_SGN08,
  CNC_OBJECT_TYPE_UNS16,
  CNC_OBJECT_TYPE_SGN16,
  CNC_OBJECT_TYPE_UNS32,
  CNC_OBJECT_TYPE_SGN32,
  CNC_OBJECT_TYPE_UNS64,
  CNC_OBJECT_TYPE_SGN64,
  CNC_OBJECT_TYPE_REAL64,
  CNC_OBJECT_TYPE_STRUCT,
  CNC_OBJECT_TYPE_REAL32,
  CNC_OBJECT_TYPE_CHAR,
  CNC_OBJECT_TYPE_STRING,
  CNC_OBJECT_TYPE_ERROR = 99

} E_CNC_OBJECT_DATA_TYPE;

typedef enum _e_cnc_object_domain
{
  CNC_NONE = 0,
  CNC_PLATFORM,
  CNC_CHANNEL,
  CNC_AXIS,
  CNC_SAI

} E_CNC_OBJECT_DOMAIN;

typedef enum _e_log_msg_class
{
  CNC_LOG_INFO = 0, // #MSG
  CNC_LOG_WARNING,
  CNC_LOG_ERROR,
  CNC_LOG_EXCEPTION,
  CNC_LOG_DEBUG

} E_CNC_LOG_MSG_CLASS;

typedef enum _e_serc_element_states
{
  SOE_ELEMENT_STATE_DATASTATE =                            0x0000,
  SOE_ELEMENT_STATE_NAME      =                            0x0010,
  SOE_ELEMENT_STATE_ATTRIBUTE =                            0x0018,
  SOE_ELEMENT_STATE_UNIT      =                            0x0020,
  SOE_ELEMENT_STATE_MIN       =                            0x0028,
  SOE_ELEMENT_STATE_MAX       =                            0x0030,
  SOE_ELEMENT_STATE_VALUE     =                            0x0038

} E_SERC_ELEMENT_STATE;

/*
+-----------------------------------------------------------------------------+
| Structure definitions                                                       |
+-----------------------------------------------------------------------------+
*/
typedef struct _cnc_object_id
{
  E_CNC_TASK_ID             iThread;
  uint32_t                  iGroup;
  uint32_t                  iOffset;
} CNC_OBJECT_ID;

typedef struct _cnc_object
{
  CNC_OBJECT_ID             ObjectID;
  char                      Name[256];
  E_CNC_OBJECT_DATA_TYPE    Type;
  uint32_t                  Length;
  char                      Unit[128];
  bool                      Writeable;
} CNC_OBJECT;

typedef sem_t              *SDK_HSEM;

/*
+-----------------------------------------------------------------------------+
| Function prototypes                                                         |
+-----------------------------------------------------------------------------+
*/
#ifdef __cplusplus
extern "C" {
#endif

  char                      *cnc_get_version                               (void);
  uint32_t                   cnc_get_max_channels                          (void);
  uint32_t                   cnc_get_max_axes                              (void);

  int32_t                    cnc_pre_initialize                            (void);                           /* Deprecated: Is now called implicitly during startup. */
  int32_t                    cnc_startup                                   (char *cnc_startup_file);
  int32_t                    cnc_shutdown                                  (void);
  int32_t                    cnc_schedule_threads                          (void);

  int32_t                    cnc_register_schedule_trigger                 (void *hTriggerCnc);              /* Deprecated: use cnc_register_sync_trigger instead */
  int32_t                    cnc_register_sync_trigger                     (SDK_HSEM hTriggerCnc);

  int32_t                    cnc_register_hli_platform                     (void *hli_platform);
  int32_t                    cnc_register_hli_channel                      (uint32_t index, void *hli_channel);
  int32_t                    cnc_register_hli_axis                         (uint32_t index, void *hli_axis);

  int32_t                    cnc_register_drive_command_interface          (uint32_t index, void *drive_command, uint32_t length);
  int32_t                    cnc_register_drive_feedback_interface         (uint32_t index, void *drive_feedback, uint32_t length);
  int32_t                    cnc_register_external_variables_platform      (void *p_memory, uint32_t length);
  int32_t                    cnc_register_external_variables_channel       (uint32_t index, void *p_memory, uint32_t length);
  int32_t                    cnc_register_function                         (int32_t(*p_function)(uint32_t), char* name);
  int32_t                    (*cnc_lookup_function(char* name))            (uint32_t);

  size_t                     cnc_get_sizeof_hli_platform                   (void);
  size_t                     cnc_get_sizeof_hli_channel                    (void);
  size_t                     cnc_get_sizeof_hli_axis                       (void);

  int32_t                    cnc_create_plc_interfaces                     (uint32_t number_of_channels, uint32_t number_of_axes);
  int32_t                    cnc_connect_plc_interfaces                    (uint32_t number_of_channels, uint32_t number_of_axes);
  int32_t                    cnc_delete_plc_interfaces                     (uint32_t number_of_channels, uint32_t number_of_axes);

  /* Access to cnc objects */
  int32_t                    cnc_query_number_of_objects                   (void);
  int32_t                    cnc_query_object                              (uint32_t index, CNC_OBJECT* cnc_object);

  E_CNC_OBJECT_DOMAIN        cnc_query_object_domain                       (CNC_OBJECT_ID object_id);
  E_CNC_OBJECT_DATA_TYPE     cnc_get_object_data_type                      (CNC_OBJECT_ID cnc_object_id);

  /* Read/write of cnc objects */
  int32_t                    cnc_read_value                                (CNC_OBJECT_ID cnc_object_id, void* value, uint32_t length);
  int32_t                    cnc_write_value                               (CNC_OBJECT_ID cnc_object_id, void* value, uint32_t length);
  int32_t                    cnc_read_write_value                          (CNC_OBJECT_ID cnc_object_id, void* value, uint32_t read_length, uint32_t write_length);

  /* Data type specific acces read/write functions to cnc objects */
  int32_t                    cnc_read_BOOL                                 (CNC_OBJECT_ID cnc_object_id, bool *value);
  int32_t                    cnc_write_BOOL                                (CNC_OBJECT_ID cnc_object_id, bool  value);

  int32_t                    cnc_read_CHAR                                 (CNC_OBJECT_ID cnc_object_id, char *value);
  int32_t                    cnc_write_CHAR                                (CNC_OBJECT_ID cnc_object_id, char  value);

  int32_t                    cnc_read_SGN08                                (CNC_OBJECT_ID cnc_object_id, int8_t *value);
  int32_t                    cnc_write_SGN08                               (CNC_OBJECT_ID cnc_object_id, int8_t  value);

  int32_t                    cnc_read_UNS08                                (CNC_OBJECT_ID cnc_object_id, uint8_t *value);
  int32_t                    cnc_write_UNS08                               (CNC_OBJECT_ID cnc_object_id, uint8_t  value);

  int32_t                    cnc_read_SGN16                                (CNC_OBJECT_ID cnc_object_id, int16_t *value);
  int32_t                    cnc_write_SGN16                               (CNC_OBJECT_ID cnc_object_id, int16_t  value);

  int32_t                    cnc_read_UNS16                                (CNC_OBJECT_ID cnc_object_id, uint16_t *value);
  int32_t                    cnc_write_UNS16                               (CNC_OBJECT_ID cnc_object_id, uint16_t  value);

  int32_t                    cnc_read_SGN32                                (CNC_OBJECT_ID cnc_object_id, int32_t *value);
  int32_t                    cnc_write_SGN32                               (CNC_OBJECT_ID cnc_object_id, int32_t  value);

  int32_t                    cnc_read_UNS32                                (CNC_OBJECT_ID cnc_object_id, uint32_t *value);
  int32_t                    cnc_write_UNS32                               (CNC_OBJECT_ID cnc_object_id, uint32_t  value);

  int32_t                    cnc_read_REAL64                               (CNC_OBJECT_ID cnc_object_id, double *value);
  int32_t                    cnc_write_REAL64                              (CNC_OBJECT_ID cnc_object_id, double value);

  int32_t                    cnc_read_SGN64                                (CNC_OBJECT_ID cnc_object_id, int64_t *value);
  int32_t                    cnc_write_SGN64                               (CNC_OBJECT_ID cnc_object_id, int64_t  value);

  int32_t                    cnc_read_UNS64                                (CNC_OBJECT_ID cnc_object_id, uint64_t *value);
  int32_t                    cnc_write_UNS64                               (CNC_OBJECT_ID cnc_object_id, uint64_t  value);

  int32_t                    cnc_read_STRING                               (CNC_OBJECT_ID cnc_object_id, char *value, uint32_t length);
  int32_t                    cnc_write_STRING                              (CNC_OBJECT_ID cnc_object_id, char *value, uint32_t length);

  /* CNC trigger object - write only */
  int32_t                    cnc_TRIGGER                                   (CNC_OBJECT_ID cnc_object_id);

  /* Conversion functions */
  E_CNC_OBJECT_DATA_TYPE     type_of_string                                (char* type_string);
  char                      *string_of_type                                (E_CNC_OBJECT_DATA_TYPE cnc_object_type);


  /* Logging functions */
  int32_t                    cnc_register_log_message_function             (int32_t(*p_function)(E_CNC_LOG_MSG_CLASS log_msg_class, uint32_t log_msg_id, const char* log_msg_string));
  int32_t                    cnc_register_log_message_json_function        (int32_t(*p_json_func)(const char* error_desc_js_stream));

  /* Configuration functions */
  void                       cnc_set_verbose_mode                          (bool verbose);
  void                       cnc_set_tcp_object_access                     (bool tcp_access);

  void                       cnc_set_cycle_time                            (uint32_t time_in_us);

  /* Scheduling functions */
  int16_t                    cnc_create_semaphore                          (SDK_HSEM* hsem, char* name);
  int16_t                    cnc_give_semaphore                            (SDK_HSEM  hsem);
  int16_t                    cnc_delete_semaphore                          (SDK_HSEM  hsem);

  /* Deprecated: use cnc_*_semaphore() functions instead */
//  signed short              isg_create_counting_semaphore                 (SDK_HSEM* hSem, unsigned int maxCount, unsigned int initialCount, char* name);
//  signed short              isg_give_counting_semaphore                   (SDK_HSEM  hSem);
//  signed short              isg_delete_counting_semaphore                 (SDK_HSEM  hSem);

  /* Diagnosis functions */
  void                       cnc_set_startup_error_log                     (bool  startup_error_log);
  void                       cnc_startup_error_log_file                    (char* log_file_name);

  /* sercos service channel interface functions */
  int32_t                    cnc_register_serc_read_req_function           (int32_t(*p_function)(uint32_t invokeId, int32_t logAxisNo, uint32_t sercId, uint32_t sercElement, uint32_t size));
  int32_t                    cnc_register_serc_write_req_function          (int32_t(*p_function)(uint32_t invokeId, int32_t logAxisNo, uint32_t sercId, uint32_t sercElement, uint32_t size, void* p_data));
  void                       cnc_serc_service_channel_write_res            (uint32_t invokeId, uint32_t result);
  void                       cnc_serc_service_channel_read_res             (uint32_t invokeId, uint32_t result, uint32_t size, void* pData);

  /* conopen service data objects interfance functions */
  int32_t                    cnc_register_canopen_read_req_function        (int32_t(*p_function)(uint32_t invokeId, int32_t logAxisNo, uint32_t object_index, uint32_t object_sub_index, uint32_t size));
  int32_t                    cnc_register_canopen_write_req_function       (int32_t(*p_function)(uint32_t invokeId, int32_t logAxisNo, uint32_t object_index, uint32_t object_sub_index, uint32_t size, void* value) );
  void                       cnc_canopen_sdo_write_res                     (uint32_t invokeId, uint32_t result);
  void                       cnc_canopen_sdo_read_res                      (uint32_t invokeId, uint32_t result, uint32_t size, void* pData);


  /* xml library functions */
  int32_t                    cnc_set_xml_parser_path                       (char* libxml2_path);

  /* Task functions for external scheduling */
  int32_t                    cnc_task_ipo                                  (uint32_t core_context);
  int32_t                    cnc_task_dec                                  (uint32_t core_context);
  int32_t                    cnc_task_hmi                                  (uint32_t core_context);
  int32_t                    cnc_task_sys                                  (uint32_t core_context);
  int32_t                    cnc_task_tcp                                  (uint32_t core_context);

#ifdef __cplusplus
}
#endif

#endif /* CNC_OS_IFC_INC_DEFINED */
