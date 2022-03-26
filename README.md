# About Project Axolotl

Next-generation communication ecosystem powered by decentralized nodes.

Our Hungarian Discord server: https://discord.gg/Ft65hBT59u

## Installation

As this project is in early development phase, this repository is rapidly changing. That said, the project might or might not build, and even if it builds, we can't guarantee you'll have fun using it in this stage. However if you insist on trying it we strongly recommend building it from source.

### Build requirements

- CMake 3.10 or newer
- Tested with Clang 13.0.0 on macOS Monterey 12.0.1

All the dependencies are included in the `include` directory.

### Building the app

Open up a terminal and run the following commands in the root directory of the repository.

```
cmake -B build -DCMAKE_CXX_COMPILER:FILEPATH=(c++ compiler path)
cmake --build build --config Release --target all -j4
```

### Running the app

By default the newly built binary is named `axolotl` and can be found in the `build` directory. Run it without any parameters and read through the guide it prints.