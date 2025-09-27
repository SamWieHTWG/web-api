# Example 6: CNC Objects

## Description

For general information about the CNC objects interface, please refer to the `CNC-OS-IFC API Documentation.pdf` located in the `doc` folder.

## Browse Objects

To display the available objects, an Object browse can be performed.

In this demo, CLI menu entries are added to list available CNC objects. The first entry, "**Print/Write to file available CNC objects**," illustrates how to dynamically generate a list of available CNC objects during runtime. If the option "**Enable/disable writing CSV file (on/off)**" is selected, the list of available objects is exported to a CSV file.

## Object Access

In addition to the corresponding task, objects are defined via a group offset and an index offset. These values can be identified through an [Online-Browse](#browse-objects). With few exceptions, group offset and index offset are immutable and can be used for unique identification of the objects.

```c
typedef struct _cnc_object_id
{
  E_CNC_TASK_ID             iThread;
  uint32_t                  iGroup;
  uint32_t                  iOffset;
} CNC_OBJECT_ID;
```

The following code example shows a typical object access. The object is uniquely identified via the task, group offset, and index offset. When reading, the size of the passed memory is specified. This must not be less than the size returned by the Object browse. During the call to `cnc_read_value`, the object value is transferred to the passed memory.

```c
CNC_OBJECT_ID  cnc_object = {CNC_TASK_IPO, 0x20300, 0x00001};
double cycle_time;
int ret_value = cnc_read_value( cnc_object, &cycle_time, sizeof(cycle_time) );
```

Reading objects is done analogously.

You can find several examples demonstrating how to read and write CNC objects in the *demo/src/cnc_demo_samples.c* file.

## Features

- Utilizes internal simulation axes for drives.
- Scheduling is managed externally through direct invocation of the CNC tasks.

## Run and Debug

For instructions on how to run and debug, please see the [Run and Debug section](../README.md#run-and-debug) in the main README file.
