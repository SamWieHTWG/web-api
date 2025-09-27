/*
Copyright (C)
ISG Industrielle Steuerungstechnik GmbH
https://www.isg-stuttgart.de
*/

/** \file
 * @brief TODO-DOC: Module description missing
*/
#ifndef PTHREAD_H
#define PTHREAD_H

#include <windows.h>

/*
+----------------------------------------------------------------------------+
| pthread.h                                             | Copyright    ISG   |
|                                                       |                    |
|                                                       | Gropiusplatz 10    |
|                                                       | 70563 Stuttgart    |
+-------------------------------------------------------+--------------------+
| Wrapper on threading functions according to Posix standard.                |
+----------------------------------------------------------------------------+
*/

#define PTHREAD_EXPLICIT_SCHED    0
#define PTHREAD_INHERIT_SCHED     1

/* Scheduling policies */
#define SCHED_OTHER               0
#define SCHED_IDLE                1 
#define SCHED_BATCH               2 
#define SCHED_FIFO                3 
#define SCHED_RR                  4 

#define __CPU_SETSIZE	            1024
#define __NCPUBITS	              (8 * sizeof (unsigned long int))

#define CPU_ZERO(cpusetp)
#define CPU_SET(cpu, cpusetp)


typedef   HANDLE       pthread_t;
typedef   unsigned int pthread_attr_t;

typedef struct sched_param
{
  int sched_priority;

} sched_param;

typedef struct
{
  unsigned long int __bits[__CPU_SETSIZE / __NCPUBITS];
} cpu_set_t;

int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched);

int pthread_create(pthread_t *thread,
                   const pthread_attr_t *attr,
                   void *(*start_routine)(void *),
                   void *arg);

int pthread_setschedparam(pthread_t thread, int policy,
                          const struct sched_param *param);

int pthread_setname_np(pthread_t thread, const char *name);

int pthread_setaffinity_np(pthread_t thread, size_t __cpusetsize,
                           const cpu_set_t* __cpuset);

#endif /* PTHREAD_H */
