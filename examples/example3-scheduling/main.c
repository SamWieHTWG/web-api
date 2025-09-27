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
#include "cli_flags.h"

/* Information of configuration from platform provider */
static uint16_t NumberOfChannels = CNC_OS_IFC_NO_CHANNEL_MAX;
static uint16_t NumberOfAxes     = CNC_OS_IFC_NO_AXES_MAX;

/* External and global defined variables */
bool   f_cnc_shutdown        = false;

/* Sheduling */
bool f_internal_scheduling = false;
SDK_HSEM hCncScheduleThreadsTrigger;

/*
+----------------------------------------------------------------------------+
| User functions                                                             |
+----------------------------------------------------------------------------+
*/
int cnc_user_event_ipo_pre(unsigned int context_info /*P-RTCF-00017*/)
{
  /* This function is called before each IPO task call.
  With external scheduling, it is called explicitly before the task call.
  For internal scheduling, it must be added in rtconf.lis and
  registered as a user function. It can be used to add user functionality. */
  return 0;
}
int cnc_user_event_ipo_post(unsigned int context_info /*P-RTCF-00017*/)
{
  /* This function is called after each IPO task call.
  With external scheduling, it is called explicitly before the task call.
  For internal scheduling, it must be added in rtconf.lis and
  registered as a user function. It can be used to add user functionality. */
  return 0;
}

/*
+----------------------------------------------------------------------------+
| Main                                                                       |
+----------------------------------------------------------------------------+
*/
int main(int argc, char *argv[])
{
  int ret_value = ERR_CNC_NOERROR;

  if(cli_has_flag(argc,argv,"--external")|| cli_has_flag(argc,argv,"-e"))
  {
    f_internal_scheduling = false;
  }

  /*
  +----------------------------------------------------------------------------+
  | Scheduling                                                                 |
  +----------------------------------------------------------------------------+
  */

  /* Set cnc cycle time */
  if(f_internal_scheduling == false)
  {
    cnc_set_cycle_time(1000);
  }

  /* Create and register semaphore for internal scheduling. */
  if (f_internal_scheduling == true)
  {
    if (0 != cnc_create_semaphore(&hCncScheduleThreadsTrigger, (char *)"CncSyncMutex"))
    {
      exit(-2);
    }
    if (0 != cnc_register_sync_trigger(hCncScheduleThreadsTrigger))
    {
      exit(-3);
    }
    /* register user function */
    cnc_register_function(cnc_user_event_ipo_pre,  (char*)"cnc_user_event_ipo_pre");
    cnc_register_function(cnc_user_event_ipo_post, (char*)"cnc_user_event_ipo_post");
  }

  /*
  +--------------------------------------------------------------------------+
  | CNC start-up                                                             |
  +--------------------------------------------------------------------------+
  */
  char startupfile[PATH_FILENAME_LENGTH];
  if (f_internal_scheduling)
    strcpy(startupfile, "./listen/startup-internal-sched.lis");
  else
    strcpy(startupfile, "./listen/startup-external-sched.lis");

  ret_value = cnc_startup(startupfile);
  if (ret_value != ERR_CNC_NOERROR)
    exit(ret_value);

  /* Set TCP object access */
  cnc_set_tcp_object_access(true);

  /* Create menu thread */
  unsigned int menu_activition_bitmap =   MENU_RESET_KERNEL | MENU_SHOW_POS |
    MENU_START_PROGRAM | MENU_MANUAL_INPUT | MENU_SHUTDOWN;
  cnc_demo_create_menu_thread(menu_activition_bitmap);

  /*
  +--------------------------------------------------------------------------+
  | Running state                                                            |
  | CNC is operating in cyclic mode.                                         |
  +--------------------------------------------------------------------------+
  */
  while (f_cnc_shutdown == false)
  {
    if (f_internal_scheduling == false)
    {
      /* periodic call of cnc tasks */
      cnc_user_event_ipo_pre(0);
      cnc_task_ipo(0);
      cnc_user_event_ipo_post(0);
      cnc_task_dec(0);
      cnc_task_hmi(0);

      /* These tasks are only required if the test HMI (ahmi.exe) is used. */
      cnc_task_sys(0);
      cnc_task_tcp(0);

    }
    else
    {
      /* Give cyclic tick to internal CNC scheduler */
      if (hCncScheduleThreadsTrigger != NULL)
      {
        cnc_give_semaphore(hCncScheduleThreadsTrigger);
      }
    }
    usleep(1000); // [1ms]
  } /* End of kernel loop */

  cnc_shutdown();

  return ret_value;
} /* End main() */

