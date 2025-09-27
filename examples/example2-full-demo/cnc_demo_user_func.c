/*
Copyright (C)
ISG Industrielle Steuerungstechnik GmbH
https://www.isg-stuttgart.de
*/

/** \file
 * @brief TODO-DOC: Module description missing
*/

#include "cnc_demo_user_func.h"

/*
+----------------------------------------------------------------------------+
| cnc_demo_user_func.h                                  | Copyright    ISG   |
|                                                       |                    |
|                                                       | Gropiusplatz 10    |
|                                                       | 70563 Stuttgart    |
+-------------------------------------------------------+--------------------+
| CNC SDK Demo user functions.                                               |
|                                                                            |
| Functions to run user code in the context of the CNC tasks.                |
+----------------------------------------------------------------------------+
*/

/*
+-----------------------------------------------------------------------------+
| User event functions                                                        |
+-----------------------------------------------------------------------------+
*/
int cnc_user_event_ipo(unsigned int context_info /*P-RTCF-00017*/)
{
  return 0;
}

int cnc_user_event_dec(unsigned int context_info)
{
  return 0;
}

int cnc_user_event_hmi(unsigned int context_info)
{
  return 0;
}

int cnc_user_event_sys(unsigned int context_info)
{
  return 0;
}
