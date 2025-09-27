# Example 4: Configuration

## Description

A detailed description of how the CNC can be configured can be found in the [Online Documentation](https://www.isg-stuttgart.de/fileadmin/kernel/kernel-html/en-GB/index.html#414622731).

The SDK includes meta-information for each configuration parameter and provides standard templates for setup. These files are located in the *components* directory of the SDK.

The *cfg* directory contains besides the standard startup configuration *startup.lis* multiple configuration files for the [Online Reload](#online-reload) demo function.

### XML or ASCII Configuration

The CNC is configured using so-called configuration lists. These can be in either ASCII or XML format.

To interpret configuration lists in XML format, the [libxml2](https://gitlab.gnome.org/GNOME/libxml2) library must be available in the system. The library is not part of the CNC release. The path to the library can be specified via the API function `cnc_set_xml_parser_path()`. During startup, a console print displays whether the libxml2 library could be found.

### Online Reload

It is possible to reload certain CNC parameters during runtime:

| Scope    | Topic   | Description                                                | Online Change |
| -------- | ------- | ---------------------------------------------------------- | ------------- |
| Platform | Startup | Root-description of CNC                                    | No            |
| Platform | Sched   | Scheduling definitions                                     | No            |
| Platform | Manual  | Definition for manual mode                                 | No            |
| Platform | VolComp | Configuration of relevant axis (measure values changeable) | No (mostly)   |
| Channel  | Channel | General channel parameters (e.g. M-functions)              | Yes           |
| Channel  | Clamp   | Offsets for workpiece                                      | Yes           |
| Channel  | ExtVar  | Configuration of external variables                        | No            |
| Channel  | Tool    | Tool description for internal tool management              | Yes           |
| Axis     | Axis    | Axis definitions (gear resolution fix)                     | Yes (mostly)  |
| Axis     | Comp    | All compensation offsets (dimension fix)                   | Yes (mostly)  |

The file *cnc_demo_samples.c* in the *demo/src* directory contains the function `cnc_demo_update_configuration()`. This shows how the re-loading of configuration lists can be triggered via CNC objects. In the example, this function can be activated via the CLI menu.

## Features

- Utilizes internal simulation axes for drives.
- Scheduling is managed externally through direct invocation of the CNC tasks.

## Run and Debug

For instructions on how to run and debug, please see the [Run and Debug section](../README.md#run-and-debug) in the main README file.
