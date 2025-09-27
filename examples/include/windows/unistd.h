/*
Copyright (C)
ISG Industrielle Steuerungstechnik GmbH
https://www.isg-stuttgart.de
*/

/** \file
 * @brief TODO-DOC: Module description missing
*/
#include <stdbool.h>
#include <conio.h>

/*
+----------------------------------------------------------------------------+
| unistd.h                                              | Copyright    ISG   |
|                                                       |                    |
|                                                       | Gropiusplatz 10    |
|                                                       | 70563 Stuttgart    |
+-------------------------------------------------------+--------------------+
|                                                                            |
+----------------------------------------------------------------------------+
*/

/* Disable warning concerning insecure functions */
#pragma warning (disable: 4996)

bool kbhit_w32();

#define kbhit           kbhit_w32
#define usleep(a)       Sleep(a/1000)
