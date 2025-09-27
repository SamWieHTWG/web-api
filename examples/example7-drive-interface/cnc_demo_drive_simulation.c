/*
Copyright (C)
ISG Industrielle Steuerungstechnik GmbH
https://www.isg-stuttgart.de
*/

/** \file
 * @brief TODO-DOC: Module description missing
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "cnc_os_ifc.h"
#include "cnc_demo.h"
#include "cnc_demo_drive_simulation.h"

/*
+----------------------------------------------------------------------------+
| cnc_demo_drive_simulation.c                           | Copyright    ISG   |
|                                                       |                    |
|                                                       | Gropiusplatz 10    |
|                                                       | 70563 Stuttgart    |
+-------------------------------------------------------+--------------------+
| CNC SDK Demo drive simulation.                                             |
|                                                                            |
| CNC SDK Demo drive simulation for CoE/SoE.                                 |
+----------------------------------------------------------------------------+
*/

/*
+-----------------------------------------------------------------------------+
| Definitions                                                                 |
+-----------------------------------------------------------------------------+
*/

static int            c_drive_simulation_tick         = 0;
static int            drive_simulation_cycle          = 10000;
static bool           f_enable_drive_simulation_print = false;

static unsigned int   i_drive_print_1 = 99; /* 99 = disabled */
static unsigned int   i_drive_print_2 = 99;
static unsigned int   i_drive_print_3 = 99;

static bool           f_drive_simulation_init         = false;
static bool           f_drive_error_simu_activated    = false;

static bool           f_simulated_drive_init[CNC_OS_IFC_NO_AXES_MAX];

static CNC_SOE_SERV_CHAN_REQUEST soe_serv_chan_request;
static CNC_COE_SDO_REQUEST       coe_sdo_request;

/*
+-----------------------------------------------------------------------------+
| Declarations                                                                |
+-----------------------------------------------------------------------------+
*/
/*
+-----------------------------------------------------------------------------+
| Simulation of CANopen drives with position control                          |
|                                                                             |
| PDO command:  1. DRIVE_CTRL   (4 bytes)                                     |
|               2. POS_NOM      (4 bytes)                                     |
|               3. VELO_NOM     (4 bytes)                                     |
|               4. OP_MODE      (1 byte)                                      |
|                                                                             |
| PDO feedback: 1. DRIVE_STATUS (4 bytes)                                     |
|               2. POS_ACT      (4 bytes)                                     |
|               3. VEL_ACT      (4 bytes)                                     |
|               4. OP_MODE_ACT  (1 byte)                                      |
|               5. WCSTATE      (2 bytes)                                     |
+-----------------------------------------------------------------------------+
*/

int cnc_demo_drive_ifc_simulation_coe(unsigned int i_drive, CNC_DRIVE_COMMAND_COE *p_drive_command, CNC_DRIVE_STATUS_COE * p_drive_feedback)
{
  /* for diagnosis */
  static unsigned short drive_control[CNC_OS_IFC_NO_AXES_MAX];
  static signed long    command_position[CNC_OS_IFC_NO_AXES_MAX];
  static signed long    command_velocity[CNC_OS_IFC_NO_AXES_MAX];
  static signed char    op_mode[CNC_OS_IFC_NO_AXES_MAX];

  static unsigned short drive_status[CNC_OS_IFC_NO_AXES_MAX];
  static signed long    feedback_position[CNC_OS_IFC_NO_AXES_MAX];
  static signed long    feedback_velocity[CNC_OS_IFC_NO_AXES_MAX];
  static signed char    op_mode_act[CNC_OS_IFC_NO_AXES_MAX];
  static unsigned short wc_state[CNC_OS_IFC_NO_AXES_MAX];
  static unsigned long  request_iteration_counter;
  static unsigned short torque_limit;
  static unsigned long  sdo_val;

  c_drive_simulation_tick++;

  if (f_drive_simulation_init == false)
  {
    memset(&f_simulated_drive_init[0], 0, CNC_OS_IFC_NO_AXES_MAX);
    f_drive_simulation_init = true;
  }


  // Input command values
  drive_control[i_drive]    = (unsigned short)p_drive_command->drive_ctrl;
  command_position[i_drive] = p_drive_command->cmd_pos;
  command_velocity[i_drive] = p_drive_command->cmd_vel;
  op_mode[i_drive]          = p_drive_command->cmd_op_mode;


  if (f_simulated_drive_init[i_drive] == false)
  {
    f_simulated_drive_init[i_drive] = true;
    /*
    +-------------------------------------------------------------------------+
    | Initialization                                                          |
    +-------------------------------------------------------------------------+
    */
    wc_state[i_drive] = 0;
    drive_status[i_drive] = COE_LEVEL_1_SWITCH_ON_DISABLED;
    feedback_position[i_drive] = 0;
    feedback_velocity[i_drive] = 0;
    op_mode_act[i_drive] = op_mode[i_drive];
    wc_state[i_drive] = 0x0;
  }
  else
  {
    drive_control[i_drive]    = (unsigned short)p_drive_command->drive_ctrl;
    command_position[i_drive] = p_drive_command->cmd_pos;
    command_velocity[i_drive] = p_drive_command->cmd_vel;
    op_mode[i_drive]          = p_drive_command->cmd_op_mode;

    /*
    +----------------------------------------------------------------------------+
    | drive state machine                                                        |
    +----------------------------------------------------------------------------+
    */
    switch ( drive_status[i_drive] )
    {
      // ************************************************
    case COE_LEVEL_0_NOT_READY_TO_SWITCH_ON:
       drive_status[i_drive]  = COE_LEVEL_1_SWITCH_ON_DISABLED;

      // Write initial position
       command_position[i_drive] = 0;
      break;

      // ************************************************
    case COE_LEVEL_1_SWITCH_ON_DISABLED:
      if (drive_control[i_drive]  == COE_GO_TO_LEVEL_2_READY_TO_SWITCH_ON)
      {
         drive_status[i_drive]  = COE_LEVEL_2_READY_TO_SWITCH_ON;
      }
      break;

      // ************************************************
    case COE_LEVEL_2_READY_TO_SWITCH_ON:
      if (drive_control[i_drive]  == COE_GO_TO_LEVEL_3_SWITCHED_ON)
      {
         drive_status[i_drive]  = COE_LEVEL_3_SWITCHED_ON;
      }
      else if (drive_control[i_drive]  == COE_GO_TO_LEVEL_1_SWITCH_ON_DISABLED)
      {
         drive_status[i_drive]  = COE_LEVEL_1_SWITCH_ON_DISABLED;
      }
      break;

      // ************************************************
    case COE_LEVEL_3_SWITCHED_ON:
      if ((drive_control[i_drive]  == COE_GO_TO_LEVEL_4_OPERATION_ENABLED)
        || (drive_control[i_drive]  == COE_GO_TO_LEVEL_4_OPERATION_ENABLED_VELO)
        )
      {
         drive_status[i_drive]  = COE_LEVEL_4_OPERATION_ENABLED;
      }
      else if (drive_control[i_drive]  == COE_GO_TO_LEVEL_2_READY_TO_SWITCH_ON)
      {
         drive_status[i_drive]  = COE_LEVEL_2_READY_TO_SWITCH_ON;
      }
      break;

      // ************************************************
    case COE_LEVEL_4_OPERATION_ENABLED:
      if (drive_control[i_drive]  == COE_GO_TO_LEVEL_3_SWITCHED_ON)
      {
         drive_status[i_drive]  = COE_LEVEL_3_SWITCHED_ON;
      }
      else if (drive_control[i_drive]  == COE_GO_TO_LEVEL_1_SWITCH_ON_DISABLED)
      {
         drive_status[i_drive]  = COE_LEVEL_1_SWITCH_ON_DISABLED;
      }

      break;

      // ************************************************
    default:
       drive_status[i_drive]  = COE_LEVEL_1_SWITCH_ON_DISABLED;
      break;
    }

    if( op_mode[i_drive] == 9 )
      feedback_position[i_drive] += command_velocity[i_drive];      // Velocity control mode
    else
      feedback_position[i_drive] = command_position[i_drive];      // Position control mode

    feedback_velocity[i_drive] = command_velocity[i_drive];
    op_mode_act[i_drive] = op_mode[i_drive];
  }

  /*
  +----------------------------------------------------------------------------+
  | error simulation to test service channel reset.                            |
  +----------------------------------------------------------------------------+
  */
  if (i_drive == 0)
  {
    if (f_drive_error_simu_activated )
      drive_status[i_drive] |= COE_STATE_ERROR;
    else
      drive_status[i_drive]   &= ~COE_STATE_ERROR;

    if (drive_control[i_drive] & COE_CMD_STATE_ERROR_QUIT)
    {
      f_drive_error_simu_activated = false;
    }
  }

  // Output actual values
  p_drive_feedback->drive_status = (unsigned short) drive_status[i_drive];
  p_drive_feedback->act_pos      = feedback_position[i_drive];
  p_drive_feedback->act_vel      = feedback_velocity[i_drive];
  p_drive_feedback->act_op_mode  = op_mode_act[i_drive];
  p_drive_feedback->wcstate      = wc_state[i_drive];


  /*
  +----------------------------------------------------------------------------+
  |                                                                            |
  | Asynchronous SDO Communication                                             |
  | Requests are processed here as a simple example.                           |
  | To simulate the processing time, the response is waited for a few ticks.   |
  +----------------------------------------------------------------------------+
  */
  CNC_COE_SDO_REQUEST* request = &coe_sdo_request;
  if(request->request_active)
  {

    if (request_iteration_counter == 0)
    {
      /* set iteration counter if it is on initial value */
      request_iteration_counter = CANOPEN_ASYN_SIMULATION_PROCESSING_TICKS;
    }
    else
    {
      /* decrease iteration counter to simulate processing time of the device */
      request_iteration_counter -= 1;
    }

    /* The simulated processing time has elapsed */
    /* -> process the request and send response  */
    if(request_iteration_counter==0)
    {
      switch (request->object_index)
      {
        case ID_SDO_TORQUE_LIMIT:
        {
          if (request->is_read_request == false)
          {
            torque_limit = *(unsigned short*)request->pData;
            /* ... value is written to device ... */
          }
          else
          {
            request->pData = &torque_limit;
          }
        }
        break;
        default:
        if (request->is_read_request == true)
        {
          request->pData = &sdo_val;
        }       
      }

      /* Request is processed -> send response */
      if (request->is_read_request)
      {
        cnc_canopen_sdo_read_res( request->invokeId, CANOPEN_SDO_NO_ERROR, request->size, request->pData);
      }
      else
      {
        cnc_canopen_sdo_write_res( request->invokeId, CANOPEN_SDO_NO_ERROR);
      }
      request->request_active = false;
    }
  }

  // Diagnosis prints
  if (f_enable_drive_simulation_print == true)
  {
    if (c_drive_simulation_tick %drive_simulation_cycle == 0)
    {
      if (i_drive_print_1 != 99)
      {
        printf("A%ld: CMD_POS=%ld CMD_VEL=%ld OP_MODE=%d", i_drive_print_1+1, command_position[i_drive_print_1], command_velocity[i_drive_print_1], op_mode[i_drive_print_1]);
        printf(" | POS_ACT=%ld VEL_ACT=%ld OP_MODE_ACT=%d\n", feedback_position[i_drive_print_1], feedback_velocity[i_drive_print_1], op_mode_act[i_drive_print_1]);
      }

      if (i_drive_print_2 != 99)
      {
        printf("A%ld: CMD_POS=%ld CMD_VEL=%ld OP_MODE=%d", i_drive_print_2+1, command_position[i_drive_print_2], command_velocity[i_drive_print_2], op_mode[i_drive_print_2]);
        printf(" | POS_ACT=%ld VEL_ACT=%ld OP_MODE_ACT=%d\n", feedback_position[i_drive_print_2], feedback_velocity[i_drive_print_2], op_mode_act[i_drive_print_2]);
      }

      if (i_drive_print_3 != 99)
      {
        printf("A%ld: CMD_POS=%ld CMD_VEL=%ld OP_MODE=%d", i_drive_print_3+1, command_position[i_drive_print_3], command_velocity[i_drive_print_3], op_mode[i_drive_print_3]);
        printf(" | POS_ACT=%ld VEL_ACT=%ld OP_MODE_ACT=%d\n", feedback_position[i_drive_print_3], feedback_velocity[i_drive_print_3], op_mode_act[i_drive_print_3]);
      }
    }
  }

  return(0);

} /* End: cnc_demo_drive_ifc_simulation_coe() */

/*
+----------------------------------------------------------------------------+
| send request to drive - simple example                                     |
+----------------------------------------------------------------------------+
*/
void cnc_demo_sdo_request_to_drive(unsigned long invokeId, long logAxisNo, unsigned long object_index, unsigned long object_sub_index, unsigned long size, void* pData, bool is_read)
{
  /*
  +----------------------------------------------------------------------------+
  | Saving the request for further processing by the drive.                    |
  | Here only as an simple example - this would be typically be                |
  | done by a FIFO buffer.                                                     |
  +----------------------------------------------------------------------------+
  */
  if (coe_sdo_request.request_active == false)
  {
    coe_sdo_request.request_active   = true;
    coe_sdo_request.is_read_request  = is_read;
    coe_sdo_request.invokeId         = invokeId;
    coe_sdo_request.logAxisNo        = logAxisNo;
    coe_sdo_request.object_index     = object_index;
    coe_sdo_request.object_sub_index = object_sub_index;
    coe_sdo_request.size             = size;
    coe_sdo_request.pData            = pData;
  }
}

/*
+----------------------------------------------------------------------------+
| service data object write request callback - example implementation        |
+----------------------------------------------------------------------------+
*/
int32_t cnc_demo_canopen_asynch_callback_write_req  (uint32_t invokeId, int32_t logAxisNo, uint32_t object_index, uint32_t object_sub_index, uint32_t size, void* value)
{
  cnc_demo_sdo_request_to_drive(invokeId, logAxisNo, object_index, object_sub_index, size, value, false);
  return CANOPEN_SDO_REQ_CALLBACK_NO_ERR;
}

/*
+----------------------------------------------------------------------------+
| service data object read request callback - example implementation         |
+----------------------------------------------------------------------------+
*/
int32_t cnc_demo_canopen_asynch_callback_read_req   (uint32_t invokeId, int32_t logAxisNo, uint32_t object_index, uint32_t object_sub_index, uint32_t size)
{
  cnc_demo_sdo_request_to_drive(invokeId, logAxisNo, object_index, object_sub_index, size, 0, true);
  return CANOPEN_SDO_REQ_CALLBACK_NO_ERR;
}

/*
+-----------------------------------------------------------------------------+
| Simulation of Sercos drives with position control                           |
+-----------------------------------------------------------------------------+
*/
int cnc_demo_drive_ifc_simulation_soe(unsigned int i_drive, CNC_DRIVE_COMMAND_SOE *p_drive_command, CNC_DRIVE_STATUS_SOE * p_drive_feedback)
{
  unsigned short drive_control;
  unsigned short drive_status;
  signed long    command_position;
  static unsigned short bus_state = 0;
  static unsigned long  request_iteration_counter;
  static unsigned long  soe_val;

  c_drive_simulation_tick++;

  /*
  +----------------------------------------------------------------------------+
  | Synchronous Communication                                                  |
  +----------------------------------------------------------------------------+
  */

  drive_control    = (unsigned short)p_drive_command->drive_ctrl;
  command_position = p_drive_command->cmd_pos;

  if (f_simulated_drive_init[i_drive] == false)
  {
    /* Initialization  */
    f_simulated_drive_init[i_drive] = true;

    /* set feedback to setpoint */
    p_drive_feedback->act_pos = 0;    // actual position of drive

    /* Set bus to valid / drive to powered */
    drive_status = SERC_BUS_STATUS_DRIVE_POWER;
    bus_state = SERC_BUS_STATE_CYCLIC_COMM | SERC_BUS_STATE_ACYCLIC_COMM;
  }
  else
  {
    if ((drive_control & SERC_DRIVE_CTRL_ENABLE) == SERC_DRIVE_CTRL_ENABLE)
    {
      /* set bus valid flags */
      drive_status = SERC_BUS_STATUS_DRIVE_POWER | SERC_BUS_STATUS_DRIVE_READY;
      p_drive_feedback->act_pos = p_drive_command->cmd_pos;
    }
    else
    {
      drive_status = SERC_BUS_STATUS_DRIVE_POWER;
    }
  }


  /*
  +----------------------------------------------------------------------------+
  |                                                                            |
  | Asynchronous Communication (service channel)                               |
  | Requests are processed here as a simple example.                           |
  | To simulate the processing time, the response is waited for a few ticks.   |
  +----------------------------------------------------------------------------+
  */
  CNC_SOE_SERV_CHAN_REQUEST* request = &soe_serv_chan_request;
  if(request->request_active)
  {

    if (request_iteration_counter == 0)
    {
      /* set iteration counter if it is on initial value */
      request_iteration_counter = SERC_ASYN_SIMULATION_PROCESSING_TICKS;
    }
    else
    {
      /* decrease iteration counter to simulate processing time of the device */
      request_iteration_counter -= 1;
    }

    /* The simulated processing time has elapsed */
    /* -> process the request and send response  */
    if(request_iteration_counter==0)
    {
      switch (request->sercId)
      {
      case ID_NUMBER_TORQUE_LIMIT_POS:
        {
          if (request->is_read_request == false)
          {
            unsigned short torque_limit = *(unsigned short*)request->pData;
            /* ... value is written to device ... */
          }
        }
        break;
      case ID_NUMBER_TSCYC:
        {
          if (request->is_read_request)
          {
            /* request data is filled with dummy value */
            unsigned short time = 0;
            request->pData = &time;
            request->size = sizeof(time);
          }
        }
        break;

      case ID_NUMBER_RESET:
        if (request->is_read_request)
        {
          short sercos_data_status = SERC_CMD_STATE_STARTED_AND_FINISHED;
          request->pData = &sercos_data_status;
          request->size = sizeof(sercos_data_status);
        }
        else
        {
          /* reset simulated sercos error */
          f_drive_error_simu_activated = false;
        }
        break;

      default:
        if (request->is_read_request == true)
        {
          /* use placeholder data if not processed */
          request->pData = &soe_val;
        }
      }

      /* Request is processed -> send response */
      if (request->is_read_request)
      {
        cnc_serc_service_channel_read_res( request->invokeId, SERC_SERV_CHAN_NO_ERROR, request->size, request->pData);
      }
      else
      {
        cnc_serc_service_channel_write_res( request->invokeId, SERC_SERV_CHAN_NO_ERROR);
      }
      request->request_active = false;
    }
  }

  /*
  +----------------------------------------------------------------------------+
  | error simulation to test service channel reset.                            |
  +----------------------------------------------------------------------------+
  */
  if (i_drive == 0)
  {
    if (f_drive_error_simu_activated )
      drive_status |= SERC_BUS_STATUS_DRIVE_ERROR;
    else
      drive_status &= ~SERC_BUS_STATUS_DRIVE_ERROR;
  }

  /* Output actual values */
  p_drive_feedback->drive_status = (unsigned short) drive_status;
  p_drive_feedback->bus_state    = bus_state;

  return(0);

} /* End: cnc_demo_drive_ifc_simulation_soe() */

/*
+----------------------------------------------------------------------------+
| send request to drive - simple example                                     |
+----------------------------------------------------------------------------+
*/
void cnc_demo_send_request_to_drive(unsigned long invokeId, long logAxisNo, unsigned long sercId, unsigned long sercElement, unsigned long size, void* pData, bool is_read)
{
  /*
  +----------------------------------------------------------------------------+
  | Saving the request for further processing by the drive.                    |
  | Here only as an simple example - this would be typically be                |
  | done by a FIFO buffer.                                                     |
  +----------------------------------------------------------------------------+
  */
  if (soe_serv_chan_request.request_active == false)
  {
    soe_serv_chan_request.request_active = true;
    soe_serv_chan_request.is_read_request = is_read;
    soe_serv_chan_request.invokeId = invokeId;
    soe_serv_chan_request.logAxisNo = logAxisNo;
    soe_serv_chan_request.sercId = sercId;
    soe_serv_chan_request.sercElement = (E_SERC_ELEMENT_STATE) sercElement;
    soe_serv_chan_request.size = size;
    soe_serv_chan_request.pData = pData;
  }
}

/*
+----------------------------------------------------------------------------+
| sercos service channel write request callback - example implementation     |
+----------------------------------------------------------------------------+
*/
int cnc_demo_sercos_asynch_callback_write_req(unsigned int invokeId, int logAxisNo, unsigned int sercId, unsigned int sercElement, unsigned int size, void* pData)
{
  cnc_demo_send_request_to_drive(invokeId, logAxisNo, sercId, sercElement, size, pData, false);
  return SERC_SERV_CHAN_REQ_CALLBACK_NO_ERR;
}

/*
+----------------------------------------------------------------------------+
| sercos service channel read request callback - example implementation      |
+----------------------------------------------------------------------------+
*/
int cnc_demo_sercos_asynch_callback_read_req(unsigned int invokeId, int logAxisNo, unsigned int sercId, unsigned int sercElement, unsigned int size)
{
  cnc_demo_send_request_to_drive(invokeId, logAxisNo, sercId, sercElement, size, 0, true);
  return SERC_SERV_CHAN_REQ_CALLBACK_NO_ERR;
}

/*
+----------------------------------------------------------------------------+
| activation of sercos error simulation                                      |
+----------------------------------------------------------------------------+
*/
void cnc_demo_set_drive_err_simu(bool activate)
{
  f_drive_error_simu_activated = activate;
}
