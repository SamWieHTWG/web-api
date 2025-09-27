# Example 2: Full Demo

## Description

In this extended demo, various components for CNC integration are already implemented.

By passing specific arguments, users can switch between different modes such as CanOpen/Sercos drive interface or internal/external scheduling. A self-test can also be initiated via the command line interface (CLI).

The demo demonstrates how to register memory and callback functionalities within the CNC environment. This includes:

- Registering logging functionality
- Registering user-defined functions for task execution
- Registering callback methods for tool data management
- Creating and registering memory for the High-Level Interface
- Creating and registering memory for external variables
- Creating and registering memory for the drive interface

A comprehensive menu enables users to execute various example functionalities, such as starting NC programs or displaying positional data.

### Demo Functions

#### Sample Functions

The following functions are implemented as example functions in [cnc_demo_samples.c](../src/cnc_demo_samples.c) and can mostly be triggered in the demo via the CLI menu.

| Function Name                        | Description                                                                   |
| :----------------------------------- | :---------------------------------------------------------------------------- |
| `cnc_demo_enable_disable_drives()`   | Setting or removing the drive enable.                                         |
| `cnc_demo_start_nc_program()`        | Starting the specified NC program.                                            |
| `cnc_demo_start_manual_data_input()` | Executing the specified manual data input block.                              |
| `cnc_demo_stop_resume_nc_program()`  | Stopping or resuming the active NC program.                                   |
| `cnc_demo_reset()`                   | Resetting the ISG Kernel. Active programs are terminated.                     |
| `cnc_demo_streaming_program()`       | Function for streaming the specified file. Must be called cyclically.         |
| `cnc_demo_start_streaming_program()` | Starting the Streaming Mode.                                                  |
| `cnc_demo_end_streaming_program()`   | Ending the Streaming Mode.                                                    |
| `cnc_demo_get_axis_name()`           | Returns the identifiers of the configured axes in an array.                   |
| `cnc_demo_get_active_positions()`    | Returns the current setpoints in an array.                                    |
| `cnc_demo_get_current_positions()`   | Returns the current actual values in an array.                                |
| `cnc_demo_error_check()`             | Checks if an error has occurred.                                              |

#### External Tool Management

| Function Name                 | Description                                                    |
| :---------------------------- | :------------------------------------------------------------- |
| `cnc_demo_tool_change_info()` | Callback function for information about a tool change.         |
| `cnc_demo_tool_read_data()`   | Callback function for reading tool data.                       |
| `cnc_demo_tool_write_data()`  | Callback function for writing tool data.                       |

The callback functions for using the external tool management are registered via the corresponding register functions (see API-documentation).

### Error Output

Warnings and error messages are output in brief form on the console. Detailed messages are written to a log file. This is located in the directory

## Run and Debug

For instructions on how to run and debug, please see the [Run and Debug section](../README.md#run-and-debug) in the main README file.
