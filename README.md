# OCLC: Simple offline OpenCL compiliation checker using OpenCL API.

oclc maybe good to check your OpenCL kernel code on each vendor's OpenCL compiler. 

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


## Example

    $ ./oclc
    Usage: ./oclc <options> input.cl
      <options>
    
      --verbose           Verbose mode.
      --platform=N        Specify platform ID.
      --device=N          Specify device ID.
      --clopt=STRING      Specify compiler options for OpenCL compiler.


    $ ./oclc test.cl 
    [OCL] clBuildProgram failed. err = -11
    err: <program source>:1:1: error: unknown type name 'aafloat'; did you mean 'float'?
    aafloat aa()
    ^

## TODO

* custom header inclusion.
* binary kernel generation.


## License

OCLC is licensed under BSD license.

OptionParser ( https://github.com/weisslj/cpp-optparse ) is

Copyright (C) 2010 Johannes Weißl <jargon@molb.org>
License: your favourite BSD-style license
