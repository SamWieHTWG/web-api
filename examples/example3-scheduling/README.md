# Example 3: Scheduling

## Description

| Task     | API Function Name | Description                                                       |
| -------- | ----------------- | ----------------------------------------------------------------- |
| TASK_IPO | cnc_task_ipo()    | Interpolator task.                                                |
| TASK_DEC | cnc_task_dec()    | Decoder task (Decoding G-Code programs.)                          |
| TASK_HMI | cnc_task_hmi()    | Communication task for HMI.                                       |
| TASK_SYS | cnc_task_sys()    | Optional. Startup and system task. Needed for ahmi communication. |
| TASK_TCP | cnc_task_tcp()    | Optional. Needed for TCP/IP communication.                        |

This example demonstrates the different types of scheduling that can trigger this CNC tasks.

### Scheduling Mode

In this example, the scheduling mode can be selected by setting the flag `f_internal_scheduling` in the code.

#### Internal Scheduling

Internal scheduling is configured via the _rtconf.lis_ file. For configuration, refer to the relevant
documentation [Configuration of Real-Time Parameters](https://www.isg-stuttgart.de/fileadmin/kernel/kernel-html/en-GB/index.html#911266827).

For the operation of the internal scheduler, either a timer can be configured, or it can be triggered via an external trigger signal (semaphore).
The semaphore can be created using the API function cnc_create_semaphore() and triggered using the function cnc_give_semaphore().

Additionally, it is possible to insert custom [User Functions](https://www.isg-stuttgart.de/fileadmin/kernel/kernel-html/en-GB/index.html#108365195) before or after task calls.

In the example, the internal scheduler was configured using the configuration file _rtconf.lis_. For each CNC task, the corresponding thread is set with priority and CPU instance. The triggering of the CNC in this example is done by setting the semaphore with the function `cnc_give_semaphore()`.

##### Linux Settings

In principle, any Linux distribution can be used to run this sdk example with internal scheduling. Some restrictions apply to WSL, which are discussed below.

The scheduling in the ISG Kernel is designed for real-time applications, so in Linux, the necessary real-time priorities must be set accordingly so that even a normal user can execute the demo. Without this setting, the sdk example can only be executed correctly with superuser rights.
To do this, the following line must be added (with superuser rights) to the file `/etc/security/limits.conf`.

```bash
user    -    rtprio    99
```

"user" must be replaced by the corresponding username.

After adding the entry to 'limits.conf', a system restart is necessary.

###### Limitations with WSL

The sdk example can also be executed under WSL, but some restrictions apply:

- WSL is executed on the first core of the CPU, meaning that distributing the ISG Kernel threads across multiple processor cores is not possible.
- Setting the real-time priorities for the user in use is possible, but has no effect without switching to a user with root permissions. The sdk examples must therefore always be started under WSL with superuser rights.

#### External Scheduling

The _rtconf.lis_ configuration file is not required for external scheduling.

External scheduling involves exporting kernel task functions so they can be managed by an external scheduler, with the internal scheduler being deactivated. Before starting up, the cycle time must be specified from outside. The function `cnc_set_cycle_time()` is used for this purpose, and the parameter to be passed is specified in microseconds (μs).

When this type of scheduling is active, the individual CNC tasks are called directly from the example program in the loop.

#### Mixed Mode

It is also possible to use a combination of internal and external scheduling. For this, the tasks assigned to the internal scheduler are configured accordingly in _rtconf.lis_. Otherwise, proceed as described above.

It should be noted that the parameter [P-RTCF-00001](<https://www.isg-stuttgart.de/fileadmin/kernel/kernel-html/en-GB/index.html#108349835>) in _rtconf.lis_ contains the following value:

`interrupt_source 3 # external Semaphore`

Also, the cycle time for scheduling should be entered in  _rtconf.lis_. Setting this via the API
function `cnc_set_cycle_time()` is omitted at this point.

### User functions

To add user functionality before/after a CNC task call, user functions can be added.

As the tasks are called directly in external scheduling, the user functions must be called explicitly from outside.

With internal scheduling, the functions are called as a callback. To accomplish this, they must be configured in *rt_conf.lis*.

In this example, a user function call was configured before and after the IPO task for internal scheduling. For the other tasks, it is possible to configure user functions in the same way. The configured user function must still be configured before the CNC is started up:

```c
/* register user function */
cnc_register_function(cnc_user_event_ipo_pre,  (char*)"cnc_user_event_ipo_pre");
cnc_register_function(cnc_user_event_ipo_post, (char*)"cnc_user_event_ipo_post");
```

## Features

- Utilizes internal simulation axes for drives.

## Run and Debug

For instructions on how to run and debug, please see the [Run and Debug section](../README.md#run-and-debug) in the main README file.
