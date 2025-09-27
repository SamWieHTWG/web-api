# CNC Library Examples

## List of Examples

- [Hello CNC](./example1-hello-cnc/README.md): Minimal example for starting the CNC
- [Full Demo](./example2-full-demo/README.md): Extended programming example where various components for CNC integration are already implemented
- [Scheduling](./example3-scheduling/README.md): Shows different scheduling modes of the CNC
- [Configuration](./example4-configuration/README.md): Various examples for configuring the CNC
- [Diagnosis](./example5-diagnosis/README.md): Diagnostic functionalites of the CNC
- [CNC Objects](./example6-cnc-objects/README.md): General information about the CNC objects interface
- [Drive Interface](./example7-drive-interface/README.md): How drives can be connected to the CNC

## Run and Debug

Ensure the following are installed on your system:

- A C/C++ compiler (e.g., GCC, MSVC, Clang)
- CMake (version 3.10 or later)

### Build and Run

The examples included in this SDK use CMake for building. Each example is located in its own directory and includes a `CMakeLists.txt` file with the build instructions.

To build and run an example:

1. Open a terminal and navigate to the desired example directory.
2. Create a build directory and run CMake:

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

After building, navigate to the dist directory and start the executable.

### Debug

#### Visual Studio Code

To effectively debug the SDK examples, we recommend using Visual Studio Code.

##### Prerequisites

- Visual Studio Code

Visual Studio Code extensions:

- C/C++ for Visual Studio Code (Microsoft)
- C/C++ Extension Pack (Microsoft)
- CMake Tools (Microsoft)

##### Debug with VSCode

Steps to Debug an Example:

- Open Visual Studio Code.
- Navigate to "File -> Open Folder" and open the directory of the example you want to debug.
- Go to "View -> Command Palette...", type and select "CMake: Select a Kit" and choose the compiler you wish to use.
- Go to "View -> Command Palette..." type and select "CMake: Configure" and press Enter.

This process sets up a CMake configuration in Visual Studio Code.

To Debug the Example:

- Open the Command Palette again ("View -> Command Palette...").
- Type and select "CMake: Debug" to start debugging the selected configuration.

#### Visual Studio

It is also possible to debug the CMake project using Visual Studio. A detailed overview of how to open and debug the CMake project with Visual Studio can be found [online](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-170).
