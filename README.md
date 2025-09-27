# CncKernelSDK

## Contents

### CNC Library

The package contains the ISG CNC Kernel as shared library (Linux) resp. DLL (Windows) available for the following platforms:

| OS      | CPU             | Library       |
|---------|-----------------|---------------|
| Linux   | x86_64 / AMD64  | libCncSDK.so  |
| Linux   | aarch64 / ARM64 | libCncSDK.so  |
| Windows | x86_64 / AMD64  | libCncSDK.dll |

The demo libraries are subjected to several limitations:

- Time limitation for 60 minutes, i.e. after this period it is not possible to move any axis.
- The maximum number of axes is limited to 4.
- Only 1 channel can be configured.

The corresponding directories are:

- **lib**: Libraries
- **include**: Corresponding header file
- **doc**: API documentation

### Examples

The *examples* directory contains various minimal examples for CNC integration. This includes a [Hello-CNC](./examples/example1-hello-cnc/README.md) example that shows the CNC startup with minimal code and an [Extended Demo](./examples/example2-full-demo/README.md) in which the various components for CNC integration are already implemented.

A more detailed [documentation](./examples/README.md) can be found in the *examples* directory.

### Components

#### Configuration

The SDK includes meta-information for each configuration parameter and provides standard templates for setup. These files are located in the *components/config* directory of the SDK.

#### Error

Error information is contained in the *components/error* directory. Information on how to integrate this error information can be found in the [diagnosis example](./examples/example5-diagnosis/README.md).

#### Diagnosis

#### Ahmi

The ahmi in the *diagnosis/ahmi* directory offers a very simple console HMI, which can be used for diagnosis. Once the CNC has been started via an example, the ahmi can be launched using the included scripts.

The ahmi is currently only available in German.

### Documentation

For further information and documentation please visit our homepage on
[https://www.isg-stuttgart.de](https://www.isg-stuttgart.de).

## About

ISG Industrielle Steuerungstechnik GmbH
Gropiusplatz 10
70563 Stuttgart
Germany
www.isg-stuttgart.de
