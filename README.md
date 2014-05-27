# OCLC: Simple offline OpenCL compiler using OpenCL API.

Maybe good to test each vendor's OpenCL compilation. 

## Requirement

* OpenCL SDK from each vendor
* premake4
* C++ compiler

## Build

    $ premake4 gmake
    $ make

If you need to specify path to OpenCL SDK, use --opencl-path and --opencl-libpath flag when invoking premake4

    $ premake4 --opencl-path=<OPENCL_SDK_PATH> --opencl-libpath=<OPENCL_SDK_LIB_PATH> gmake

## Usage

    $ oclcc <options> input.cl


## License

OCLC is licensed under BSD license.

OptionParser ( https://github.com/weisslj/cpp-optparse ) is

Copyright (C) 2010 Johannes Weißl <jargon@molb.org>
License: your favourite BSD-style license
