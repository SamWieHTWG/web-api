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
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#include "cnc_tool_ifc.h"
#include "cnc_demo_samples.h"
#include "keyboard_helper.h"
#include "../example1-hello-cnc/cnc-wrapper.h"

/*
+----------------------------------------------------------------------------+
| cnc_demo_samples.h                                    | Copyright    ISG   |
|                                                       |                    |
|                                                       | Gropiusplatz 10    |
|                                                       | 70563 Stuttgart    |
+-------------------------------------------------------+--------------------+
| CNC SDK Demo sample functions.                                             |
|                                                                            |
| Various sample functions to demonstrate how the SDK may be used.           |
+----------------------------------------------------------------------------+
*/

/*
+----------------------------------------------------------------------------+
| External and global defined variables                                      |
+----------------------------------------------------------------------------+
*/
extern bool         f_cnc_shutdown;

/*
+----------------------------------------------------------------------------+
| Tool data of external tool data management                                 |
+----------------------------------------------------------------------------+
*/
CNC_TOOL_DESC       ext_tool_data[NUMBER_EXT_TOOL_DATA];
CNC_TOOL_DATA_IN    ext_tool_life_data[NUMBER_EXT_TOOL_DATA + 1];

bool cnc_demo_object_access( void)
{
  int   ret_value;
  /*
  +----------------------------------------------------------------------------+
  | Test Access GEO                                                            |
  +----------------------------------------------------------------------------+
  */
  // 1;0x20300;0x00001;cycle time;REAL64;8;false;s;PLATFORM;TASK_IPO
  CNC_OBJECT_ID  cnc_object2 = {CNC_TASK_IPO, 0x20300, 0x00001};
  double cycle_time;
  ret_value = cnc_read_value( cnc_object2, &cycle_time, sizeof(cycle_time) );

  // 165;0x20300;0x10043;TEMPC::coefficient;REAL64;8;true;µm/m;AXIS;TASK_IPO
  CNC_OBJECT_ID  cnc_object3 = {CNC_TASK_IPO, 0x20300, 0x10043};
  double coefficient =0.1234;
  ret_value = cnc_write_value( cnc_object3, &coefficient, sizeof(coefficient) );

  return true;
} // cnc_demo_object_access() */
/*
+----------------------------------------------------------------------------+
| Enable/disable drives                                                      |
+----------------------------------------------------------------------------+
*/
bool cnc_demo_enable_disable_drives( bool f_on )
{
  CNC_OBJECT_ID  cnc_object = {CNC_TASK_HMI, 0x0, 0x0};
  unsigned long  i;
  unsigned char  f_drives_on = 0;

           int   ret_value;

  AXIS_ID        axis_ids;

  if ( f_on == true )
  {
    f_drives_on = 1;
  }
  else
  {
    f_drives_on = 0;
  }

  /* Query number of axes and axis ids */
  cnc_object.iGroup  = CNC_IGRP_AC_AXIS + 1;
  cnc_object.iOffset = 0x110A;

  memset(&axis_ids, 0, sizeof(axis_ids));

  if ( ERR_CNC_NOERROR != cnc_read_value(cnc_object, &axis_ids, sizeof(axis_ids)) )
  {
    return false;
  }

  for ( i = 0; i < axis_ids.n_ax; i++ )
  {
    /* ac_<i>_drive_on */
    cnc_object.iGroup  = CNC_IGRP_AC_AXIS + axis_ids.ax_id[i];
    cnc_object.iOffset = AC_X_DRIVE_ON;
    ret_value          = cnc_write_value( cnc_object, &f_drives_on, sizeof(f_drives_on) );
    if ( ERR_CNC_NOERROR != ret_value )
    {
      printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
      return false;
    }
  }

  if ( f_on )
  {
    cnc_demo_wait_cnc_ticks(50 );
  }

  for ( i = 0; i < axis_ids.n_ax; i++ )
  {
    /* ac_<i>_torque_permission_w */
    cnc_object.iGroup  = CNC_IGRP_AC_AXIS + axis_ids.ax_id[i];
    cnc_object.iOffset = AC_X_TORQUE_PERMISSION;
    ret_value          = cnc_write_value( cnc_object, &f_drives_on, sizeof(f_drives_on) );
    if ( ERR_CNC_NOERROR != ret_value )
    {
      printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
      return false;
    }
  }

  if ( f_on )
  {
    cnc_demo_wait_cnc_ticks(50);
  }

  for ( i = 0; i < axis_ids.n_ax; i++ )
  {
    /* ac_<i>_release_feedhold_w */
    cnc_object.iGroup  = CNC_IGRP_AC_AXIS + axis_ids.ax_id[i];
    cnc_object.iOffset = AC_X_RELEASE_FEEDHOLD;
    ret_value = cnc_write_value( cnc_object, &f_drives_on, sizeof(f_drives_on) );
    if ( ERR_CNC_NOERROR != ret_value )
    {
      printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
      return false;
    }
  }

  if ( f_on )
  {
    cnc_demo_wait_cnc_ticks(50);
  }

  /*
  +----------------------------------------------------------------------------+
  | Test Access GEO                                                            |
  +----------------------------------------------------------------------------+
  */
  cnc_demo_object_access();

  return true;
} /* End of cnc_demo_enable_disable_drives() */

/*
+----------------------------------------------------------------------------+
| Wait for specified number of cnc ticks                                     |
+----------------------------------------------------------------------------+
*/
void cnc_demo_wait_cnc_ticks(unsigned int cnc_ticks_to_wait)
{
  /* read tick counter */
  CNC_OBJECT_ID   cnc_object = { CNC_TASK_IPO, CNC_IGRP_TICK_COUNTER, CNC_IOFF_TICK_COUNTER };
  unsigned int cnc_ticks, cnc_ticks_start;

  if (ERR_CNC_NOERROR != cnc_read_value(cnc_object, &cnc_ticks, sizeof(cnc_ticks)))
  {
    printf("\n Error: Could not read cnc ticks");
    return;
  }
  cnc_ticks_start = cnc_ticks;

  while (true)
  {
    if (ERR_CNC_NOERROR != cnc_read_value(cnc_object, &cnc_ticks, sizeof(cnc_ticks)))
    {
      printf("\n Error: Could not read cnc ticks");
      return;
    }
    if (cnc_ticks - cnc_ticks_start > cnc_ticks_to_wait)
      return;
    usleep(WAIT_TICKS_SLEEP_US);
  }
} /* End of cnc_demo_wait_cnc_ticks() */

/*
+----------------------------------------------------------------------------+
| Start program                                                              |
+----------------------------------------------------------------------------+
*/
bool cnc_demo_start_nc_program( char file_name[PATH_FILENAME_LENGTH] )
{
  CNC_OBJECT_ID   cnc_object = {CNC_TASK_HMI, CNC_IGRP_MCM_COMMAND, 0x0};
  MCM_MODE_STATE  active;
  MCM_MODE_STATE  next;

  unsigned int    i;
           int    ret_value;

  /* Mode automatic, state selected? */
  for ( i = 0; i < 100; i++ )
  {
    /* Read active mode and state */
    cnc_object.iGroup  = CNC_IGRP_MCM_COMMAND;
    cnc_object.iOffset = MCM_ACTIVE;

    if ( ERR_CNC_NOERROR != cnc_read_value_wrapper( cnc_object.iThread, cnc_object.iGroup, cnc_object.iOffset, &active, sizeof(active)) )
    {
      printf("\n Error: Could not read active MCM state.");
      return false;
    }

    if ( (active.mode == MCM_MODE_AUTOMATIC) && (active.state == MCM_STATE_SELECTED) )
    {
      break;
    }

    next.mode  = MCM_MODE_AUTOMATIC;
    next.state = MCM_STATE_SELECTED;

    /* mcm_command_to_mode */
    cnc_object.iOffset = MCM_COMMAND_TO_MODE;
    ret_value = cnc_write_value_wrapper( cnc_object.iThread, cnc_object.iGroup, cnc_object.iOffset, &next.mode, sizeof(next.mode));
    if ( ERR_CNC_NOERROR != ret_value )
    {
      printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
      return false;
    }

    /* mcm_command_to_state */
    cnc_object.iOffset = MCM_COMMAND_TO_STATE;
    ret_value = cnc_write_value_wrapper( cnc_object.iThread, cnc_object.iGroup, cnc_object.iOffset, &next.state, sizeof(next.state));
    if ( ERR_CNC_NOERROR != ret_value )
    {
      printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
      return false;
    }

    active.mode  = 0;
    active.state = 0;

    /* mcm_command_from_mode_w */
    cnc_object.iOffset = MCM_COMMAND_FROM_MODE;
    ret_value = cnc_write_value_wrapper( cnc_object.iThread, cnc_object.iGroup, cnc_object.iOffset, &active.mode, sizeof(active.mode));
    if ( ERR_CNC_NOERROR != ret_value )
    {
      printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
      return false;
    }

    /* mcm_command_from_state_w */
    cnc_object.iOffset = MCM_COMMAND_FROM_STATE;
    ret_value = cnc_write_value_wrapper( cnc_object.iThread, cnc_object.iGroup, cnc_object.iOffset, &active.state, sizeof(active.state));
    if ( ERR_CNC_NOERROR != ret_value )
    {
      printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
      return false;
    }

    cnc_demo_wait_cnc_ticks(10);

    /* mcm_command_parameter_w */
    cnc_object.iOffset = MCM_COMMAND_PARAMETER;
    ret_value = ERR_OBJ_ACCESS_BUSY;
    while (ret_value == ERR_OBJ_ACCESS_BUSY)
    {
      ret_value = cnc_write_value(cnc_object, file_name, (unsigned int)strlen(file_name) + 1);
      cnc_demo_wait_cnc_ticks(10);
    }

    if ( ERR_CNC_NOERROR != ret_value )
    {
      printf("\n Error: could not write object 0x%x, 0x%x with value %s", cnc_object.iGroup, cnc_object.iOffset, file_name);
      return false;
    }

    cnc_demo_wait_cnc_ticks(10 );
  }

  if ( i > 100 )
  {
    return false;
  }

  next.mode  = MCM_MODE_AUTOMATIC;
  next.state = MCM_STATE_ACTIVE;

  /* mcm_command_to_mode */
  cnc_object.iGroup  = 0x20101;
  cnc_object.iOffset = MCM_COMMAND_TO_MODE;
  ret_value = cnc_write_value( cnc_object, &next.mode, sizeof(next.mode));
  if ( ERR_CNC_NOERROR != ret_value )
  {
    printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
    return false;
  }

  /* mcm_command_to_state */
  cnc_object.iOffset = MCM_COMMAND_TO_STATE;
  ret_value = cnc_write_value( cnc_object, &next.state, sizeof(next.state));
  if ( ERR_CNC_NOERROR != ret_value )
  {
    printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
    return false;
  }

  active.mode  = 0;
  active.state = 0;

  /* mcm_command_from_mode_w */
  cnc_object.iOffset = MCM_COMMAND_FROM_MODE;
  ret_value = cnc_write_value( cnc_object, &active.mode, sizeof(active.mode));
  if ( ERR_CNC_NOERROR != ret_value )
  {
    printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
    return false;
  }

  /* mcm_command_from_state_w */
  cnc_object.iOffset = MCM_COMMAND_FROM_STATE;
  ret_value = cnc_write_value( cnc_object, &active.state, sizeof(active.state));
  if ( ERR_CNC_NOERROR != ret_value )
  {
    printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
    return false;
  }

  /* mcm_command_parameter_w */
  cnc_object.iOffset = MCM_COMMAND_PARAMETER;
  ret_value = cnc_write_value( cnc_object, file_name, (unsigned int)strlen(file_name)+1);
  if ( ERR_CNC_NOERROR != ret_value )
  {
    printf("\n Error: could not write object 0x%x, 0x%x with value %s", cnc_object.iGroup, cnc_object.iOffset, file_name);
    return false;
  }

  return true;
} /* End of cnc_demo_start_nc_program() */

/*
+----------------------------------------------------------------------------+
| Manual data input                                                          |
+----------------------------------------------------------------------------+
*/
bool cnc_demo_start_manual_data_input( char str_mdi[256] )
{
  CNC_OBJECT_ID   cnc_object = {CNC_TASK_HMI, CNC_IGRP_MCM_COMMAND, 0x0};
  MCM_MODE_STATE  active;
  MCM_MODE_STATE  next;

  unsigned int    i;
           int    ret_value;

  /* Mode automatic, state selected? */
  for ( i = 0; i < 1; i++ )
  {
    /* Read active mode and state */
    cnc_object.iGroup  = CNC_IGRP_MCM_COMMAND;
    cnc_object.iOffset = MCM_ACTIVE;

    if ( ERR_CNC_NOERROR != cnc_read_value_wrapper( cnc_object.iThread, cnc_object.iGroup, cnc_object.iOffset, &active, sizeof(active)) )
    {
      printf("\n Error: Could not read active MCM state.");
      return false;
    }

    if ( (active.mode == MCM_MODE_MDI) && (active.state == MCM_STATE_SELECTED) )
    {
      break;
    }

    next.mode  = MCM_MODE_MDI;
    next.state = MCM_STATE_SELECTED;

    /* mcm_command_to_mode */
    cnc_object.iOffset = MCM_COMMAND_TO_MODE;
    ret_value = cnc_write_value_wrapper( cnc_object.iThread, cnc_object.iGroup, cnc_object.iOffset, &next.mode, sizeof(next.mode));
    if ( ERR_CNC_NOERROR != ret_value )
    {
      printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
      return false;
    }

    /* mcm_command_to_state */
    cnc_object.iOffset = MCM_COMMAND_TO_STATE;
    ret_value = cnc_write_value_wrapper( cnc_object.iThread, cnc_object.iGroup, cnc_object.iOffset, &next.state, sizeof(next.state));
    if ( ERR_CNC_NOERROR != ret_value )
    {
      printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
      return false;
    }

    active.mode  = 0;
    active.state = 0;

    /* mcm_command_from_mode_w */
    cnc_object.iOffset = MCM_COMMAND_FROM_MODE;
    ret_value = cnc_write_value_wrapper( cnc_object.iThread, cnc_object.iGroup, cnc_object.iOffset, &active.mode, sizeof(active.mode));
    if ( ERR_CNC_NOERROR != ret_value )
    {
      printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
      return false;
    }

    /* mcm_command_from_state_w */
    cnc_object.iOffset = MCM_COMMAND_FROM_STATE;
    ret_value = cnc_write_value_wrapper( cnc_object.iThread, cnc_object.iGroup, cnc_object.iOffset, &active.state, sizeof(active.state));
    if ( ERR_CNC_NOERROR != ret_value )
    {
      printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
      return false;
    }

    /* mcm_command_parameter_w */
    cnc_object.iOffset = MCM_COMMAND_PARAMETER;
    ret_value = cnc_write_value_wrapper( cnc_object.iThread, cnc_object.iGroup, cnc_object.iOffset, str_mdi, (unsigned int)strlen(str_mdi)+1);
    if ( ERR_CNC_NOERROR != ret_value )
    {
      printf("\n Error: could not write object 0x%x, 0x%x with value %s", cnc_object.iGroup, cnc_object.iOffset, str_mdi);
      return false;
    }
    cnc_demo_wait_cnc_ticks(10 );
  }

  for ( i = 0; i < 100; i++ )
  {
    /* Read active mode and state */
    cnc_object.iGroup  = CNC_IGRP_MCM_COMMAND;
    cnc_object.iOffset = MCM_ACTIVE;

    if ( ERR_CNC_NOERROR != cnc_read_value_wrapper( cnc_object.iThread, cnc_object.iGroup, cnc_object.iOffset, &active, sizeof(active)) )
    {
      printf("\n Error: Could not read active MCM state.");
      return false;
    }

    if ( (active.mode == MCM_MODE_MDI) && (active.state == MCM_STATE_SELECTED) )
    {
      break;
    }
    cnc_demo_wait_cnc_ticks(10 );
  }

  if ( i > 100 )
  {
    return false;
  }

  next.mode  = MCM_MODE_MDI;
  next.state = MCM_STATE_ACTIVE;

  /* mcm_command_to_mode */
  cnc_object.iGroup  = CNC_IGRP_MCM_COMMAND;
  cnc_object.iOffset = MCM_COMMAND_TO_MODE;
  ret_value = cnc_write_value( cnc_object, &next.mode, sizeof(next.mode));
  if ( ERR_CNC_NOERROR != ret_value )
  {
    printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
    return false;
  }

  /* mcm_command_to_state */
  cnc_object.iOffset = MCM_COMMAND_TO_STATE;
  ret_value = cnc_write_value( cnc_object, &next.state, sizeof(next.state));
  if ( ERR_CNC_NOERROR != ret_value )
  {
    printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
    return false;
  }

  /* mcm_command_from_mode_w */
  cnc_object.iOffset = MCM_COMMAND_FROM_MODE;
  ret_value = cnc_write_value( cnc_object, &active.mode, sizeof(active.mode));
  if ( ERR_CNC_NOERROR != ret_value )
  {
    printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
    return false;
  }

  /* mcm_command_from_state_w */
  cnc_object.iOffset = MCM_COMMAND_FROM_STATE;
  ret_value = cnc_write_value( cnc_object, &active.state, sizeof(active.state));
  if ( ERR_CNC_NOERROR != ret_value )
  {
    printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
    return false;
  }

  /* mcm_command_parameter_w */
  cnc_object.iOffset = MCM_COMMAND_PARAMETER;
  ret_value = cnc_write_value_wrapper( cnc_object.iThread, cnc_object.iGroup, cnc_object.iOffset, str_mdi, (unsigned int)strlen(str_mdi)+1);
  if ( ERR_CNC_NOERROR != ret_value )
  {
    printf("\n Error: could not write object 0x%x, 0x%x with value %s", cnc_object.iGroup, cnc_object.iOffset, str_mdi);
    return false;
  }

  return true;
} /* End of cnc_demo_start_manual_data_input() */

/*
+----------------------------------------------------------------------------+
| Kernel reset                                                               |
+----------------------------------------------------------------------------+
*/
bool cnc_demo_reset( void )
{
  CNC_OBJECT_ID   cnc_object = {CNC_TASK_HMI, CNC_IGRP_MCM_COMMAND, 0x0};
  MCM_MODE_STATE  active;

  int             ret_value;

  /* Read active mode and state */
  cnc_object.iGroup  = CNC_IGRP_MCM_COMMAND;
  cnc_object.iOffset = MCM_ACTIVE;

  if ( ERR_CNC_NOERROR != cnc_read_value( cnc_object, &active, sizeof(active)) )
  {
    printf("\n Error: Could not read active MCM state.");
    return false;
  }

  /* Reset in STANDBY mode */
  if ( active.mode == MCM_MODE_STANDBY )
  {
	  cnc_object.iOffset = MCM_STANDBY_RESET;
	  ret_value = cnc_TRIGGER( cnc_object );
	  if ( ERR_CNC_NOERROR != ret_value )
	  {
	    printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
	    return false;
	  }
  }

  /* Reset in AUTOMATIC mode */
  if ( active.mode == MCM_MODE_AUTOMATIC )
  {
	  cnc_object.iOffset = MCM_AUTOMATIC_RESET;
	  ret_value = cnc_TRIGGER( cnc_object );
	  if ( ERR_CNC_NOERROR != ret_value )
	  {
	    printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
	    return false;
	  }
  }

  /* Reset in MDI mode */
  if ( active.mode == MCM_MODE_MDI )
  {
	  cnc_object.iOffset = MCM_MDI_RESET;
	  ret_value = cnc_TRIGGER( cnc_object );
	  if ( ERR_CNC_NOERROR != ret_value )
	  {
	    printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
	    return false;
	  }
  }

  return true;
} /* End of cnc_demo_reset() */

/*
+----------------------------------------------------------------------------+
| Stop / resume nc program                                                   |
+----------------------------------------------------------------------------+
*/
bool cnc_demo_stop_resume_nc_program( bool f_stop_resume )
{
  CNC_OBJECT_ID   cnc_object = {CNC_TASK_HMI, CNC_IGRP_MCM_COMMAND, 0x0};
  MCM_MODE_STATE  active;

  int             ret_value;
  char            f_stop = 1;

  /* Read active mode and state */
  cnc_object.iGroup  = CNC_IGRP_MCM_COMMAND;
  cnc_object.iOffset = MCM_ACTIVE;

  if ( ERR_CNC_NOERROR != cnc_read_value( cnc_object, &active, sizeof(active)) )
  {
    printf("\n Error: Could not read active MCM state.");
    return false;
  }

  /* Stop nc program */
  if ( (active.mode   == MCM_MODE_AUTOMATIC) &&
       (active.state  == MCM_STATE_ACTIVE)   &&
       (f_stop_resume == true)                  )
  {
    cnc_object.iGroup  = CNC_IGRP_MCM_COMMAND;
    cnc_object.iOffset = MCM_AUTOMATIC_STOP;
    ret_value = cnc_write_value( cnc_object, &f_stop, sizeof(f_stop));
    if ( ERR_CNC_NOERROR != ret_value )
    {
      printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
      return false;
    }
  }

  /* Resume nc program */
  if ( (active.mode   == MCM_MODE_AUTOMATIC) &&
       (active.state  == MCM_STATE_HOLD)     &&
       (f_stop_resume == false)                 )
  {
    cnc_object.iGroup  = CNC_IGRP_MCM_COMMAND;
    cnc_object.iOffset = MCM_AUTOMATIC_RESUME;
    ret_value = cnc_write_value( cnc_object, &f_stop, sizeof(f_stop));
    if ( ERR_CNC_NOERROR != ret_value )
    {
      printf("\n Error: could not write object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
      return false;
    }
  }

  return true;
} /* End of cnc_demo_stop_resume_nc_program() */

/*
+----------------------------------------------------------------------------+
| NC program streaming                                                       |
+----------------------------------------------------------------------------+
*/
bool cnc_demo_streaming_program( FILE *p_nc_file, char *p_buf )
{
  CNC_OBJECT_ID   cnc_object = {CNC_TASK_HMI, CNC_IGRP_MCM_COMMAND, 0x0};
  MCM_MODE_STATE  active;

  size_t          len;
  int             ret_value;

  if ( NULL == p_nc_file )
  {
    return false;
  }

  /* Read active mode and state */
  cnc_object.iGroup  = CNC_IGRP_MCM_COMMAND;
  cnc_object.iOffset = MCM_ACTIVE;
  
  if ( ERR_CNC_NOERROR != cnc_read_value( cnc_object, &active, sizeof(active)) )
  {
    printf("\n Error: Could not read active MCM state.");
    return false;
  }

  if ( (active.mode   != MCM_MODE_AUTOMATIC) ||
       ((active.state != MCM_STATE_ACTIVE) && (active.state != MCM_STATE_HOLD)) )
  {
    printf("\n Program streaming aborted.\n");
    return false;
  }

  cnc_object.iGroup  = CNC_IGRP_MCM_COMMAND;
  cnc_object.iOffset = MC_PROGRAM_STREAM;

  while ( (*p_buf != '\0') || NULL != fgets( p_buf, MAX_BUF_SIZE-1, p_nc_file ) )
  {
    /* if messing, append line feed */
    len = strlen( p_buf );
    if ( len > 0 )
    {
      if ( p_buf[len-1] != '\n' )
      {
        p_buf[len++] = '\n';
        p_buf[len]   = '\0';
      }

      ret_value = cnc_write_value( cnc_object, p_buf, (unsigned int)len );
      if ( ERR_CNC_NOERROR != ret_value )
      {
        /* Streaming buffer full --> wait */
        return true;
      }
    }
    *p_buf = '\0';
  }

  printf("\nCNC-MSG: End of streaming file.");
  return false;
} /* End of cnc_demo_streaming_program() */

bool cnc_demo_start_streaming_program( FILE **p_file, char file_name[256] )
{
  CNC_OBJECT_ID   cnc_object = {CNC_TASK_HMI, CNC_IGRP_MCM_COMMAND, 0x0};
  MCM_MODE_STATE  active;

  if ( *p_file != NULL )
  {
    fclose( *p_file );
    *p_file = NULL;
  }

  char streaming_file_name[PATH_FILENAME_LENGTH] = "streaming_prog.nc";
  if ( false == cnc_demo_start_nc_program(streaming_file_name) )
  {
    return false;
  }

  cnc_demo_wait_cnc_ticks(400 );

  /* Read active mode and state */
  cnc_object.iGroup  = CNC_IGRP_MCM_COMMAND;
  cnc_object.iOffset = MCM_ACTIVE;
  
  if ( ERR_CNC_NOERROR != cnc_read_value( cnc_object, &active, sizeof(active)) )
  {
    printf("\n Error: Could not read active MCM state.");
    return false;
  }

  if ( (active.mode   != MCM_MODE_AUTOMATIC) ||
       ((active.state != MCM_STATE_ACTIVE) && (active.state != MCM_STATE_HOLD)) )
  {
    return false;
  }

  *p_file = fopen(file_name, "rt");

  if ( NULL == *p_file )
  {
    printf("\n Error: Could not open file '%s' for streaming.", file_name);
    return false;
  }

  return true;
} /*End of cnc_demo_start_streaming_program() */

bool cnc_demo_end_streaming_program( FILE **p_file )
{
  if ( *p_file != NULL )
  {
    fclose(*p_file);
    *p_file = NULL;
    return true;
  }

  return false;
} /* End of cnc_demo_end_streaming_program() */

/*
+----------------------------------------------------------------------------+
| Get axis names                                                             |
+----------------------------------------------------------------------------+
*/
AXIS_NAMES cnc_demo_get_axis_names ( void )
{
  CNC_OBJECT_ID  cnc_object = {CNC_TASK_HMI, 0x0, 0x0};
  int            ret_value;
  AXIS_NAMES     axis_names;

  cnc_object.iGroup  = CNC_IGRP_AC_AXIS + 1;
  cnc_object.iOffset = AC_X_AXES_NAMES;

  memset(&axis_names, 0, sizeof(axis_names));

  ret_value = cnc_read_value(cnc_object, &axis_names, sizeof(axis_names));

  if ( ERR_CNC_NOERROR != ret_value )
  {
    printf("\n Error: could not read object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
  }

  return axis_names;
} /* End of cnc_demo_get_axis_names() */

/*
+----------------------------------------------------------------------------+
| Get active positions                                                       |
+----------------------------------------------------------------------------+
*/
AXIS_POS cnc_demo_get_active_positions ( void )
{
  CNC_OBJECT_ID  cnc_object = {CNC_TASK_HMI, 0x0, 0x0};
  int            ret_value;
  AXIS_POS       axis_act_pos;

  cnc_object.iGroup  = CNC_IGRP_AC_AXIS + 1;
  cnc_object.iOffset = AC_X_AXES_ACT_POS_ACS;

  memset(&axis_act_pos, 0, sizeof(axis_act_pos));

  ret_value = cnc_read_value(cnc_object, &axis_act_pos, sizeof(axis_act_pos));

  if ( ERR_CNC_NOERROR != ret_value )
  {
    printf("\n Error: could not read object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
  }

  return axis_act_pos;
} /* End of cnc_demo_get_active_positions() */

/*
+----------------------------------------------------------------------------+
| Get current positions                                                      |
+----------------------------------------------------------------------------+
*/
AXIS_POS cnc_demo_get_current_positions ( void )
{
  CNC_OBJECT_ID  cnc_object = {CNC_TASK_HMI, 0x0, 0x0};
  int            ret_value;
  AXIS_POS       axis_cur_pos;

  cnc_object.iGroup  = CNC_IGRP_AC_AXIS + 1;
  cnc_object.iOffset = AC_X_AXES_CUR_POS_ACS;

  memset(&axis_cur_pos, 0, sizeof(axis_cur_pos));

  ret_value = cnc_read_value(cnc_object, &axis_cur_pos, sizeof(axis_cur_pos));

  if ( ERR_CNC_NOERROR != ret_value )
  {
    printf("\n Error: could not read object 0x%x, 0x%x", cnc_object.iGroup, cnc_object.iOffset);
  }

  return axis_cur_pos;
} /* End of cnc_demo_get_current_positions() */

/*
+----------------------------------------------------------------------------+
| Cyclic check for errors                                                    |
+----------------------------------------------------------------------------+
*/
bool cnc_demo_error_check ( void )
{
  CNC_OBJECT_ID   cnc_object = {CNC_TASK_HMI, CNC_IGRP_MCM_COMMAND, MCM_ACTIVE};
  MCM_MODE_STATE  active;

  /* Read active mode and state */
  cnc_object.iGroup  = CNC_IGRP_MCM_COMMAND;
  cnc_object.iOffset = MCM_ACTIVE;
  if ( ERR_CNC_NOERROR != cnc_read_value( cnc_object, &active, sizeof(active)) )
  {
    printf("\n Error: Could not read active MCM state.");
    return false;
  }

  if ( active.state == MCM_STATE_ERROR )
  {
    return true;
  }

  return false;
} /* End of cnc_demo_error_check() */

/*
+----------------------------------------------------------------------------+
| Print Kernel runtime statistic                                             |
+----------------------------------------------------------------------------+
*/
void cnc_demo_print_runtime_statistic( void )
{
  CNC_OBJECT_ID       cnc_object = { CNC_TASK_HMI, 0x20101, 0x0 };

  unsigned int        time_stat     = 0;
  uint64_t            max_tick      = 0;

  printf("\n Runtime statistics:\n");
  printf(" -------------------\n");

  cnc_object.iOffset = 0x273;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_cycle_update_min        %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x274;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_cycle_update_max        %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x275;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_cycle_update_act        %u [0.1 us]\n\n", time_stat);


  cnc_object.iOffset = 0x28b;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_GeoCycleInterrupt_min   %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x28c;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_GeoCycleInterrupt_max   %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x28d;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_GeoCycleInterrupt_act   %u [0.1 us]\n\n", time_stat);


  cnc_object.iOffset = 0x258;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_geo_min                 %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x259;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_geo_max                 %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x2B1;
  cnc_read_UNS64(cnc_object, &max_tick);
  printf("  cnc_time_geo_max_tick            %llu\n", max_tick);

  cnc_object.iOffset = 0x25A;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_geo_act                 %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x288;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_geo_avg                 %u [0.1 us]\n\n", time_stat);


  cnc_object.iOffset = 0x25B;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_sda_min                 %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x25C;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_sda_max                 %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x2B2;
  cnc_read_UNS64(cnc_object, &max_tick);
  printf("  cnc_time_sda_max_tick            %llu\n", max_tick);

  cnc_object.iOffset = 0x25D;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_sda_act                 %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x289;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_sda_avg                 %u [0.1 us]\n\n", time_stat);


  cnc_object.iOffset = 0x25E;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_com_min                 %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x25F;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_com_max                 %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x2B3;
  cnc_read_UNS64(cnc_object, &max_tick);
  printf("  cnc_time_com_max_tick            %llu\n", max_tick);

  cnc_object.iOffset = 0x260;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_com_act                 %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x28A;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_com_avg                 %u [0.1 us]\n\n", time_stat);


  cnc_object.iOffset = 0x296;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_user_event_ipo_min      %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x297;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_user_event_ipo_max      %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x298;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_user_event_ipo_act      %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x299;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_user_event_ipo_avg      %u [0.1 us]\n\n", time_stat);


  cnc_object.iOffset = 0x29a;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_user_event_dec_min      %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x29b;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_user_event_dec_max      %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x29c;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_user_event_dec_act      %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x29d;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_user_event_dec_avg      %u [0.1 us]\n\n", time_stat);


  cnc_object.iOffset = 0x29e;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_user_event_hmi_min      %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x29f;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_user_event_hmi_max      %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x2a0;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_user_event_hmi_act      %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x2a1;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_user_event_hmi_avg      %u [0.1 us]\n\n", time_stat);


  cnc_object.iOffset = 0x2a2;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_user_event_sys_min      %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x2a3;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_user_event_sys_max      %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x2a4;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_user_event_sys_act      %u [0.1 us]\n", time_stat);

  cnc_object.iOffset = 0x2a5;
  cnc_read_UNS32(cnc_object, &time_stat);
  printf("  cnc_time_user_event_sys_avg      %u [0.1 us]\n\n", time_stat);

  printf("\n\n");
} /* End of cnc_demo_print_runtime_statistic() */

/*
+----------------------------------------------------------------------------+
| Reset Kernel runtime statistic                                             |
+----------------------------------------------------------------------------+
*/
void cnc_demo_reset_runtime_statistic( void )
{
  CNC_OBJECT_ID cnc_object = { CNC_TASK_HMI, 0x20101, 0x261 };

  /* Use 'cnc_write_value' because object cnc_time_reset */
  /* has no data type (TYP_NONE)                         */
  cnc_TRIGGER(cnc_object);

  printf("\n Runtime statistics resetted\n\n");
} /* End of cnc_demo_reset_runtime_statistic() */

/*
+----------------------------------------------------------------------------+
| Show axis names and positions                                              |
+----------------------------------------------------------------------------+
*/
void cnc_demo_show_positions(void)
{
  uint32_t       i;

  AXIS_NAMES     axis_names;
  AXIS_POS       axis_act_pos;
  AXIS_POS       axis_cur_pos;

  /* Get axis names */
  axis_names   = cnc_demo_get_axis_names();
  axis_act_pos = cnc_demo_get_active_positions();
  axis_cur_pos = cnc_demo_get_current_positions();

  printf("\n        Axis name  Active position  Current position");
  printf("\n-----------------------------------------------------");
  for (i = 0; i < axis_names.n_ax; i++)
  {
    double act_pos = axis_act_pos.pos_ax[i] / 10000.0;
    double cur_pos = axis_cur_pos.pos_ax[i] / 10000.0;
    printf("\n %16s  %-8.4f         %-8.4f", axis_names.ax_name[i], act_pos, cur_pos);
  }
  printf("\n\n");
} /* End of cnc_demo_show_positions() */

/*
+----------------------------------------------------------------------------+
| Print object ids                                                           |
+----------------------------------------------------------------------------+
*/
void cnc_demo_print_object_ids( bool f_write_csv )
{
  CNC_OBJECT   cnc_object;

  FILE        *csv_file;
  int          n;
  char         domain[30];
  char         task[30];

  memset(&cnc_object, 0, sizeof(cnc_object));

  if (f_write_csv == true)
  {
    printf("\n Started writing file...");
    WRITE_CNC_OBJECT_HEADER_TO_FILE
  }
  else
  {
    csv_file = NULL;
  }

  n = cnc_query_number_of_objects();

  for (int m = 0; m < n; m++)
  {
    if (cnc_query_object(m, &cnc_object) == ERR_CNC_NOERROR)
    {
      E_CNC_OBJECT_DOMAIN dom = cnc_query_object_domain(cnc_object.ObjectID);
      switch (dom)
      {
        case CNC_PLATFORM:
        {
          if ( f_write_csv == false )
            printf("        \tdomain: PLATFORM\n");
          strcpy(domain, "PLATFORM");
          break;
        }
        case CNC_CHANNEL:
        {
          if ( f_write_csv == false )
            printf("        \tdomain: CHANNEL\n");
          strcpy(domain, "CHANNEL");
          break;
        }
        case CNC_AXIS:
        {
          if ( f_write_csv == false )
            printf("        \tdomain: AXIS\n");
          strcpy(domain, "AXIS");
          break;
        }
        case CNC_SAI:
        {
          if ( f_write_csv == false )
            printf("        \tdomain: SAI\n");
          strcpy(domain, "SAI");
          break;
        }
        default:
        {
          ;
        }
      }
      switch (cnc_object.ObjectID.iThread)
      {
        case 1:
        {
          if ( f_write_csv == false )
            printf("        \ttask: CNC_TASK_IPO\n");
          strcpy(task, "TASK_IPO");
          break;
        }
        case 2:
        {
          if ( f_write_csv == false )
            printf("        \ttask: CNC_TASK_DEC\n");
          strcpy(task, "TASK_DEC");
          break;
        }
        case 3:
        {
          if ( f_write_csv == false )
            printf("        \ttask: CNC_TASK_HMI\n");
          strcpy(task, "TASK_HMI");
          break;
        }
      }

      if ( f_write_csv == true )
      {
        WRITE_CNC_OBJECT_TO_FILE(m, cnc_object, domain, task)
      }
      else
      {
        PRINT_CNC_OBJECT(m, cnc_object)
      }
    }
  }

  if (f_write_csv == true)
  {
    printf("\n Finished writing file.\n\n");
  }
} /* End of cnc_demo_print_object_ids() */

/*
+----------------------------------------------------------------------------+
| External Tool Management                                                   |
+----------------------------------------------------------------------------+
*/
int cnc_demo_tool_change_info(const CNC_TOOL_DESC* pToolData)
{
  int ret_value = ERR_CNC_NOERROR;

  if (pToolData != NULL)
  {
    if ( (pToolData->tool_id.basic < 0) || (pToolData->tool_id.basic > NUMBER_EXT_TOOL_DATA) )
    {
      ret_value = ERR_CNC_INVALID_TOOL_NR;
    }
    else
    {
      printf("\nExt-Tool-Management: tool change info received (Tool no. %d)\n", pToolData->tool_id.basic);
      ret_value = ERR_CNC_NOERROR;
    }
  }
  else
  {
    printf("\nError: tool data pointer invalid\n");
    ret_value = ERR_CNC_TOOL_DATA_PTR_INVALID;
  }

  return ret_value;
} /* End of cnc_demo_tool_change_info() */

int cnc_demo_tool_read_data(CNC_TOOL_DESC* pToolData)
{
  int ret_value = ERR_CNC_NOERROR;

  if (pToolData != NULL)
  {
    if ((pToolData->tool_id.basic < 0) || (pToolData->tool_id.basic > NUMBER_EXT_TOOL_DATA))
    {
      ret_value = ERR_CNC_INVALID_TOOL_NR;
    }
    else
    {
      CNC_TOOL_DESC* pExtToolData = &ext_tool_data[pToolData->tool_id.basic];
      *pToolData = *pExtToolData;

      printf("\nExt-Tool-Management: job to read tool data received (Tool no. %d)\n", pToolData->tool_id.basic);
      ret_value = ERR_CNC_NOERROR;
    }
  }
  else
  {
    printf("\nError: tool data pointer invalid\n");
    ret_value = ERR_CNC_TOOL_DATA_PTR_INVALID;
  }

  return ret_value;
} /* End of cnc_demo_tool_read_data() */

int cnc_demo_tool_write_data(const CNC_TOOL_DATA_IN *pToolDataIn)
{
  int ret_value = ERR_CNC_NOERROR;

  if (pToolDataIn != NULL)
  {
    if ((pToolDataIn->tool_id.basic < 0) || (pToolDataIn->tool_id.basic > NUMBER_EXT_TOOL_DATA))
    {
      ret_value = ERR_CNC_INVALID_TOOL_NR;
    }
    else
    {
      CNC_TOOL_DATA_IN* pExtToolLifeData = &ext_tool_life_data[pToolDataIn->tool_id.basic];

      pExtToolLifeData->time_used += pToolDataIn->time_used;
      pExtToolLifeData->dist_used += pToolDataIn->dist_used;

      printf("\nExt-Tool-Management: job to write tool data received (Tool no. %d, time_used=%f, dist_used=%f)\n",
             pToolDataIn->tool_id.basic, pToolDataIn->time_used, pToolDataIn->dist_used);
      printf("\nExt-Tool-Management: total tool life data (Tool no. %d, time_used=%f, dist_used=%f)\n",
             pExtToolLifeData->tool_id.basic, pExtToolLifeData->time_used, pExtToolLifeData->dist_used);

      ret_value = ERR_CNC_NOERROR;
    }
  }
  else
  {
    printf("\nError: tool data pointer invalid\n");
    ret_value = ERR_CNC_TOOL_DATA_PTR_INVALID;
  }

  return ret_value;
} /* End of cnc_demo_tool_write_data() */


/**
 * Definition of path to libxml2 library.
 * If no path is defined before startup, "libxml2.dll/libxml2.so" is used
 * as default.
 *
 * @param path path to libxml2 library
 *
 * @return ERR_CNC_NOERROR:        Success
 *         ERR_CNC_PATH_TOO_LONG:  Path too long
*/
int cnc_demo_set_libxml2_path(char* libxml2_path)
{
  return cnc_set_xml_parser_path(libxml2_path);
} /* cnc_demo_set_libxml2_path() */

//Update of the configuration during cnc runtime.
//By default, the active parameter are active.
//It's possible to define only single parameter in the update file.
int32_t cnc_demo_update_axis_config(uint32_t log_ax_no, char* new_config_path)
{
    CNC_OBJECT   cnc_object = {CNC_TASK_HMI, 0x20200+log_ax_no, 0x18};
    return cnc_write_value(cnc_object.ObjectID, new_config_path, (uint32_t)strlen(new_config_path)+1);
}

int32_t cnc_demo_update_channel_config(uint32_t chan_no, char* new_config_path)
{
    CNC_OBJECT   cnc_object = {CNC_TASK_HMI, 0x20100 + chan_no, 0x34};
    return cnc_write_value(cnc_object.ObjectID, new_config_path, (uint32_t)strlen(new_config_path)+1);
}

int32_t cnc_demo_update_zero_offset_config(uint32_t chan_no, char* new_config_path)
{
    CNC_OBJECT   cnc_object = { CNC_TASK_HMI, 0x20100 + chan_no, 0x2e };
    return cnc_write_value(cnc_object.ObjectID, new_config_path, (uint32_t)strlen(new_config_path) + 1);
}

int32_t cnc_demo_update_tool_data_config(uint32_t chan_no, char* new_config_path)
{
    CNC_OBJECT   cnc_object = { CNC_TASK_HMI, 0x20100 + chan_no, 0x30 };
    return cnc_write_value(cnc_object.ObjectID, new_config_path, (uint32_t)strlen(new_config_path) + 1);
}
int32_t cnc_demo_update_clamp_position_config(uint32_t chan_no, char* new_config_path)
{
    CNC_OBJECT   cnc_object = { CNC_TASK_HMI, 0x20100 + chan_no, 0x32 };
    return cnc_write_value(cnc_object.ObjectID, new_config_path, (uint32_t)strlen(new_config_path) + 1);
}
int32_t cnc_demo_update_axis_comp_config(uint32_t log_ax_no, char* new_config_path)
{
    CNC_OBJECT   cnc_object = { CNC_TASK_HMI, 0x2020 + log_ax_no, 0x19 };
    return cnc_write_value(cnc_object.ObjectID, new_config_path, (uint32_t)strlen(new_config_path) + 1);
}

int cnc_demo_update_configuration(void)
{
    int ret;
    CNC_OBJECT   cnc_object;

    /*
    +----------------------------------------------------------------------------+
    | Example: Update channel configuration                                      |
    +----------------------------------------------------------------------------+
    */

    char main_spdl_name[20];

    cnc_object.ObjectID.iThread = CNC_TASK_DEC;
    cnc_object.ObjectID.iGroup = 0x22301;
    cnc_object.ObjectID.iOffset = 0x0007a;
    ret = cnc_read_value(cnc_object.ObjectID, main_spdl_name, sizeof(main_spdl_name));
    if (ret != ERR_CNC_NOERROR)
      return ret;

    printf("\nMain spindle name before update: %s", main_spdl_name);
    ret = cnc_demo_update_channel_config(1, (char*)"./listen/Update/sda_mds1-Update.xml");
    if (ret != ERR_CNC_NOERROR)
      return ret;

    ret = cnc_read_value(cnc_object.ObjectID, main_spdl_name, sizeof(main_spdl_name));
    if (ret != ERR_CNC_NOERROR)
      return ret;

    printf("\nMain spindle name after update: %s", main_spdl_name);

    /*
    +----------------------------------------------------------------------------+
    | Example: Update axis configuration                                         |
    +----------------------------------------------------------------------------+
    */
    AXIS_NAMES ax_names = cnc_demo_get_axis_names();
    printf("\nName of axis 1 before update: %s", ax_names.ax_name[0]);

    ret = cnc_demo_update_axis_config(1, (char*)"./listen/Update/achsmds1-Update.xml");
    if (ret != ERR_CNC_NOERROR)
        return ret;

    ax_names = cnc_demo_get_axis_names();
    printf("\nName of axis 1 after update: %s", ax_names.ax_name[0]);

    /* reset to old axis name for further demo */
    ret = cnc_demo_update_axis_config(1, (char*)"./listen/Update/achsmds1-Revert.xml");
    if (ret != ERR_CNC_NOERROR)
        return ret;

    /*
    +----------------------------------------------------------------------------+
    | Example: Update zero offset configuration                                  |
    +----------------------------------------------------------------------------+
    */

    /* read variable by name  */
    cnc_object.ObjectID.iThread = CNC_TASK_DEC;
    cnc_object.ObjectID.iGroup = 0x22301;
    cnc_object.ObjectID.iOffset = 0x48;

    char mem[] = "V.G.NP[1].V.Y\0";
    ret = cnc_read_write_value(cnc_object.ObjectID, mem, sizeof(mem), sizeof(mem));
    if (ret != ERR_CNC_NOERROR)
      return ret;

    printf("\nG54 offset axis 2 before update %i", *(int*)mem);

    ret = cnc_demo_update_zero_offset_config(1, (char*)"./listen/Update/zero_offset-Update.xml");
    if (ret != ERR_CNC_NOERROR)
        return ret;

    strcpy(mem, "V.G.NP[1].V.Y\0");
    ret = cnc_read_write_value(cnc_object.ObjectID, mem, sizeof(mem), sizeof(mem));
    if (ret != ERR_CNC_NOERROR)
      return ret;

    printf("\nG54 offset axis 2 after update %i", *(int*)mem);


    /*
    +----------------------------------------------------------------------------+
    | Further examples.                                                          |
    | Here, only the startup list is reloaded.                                   |
    +----------------------------------------------------------------------------+
    */

    printf("\nStart config update: clamp position");
    ret = cnc_demo_update_clamp_position_config(1,(char*)"./listen/pzv_d1.lis");
    if (ret != ERR_CNC_NOERROR)
        return ret;

    printf("\nStart config update: tool data");
    ret = cnc_demo_update_tool_data_config(1, (char*)"./listen/werkz_d1.lis");
    if (ret != ERR_CNC_NOERROR)
        return ret;

    printf("\nStart config update: axis comp");
    ret = cnc_demo_update_axis_comp_config(1, (char*)"./listen/achskw1.lis");
    if (ret != ERR_CNC_NOERROR)
        return ret;

    printf("\nConfiguration update successful");

    return ERR_CNC_NOERROR;
}

/*
+----------------------------------------------------------------------------+
| Sample implementation of cnc logging callback                              |
+----------------------------------------------------------------------------+
*/
int cnc_demo_log_message(E_CNC_LOG_MSG_CLASS log_msg_class, unsigned int log_msg_id, const char *log_msg_string)
{
  int ret_val = ERR_CNC_NOERROR;

  switch (log_msg_class)
  {
  case CNC_LOG_WARNING:
  {
    printf("\nCNC-WARNING: %s\n", log_msg_string);
    break;
  }
  case CNC_LOG_ERROR:
  {
    printf("\nCNC-ERROR: %s\n", log_msg_string);
    break;
  }
  case CNC_LOG_INFO:
  {
    printf("CNC-MSG: %s\n", log_msg_string);
    break;
  }
  case CNC_LOG_EXCEPTION:
  {
    printf("\nCNC-EXCEPTION: %s\n", log_msg_string);
    break;
  }
  case CNC_LOG_DEBUG:
  {
    printf("CNC-DEBUG: %s\n", log_msg_string);
    break;
  }
  }
  return ret_val;
}

/*
+----------------------------------------------------------------------------+
| Sample implementation of cnc json logging callback                         |
+----------------------------------------------------------------------------+
*/
int cnc_demo_log_message_json(const char *json_stream)
{
  FILE *file;
  time_t now = time(NULL);
  struct tm *tm = localtime(&now);
  char filename[256];
  char datetime[50];
  int ret_val = ERR_CNC_NOERROR;

  /* Format the filename with the current date and time */
  strftime(datetime, sizeof(datetime), "%Y-%m-%d_%H-%M-%S", tm);
  sprintf(filename, "diagnose/%s.json", datetime);

  /* Open the file for writing */
  file = fopen(filename, "w");
  if (!file)
  {
    printf("Failed to open file");
    return 1; // Return error code
  }

  // Write the JSON stream to the file
  fprintf(file, "%s", json_stream);

  // Close the file
  if (fclose(file) != 0)
  {
    printf("Failed to close file");
    return 1; // Return error code
  }

  return ret_val;
}

/*
+----------------------------------------------------------------------------+
| sample implementation to trigger the cnc diagnosis data upload             |
+----------------------------------------------------------------------------+
*/
void cnc_demo_upload_diag_data(void)
{
  CNC_OBJECT cnc_object;

  cnc_object.ObjectID.iThread = CNC_TASK_HMI;
  cnc_object.ObjectID.iGroup = 0x20101;
  cnc_object.ObjectID.iOffset = 0x2ab;

  int ret_value = cnc_write_BOOL(cnc_object.ObjectID, true);
  if (ret_value != ERR_CNC_NOERROR)
  {
    printf("\nFailed to upload diagnostic data.");
  }
}

/*
+----------------------------------------------------------------------------+
| demo feature to read line from cli menu                                    |
+----------------------------------------------------------------------------+
*/
void cnc_demo_read_cli_line(char* buf, int maxlen) {
  int i = 0;
  int ch;
  while (i < maxlen - 1) {
    ch = getchar();
    if (ch == '\r' || ch == '\n' || ch == EOF)
      break;
    buf[i++] = ch;
  }
  buf[i] = '\0';
}

/*
+----------------------------------------------------------------------------+
| demo cli menu                                                              |
+----------------------------------------------------------------------------+
*/
void *cnc_demo_menu(void* arg)
{

  const unsigned int* menu_activation_bitmap = (const unsigned int*)arg;

  char input[PATH_FILENAME_LENGTH] = "?";
  char command = '?';

  char ncprogramfile[PATH_FILENAME_LENGTH] = "nc_test.nc";
  char streamingfile[PATH_FILENAME_LENGTH] = "./prg/nc_stream_test.nc";
  char manualdatainput[PATH_FILENAME_LENGTH] = "X100 Y200 Z300 F100";

  FILE *p_nc_file = NULL;

  char file_name[PATH_FILENAME_LENGTH];
  char data_input[PATH_FILENAME_LENGTH];

  bool f_logging = true;
  bool f_error_messages = true;
  bool f_verbose = true;

  bool f_drives_on = false;
  bool f_stop_resume = false;
  bool f_streaming_active = false;
  bool f_write_csv = false;

  while (f_cnc_shutdown == false)
  {

    switch (command)
    {
    case 'm':
    case 'M':
    case '?':
    case '\r':
      printf("\n\n CncSDKDemo");
      printf("\n ==========");
      printf("\n\n Menu:");
      printf("\n m, M, ? : this help menu.");
      if (*menu_activation_bitmap & MENU_PRINT_RUNTIME_STATISTICS)
        printf("\n a, A    : Runtime statistics.");
      if (*menu_activation_bitmap & MENU_RESET_RUNTIME_STATISTICS)
        printf("\n b, B    : Reset runtime statistics.");
      if (*menu_activation_bitmap & MENU_WRITE_CSV)
        printf("\n c, C    : Enable/disable writing csv file (on/off).");
      if (*menu_activation_bitmap & MENU_ENABLE_DRIVES)
        printf("\n d, D    : Enable/disable drives (on/off).");
      if (*menu_activation_bitmap & MENU_ERROR_MESSAGES)
        printf("\n e, E    : Enable/disable error messages (on/off)");
      if (*menu_activation_bitmap & MENU_STOP_RESUME_PROGRAM)
        printf("\n h, H    : Stop / resume nc program.");
      if (*menu_activation_bitmap & MENU_LOGGING)
        printf("\n l, L    : Enable/disable logging (on/off).");
      if (*menu_activation_bitmap & MENU_GENERATE_DIAG_DATA)
        printf("\n i, I    : Generate diagnosis data.");
      if (*menu_activation_bitmap & MENU_SHOW_POS)
        printf("\n p, P    : Show axis positions");
      if (*menu_activation_bitmap & MENU_SHOW_CNC_OBJECTS)
        printf("\n o, O    : Print/Write to file available cnc objects.");
      if (*menu_activation_bitmap & MENU_SHUTDOWN)
        printf("\n q, Q    : Shutdown CncSDKDemo.");
      if (*menu_activation_bitmap & MENU_RESET_KERNEL)
        printf("\n r, R    : Reset ISG kernel.");
      if (*menu_activation_bitmap & MENU_START_PROGRAM)
        printf("\n s, S    : Start program.");
      if (*menu_activation_bitmap & MENU_MANUAL_INPUT)
        printf("\n t, T    : Manual data input.");
      if (*menu_activation_bitmap & MENU_UPDATE_CONFIG)
        printf("\n u, U    : Update configuration.");
      if (*menu_activation_bitmap & MENU_VERBOSE)
        printf("\n v, V    : Kernel verbose mode on/off.");
      if (*menu_activation_bitmap & MENU_STREAMING_MODE)
        printf("\n x, X    : Activate streaming mode.");
      printf("\n\n");
      break;

    case 'a':
    case 'A':
      if (*menu_activation_bitmap & MENU_PRINT_RUNTIME_STATISTICS)
      {
        cnc_demo_print_runtime_statistic();
      }
      break;

    case 'b':
    case 'B':
      if (*menu_activation_bitmap & MENU_RESET_RUNTIME_STATISTICS)
      {
        cnc_demo_reset_runtime_statistic();
      }
      break;

    case 'c':
    case 'C':
      if (*menu_activation_bitmap & MENU_WRITE_CSV)
      {
        f_write_csv = !f_write_csv;
        printf("CNC-MSG: Write csv file %s\n", f_write_csv ? "on" : "off");
      }
      break;

    case 'd':
    case 'D':
      if (*menu_activation_bitmap & MENU_ENABLE_DRIVES)
      {
        f_drives_on = !f_drives_on;
        printf("CNC-MSG: Drives switched %s.\n", f_drives_on ? "on" : "off");
        cnc_demo_enable_disable_drives(f_drives_on);
      }
      break;

    case 'e':
    case 'E':
      if (*menu_activation_bitmap & MENU_ERROR_MESSAGES)
      {
        f_error_messages = !f_error_messages;
        printf("CNC-MSG: Error messages switched %s.\n", f_error_messages ? "on" : "off");
      }
      break;

    case 'h':
    case 'H':
      if (*menu_activation_bitmap & MENU_STOP_RESUME_PROGRAM)
      {
        f_stop_resume = !f_stop_resume;
        printf("CNC-MSG: %s nc program.\n", f_stop_resume ? "Stopping" : "Resuming");
        cnc_demo_stop_resume_nc_program(f_stop_resume);
      }
      break;

    case 'l':
    case 'L':
      if (*menu_activation_bitmap & MENU_LOGGING)
      {
        f_logging = !f_logging;
        printf("CNC-MSG: Switched logging %s\n", f_logging ? "on" : "off");
      }
      break;

    case 'i':
    case 'I':
      if (*menu_activation_bitmap & MENU_GENERATE_DIAG_DATA)
      {
        printf("CNC-MSG: Uploading diagnostic data.\n");
        cnc_demo_upload_diag_data();
      }
      break;

    case 'o':
    case 'O':
      if (*menu_activation_bitmap & MENU_SHOW_CNC_OBJECTS)
      {
        cnc_demo_print_object_ids(f_write_csv);
      }
      break;

    case 'p':
    case 'P':
      if (*menu_activation_bitmap & MENU_SHOW_POS)
      {
        cnc_demo_show_positions();
      }
      break;

    case 'q':
    case 'Q':
      if (*menu_activation_bitmap & MENU_SHUTDOWN)
      {
        f_cnc_shutdown = true;
      }
      break;

    case 'r':
    case 'R':
      if (*menu_activation_bitmap & MENU_RESET_KERNEL)
      {
        printf("CNC-MSG: CNC Reset.\n");
        cnc_demo_reset();
      }
      break;

    case 's':
    case 'S':
      if (*menu_activation_bitmap & MENU_START_PROGRAM)
      {
        printf("CNC-MSG: Enter nc program file name [%s]:\n", ncprogramfile);
        file_name[0] = '\0';
        cnc_demo_read_cli_line(file_name, sizeof(file_name));

        if (file_name[0] != '\0' && file_name[0] != '\n')
        {
          memset(ncprogramfile, 0, sizeof(file_name));
          strncpy(ncprogramfile, file_name, strlen(file_name));
        }
        printf("CNC-MSG: Start nc program file: %s\n", ncprogramfile);
        cnc_demo_start_nc_program(ncprogramfile);
        printf("CNC-MSG: Program %s is running.\n", ncprogramfile);
      }
      break;

    case 't':
    case 'T':
      if (*menu_activation_bitmap & MENU_MANUAL_INPUT)
      {
        printf("CNC-MSG: Enter manual data input  [%s]:\n", manualdatainput);
        data_input[0] = '\0';
        cnc_demo_read_cli_line(data_input, sizeof(data_input));

        if (data_input[0] != '\0' && data_input[0] != '\n')
        {
          memset(manualdatainput, 0, sizeof(data_input));
          strncpy(manualdatainput, data_input, strlen(data_input));
        }
        printf("CNC-MSG: Execute manual data input: %s\n", manualdatainput);
        cnc_demo_start_manual_data_input(manualdatainput);
      }
      break;

    case 'u':
    case 'U':
      if (*menu_activation_bitmap & MENU_UPDATE_CONFIG)
      {
        cnc_demo_update_configuration();
      }
      break;

    case 'v':
    case 'V':
      if (*menu_activation_bitmap & MENU_VERBOSE)
      {
        f_verbose = !f_verbose;
        cnc_set_verbose_mode(f_verbose);
      }
      break;

    case 'x':
    case 'X':
      if (*menu_activation_bitmap & MENU_STREAMING_MODE)
      {
        printf("CNC-MSG: Enter streaming program file name [%s]:\n", streamingfile);
        data_input[0] = '\0';
        cnc_demo_read_cli_line(file_name, sizeof(file_name));
        if (file_name[0] != '\0' && file_name[0] != '\n')
        {
          memset(streamingfile, 0, sizeof(file_name));
          strncpy(streamingfile, file_name, strlen(file_name));
        }
        printf("CNC-MSG: Current streaming program file: %s\n", streamingfile);

        f_streaming_active = cnc_demo_start_streaming_program(&p_nc_file, streamingfile);
        printf("\nCNC-MSG: Streaming mode %s.", f_streaming_active ? "active" : "activation failed");
      }
      break;
    }

    if (f_cnc_shutdown)
      break;

    cnc_demo_read_cli_line(input, sizeof(input));
    command = input[0];

  }

  free((void*)menu_activation_bitmap);

  return 0;
}

/*
+----------------------------------------------------------------------------+
| Creation of cli menu thread                                                |
+----------------------------------------------------------------------------+
*/
void cnc_demo_create_menu_thread(unsigned int menu_bitmap)
{
  pthread_t id;
  pthread_attr_t attr;
  struct sched_param sched;

  pthread_attr_init(&attr);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

  unsigned int* p_bitmap = (unsigned int*) malloc(sizeof(unsigned int));
  *p_bitmap = menu_bitmap;

  pthread_create(&id, &attr, cnc_demo_menu, p_bitmap);

  sched.sched_priority = 0;
  pthread_setschedparam(id, SCHED_OTHER, &sched);
  pthread_setname_np(id, "MENU_THREAD");

  if (!id)
  {
    printf("Thread 'MENU_THREAD' could not be created!\n");
  }
  else
  {
    printf("Thread 'MENU_THREAD' created!\n");
  }
}

