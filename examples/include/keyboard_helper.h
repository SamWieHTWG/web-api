/*
Copyright (C)
ISG Industrielle Steuerungstechnik GmbH
https://www.isg-stuttgart.de
*/

/** \file
 * @brief TODO-DOC: Module description missing
*/

#ifndef KEYBOARD_HELPER_H
#define KEYBOARD_HELPER_H

/*
+----------------------------------------------------------------------------+
| keyboard_helper.h                                     | Copyright    ISG   |
|                                                       |                    |
|                                                       | Gropiusplatz 10    |
|                                                       | 70563 Stuttgart    |
+-------------------------------------------------------+--------------------+
|  Windows helper functions for keyboard.                                    |
+----------------------------------------------------------------------------+
*/

/*
+----------------------------------------------------------------------------+
| Function prototypes                                                        |
+----------------------------------------------------------------------------+
*/
bool kbhit(void);
void echo_off();
void echo_on();

#endif /* KEYBOARD_HELPER_H */
