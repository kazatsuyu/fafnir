[日本語](./README_ja.md)
# Fafnir

Clang MSBuild toolset for Visual Studio 2017

## What's this?

This is a MSBuild toolset with LLVM Clang for Windows.

## Requirements

* Visual Studio 2017
* Clang for Windows
  [Download](http://releases.llvm.org/download.html) from the LLVM website and install it, or use the [snapshot](http://llvm.org/builds/). You can also use one of your builds.

## Usage

First, download the [latest release](https://github.com/kazatsuyu/fafnir/releases/). Its host architecture must be the same to that of Clang for Windows.

### How to install

Run install.bat, and set the LLVM path and a toolset name. Administrator authority is required for installation. To use with CMake, the toolset name must match `v[0-9]+_clang_.*`.  

### How to use with Visual Studio IDE

Open the project property, select \[Configuration Properties -> General\] in the left list, select \[Platform toolset\] in \[General\] group, and select the toolset name that you set during the installation (the default name is v100_clang_fafnir). Then you can build the project using LLVM clang.

### How to use with CMake

When you run cmake command, specify `"Visual Studio 15 2017"` or `"Visual Studio 15 2017 Win64"` to the option `-G`, specify the toolset name, that you set during the installation, to the option `-T`.

## How to build Fafnir

The toolset installed by Fafnir is required to build Fafnir. Also, LLVM that supports C++17 must be used (e.g. the release binaries of Fafnir are built with the [LLVM Snapshot Build](http://llvm.org/builds/)). CMake is also required.

* Build 64bit version
```
cmake -G "Visual Studio 15 2017 Win64" -T <installed Fafnir toolset> -DCMAKE_INSTALL_PREFIX=<path to install>
cmake --build . --config Release --target INSTALL
```

* Build 32bit version
```
cmake -G "Visual Studio 15 2017" -T <installed Fafnir toolset> -DCMAKE_INSTALL_PREFIX=<path to install>
cmake --build . --config Release --target INSTALL
```

Run the above commands, and then you will find deployed files in the install path.
