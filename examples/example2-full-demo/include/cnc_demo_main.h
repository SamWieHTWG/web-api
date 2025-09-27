/*
Copyright (C)
ISG Industrielle Steuerungstechnik GmbH
https://www.isg-stuttgart.de
*/

/** \file
 * @brief TODO-DOC: Module description missing
*/

#ifndef CNC_KERNEL_DEMO_MAIN_H
#define CNC_KERNEL_DEMO_MAIN_H

#include "cnc_os_ifc.h"
#include "cnc_tool_ifc.h"

/*
+----------------------------------------------------------------------------+
| cnc_demo_main.h                                       | Copyright    ISG   |
|                                                       |                    |
|                                                       | Gropiusplatz 10    |
|                                                       | 70563 Stuttgart    |
+-------------------------------------------------------+--------------------+
| CNC SDK Demo main application header file.                                 |
+----------------------------------------------------------------------------+
*/

/*
+----------------------------------------------------------------------------+
| Structures                                                                 |
+----------------------------------------------------------------------------+
*/
typedef struct _cli_description
{
  char             option[64];
  char             text[64];
  void            *parameter;
  char             format[10];
  bool            *flag;
  bool             value;

} CLI_DESCRIPTION;

/*
+----------------------------------------------------------------------------+
| Function prototypes                                                        |
+----------------------------------------------------------------------------+
*/
/* Logging function */
int  cnc_log_message                  ( E_CNC_LOG_MSG_CLASS log_msg_class, unsigned int log_msg_id, const char* log_msg_string);
int  cnc_log_message_json             ( const char* json_stream);

void cnc_kernel_trigger               ( void );
void cnc_create_kernel_trigger_thread ( void );

#endif /* CNC_KERNEL_DEMO_MAIN_H */
