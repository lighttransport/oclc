//
// Implementation of MUDA classes and MUDA structs.
//
#ifndef MUDA_IMPL_H
#define MUDA_IMPL_H

#ifdef HAVE_OPENCL
#ifdef WIN32
#include <CL/cl.h>
#else
#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif
#endif
#endif	// HAVE_OPENCL

#ifdef HAVE_CUDA
#include <cuda.h>
#endif

namespace muda {

// MUDA memory object.
struct _MUDAMemory
{

    size_t  size;
    void   *ptr;                /// @todo { Change type to uint64_t? }

#if HAVE_OPENCL

    cl_mem  memObjOCL;

#endif

#if HAVE_CUDA

    MUDAMemoryType type;
    CUdeviceptr memObjCUDA;

#endif

    int dummy;

};

// MUDA module object.
struct _MUDAModule
{

#if HAVE_OPENCL

    cl_program  moduleObjOCL;

#endif

#if HAVE_CUDA

    CUmodule    moduleObjCUDA;

#endif

    int dummy;

};

// MUDA program object.
struct _MUDAProgram
{

#if HAVE_OPENCL

    cl_program  progObjOCL;

#endif

#if HAVE_CUDA

    CUmodule    progObjCUDA;

#endif

    int dummy;

};

// MUDA kernel object.
struct _MUDAKernel
{

#if HAVE_OPENCL

    cl_kernel  kernObjOCL;

#endif

#if HAVE_CUDA

    CUfunction kernObjCUDA;

#endif

    int dummy;

};

// MUDA event object.
struct MUDAEvent
{

#if HAVE_OPENCL

    cl_event    eventCL;

#endif

#if HAVE_CUDA

    CUevent     eventCU;

#endif

    int dummy;

};

}

#endif  // MUDA_IMPL_H
