/*
Copyright (C)
ISG Industrielle Steuerungstechnik GmbH
https://www.isg-stuttgart.de
*/

/** \file
 * @brief TODO-DOC: Module description missing
*/
#include <conio.h>
#include <pthread.h>
#include <windows.h>
#include <unistd.h>

/*
+----------------------------------------------------------------------------+
| demo_helper_w32.c                                     | Copyright    ISG   |
|                                                       |                    |
|                                                       | Gropiusplatz 10    |
|                                                       | 70563 Stuttgart    |
+-------------------------------------------------------+--------------------+
| Windows helper functions for keyboard and wrapper on threading functions   |
| according to Posix standard.                                               |
+----------------------------------------------------------------------------+
*/

bool kbhit_w32()
{
  return _kbhit() > 0 ? true : false;
}

void echo_on()
{
  return;
}

void echo_off()
{
  return;
}

int pthread_attr_init(pthread_attr_t *attr)
{
  return 0;
}

int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched)
{
  return 0;
}

int pthread_create(pthread_t *thread,
                   const pthread_attr_t *attr,
                   void *(*start_routine)(void *),
                   void *arg)
{
  DWORD thread_id;
  HANDLE hThread = CreateThread(
    NULL,                // default security attributes
    0,                   // default stack size
    (LPTHREAD_START_ROUTINE)start_routine,
    arg,                 // pass user argument
    0,                   // run immediately
    &thread_id           // optional: thread ID
  );

  if (hThread == NULL)
    return -1;

  *thread = hThread;
  return 0;
}


int pthread_setschedparam(pthread_t thread, int policy,
                          const struct sched_param *param)
{
  int priority = THREAD_PRIORITY_NORMAL;
  if (param->sched_priority > 50)
  {
    priority = THREAD_PRIORITY_HIGHEST;
  }
  else if (param->sched_priority > 0)
  {
    priority = THREAD_PRIORITY_ABOVE_NORMAL;
  }
  if (TRUE == SetThreadPriority(thread, priority))
  {
    return 0;
  }
  return -1;
}

int pthread_setname_np(pthread_t thread, const char *name)
{
  return 0;
}

int pthread_setaffinity_np(pthread_t thread, size_t __cpusetsize, const cpu_set_t* __cpuset)
{
  return 0;
}
