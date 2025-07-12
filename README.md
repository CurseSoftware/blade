# C++ Template Project built using CMake
This is a cross-platform template C++ project using the CMake build system.

## Building
#### Build Requirements
C++ 20 compliant compiler

#### Build commands
There are included `go.sh` and `go.ps` scripts in the project root directory.

Run the command `./go.sh Build-All` to run the build commands on Linux or MacOS.

Run the command `./go.ps Build-All` to run the build commands on Windows.

Alternatively, you can just run the build commands yourself:
```
cmake -B build .
cmake --build build
```
These commands place all build utilities CMake generates in the `build` directory which is preferable to cluttering the root dir.

## Using compile_commands.json
If you are using the clangd language server, CMake can generate a `compile_commands.json` file that works with clangd in order to allow the LSP to work.

Because we are placing CMake build outputs in our `build` directory, this places the json file there as well. 

On Linux you can run `ln -s build/compile_commands.json .` to create a symlink. This also means it will change as the source json file changes since it is a link.

On Windows I am gonna do that later lol.

