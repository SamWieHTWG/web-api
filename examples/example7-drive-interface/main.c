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
#include "cnc_demo.h"
#include "cnc_demo_samples.h"
#include "keyboard_helper.h"
#include "cnc_demo_drive_simulation.h"
#include "cli_flags.h"

/* Information of configuration from platform provider */
static uint16_t NumberOfChannels = CNC_OS_IFC_NO_CHANNEL_MAX;
static uint16_t NumberOfAxes     = CNC_OS_IFC_NO_AXES_MAX;

/* External and global defined variables */
bool f_cnc_shutdown = false;
bool f_use_sercos = false;

/*
+----------------------------------------------------------------------------+
| Main                                                                       |
+----------------------------------------------------------------------------+
*/
int main(int argc, char *argv[])
{
  int ret_value = ERR_CNC_NOERROR;

  if(cli_has_flag(argc,argv,"--sercos")|| cli_has_flag(argc,argv,"-s"))
  {
    f_use_sercos = true;
  }

  /*
  +----------------------------------------------------------------------------+
  | Initialize sdk demo                                                        |
  +----------------------------------------------------------------------------+
  */

  /* Set cnc cycle time */
  cnc_set_cycle_time(1000);

  /*
  +--------------------------------------------------------------------------+
  | Allocate and register memory for drive interface                         |
  +--------------------------------------------------------------------------+
  */
  void *p_drive_command[CNC_OS_IFC_NO_AXES_MAX];
  void *p_drive_feedback[CNC_OS_IFC_NO_AXES_MAX];

  short size_feedback_telegram = f_use_sercos ? sizeof(CNC_DRIVE_STATUS_SOE) : sizeof(CNC_DRIVE_STATUS_COE);
  short size_command_telegram = f_use_sercos ? sizeof(CNC_DRIVE_COMMAND_SOE) : sizeof(CNC_DRIVE_COMMAND_COE);

  for (int i_drive = 0; i_drive < NumberOfAxes; i_drive++)
  {
    p_drive_command[i_drive] = malloc(size_command_telegram);
    cnc_register_drive_command_interface(i_drive, p_drive_command[i_drive], size_command_telegram);
    memset(p_drive_command[i_drive], 0, size_command_telegram);

    p_drive_feedback[i_drive] = malloc(size_feedback_telegram);
    cnc_register_drive_feedback_interface(i_drive, p_drive_feedback[i_drive], size_feedback_telegram);
    memset(p_drive_feedback[i_drive], 0, size_feedback_telegram);
  }

  /* register callbacks for service channel requests */
  if (f_use_sercos)
  {
    /* register callbacks for sercos service channel */
    cnc_register_serc_write_req_function(cnc_demo_sercos_asynch_callback_write_req);
    cnc_register_serc_read_req_function(cnc_demo_sercos_asynch_callback_read_req);
  }
  else
  {
    /* register callbacks for canopen SDO access */
    cnc_register_canopen_write_req_function(cnc_demo_canopen_asynch_callback_write_req);
    cnc_register_canopen_read_req_function(cnc_demo_canopen_asynch_callback_read_req);
  }

  /*
  +--------------------------------------------------------------------------+
  | CNC start-up                                                             |
  +--------------------------------------------------------------------------+
  */
  char startupfile[PATH_FILENAME_LENGTH];
  if(f_use_sercos)
    sprintf(startupfile, "./listen/startup-SoE.lis");
  else
    sprintf(startupfile, "./listen/startup-CoE.lis");

  ret_value = cnc_startup(startupfile);
  if (ret_value != ERR_CNC_NOERROR)
    exit(ret_value);

  /* Set TCP object access */
  cnc_set_tcp_object_access(true);

  /* Create menu thread */
  unsigned int menu_activition_bitmap =   MENU_ENABLE_DRIVES | MENU_RESET_KERNEL |
    MENU_SHOW_POS | MENU_START_PROGRAM | MENU_MANUAL_INPUT | MENU_SHUTDOWN;
  cnc_demo_create_menu_thread(menu_activition_bitmap);

  /*
  +--------------------------------------------------------------------------+
  | Running state                                                            |
  | CNC is operating in cyclic mode.                                         |
  +--------------------------------------------------------------------------+
  */
  uint32_t iteration = 0;
  while (f_cnc_shutdown == false)
  {
    /* periodic call of cnc tasks */
    cnc_task_ipo(0);
    cnc_task_dec(0);
    cnc_task_hmi(0);

    /*
    +------------------------------------------------------------------------+
    | Simulation of drive interface                                          |
    +------------------------------------------------------------------------+
    */
    for (int i_drive = 0; i_drive < NumberOfAxes; i_drive++)
    {
      if (f_use_sercos)
        cnc_demo_drive_ifc_simulation_soe(i_drive, (CNC_DRIVE_COMMAND_SOE *)p_drive_command[i_drive], (CNC_DRIVE_STATUS_SOE *)p_drive_feedback[i_drive]);
      else
        cnc_demo_drive_ifc_simulation_coe(i_drive, (CNC_DRIVE_COMMAND_COE *)p_drive_command[i_drive], (CNC_DRIVE_STATUS_COE *)p_drive_feedback[i_drive]);
    }

    /* These tasks are only required if the test HMI (ahmi.exe) is used. */
    cnc_task_sys(0);
    cnc_task_tcp(0);

    if (iteration == 5000)
    {
      /* After 5 seconds, a drive error is simulated. */
      /* The error will be reset via the service channel */
      /* after a CNC reset is triggered. */
      cnc_demo_set_drive_err_simu(true);
      printf("CNC-MSG: Drive error simulation activated.\n");
    }

    usleep(1000); // [1ms]
    iteration++;
  } /* End of kernel loop */

  cnc_shutdown();

  return ret_value;
} /* End main() */
