/*
Copyright (C)
ISG Industrielle Steuerungstechnik GmbH
https://www.isg-stuttgart.de
*/

/** \file
 * @brief TODO-DOC: Module description missing
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#include "cnc_os_ifc.h"
#include "cnc_demo_drive_simulation.h"
#include "cnc_demo_self_test.h"
#include "cnc_demo_samples.h"

/*
+----------------------------------------------------------------------------+
| cnc_demo_self_test.c                                  | Copyright    ISG   |
|                                                       |                    |
|                                                       | Gropiusplatz 10    |
|                                                       | 70563 Stuttgart    |
+-------------------------------------------------------+--------------------+
| CNC SDK Demo self test.                                                    |
|                                                                            |
| This function allows to automate SDK functions in a certain order.         |
+----------------------------------------------------------------------------+
*/

/*
+----------------------------------------------------------------------------+
| Self test function                                                         |
+----------------------------------------------------------------------------+
*/
int cnc_demo_self_test(TEST_PROCEDURE* test_run, TEST_CONFIGURATION test_configuration, void* p_drive_command[CNC_OS_IFC_NO_AXES_MAX], void* p_drive_feedback[CNC_OS_IFC_NO_AXES_MAX])
{
  unsigned int    i_drive;
  unsigned int    wait_cycles = 0;
  int             ret_val = ERR_CNC_NOERROR;

  bool            f_test_ready = false;
  bool            f_test_error = false;

  /*
  +----------------------------------------------------------------------------+
  | Get first test step                                                        |
  +----------------------------------------------------------------------------+
  */
  TEST_STATES     e_test_state = test_run->ts_test_state;
  TEST_STATES     e_next_test_state = test_run->ts_next_state;
  unsigned int    max_wait_cycles = test_run->max_wait_cycles;

  printf("\nCNC-MSG: Starting CncSDKDemo Self Test...\n");
  printf("CNC-MSG: --------------------------------\n");

  while (f_test_ready == false)
  {

    if (f_test_error == true)
    {
      e_test_state = TS_ERROR;
    }

    switch (e_test_state)
    {
      /* ------------------------- */
      case TS_DRIVES_ON:
      {
        if (cnc_demo_enable_disable_drives(true) == true)
        {
          printf("CNC-MSG: Drives switched on.\n");
  
          test_run++;
          wait_cycles = 0;
          max_wait_cycles = test_run->max_wait_cycles;
          e_test_state = test_run->ts_test_state;
          e_next_test_state = test_run->ts_next_state;
        }
        else
        {
          printf("CNC-ERROR: Drives could not switched on.\n");
  
          wait_cycles = 0;
          max_wait_cycles = 50;
          e_test_state = TS_WAIT_CYCLES;
          e_next_test_state = TS_ERROR;
        }
  
        break;
      }
      /* ------------------------- */
      case TS_DRIVES_OFF:
      {
        if (cnc_demo_enable_disable_drives(false) == true)
        {
          printf("CNC-MSG: Drives switched off.\n");
  
          test_run++;
          wait_cycles = 0;
          max_wait_cycles = test_run->max_wait_cycles;
          e_test_state = test_run->ts_test_state;
          e_next_test_state = test_run->ts_next_state;
        }
        else
        {
          printf("CNC-ERROR: Drives could not switched off.\n");
  
          wait_cycles = 0;
          max_wait_cycles = 50;
          e_test_state = TS_WAIT_CYCLES;
          e_next_test_state = TS_ERROR;
        }
  
        break;
      }
      /* ------------------------- */
      case TS_RESET:
      {
        if (cnc_demo_reset() == true)
        {
          printf("CNC-MSG: CNC Reset.\n");
  
          test_run++;
          wait_cycles = 0;
          max_wait_cycles = test_run->max_wait_cycles;
          e_test_state = test_run->ts_test_state;
          e_next_test_state = test_run->ts_next_state;
        }
        else
        {
          printf("CNC-ERROR: Reset not successful.\n");
  
          wait_cycles = 0;
          max_wait_cycles = 50;
          e_test_state = TS_WAIT_CYCLES;
          e_next_test_state = TS_ERROR;
        }
  
        break;
      }
      /* ------------------------- */
      case TS_START_NC_PROGRAM:
      {
        if (cnc_demo_start_nc_program(test_run->common_string) == true)
        {
          printf("CNC-MSG: Started nc program '%s'.\n", test_run->common_string);
  
          test_run++;
          wait_cycles = 0;
          max_wait_cycles = test_run->max_wait_cycles;
          e_test_state = test_run->ts_test_state;
          e_next_test_state = test_run->ts_next_state;
        }
        else
        {
          printf("CNC-ERROR: Error while starting nc program '%s'.\n", test_run->common_string);
  
          wait_cycles = 0;
          max_wait_cycles = 50;
          e_test_state = TS_WAIT_CYCLES;
          e_next_test_state = TS_ERROR;
        }
  
        break;
      }
      /* ------------------------- */
      case TS_MANUAL_DATA_INPUT:
      {
        if (cnc_demo_start_manual_data_input(test_run->common_string) == true)
        {
          printf("CNC-MSG: Started MDI '%s'.\n", test_run->common_string);
  
          test_run++;
          wait_cycles = 0;
          max_wait_cycles = test_run->max_wait_cycles;
          e_test_state = test_run->ts_test_state;
          e_next_test_state = test_run->ts_next_state;
        }
        else
        {
          printf("CNC-ERROR: Error while starting MDI.\n");
  
          wait_cycles = 0;
          max_wait_cycles = 50;
          e_test_state = TS_WAIT_CYCLES;
          e_next_test_state = TS_ERROR;
        }
  
        break;
      }
      /* ------------------------- */
      case TS_SHOW_POSITIONS:
      {
        cnc_demo_show_positions();
  
        test_run++;
        wait_cycles = 0;
        max_wait_cycles = test_run->max_wait_cycles;
        e_test_state = test_run->ts_test_state;
        e_next_test_state = test_run->ts_next_state;
  
        break;
      }
      /* ------------------------- */
      case TS_RESET_RUNTIME_STATISTICS:
      {
        cnc_demo_reset_runtime_statistic();
  
        test_run++;
        wait_cycles = 0;
        max_wait_cycles = test_run->max_wait_cycles;
        e_test_state = test_run->ts_test_state;
        e_next_test_state = test_run->ts_next_state;
  
        break;
      }
      /* ------------------------- */
      case TS_SHOW_RUNTIME_STATISTICS:
      {
        cnc_demo_print_runtime_statistic();
  
        test_run++;
        wait_cycles = 0;
        max_wait_cycles = test_run->max_wait_cycles;
        e_test_state = test_run->ts_test_state;
        e_next_test_state = test_run->ts_next_state;
  
        break;
      }
      /* ------------------------- */
      case TS_WAIT_CYCLES:
      {
        if (wait_cycles >= max_wait_cycles)
        {
          e_test_state = e_next_test_state;
        }
        else
        {
          wait_cycles++;
        }
        break;
      }
      /* ------------------------- */
      case TS_PRINT:
      {
        printf("%s\n", test_run->common_string);
  
        test_run++;
        wait_cycles = 0;
        max_wait_cycles = test_run->max_wait_cycles;
        e_test_state = test_run->ts_test_state;
        e_next_test_state = test_run->ts_next_state;
  
        break;
      }
      /* ------------------------- */
      case TS_ERROR:
      {
        printf("CNC-ERROR: Terminating CncSDKDemo self test.\n");
  
        e_test_state = TS_TEST_READY;
        f_test_ready = true;
        break;
      }
      /* ------------------------- */
      case TS_TEST_READY:
      {
        f_test_ready = true;
        break;
      }
      /* ------------------------- */
      default:
      {
        break;
      }
    }

    /*
    +------------------------------------------------------------------------+
    | Simulation of drive interface                                          |
    +------------------------------------------------------------------------+
    */
    for (i_drive = 0; i_drive < test_configuration.NumberOfAxes; i_drive++)
    {
      if (test_configuration.f_use_sercos_drive_interface)
      {
        cnc_demo_drive_ifc_simulation_soe(i_drive,  (CNC_DRIVE_COMMAND_SOE*) p_drive_command[i_drive],  (CNC_DRIVE_STATUS_SOE*) p_drive_feedback[i_drive]);
      }
      else
      {
        cnc_demo_drive_ifc_simulation_coe(i_drive,  (CNC_DRIVE_COMMAND_COE*) p_drive_command[i_drive],  (CNC_DRIVE_STATUS_COE*) p_drive_feedback[i_drive]);
      }
    }

    usleep(1000);
    f_test_error = cnc_demo_error_check();


  } /* while() */

  return ret_val;
} /* End of cnc_demo_self_test() */
