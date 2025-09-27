/*
Copyright (C)
ISG Industrielle Steuerungstechnik GmbH
https://www.isg-stuttgart.de
*/

/** \file
 * @brief TODO-DOC: Module description missing
*/
#ifndef CNC_DEMO_SAMPLES_H
#define CNC_DEMO_SAMPLES_H

#include "cnc_demo.h"
#include "cnc_os_ifc.h"
#include "cnc_tool_ifc.h"

/*
+----------------------------------------------------------------------------+
| cnc_demo_samples.h                                    | Copyright    ISG   |
|                                                       |                    |
|                                                       | Gropiusplatz 10    |
|                                                       | 70563 Stuttgart    |
+-------------------------------------------------------+--------------------+
| CNC SDK Demo sample functions header file.                                 |
|                                                                            |
| Various sample functions to demonstrate who the SDK may be used.           |
+----------------------------------------------------------------------------+
*/

/*
+-----------------------------------------------------------------------------+
| Structure definitions                                                       |
+-----------------------------------------------------------------------------+
*/
typedef struct _axis_id
{
  uint32_t      n_ax;
  uint16_t      ax_id[CNC_OS_IFC_NO_AXES_MAX];
} AXIS_ID;

typedef struct _axis_nameS
{
  uint32_t      n_ax;
  char          ax_name[CNC_OS_IFC_NO_AXES_MAX][16];
} AXIS_NAMES;

typedef struct _axis_pos
{
  uint32_t      n_ax;
  int32_t       pos_ax[CNC_OS_IFC_NO_AXES_MAX];
} AXIS_POS;

typedef struct _mcm_mode_state
{
  int32_t       mode;
  int32_t       state;
} MCM_MODE_STATE;

/*
+-----------------------------------------------------------------------------+
| Defines                                                                     |
+-----------------------------------------------------------------------------+
*/
#define MCM_MODE_STANDBY        1
#define MCM_MODE_AUTOMATIC      2
#define MCM_MODE_MDI            3

#define MCM_STATE_DESELECT      1
#define MCM_STATE_SELECTED      2
#define MCM_STATE_READY         3
#define MCM_STATE_ACTIVE        4
#define MCM_STATE_HOLD          5
#define MCM_STATE_ERROR         6

#define MAX_BUF_SIZE            1208
#define ERROR_MSG_SIZE          1400

#define NUMBER_EXT_TOOL_DATA    100

#define CHAN_AXMAX_FULL         32

#define WAIT_TICKS_SLEEP_US     1000

/*
+-----------------------------------------------------------------------------+
| Cnc objects constants                                                       |
+-----------------------------------------------------------------------------+
*/
/* iGroup  */
#define CNC_IGRP_AC_AXIS         0x20200
#define CNC_IGRP_MCM_COMMAND     0x20101
#define CNC_IGRP_TICK_COUNTER    0x20300

/* iOffset */
#define CNC_IOFF_TICK_COUNTER    0x7
#define MC_ERROR_MESSAGE         0x26

#define AC_X_DRIVE_ON            0x33
#define AC_X_TORQUE_PERMISSION   0x34
#define AC_X_RELEASE_FEEDHOLD    0x35

#define AC_X_AXES_ACT_POS_ACS    0x1100
#define AC_X_AXES_CUR_POS_ACS    0x1101
#define AC_X_AXES_NAMES          0x1103
#define AC_X_AXES_IDS            0x110A

#define MC_PROGRAM_STREAM        0x90

#define MCM_ACTIVE               0x103
#define MCM_COMMAND_TO_MODE      0x104
#define MCM_COMMAND_TO_STATE     0x105
#define MCM_COMMAND_FROM_MODE    0x106
#define MCM_COMMAND_FROM_STATE   0x107
#define MCM_COMMAND_PARAMETER    0x108

#define MCM_STANDBY_RESET        0x11D
#define MCM_AUTOMATIC_RESET      0x13C
#define MCM_MDI_RESET            0x155

#define MCM_AUTOMATIC_STOP       0x136
#define MCM_AUTOMATIC_RESUME     0x137

#define	ERR_OBJ_ACCESS_BUSY      0x0708

#define MENU_PRINT_RUNTIME_STATISTICS     (1 << 0)
#define MENU_RESET_RUNTIME_STATISTICS     (1 << 1)
#define MENU_WRITE_CSV                    (1 << 2)
#define MENU_ENABLE_DRIVES                (1 << 3)
#define MENU_ERROR_MESSAGES               (1 << 4)
#define MENU_STOP_RESUME_PROGRAM          (1 << 5)
#define MENU_LOGGING                      (1 << 6)
#define MENU_GENERATE_DIAG_DATA           (1 << 7)
#define MENU_SHOW_CNC_OBJECTS             (1 << 8)
#define MENU_SHOW_POS                     (1 << 9)
#define MENU_SHUTDOWN                     (1 << 10)
#define MENU_RESET_KERNEL                 (1 << 11)
#define MENU_START_PROGRAM                (1 << 12)
#define MENU_MANUAL_INPUT                 (1 << 13)
#define MENU_UPDATE_CONFIG                (1 << 14)
#define MENU_VERBOSE                      (1 << 15)
#define MENU_STREAMING_MODE               (1 << 16)

/*
+----------------------------------------------------------------------------+
| External Tool Management                                                   |
+----------------------------------------------------------------------------+
*/
int cnc_demo_tool_change_info   ( const CNC_TOOL_DESC *pToolData );
int cnc_demo_tool_read_data     ( CNC_TOOL_DESC* pToolData );
int cnc_demo_tool_write_data    ( const CNC_TOOL_DATA_IN* pToolDataIn );

/*
+-----------------------------------------------------------------------------+
| Demo/sample functions                                                       |
+-----------------------------------------------------------------------------+
*/
bool         cnc_demo_object_access           (void);
bool         cnc_demo_enable_disable_drives   ( bool f_on );
bool         cnc_demo_start_nc_program        ( char file_name[256] );
bool         cnc_demo_start_manual_data_input ( char str_mdi[256] );
bool         cnc_demo_reset                   ( void );
bool         cnc_demo_stop_resume_nc_program  ( bool f_stop_resume );
bool         cnc_demo_streaming_program       ( FILE *p_nc_file, char *p_buf );
bool         cnc_demo_start_streaming_program ( FILE **p_file, char file_name[256] );
bool         cnc_demo_end_streaming_program   ( FILE **p_file );
AXIS_NAMES   cnc_demo_get_axis_names          ( void );
AXIS_POS     cnc_demo_get_active_positions    ( void );
AXIS_POS     cnc_demo_get_current_positions   ( void );
bool         cnc_demo_error_check             ( void );
void         cnc_demo_show_positions          ( void );
void         cnc_demo_wait_cnc_ticks          ( unsigned int cnc_ticks_to_wait);

void cnc_demo_print_runtime_statistic             ( void );
void cnc_demo_reset_runtime_statistic             ( void );
void cnc_demo_print_object_ids                    ( bool f_write_csv );
int  cnc_demo_set_libxml2_path                    (char* path);
int  cnc_demo_update_configuration(void);

/* Logging function */
int  cnc_demo_log_message                     ( E_CNC_LOG_MSG_CLASS log_msg_class, unsigned int log_msg_id, const char* log_msg_string );
int  cnc_demo_log_message_json                ( const char* json_stream );

/* cli menu functions */
void cnc_demo_read_cli_line                   ( char* buf, int maxlen );
void cnc_demo_create_menu_thread              ( unsigned int menu_bitmap );

/*
+----------------------------------------------------------------------------+
| Macros                                                                     |
+----------------------------------------------------------------------------+
*/
#define PRINT_CNC_OBJECT(n,obj) \
        printf("\n (%4d) \tname:      %s\n", n, obj.Name);                                                                      \
        printf("        \tobject id: [0x%05x, 0x%05x]\n", obj.ObjectID.iGroup, obj.ObjectID.iOffset);                           \
        printf("        \ttype:      %s\n", string_of_type(obj.Type));                                                          \
        printf("        \tlength:    %ld\n", obj.Length);                                                                       \
        printf("        \tunit:      %s\n", obj.Unit);                                                                          \
        printf("        \twriteable: %s\n", obj.Writeable?"true":"false");

#define WRITE_CNC_OBJECT_HEADER_TO_FILE                                                                                         \
        if (f_write_csv)                                                                                                        \
        {                                                                                                                       \
          csv_file = fopen("cnc_objects.csv", "w+");                                                                            \
          if (csv_file!=NULL)                                                                                                   \
          {                                                                                                                     \
            fprintf(csv_file, "Number;IndexGroup;IndexOffset;Description;Type;Length;Writable;Unit;Context;Task\n");            \
            fclose(csv_file);                                                                                                   \
          }                                                                                                                     \
        }

#define WRITE_CNC_OBJECT_TO_FILE(n,obj,domain,task)                                                                             \
        if (f_write_csv)                                                                                                        \
        {                                                                                                                       \
          csv_file = fopen("cnc_objects.csv", "a+");                                                                            \
          if (csv_file!=NULL)                                                                                                   \
          {                                                                                                                     \
            fprintf(csv_file, "%4d;0x%05x;0x%05x;%s;%s;%ld;%s;%s;%s;%s\n", n, obj.ObjectID.iGroup,                              \
                    obj.ObjectID.iOffset, obj.Name, string_of_type(obj.Type), obj.Length,                                       \
                    obj.Writeable?"true":"false", obj.Unit, domain, task);                                                      \
            fclose(csv_file);                                                                                                   \
          }                                                                                                                     \
        }


#endif /* CNC_DEMO_SAMPLES_H */
