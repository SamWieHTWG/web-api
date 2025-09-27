/*
Copyright (C)
ISG Industrielle Steuerungstechnik GmbH
https://www.isg-stuttgart.de
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>

#include "cnc_demo.h"
#include "cnc_demo_main.h"
#include "cnc_demo_drive_simulation.h"
#include "cnc_demo_samples.h"
#include "cnc_demo_user_func.h"
#include "cnc_demo_self_test.h"
#include "keyboard_helper.h"

/*
+----------------------------------------------------------------------------+
| cnc_demo_main.c                                       | Copyright    ISG   |
|                                                       |                    |
|                                                       | Gropiusplatz 10    |
|                                                       | 70563 Stuttgart    |
+-------------------------------------------------------+--------------------+
| CNC SDK Demo main application.                                             |
+----------------------------------------------------------------------------+
*/

/*
+----------------------------------------------------------------------------+
| Function prototypes                                                        |
+----------------------------------------------------------------------------+
*/
bool              check_cli_args            ( int argc, char* argv[] );
int               cnc_drive_simulation_ipo  (unsigned int context_info);

/*
+----------------------------------------------------------------------------+
| Addresses of memories of HLI and drive interface structures                |
+----------------------------------------------------------------------------+
*/
#define USE_PLC_INTERFACE 0

#if !USE_PLC_INTERFACE
void *p_hli_platform;

void *p_hli_channel[CNC_OS_IFC_NO_CHANNEL_MAX];
void *p_hli_axis[CNC_OS_IFC_NO_AXES_MAX];
#endif

void *p_ve_platform;
void *p_ve_channel[CNC_OS_IFC_NO_CHANNEL_MAX];

void *p_drive_command[CNC_OS_IFC_NO_AXES_MAX];
void *p_drive_feedback[CNC_OS_IFC_NO_AXES_MAX];

/*
+----------------------------------------------------------------------------+
| External and global defined variables                                      |
+----------------------------------------------------------------------------+
*/

char startupfile[PATH_FILENAME_LENGTH]       = "\0";
char config_list_path[]                    = "./listen/";
char diagnosis_path[]                      = "./diagnose/";

char startup_file_name[]                   = "hochlauf.lis";

char startup_error_log_file[PATH_FILENAME_LENGTH];

char ncprogramfile[PATH_FILENAME_LENGTH]   = "nc_test.nc";
char streamingfile[PATH_FILENAME_LENGTH]   = "./prg/nc_stream_test.nc";
char manualdatainput[PATH_FILENAME_LENGTH] = "X100 Y200 Z300 F10";

bool f_cnc_shutdown                      = false;
bool f_logging                           = true;
bool f_error_messages                    = true;

bool f_drives_on                         = false;
bool f_stop_resume                       = false;
bool f_streaming_active                  = false;
bool f_write_csv                         = false;

bool f_cli_self_test                     = false;
bool f_cli_verbose                       = false;

bool f_cli_startup                       = false;
bool f_cli_external_threading            = false;

bool f_cli_use_sercos_simulation         = false;

static long long  tick_counter = 0;


/*
+----------------------------------------------------------------------------+
| Information of configuration from platform provider                        |
+----------------------------------------------------------------------------+
*/
static uint16_t     NumberOfChannels      = CNC_OS_IFC_NO_CHANNEL_MAX;
static uint16_t     NumberOfAxes          = CNC_OS_IFC_NO_AXES_MAX;

static int          SizeExternalVariables = (1000 * 24); // 1000 variables (24 bytes of working data are necessary per variable)

       SDK_HSEM     hCncScheduleThreadsTrigger;          // trigger schedule defined by RtConf.li:  see cycle_time  2000    # Zykluszeit in mikro s:

/*
+----------------------------------------------------------------------------+
| Tool data of external tool data management                                 |
+----------------------------------------------------------------------------+
*/
extern      CNC_TOOL_DESC       ext_tool_data[];
extern      CNC_TOOL_DATA_IN    ext_tool_life_data[];

/*
+----------------------------------------------------------------------------+
| CLI commands                                                               |
+----------------------------------------------------------------------------+
*/
CLI_DESCRIPTION cli_description[] =
{
  { "-v",           "Verbose mode",                    NULL,              "",      &f_cli_verbose,                true },
  { "-e",           "External threading",              NULL,              "",      &f_cli_external_threading,     true },
  { "-f",           "Start up file <file>",            &startupfile,      "%s",    &f_cli_startup,                true },
  { "-s",           "Use Sercos simulation",           NULL,              "",      &f_cli_use_sercos_simulation,  true },
  { "-test",        "Self test of CncSDKDemo",         NULL,              "",      &f_cli_self_test,              true }
};

/**
 * Initialization of the CNC.
*/
void cnc_demo_initialize(void)
{
  char                       log_string[200] = "";
  unsigned int               i_channel;
  unsigned int               i_drive;
  unsigned int               i_tool;

  int                        ret_value     = ERR_CNC_NOERROR;
  time_t                     starting_time = 0;

  /*
  +--------------------------------------------------------------------------+
  | Pre initialization before start-up of CNC, e.g. read license info        |
  +--------------------------------------------------------------------------+
  */
  cnc_register_log_message_function(cnc_log_message);
  cnc_register_log_message_json_function(cnc_log_message_json);

  /*
  +----------------------------------------------------------------------------+
  | Set access method to CNC objects. As long as the object access is not      |
  | synchronous with the CNC task call, this must take place via TCP.          |
  +----------------------------------------------------------------------------+
  */
  cnc_set_tcp_object_access(true);

  /*
  +----------------------------------------------------------------------------+
  | Get current date/time                                                      |
  +----------------------------------------------------------------------------+
  */
  time(&starting_time);

  /*
  +--------------------------------------------------------------------------+
  | Check version, maximum number of channel and axes...                     |
  +--------------------------------------------------------------------------+
  */
  cnc_log_message(CNC_LOG_INFO, 0, "--------------------------------------------------");
  sprintf(log_string, " CNC version %s", cnc_get_version());
  cnc_log_message(CNC_LOG_INFO, 0, log_string);
  sprintf(log_string, " Maximum    number of channels %u, axes %u", cnc_get_max_channels(), cnc_get_max_axes());
  cnc_log_message(CNC_LOG_INFO, 0, log_string);
  sprintf(log_string, " Configured number of channels %u, axes %u", NumberOfChannels, NumberOfAxes);
  cnc_log_message(CNC_LOG_INFO, 0, log_string);
  sprintf(log_string, " CncSDKDemo started at %s", ctime(&starting_time));
  cnc_log_message(CNC_LOG_INFO, 0, log_string);
  cnc_log_message(CNC_LOG_INFO, 0, "--------------------------------------------------");

  /*
  +----------------------------------------------------------------------------+
  | Check if tool data structure of ISG and customer are identical             |
  +----------------------------------------------------------------------------+
  */
  if (sizeof(CNC_TOOL_DESC) != cnc_get_sizeof_cnc_tool_desc())
  {
    cnc_log_message(CNC_LOG_ERROR, 0, " Error: customer CNC_TOOL_DESC structure does not fit to NC-Kernel!");
    sprintf(log_string, "        customer CNC_TOOL_DESC=%zu, kernel CNC_TOOL_DESC=%lld", sizeof(CNC_TOOL_DESC), cnc_get_sizeof_cnc_tool_desc());
    cnc_log_message(CNC_LOG_ERROR, 0, log_string);
    exit(-1);
  }

  /*
  +----------------------------------------------------------------------------+
  | Initialize tool data base                                                  |
  +----------------------------------------------------------------------------+
  */
  for (i_tool = 0; i_tool < NUMBER_EXT_TOOL_DATA; i_tool++)
  {
    memset(&ext_tool_data[i_tool], 0, sizeof(CNC_TOOL_DESC));
    memset(&ext_tool_life_data[i_tool], 0, sizeof(CNC_TOOL_DATA_IN));

    ext_tool_data[i_tool].tool_id.basic   = i_tool;
    ext_tool_data[i_tool].tool_id.variant = i_tool;
    ext_tool_data[i_tool].gueltig         = 1;

    ext_tool_life_data[i_tool].tool_id.basic   = i_tool;
    ext_tool_life_data[i_tool].tool_id.variant = i_tool;
  }

  /*
  +==========================================================================+
  | Start-up state                                                           |
  +==========================================================================+
  | Allocate all necessary resources for CNC.                                |
  | Call start-up of CNC.                                                    |
  +==========================================================================+
  */

  cnc_register_function(cnc_user_event_ipo, (char*)"cnc_user_event_ipo");
  cnc_register_function(cnc_user_event_dec, (char*)"cnc_user_event_dec");
  cnc_register_function(cnc_user_event_hmi, (char*)"cnc_user_event_hmi");
  cnc_register_function(cnc_user_event_sys, (char*)"cnc_user_event_sys");

  cnc_register_function(cnc_drive_simulation_ipo, (char*)"cnc_drive_simulation_ipo");

  cnc_register_tool_change_info(cnc_demo_tool_change_info);
  cnc_register_read_tool_data(cnc_demo_tool_read_data);
  cnc_register_write_tool_data(cnc_demo_tool_write_data);

  /*
  +--------------------------------------------------------------------------+
  | Create and register semaphore for scheduling.                            |
  +--------------------------------------------------------------------------+
  */
  if (f_cli_external_threading == false)
  {
    if (0 != cnc_create_semaphore(&hCncScheduleThreadsTrigger, (char*)"CncSyncMutex"))
    {
      printf("\n Scheduling semaphore 'CncSyncMutex' could not be created!");
      exit(-2);
    }

    if (0 != cnc_register_sync_trigger(hCncScheduleThreadsTrigger))
    {
      printf("\n Scheduling semaphore 'CncSyncMutex' could not be registered!");
      exit(-3);
    }
  }
  else
  {
    cnc_set_cycle_time(1000);
  }

  /*
  +--------------------------------------------------------------------------+
  | Allocate memory and register addresses for HLI structures                |
  +--------------------------------------------------------------------------+
  */
#if !USE_PLC_INTERFACE
  p_hli_platform = (void*)malloc(cnc_get_sizeof_hli_platform());
  if (ERR_CNC_NOERROR != cnc_register_hli_platform(p_hli_platform))
    exit(-5);

  for (i_channel = 0; i_channel < NumberOfChannels; i_channel++)
  {
    p_hli_channel[i_channel] = (void*)malloc(cnc_get_sizeof_hli_channel());
    if (ERR_CNC_NOERROR != cnc_register_hli_channel(i_channel, p_hli_channel[i_channel]))
      exit(-7);
  }

  for (i_drive = 0; i_drive < NumberOfAxes; i_drive++)
  {
    p_hli_axis[i_drive] = (void*)malloc(cnc_get_sizeof_hli_axis());
    if (ERR_CNC_NOERROR != cnc_register_hli_axis(i_drive, p_hli_axis[i_drive]))
      exit(-9);
  }
#endif

  /*
  +--------------------------------------------------------------------------+
  | Allocate memory and register addresses of V.E. in HLI in global struct   |
  +--------------------------------------------------------------------------+
  */
  p_ve_platform  = (void*)malloc(SizeExternalVariables);
  if (ERR_CNC_NOERROR != cnc_register_external_variables_platform(p_ve_platform, SizeExternalVariables))
    exit(-6);

  for (i_channel = 0; i_channel < NumberOfChannels; i_channel++)
  {
    p_ve_channel[i_channel] = (void*)malloc(SizeExternalVariables);
    if (ERR_CNC_NOERROR != cnc_register_external_variables_channel(i_channel, p_ve_channel[i_channel], SizeExternalVariables))
      exit(-8);
  }

  /*
  +----------------------------------------------------------------------------+
  | Create PLC shared memory interfaces                                        |
  +----------------------------------------------------------------------------+
  */
#if USE_PLC_INTERFACE
  cnc_create_plc_interfaces(NumberOfChannels, NumberOfAxes);
#endif

  /*
  +--------------------------------------------------------------------------+
  | Allocate and register memory for drive interface                         |
  +--------------------------------------------------------------------------+
  */
  short size_feedback_telegram = f_cli_use_sercos_simulation ? sizeof(CNC_DRIVE_STATUS_SOE) : sizeof(CNC_DRIVE_STATUS_COE);
  short size_command_telegram  = f_cli_use_sercos_simulation ? sizeof(CNC_DRIVE_COMMAND_SOE) : sizeof(CNC_DRIVE_COMMAND_COE);

  for (i_drive = 0; i_drive < NumberOfAxes; i_drive++)
  {
    p_drive_command[i_drive] = malloc(size_command_telegram);
    cnc_register_drive_command_interface(i_drive, p_drive_command[i_drive], size_command_telegram);
    memset(p_drive_command[i_drive], 0, size_command_telegram);

    p_drive_feedback[i_drive] = malloc(size_feedback_telegram);
    cnc_register_drive_feedback_interface(i_drive, p_drive_feedback[i_drive], size_feedback_telegram);
    memset(p_drive_feedback[i_drive], 0, size_feedback_telegram);
  }

  if (f_cli_use_sercos_simulation)
  {
    /* register callbacks for sercos service channel */
    cnc_register_serc_write_req_function(cnc_demo_sercos_asynch_callback_write_req);
    cnc_register_serc_read_req_function(cnc_demo_sercos_asynch_callback_read_req);
  }
  else
  {
    /* register callbacks for canopen SDO access */
    cnc_register_canopen_write_req_function(cnc_demo_canopen_asynch_callback_write_req);
    cnc_register_canopen_read_req_function(cnc_demo_canopen_asynch_callback_read_req);
  }

} // cnc_demo_initialize()

void cnc_demo_shutdown()
{
  /*
  +==========================================================================+
  | Shutdown state                                                           |
  +==========================================================================+
  | CNC is stopped, and all resources are released.                          |
  +==========================================================================+
  */
  cnc_shutdown();

  if (f_cli_external_threading == false)
    cnc_delete_semaphore(hCncScheduleThreadsTrigger);

  #if USE_PLC_INTERFACE
    cnc_delete_plc_interfaces(NumberOfChannels, NumberOfAxes);
  #else
    if (p_hli_platform) free(p_hli_platform);

    for (int i_channel = 0; i_channel < NumberOfChannels; i_channel++)
    {
      if (p_hli_channel[i_channel]) free(p_hli_channel[i_channel]);
    }

    for (int i_drive = 0; i_drive < NumberOfAxes; i_drive++)
    {
      if (p_hli_axis[i_drive]) free(p_hli_axis[i_drive]);
    }
  #endif

    if (p_ve_platform)  free(p_ve_platform);

    for (int i_channel = 0; i_channel < NumberOfChannels; i_channel++)
    {
      if (p_ve_channel[i_channel])  free(p_ve_channel[i_channel]);
    }

    for (int i_drive = 0; i_drive < NumberOfAxes; i_drive++)
    {
      if (p_drive_command[i_drive])  free(p_drive_command[i_drive]);
      if (p_drive_feedback[i_drive]) free(p_drive_feedback[i_drive]);
    }
}

/*
+----------------------------------------------------------------------------+
| M A I N                                                                    |
+----------------------------------------------------------------------------+
*/
int main ( int argc, char* argv[] )
{
  char                       log_string[80] = "";
  char                       command        = '?';

  int                        ret_value         = ERR_CNC_NOERROR;

  char                       input[PATH_FILENAME_LENGTH];
  char                       file_name[PATH_FILENAME_LENGTH];
  char                       data_input[PATH_FILENAME_LENGTH];

  bool                       old_logging_state = false;
  bool                       message_logged    = false;

  unsigned char              f_verbose         = false;

  /* Streaming mode */
  FILE                      *p_nc_file         = NULL;
  char                       buf[MAX_BUF_SIZE] = {'\0'};

  cnc_log_message(CNC_LOG_INFO, 0, "Starting ISG kernel Demo...");

  /*
  +----------------------------------------------------------------------------+
  | Check for command line options                                             |
  +----------------------------------------------------------------------------+
  */
  check_cli_args(argc, argv);

  if (f_cli_verbose == true)
  {
    f_verbose = true;
  }

  /*
  +----------------------------------------------------------------------------+
  | Initialize sdk demo                                                        |
  +----------------------------------------------------------------------------+
  */
  cnc_demo_initialize();

  /*
  +--------------------------------------------------------------------------+
  | CNC start-up                                                             |
  +--------------------------------------------------------------------------+
  */
  /* Activate logging of startup errors. */
  cnc_set_startup_error_log(true);
  strcpy(startup_error_log_file, diagnosis_path);
  strcat(startup_error_log_file, "ErrorsWhileStartUp.log");
  cnc_startup_error_log_file(startup_error_log_file);

  if( f_cli_startup == false )
  {
    /* create startup path */
    strcpy(startupfile, config_list_path);

    /* depending on mode - choose startup path*/
    if ( f_cli_external_threading == true )
    {
      strcat(startupfile, "ExtSchedule/");
    }
    else if (f_cli_use_sercos_simulation)
    {
      strcat(startupfile, "SoE/");
    }

    /* add startup file name to path*/
    strcat(startupfile, startup_file_name);
  }

  sprintf( log_string, "Starting kernel with startupfile '%s'", startupfile );
  cnc_log_message(CNC_LOG_INFO, 0, log_string);
  ret_value = cnc_startup( startupfile);

  if( ret_value != ERR_CNC_NOERROR )
  {
    printf( "\nERROR %d during startup of CNC\n", ret_value );
    exit(-12);
  }

  /*
  +--------------------------------------------------------------------------+
  | Create and start CNC trigger thread with high priority                   |
  +--------------------------------------------------------------------------+
  */
  cnc_create_kernel_trigger_thread();

  if ( f_cli_self_test == true )
  {
    TEST_PROCEDURE test_run[] = {
      {TS_RESET_RUNTIME_STATISTICS, TS_RESET_RUNTIME_STATISTICS,     10, ""},
      {TS_DRIVES_ON,                TS_DRIVES_ON,                    10, ""},
      /* Test program */
      {TS_WAIT_CYCLES,              TS_START_NC_PROGRAM,             10, "nc_test.nc"},
      {TS_SHOW_RUNTIME_STATISTICS,  TS_SHOW_RUNTIME_STATISTICS,     100, ""},
      {TS_WAIT_CYCLES,              TS_SHOW_POSITIONS,             5000, ""},
      {TS_WAIT_CYCLES,              TS_SHOW_POSITIONS,             5000, ""},
      {TS_WAIT_CYCLES,              TS_SHOW_POSITIONS,             5000, ""},
      {TS_WAIT_CYCLES,              TS_SHOW_POSITIONS,             5000, ""},
      {TS_WAIT_CYCLES,              TS_RESET,                        10, ""},
      {TS_SHOW_RUNTIME_STATISTICS,  TS_SHOW_RUNTIME_STATISTICS,       0, ""},
      /* End of test sequence */
      {TS_TEST_READY,               TS_NONE,                          0, ""}
    };

    TEST_CONFIGURATION test_configuration = {
      NumberOfChannels,
      NumberOfAxes,
      f_cli_use_sercos_simulation
    };

    cnc_demo_self_test(&test_run[0], test_configuration, p_drive_command, p_drive_feedback);

    f_cnc_shutdown = true;
  }

  /*
  +==========================================================================+
  | Running state                                                            |
  +==========================================================================+
  | CNC is operating in cyclic mode.                                         |
  +==========================================================================+
  */
  command = '?'; /* print help menu at first start */

  while (f_cnc_shutdown == false)
  {

    switch ( command )
    {
      /* ---------------------------------------------------- */
      /* Print menu                                           */
      case 'm'  :
      case 'M'  :
      case '?'  :
      case '\r' :
      {
        printf("\n\n CncSDKDemo");
        printf("\n ==========");
        printf("\n\n Menu:");
        printf("\n a, A    : Runtime statistics.");
        printf("\n b, B    : Reset runtime statistics.");
        printf("\n c, C    : Enable/disable writing csv file (on/off).");
        printf("\n d, D    : Enable/disable drives (on/off).");
        printf("\n e, E    : Enable/disable error messages (on/off)");
        printf("\n h, H    : Stop / resume nc program.");
        printf("\n k, K    : Simulate error in drive interface.");
        printf("\n l, L    : Enable/disable logging (on/off).");
        printf("\n p, P    : Show axis positions");
        printf("\n m, M, ? : this help menu.");
        printf("\n o, O    : Print/Write to file available cnc objects.");
        printf("\n q, Q    : Shutdown CncSDKDemo.");
        printf("\n r, R    : Reset ISG kernel.");
        printf("\n s, S    : Start program.");
        printf("\n t, T    : Manual data input.");
        printf("\n u, U    : Update configuration.");
        printf("\n v, V    : Kernel verbose mode on/off.");
        printf("\n x, X    : Activate streaming mode.");
        printf("\n z, Z    : Restart CNC (shutdown/startup).");
        printf("\n\n");
        break;
      }
      /* ---------------------------------------------------- */
      /* Print runtime statistics                             */
      case 'a' :
      case 'A' :
      {
        cnc_demo_print_runtime_statistic();
        break;
      }
      /* ---------------------------------------------------- */
      /* Reset runtime statistics                             */
      case 'b' :
      case 'B' :
      {
        cnc_demo_reset_runtime_statistic();
        break;
      }
      /* ---------------------------------------------------- */
      /* Enable/disable writing csv file (on/off)             */
      case 'c' :
      case 'C' :
      {
        if ( f_write_csv == false )
        {
          f_write_csv = true;
          printf("CNC-MSG: Write csv file on\n");
        }
        else
        {
          f_write_csv = false;
          printf("CNC-MSG: Write csv file off\n");
        }

        break;
      }
      /* ---------------------------------------------------- */
      /* Enable/disable drives (on/off)                       */
      case 'd' :
      case 'D' :
      {
        if ( f_drives_on == false )
        {
          f_drives_on = true;
          printf("CNC-MSG: Drives switched on.\n");
        }
        else
        {
          f_drives_on = false;
          printf("CNC-MSG: Drives switched off.\n");
        }

        cnc_demo_enable_disable_drives( f_drives_on );

        break;
      }
      /* ---------------------------------------------------- */
      /* Enable/disable error messages (on/off)               */
      case 'e' :
      case 'E' :
      {
        if ( f_error_messages == false )
        {
          f_error_messages = true;
          printf("CNC-MSG: Error messages switched on.\n");
        }
        else
        {
          f_error_messages = false;
          printf("CNC-MSG: Error messages switched off.\n");
        }
        break;
      }
      /* ---------------------------------------------------- */
      /* Stop / resume nc program                             */
      case 'h' :
      case 'H' :
      {
        if ( f_stop_resume == false )
        {
          f_stop_resume = true;
          printf("CNC-MSG: Stopping nc program.\n");
        }
        else
        {
          f_stop_resume = false;
          printf("CNC-MSG: Resuming nc program.\n");
        }
        cnc_demo_stop_resume_nc_program( f_stop_resume );
        break;
      }
      /* ---------------------------------------------------- */
      /* Simulate Sercos drive error                          */
      case 'k' :
      case 'K' :
      {
        cnc_demo_set_drive_err_simu(true);
        break;
      }
      /* ---------------------------------------------------- */
      /* Logging on/off                                       */
      case 'l' :
      case 'L' :
      {
        if ( f_logging == false )
        {
          f_logging = true;
          printf("CNC-MSG: Switched logging on\n");
        }
        else
        {
          f_logging = false;
          printf("CNC-MSG: Switched logging off\n");
        }

        break;
      }
      /* ---------------------------------------------------- */
      /* Generate diag_data                                   */
      case 'i':
      case 'I':
      {
        CNC_OBJECT_ID obj_diag_exec   = { CNC_TASK_HMI, 0x20101, 0x2ab };
        CNC_OBJECT_ID obj_diag_file   = { CNC_TASK_HMI, 0x20101, 0x2ac };
        CNC_OBJECT_ID obj_diag_topics = { CNC_TASK_HMI, 0x20101, 0x2a9 };

        char topicstring[256];
        int ret_val = 0;

        printf("CNC_MSG: writing diagnosis data...\n");

        ret_val = cnc_write_STRING(obj_diag_file, (char*)"diag_data_test.txt", 18);

        if (ret_val == 0)
        {
          ret_val = cnc_write_BOOL(obj_diag_exec, true);
        }

        if (ret_val != 0)
        {
          printf("CNC-MSG: Couldn't start diagnosis data upload!\n");
        }

        ret_val = cnc_read_STRING(obj_diag_topics, &topicstring[0], 256);

        break;
      }
      /* ---------------------------------------------------- */
      /* Print/Write to file available cnc objects            */
      case 'o' :
      case 'O' :
      {
        cnc_demo_print_object_ids( f_write_csv );
        break;
      }
      /* ---------------------------------------------------- */
      /* Show axis positions                                  */
      case 'p' :
      case 'P' :
      {
        cnc_demo_show_positions();
        break;
      }
      /* ---------------------------------------------------- */
      /* Shutdown ISG kernel demo                             */
      case 'q' :
      case 'Q' :
      {
        f_cnc_shutdown = true;
        break;
      }
      /* ---------------------------------------------------- */
      /* Kernel reset                                         */
      case 'r' :
      case 'R' :
      {
        printf("CNC-MSG: CNC Reset.\n");
        cnc_demo_reset();
        break;
      }
      /* ---------------------------------------------------- */
      /* Start program                                        */
      case 's' :
      case 'S' :
      {
        printf("CNC-MSG: Enter nc program file name [%s]:\n", ncprogramfile);
        file_name[0] = '\0';
        cnc_demo_read_cli_line(file_name, sizeof(file_name));
        if ( file_name[0] != '\0' && file_name[0] != '\n' )
        {
          memset( &ncprogramfile, 0, sizeof(file_name) );
          strncpy( ncprogramfile, file_name, strlen(file_name) );
        }
        printf("CNC-MSG: Start nc program file: %s\n", ncprogramfile);
        cnc_demo_start_nc_program(ncprogramfile);
        printf("CNC-MSG: Program %s is running.\n", ncprogramfile);
        break;
      }
      /* ---------------------------------------------------- */
      /* Manual data input                                    */
      case 't' :
      case 'T' :
      {
        printf("CNC-MSG: Enter manual data input  [%s]:\n", manualdatainput);
        data_input[0] = '\0';
        cnc_demo_read_cli_line(data_input, sizeof(data_input));
        if ( data_input[0] != '\0' && data_input[0] != '\n' )
        {
          memset( &manualdatainput, 0, sizeof(data_input) );
          strncpy( manualdatainput, data_input, strlen(data_input));
        }
        printf("CNC-MSG: Execute manual data input: %s\n", manualdatainput);
        cnc_demo_start_manual_data_input( manualdatainput );
        break;
      }
      /* ---------------------------------------------------- */
      /* Update of configuration                              */
      case 'u' :
      case 'U' :
      {
          cnc_demo_update_configuration();
          break;
      }
      /* ---------------------------------------------------- */
      /* Kernel verbose mode on/off                           */
      case 'v' :
      case 'V' :
      {
        if ( f_verbose == false )
        {
          f_verbose = true;
        }
        else
        {
          f_verbose = false;
        }

        cnc_set_verbose_mode(f_verbose);

        break;
      }
      /* ---------------------------------------------------- */
      /* Activate streaming mode                              */
      case 'x' :
      case 'X' :
      {
        printf("CNC-MSG: Enter streaming program file name [%s]:\n", streamingfile);
        data_input[0] = '\0';        
        cnc_demo_read_cli_line(file_name, sizeof(file_name));
        if ( file_name[0] != '\0' && file_name[0] != '\n' )
        {
          memset( &streamingfile, 0, sizeof(file_name) );
          strncpy( streamingfile, file_name, strlen(file_name) );
        }
        printf("CNC-MSG: Current streaming program file: %s\n", streamingfile);
        f_streaming_active = cnc_demo_start_streaming_program( &p_nc_file, streamingfile );

        if ( f_streaming_active == true )
        {
          printf("\nCNC-MSG: Streaming mode active.");
        }
        else
        {
          printf("\nCNC-MSG: Activation of streaming mode failed.");
        }
        break;
      }
      /* ---------------------------------------------------- */
      /* Restart CNC kernel                                   */
      case 'z' :
      case 'Z' :
      {
        printf("\nCNC-MSG: CNC shutdown - restart.");

        f_cnc_shutdown = true;
        usleep(10000); /* wait for a few ticks so trigger thread is exit */
        cnc_demo_shutdown();

        usleep(1000000); // [1s]

        f_cnc_shutdown = false;
        cnc_demo_initialize();

        f_error_messages = true;
        f_logging        = true;
        cnc_startup(startupfile);

        cnc_create_kernel_trigger_thread();
      }
    }

    command = 0;

    /*
    +------------------------------------------------------------------------+
    | Cyclic call of streaming mode                                          |
    +------------------------------------------------------------------------+
    */
    if ( f_streaming_active == true )
    {
      if ( false == cnc_demo_streaming_program(p_nc_file, buf) )
      {
        f_streaming_active = cnc_demo_end_streaming_program( &p_nc_file );
      }
    }


    if (f_cnc_shutdown)
      break;

    /*
    +------------------------------------------------------------------------+
    | Check for keyboard input...                                            |
    +------------------------------------------------------------------------+
    */
    cnc_demo_read_cli_line(input, sizeof(input));
    command = input[0];
    if (command == '\0')
    {
      command = '?';
    }

    usleep( 1000 ); // [1ms]

  } /* End of menu loop */


  cnc_demo_shutdown();

  return ret_value;
} /* End main() */

/*
+----------------------------------------------------------------------------+
| Kernel trigger thread function                                             |
+----------------------------------------------------------------------------+
*/
void cnc_kernel_trigger()
{
  while (f_cnc_shutdown == false)
  {

    if (f_cli_external_threading == true)
    {
      /*
      +----------------------------------------------------------------------------+
      | External threading -> tasks are called directly                            |
      +----------------------------------------------------------------------------+
      */
      cnc_task_dec(0);
      cnc_task_dec(0);
      cnc_task_hmi(0);
      cnc_task_ipo(0);
      cnc_drive_simulation_ipo(0);
      cnc_task_hmi(0);

      /* These tasks are only required if the test HMI (ahmi.exe) is used. */
      cnc_task_sys(0);
      cnc_task_tcp(0);
    }
    else
    {
      /*
      +------------------------------------------------------------------------+
      | Give cyclic tick to internal CNC scheduler                             |
      +------------------------------------------------------------------------+
      */
      if (hCncScheduleThreadsTrigger != NULL)
      {
        cnc_give_semaphore(hCncScheduleThreadsTrigger);
      }
    }

    usleep( 1000 ); // [1ms]
  } /* End of kernel loop */
  printf("\n\n");
}

/*
+----------------------------------------------------------------------------+
| Create kernel trigger thread                                               |
+----------------------------------------------------------------------------+
*/
void cnc_create_kernel_trigger_thread(void)
{
  pthread_t           id;
  pthread_attr_t      attr;

  struct sched_param  sched;

  /*
  +----------------------------------------------------------------------------+
  | Create thread                                                              |
  +----------------------------------------------------------------------------+
  */
  pthread_attr_init( &attr );
  pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED );
  pthread_create( &id, &attr,  (void* (*)(void*))cnc_kernel_trigger, NULL );

  sched.sched_priority = 70;
  pthread_setschedparam( id, SCHED_FIFO, &sched );

  pthread_setname_np( id, "CNC_TRIGGER");

  if ( !id )
  {
    f_logging = true;
    cnc_log_message( CNC_LOG_ERROR, 0, "Thread 'CNC_TRIGGER' could not be created!");
    f_logging = false;
  }
  else
  {
    f_logging = true;
    cnc_log_message( CNC_LOG_INFO, 0, "Thread 'CNC_TRIGGER' with created!");
    f_logging = false;
  }

}

/*
+----------------------------------------------------------------------------+
| Sample impelementation of cnc_log_error_json                               |
+----------------------------------------------------------------------------+
*/
int cnc_log_message_json(const char* json_stream)
{
  int ret_val = ERR_CNC_NOERROR;

  /* processing of error stream   */
  /* printf("\n%s", json_stream); */

  return ret_val;
}

/*
+----------------------------------------------------------------------------+
| Sample implementation of cnc_log_message()                                 |
+----------------------------------------------------------------------------+
*/
int cnc_log_message( E_CNC_LOG_MSG_CLASS log_msg_class, unsigned int log_msg_id, const char* log_msg_string)
{
  int ret_val = ERR_CNC_NOERROR;

  if ( f_error_messages == true )
  {
    switch ( log_msg_class )
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
    }
  }

  if ( f_logging == true )
  {
     switch( log_msg_class )
    {
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
  }
  return ret_val;
}

/*
+----------------------------------------------------------------------------+
| Check command line arguments                                               |
+----------------------------------------------------------------------------+
*/
bool check_cli_args(int argc, char* argv[])
{
  int                 i;
  bool                flag_ende;
  CLI_DESCRIPTION    *arguments;

  for (i = 1; i < argc; i++)
  {
    flag_ende = false;
    arguments = cli_description;

    /* look for option in table */
    while ((strcmp(argv[i], arguments->option) != 0) &&
      (flag_ende == false)
      )
    {
      if (strcmp("", arguments->option) == 0)
      {
        flag_ende = true;
      }
      else
      {
        arguments++;
      }
    } /* while(..) */

    /* Option found, set value */
    if (flag_ende == false)
    {
      *arguments->flag = arguments->value;
      /* additional argument */
      if (arguments->parameter != NULL)
      {
        i++;
        if (i < argc)
        {
          if (sscanf(argv[i], arguments->format, arguments->parameter) != 1)
          {
            printf("\nExpected parameter at %s!\n", arguments->option);
          }
        }
        else
        {
          printf("\nExpected parameter at %s!\n", arguments->option);
        }
      }
    }
    else
    {
      return false;
    }
  } /* for(..) */

  return true;
}

int cnc_drive_simulation_ipo(unsigned int context_info)
{
  unsigned int               i_drive;

  /*
  +------------------------------------------------------------------------+
  | Simulation of drive interface                                          |
  +------------------------------------------------------------------------+
  */
  for (i_drive = 0; i_drive < NumberOfAxes; i_drive++)
  {
    if (f_cli_use_sercos_simulation )
    {
      cnc_demo_drive_ifc_simulation_soe(i_drive, (CNC_DRIVE_COMMAND_SOE*) p_drive_command[i_drive],  (CNC_DRIVE_STATUS_SOE*) p_drive_feedback[i_drive]);
    }
    else
    {
      cnc_demo_drive_ifc_simulation_coe(i_drive, (CNC_DRIVE_COMMAND_COE*) p_drive_command[i_drive],  (CNC_DRIVE_STATUS_COE*) p_drive_feedback[i_drive]);
    }
  }

  return 0;
}
