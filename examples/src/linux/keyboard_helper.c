/*
Copyright (C)
ISG Industrielle Steuerungstechnik GmbH
https://www.isg-stuttgart.de
*/

/** \file
 * @brief TODO-DOC: Module description missing
*/

#include <stdbool.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "keyboard_helper.h"


/*
+----------------------------------------------------------------------------+
| Check, if any key is pressed                                               |
+----------------------------------------------------------------------------+
*/
bool kbhit()
{
  struct termios current;
  struct termios terminal;
  
  int    buffered_characters = 0;
  bool   key_pressed         = 0;

  tcgetattr(STDIN_FILENO, &current);

  memcpy(&terminal, &current, sizeof(terminal));

  terminal.c_lflag &= ~ICANON;
  tcsetattr(STDIN_FILENO, TCSANOW, &terminal);

  ioctl(STDIN_FILENO, FIONREAD, &buffered_characters);

  tcsetattr(STDIN_FILENO, TCSANOW, &current);

  key_pressed = (buffered_characters != 0);

  return key_pressed;
}

/*
+----------------------------------------------------------------------------+
| Deactivate output of characters when a key is pressed                      |
+----------------------------------------------------------------------------+
*/
void echo_off()
{
  struct termios terminal;

  tcgetattr(STDIN_FILENO, &terminal);
  terminal.c_lflag &= ~ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &terminal);
}


/*
+----------------------------------------------------------------------------+
| Activate output of characters when a key is pressed                        |
+----------------------------------------------------------------------------+
*/
void echo_on()
{
  struct termios terminal;

  tcgetattr(STDIN_FILENO, &terminal);
  terminal.c_lflag |= ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &terminal);
}
