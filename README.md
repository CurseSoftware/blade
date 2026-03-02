# Blade Game & Application Library
Cross platform utility library for constructing games and applications in C++.

## NOTE:
Some development on this has been halted to efforts shifing to the [xgpu Rendering Hardware Interface](https://github.com/CurseSoftware/xgpu) which will be used inside the graphics library in Blade.
Some other development effort has been spent on the (Experimental Viper Language Toolchain)[https://github.com/CurseSoftware/viper] which is an unrelated project, but still taking development resources.

## Purpose
We wish to build a library for C++ that is useful for creating graphics applications.
To achieve this, we are maintaining it as cross-platform (Linux & Windows) with Macos support planned.

We wish for the APIs to be natural to use in most C++ environments, and we also wish that each component of the library can be used independently as a standalone utility. This means that if you just want to use the rendering API without our Windowing API, you can. As such, this means that the "sub-libraries" of the whole library are not dependent on the others.

In the future, we want there to be a way to grab the desired sub-libraries individually without fetching the entire thing.

## Building
#### Build Requirements
C++ 20 compliant compiler

#### Build commands
There are included `go.sh` and `go.ps` scripts in the project root directory.

Run the command `./go.sh Build-Debug` to build the debug build commands on Linux or MacOS.

Run the command `./go.sh Build-Release` to build the release build commands on Linux or MacOS.

Run the commnad `./go.sh Run-Example <example_name> <debug | release>` to run an example

These commands place all build utilities CMake generates in the `build` directory which is preferable to cluttering the root dir.

## Using compile_commands.json
If you are using the clangd language server, CMake can generate a `compile_commands.json` file that works with clangd in order to allow the LSP to work.

Because we are placing CMake build outputs in our `build` directory, this places the json file there as well. 

On Linux you can run `ln -s build/compile_commands.json .` to create a symlink. This also means it will change as the source json file changes since it is a link.

On Windows I am gonna do that later lol.

