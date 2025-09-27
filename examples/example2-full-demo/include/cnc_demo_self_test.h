/*
Copyright (C)
ISG Industrielle Steuerungstechnik GmbH
https://www.isg-stuttgart.de
*/

/** \file
 * @brief TODO-DOC: Module description missing
*/
#ifndef CNC_DEMO_SELF_TEST_H
#define CNC_DEMO_SELF_TEST_H

#include "cnc_demo.h"

/*
+----------------------------------------------------------------------------+
| cnc_demo_self.test.h                                  | Copyright    ISG   |
|                                                       |                    |
|                                                       | Gropiusplatz 10    |
|                                                       | 70563 Stuttgart    |
+-------------------------------------------------------+--------------------+
| CNC SDK Demo self test header file.                                        |
|                                                                            |
| This function allows to automate SDK functions in a certain order.         |
+----------------------------------------------------------------------------+
*/

/*
+----------------------------------------------------------------------------+
| Test states Enum                                                           |
+----------------------------------------------------------------------------+
*/
typedef enum _test_states
{
  TS_NONE = 0,
  TS_DRIVES_ON = 1,
  TS_DRIVES_OFF,
  TS_RESET,
  TS_START_NC_PROGRAM,
  TS_STOP_NC_PROGRAM,
  TS_RESUME_NC_PROGRAM,
  TS_MANUAL_DATA_INPUT,
  TS_SHOW_POSITIONS,
  TS_WAIT_CYCLES,
  TS_ZERO_POSITION,
  TS_RESET_RUNTIME_STATISTICS,
  TS_SHOW_RUNTIME_STATISTICS,
  TS_PRINT,
  TS_ERROR,
  TS_TEST_READY,

  TS_TEST_END = 9999

} TEST_STATES;

/*
+----------------------------------------------------------------------------+
| Test process structure                                                     |
+----------------------------------------------------------------------------+
*/
typedef struct _test_procedure
{
  TEST_STATES       ts_test_state;
  TEST_STATES       ts_next_state;
  uint16_t          max_wait_cycles;
  char              common_string[PATH_FILENAME_LENGTH];

} TEST_PROCEDURE;

typedef struct _test_configuration
{
  uint16_t            NumberOfChannels;
  uint16_t            NumberOfAxes;
  bool                f_use_sercos_drive_interface;
  void*               p_drive_command[CNC_OS_IFC_NO_AXES_MAX];
  void*               p_drive_feedback[CNC_OS_IFC_NO_AXES_MAX];

} TEST_CONFIGURATION;

/*
+----------------------------------------------------------------------------+
| Function prototypes                                                        |
+----------------------------------------------------------------------------+
*/
int cnc_demo_self_test(TEST_PROCEDURE* test_run, TEST_CONFIGURATION test_configuration, void* p_drive_command[CNC_OS_IFC_NO_AXES_MAX], void* p_drive_feedback[CNC_OS_IFC_NO_AXES_MAX]);


#endif /* CNC_DEMO_SELF_TEST_H */
