# OCLC: Simple offline OpenCL compiliation checker using OpenCL API.

`oclc` maybe good to check your OpenCL kernel code on each vendor's OpenCL compiler. 

`oclc` uses `clew`, thus no OpenCL SDK required to compile.

## Requirement

* premake5 alpha 11 or later
* C++ compiler
* Visual Studio 2013 + 64bit Windows.

## Build

### Linux and MacOSX

    $ premake5 gmake
    $ make

### Windows

    > vcbuild.bat
    
to generate a soltion file.

## Usage

    $ oclc <options> input.cl


## Example

    $ ./oclc
    Usage: ./oclc <options> input.cl
      <options>
    
      --verbose           Verbose mode.
      --platform=N        Specify platform ID.
      --device=N          Specify device ID.
      --clopt=STRING      Specify compiler options for OpenCL compiler.
      --header=FILENAME   Specify custom header file to be included. 


    $ ./oclc test.cl 
    [OCL] clBuildProgram failed. err = -11
    err: <program source>:1:1: error: unknown type name 'aafloat'; did you mean 'float'?
    aafloat aa()
    ^

    $ ./oclc --header=testheader.h test.cl 


## Supported OpenCL version

1.2(due to clew's limitation)

## TODO

* binary kernel generation.

## License

OCLC is licensed under BSD license.

### clew 

https://github.com/OpenCLWrangler/clew

```
//////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2009-2011 Organic Vectory B.V., KindDragon
//  Written by George van Venrooij
//
//  Distributed under the MIT License.
//////////////////////////////////////////////////////////////////////////
```

### OptionParser

https://github.com/weisslj/cpp-optparse

```
Copyright (C) 2010 Johannes Weißl <jargon@molb.org>
License: your favourite BSD-style license
```
