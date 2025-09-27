# Example 7: Drive Interface

## Description

This example describes the integration of the drive interface.

To connect the drive, it must be configured in the configuration files. Examples can be found in the *cfg* directory.

Information on how to configure the drives can be found in the [online documentation](https://www.isg-stuttgart.de/fileadmin/kernel/kernel-html/en-GB/index.html#117903499).

To connect the drive interface, the designated memory must be registered using the methods ``cnc_register_drive_command_interface`` and ``cnc_register_drive_feedback_interface``. The size of the memory must match the configured drive interface.

To illustrate how this drive interface can be used, a drive simulation for CANopen over EtherCAT and Sercos over EtherCAT has been added to this example.

## Features

- Scheduling is managed externally through direct invocation of the CNC tasks.

## Run and Debug

For instructions on how to run and debug, please see the [Run and Debug section](../README.md#run-and-debug) in the main README file.
