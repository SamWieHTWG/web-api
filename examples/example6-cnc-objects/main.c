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

/* Information of configuration from platform provider */
static uint16_t NumberOfChannels = CNC_OS_IFC_NO_CHANNEL_MAX;
static uint16_t NumberOfAxes     = CNC_OS_IFC_NO_AXES_MAX;

/* External and global defined variables */
bool   f_cnc_shutdown        = false;

/*
+----------------------------------------------------------------------------+
| Main                                                                       |
+----------------------------------------------------------------------------+
*/
int main(int argc, char *argv[])
{
  int ret_value = ERR_CNC_NOERROR;

  /*
  +----------------------------------------------------------------------------+
  | Initialize sdk demo                                                        |
  +----------------------------------------------------------------------------+
  */

  /* Set cnc cycle time */
  cnc_set_cycle_time(1000);

  /*
  +--------------------------------------------------------------------------+
  | CNC start-up                                                             |
  +--------------------------------------------------------------------------+
  */
  char startupfile[PATH_FILENAME_LENGTH] = "./listen/startup.lis";
  ret_value = cnc_startup(startupfile);
  if (ret_value != ERR_CNC_NOERROR)
    exit(ret_value);

  /* Set TCP object access */
  cnc_set_tcp_object_access(true);

  /* Create menu thread */
  unsigned int menu_activition_bitmap =   MENU_SHOW_CNC_OBJECTS | MENU_WRITE_CSV |
  MENU_RESET_KERNEL | MENU_SHOW_POS | MENU_START_PROGRAM | MENU_MANUAL_INPUT | MENU_SHUTDOWN;
  cnc_demo_create_menu_thread(menu_activition_bitmap);

  /*
  +--------------------------------------------------------------------------+
  | Running state                                                            |
  | CNC is operating in cyclic mode.                                         |
  +--------------------------------------------------------------------------+
  */
  while (f_cnc_shutdown == false)
  {
    /* periodic call of cnc tasks */
    cnc_task_ipo(0);
    cnc_task_dec(0);
    cnc_task_hmi(0);

    /* These tasks are only required if the test HMI (ahmi.exe) is used. */
    cnc_task_sys(0);
    cnc_task_tcp(0);

    usleep(1000); // [1ms]

  } /* End of kernel loop */

  cnc_shutdown();

  return ret_value;
} /* End main() */


