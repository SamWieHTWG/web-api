/*
Copyright (C)
ISG Industrielle Steuerungstechnik GmbH
https://www.isg-stuttgart.de
*/

/** \file
 * @brief TODO-DOC: Module description missing
*/
#ifndef CNC_DEMO_DRIVE_SIMULATION_H
#define CNC_DEMO_DRIVE_SIMULATION_H

/*
+----------------------------------------------------------------------------+
| cnc_demo_drive_simulation.h                           | Copyright    ISG   |
|                                                       |                    |
|                                                       | Gropiusplatz 10    |
|                                                       | 70563 Stuttgart    |
+-------------------------------------------------------+--------------------+
| CNC SDK Demo drive simulation header file.                                 |
|                                                                            |
| CNC SDK Demo drive simulation for CoE/SoE.                                 |
+----------------------------------------------------------------------------+
*/

/*
+----------------------------------------------------------------------------+
| Drive Interface structures                                                 |
+----------------------------------------------------------------------------+
*/
#pragma pack (push, 1)

typedef struct _cnc_drive_command_coe
{
  uint16_t          drive_ctrl;
  int32_t           cmd_pos;
  int32_t           cmd_vel;
  uint8_t           cmd_op_mode;
} CNC_DRIVE_COMMAND_COE;

typedef struct _cnc_drive_status_coe
{
  uint16_t          drive_status;
  int32_t           act_pos;
  int32_t           act_vel;
  uint8_t           act_op_mode;
  uint16_t          wcstate;
} CNC_DRIVE_STATUS_COE;

typedef struct _cnc_drive_command_soe
{
  uint16_t          drive_ctrl;
  int32_t           cmd_pos;
} CNC_DRIVE_COMMAND_SOE;

typedef struct _cnc_drive_status_soe
{
  uint16_t          drive_status;
  int32_t           act_pos;
  uint16_t          bus_state;
} CNC_DRIVE_STATUS_SOE;

typedef struct _cnc_soe_serv_chan_request
{
  bool                 request_active;
  bool                 is_read_request;
  unsigned long        invokeId;
  long                 logAxisNo;
  unsigned long        sercId;
  E_SERC_ELEMENT_STATE sercElement;
  unsigned long        size;
  void*                pData;
} CNC_SOE_SERV_CHAN_REQUEST;

typedef struct _cnc_coe_sdo_request
{
  bool                 request_active;
  bool                 is_read_request;
  unsigned long        invokeId;
  long                 logAxisNo;
  unsigned long        object_index;
  unsigned long        object_sub_index;
  unsigned long        size;
  void*                pData;
} CNC_COE_SDO_REQUEST;

#pragma pack (pop)

/* defines for CAN */
#define COE_LEVEL_0_NOT_READY_TO_SWITCH_ON                0x00 
#define COE_LEVEL_1_SWITCH_ON_DISABLED                    0x40 
#define COE_LEVEL_2_READY_TO_SWITCH_ON                    0x21
#define COE_LEVEL_3_SWITCHED_ON                           0x23 
#define COE_LEVEL_4_OPERATION_ENABLED                     0x27 
#define COE_STATE_ERROR                                   0x08
#define COE_GO_TO_LEVEL_1_SWITCH_ON_DISABLED              0x00
#define COE_GO_TO_LEVEL_2_READY_TO_SWITCH_ON              0x06
#define COE_GO_TO_LEVEL_3_SWITCHED_ON                     0x07
#define COE_GO_TO_LEVEL_4_OPERATION_ENABLED               0x1F
#define COE_GO_TO_LEVEL_4_OPERATION_ENABLED_VELO          0x0F
#define COE_CMD_STATE_ERROR_QUIT                          0x80

/* defines for sercos result */
#define SERC_SERV_CHAN_NO_ERROR                           0
#define SERC_SERV_CHAN_REQ_CALLBACK_NO_ERR                0
#define SERC_SERV_CHAN_REQ_CALLBACK_ERR                   1


#define SERC_DRIVE_CTRL_FEEDHOLD                          0x2000                            /* SERCOS MDT control bit 13, Drive release feedhold                             */
#define SERC_DRIVE_CTRL_RELEASE                           0x4000                            /* SERCOS MDT control bit 14, Drive release on/off                               */
#define SERC_DRIVE_CTRL_DRIVE_ON                          0x8000                            /* SERCOS MDT control bit 15, Drive on/off                                       */
#define SERC_DRIVE_CTRL_ENABLE                            (SERC_DRIVE_CTRL_FEEDHOLD | SERC_DRIVE_CTRL_RELEASE | SERC_DRIVE_CTRL_DRIVE_ON)


/* defines for sercos status */
#define SERC_BUS_STATUS_DRIVE_ERROR                       0x2000
#define SERC_BUS_STATUS_DRIVE_POWER                       0x4000
#define SERC_BUS_STATUS_DRIVE_READY                       0x8000
#define SERC_BUS_STATUS_DRIVE_INIT                        0xc000

/* defines for sercos bus state*/
#define SERC_BUS_STATE_ACYCLIC_COMM                       0x1   
#define SERC_BUS_STATE_CYCLIC_COMM                        0x2   
#define SERC_BUS_STATE_INVALID                            0x80

/* used sercos service ids */
#define ID_NUMBER_TSCYC                                   2
#define ID_NUMBER_TORQUE_LIMIT_POS                        82
#define ID_NUMBER_RESET                                   99

/* sercos data status */
#define SERC_CMD_STATE_INACTIVE                           0x0000
#define SERC_CMD_STATE_REQUESTED                          0x0001
#define SERC_CMD_STATE_NOT_SUSPENDED                      0x0002
#define SERC_CMD_STATE_NOT_FINISHED                       0x0004
#define SERC_CMD_STATE_ERROR                              0x0008
#define SERC_CMD_STATE_NOT_VALID                          0x0100
#define SERC_CMD_STATE_STARTED_AND_FINISHED               (SERC_CMD_STATE_REQUESTED | SERC_CMD_STATE_NOT_SUSPENDED)
#define SERC_CMD_STATE_STARTED_AND_NOT_FINISHED           (SERC_CMD_STATE_REQUESTED | SERC_CMD_STATE_NOT_SUSPENDED | SERC_CMD_STATE_NOT_FINISHED)
#define SERC_CMD_STATE_STARTED_AND_FINISHED_WITH_ERROR    (SERC_CMD_STATE_REQUESTED | SERC_CMD_STATE_NOT_SUSPENDED | SERC_CMD_STATE_NOT_FINISHED | SERC_CMD_STATE_ERROR)

#define SERC_ASYN_SIMULATION_PROCESSING_TICKS             10


#define ID_SDO_TORQUE_LIMIT                               0x6072                                     
#define CANOPEN_ASYN_SIMULATION_PROCESSING_TICKS          10

/* defines for CANopen SDO access */
#define CANOPEN_SDO_NO_ERROR                              0
#define CANOPEN_SDO_REQ_CALLBACK_NO_ERR                   0
#define CANOPEN_SDO_REQ_CALLBACK_ERR                      1
/*
+----------------------------------------------------------------------------+
| Function prototypes                                                        |
+----------------------------------------------------------------------------+
*/
int  cnc_demo_drive_ifc_simulation_coe          (unsigned int i_drive, CNC_DRIVE_COMMAND_COE* p_drive_command, CNC_DRIVE_STATUS_COE* p_drive_feedback);
int  cnc_demo_drive_ifc_simulation_soe          (unsigned int i_drive, CNC_DRIVE_COMMAND_SOE* p_drive_command, CNC_DRIVE_STATUS_SOE* p_drive_feedback);
void cnc_demo_set_drive_err_simu                (bool activate);

int32_t cnc_demo_sercos_asynch_callback_write_req  (uint32_t invokeId, int32_t logAxisNo, uint32_t sercId, uint32_t sercElement, uint32_t size, void* pData);
int32_t cnc_demo_sercos_asynch_callback_read_req   (uint32_t invokeId, int32_t logAxisNo, uint32_t sercId, uint32_t sercElement, uint32_t size);

int32_t cnc_demo_canopen_asynch_callback_write_req  (uint32_t invokeId, int32_t logAxisNo, uint32_t object_index, uint32_t object_sub_index, uint32_t size, void* value);
int32_t cnc_demo_canopen_asynch_callback_read_req   (uint32_t invokeId, int32_t logAxisNo, uint32_t object_index, uint32_t object_sub_index, uint32_t size);

# endif /* CNC_DEMO_DRIVE_SIMULATION_H */
