# Example 5: Diagnosis

## Description

This example describes the various diagnostic capabilities of the CNC.

### Error Management

The [error management documentation](https://www.isg-stuttgart.de/fileadmin/kernel/kernel-html/en-GB/index.html#574992139) provides information on the following topics:

* Output location and scope of the error output
* Error message filters, both global and channel- or axis-specific
* Adding additional information to error messages
* Output of user error messages by the user

A general description of the CNC error messages can be found in the [Diagnosis Manual](https://www.isg-stuttgart.de/fileadmin/kernel/kernel-html/en-GB/index.html#249152395).

For the CNC to display the corresponding error texts, an error message file must be specified in the start-up configuration. In this example, the file *err_text_version_eng.txt* is defined in the start-up list *cfg/startup.lis*:

`error_message_texts                                   ./error/err_text_version_eng.txt`

### Diagnostic callbacks

To integrate the CNC diagnostics, it is possible to register diagnostic callbacks in the CNC. These functions are called by the CNC in the event of an error or for diagnostic output. In the registered function, the output can then be specifically adapted to the framework, for example, to pass on the error data to the diagnostics system.

#### Json callback function

The json callback function can be registered via `cnc_register_log_message_json_function`.
When this diagnostic callback is invoked, a JSON stream is passed as an argument, which contains various error information.
A corresponding json schema file can be found in the *components/error* directory.

#### Ascii callback function

The ascii callback function can be registered via `cnc_register_log_message_function`.
The type of the diagnosis (Info, Warning, Error, Exception, Debug) is transferred as an argument to the registered diagnostic function. It also receives the error ID and a string describing the error as an argument.

If further error information is required, this can be queried within the registered function, for example via CNC objects.

### Diagnostic Upload

The [Diagnosis upload function](https://www.isg-stuttgart.de/fileadmin/kernel/kernel-html/en-GB/index.html#944019339) is used to save the current system status of the CNC to a file. It can be executed at any time while the CNC is running. The diagnosis data can then be used for a CNC analysis.

In the example, a diagnosis upload can be triggered via the CLI.

### Startup Diagnosis

During the CNC startup, the diagnostic callbacks are not yet active. To obtain diagnostic information at this stage, the startup log can be enabled via `cnc_set_startup_error_log()`. The path of the log file can be set using `cnc_startup_error_log_file()`.

## Features

* Utilizes internal simulation axes for drives.
* Scheduling is managed externally through direct invocation of the CNC tasks.

## Run and Debug

For instructions on how to run and debug, please see the [Run and Debug section](../README.md#run-and-debug) in the main README file.
