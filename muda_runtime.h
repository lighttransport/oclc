//
// Copyright 2009 - 2017 Light Transport Entertainment Inc.
//
#ifndef MUDA_RUNTIME_H
#define MUDA_RUNTIME_H

// C headers
#include <cstdlib>

// C++ headers
#include <vector>
#include <map>

#ifdef HAVE_OPENCL
#include "clew.h"
#endif // HAVE_OPENCL

namespace muda {

typedef enum {
  cpu = 1, // muda
  ocl_cpu,
  ocl_gpu,
  ocl_accel,
  cuda,
  brook,
} MUDADeviceTarget;

typedef enum {
  host = 1,
  device_global,
  device_cached_global,
  device_constant,
  device_texture,
} MUDAMemoryType;

typedef enum {
  ro, // read only.
  wo, // write only.
  rw, // read and write.
} MUDAMemoryAttrib;

// Forward decl.
struct _MUDAMemory;
typedef struct _MUDAMemory *MUDAMemory; // MUDA memory object.
struct _MUDAProgram;
typedef struct _MUDAProgram *MUDAProgram; // MUDA kernel object.
struct _MUDAKernel;
typedef struct _MUDAKernel *MUDAKernel; // MUDA kernel object.
class MUDADeviceImpl;

// Base class of MUDA device.
class MUDADevice {
public:
  MUDADevice(MUDADeviceTarget target);
  ~MUDADevice();

  //  Function: initialize
  //  Initializes device.
  bool initialize(int platformID = 0, int preferredDeviceID = 0,
                  bool verbosity = false);

  //  Function: getNumDevides
  //  Returns # of available devices.
  int getNumDevices();

  //  Function: estimateMFlops
  //  Returns the Mflops of ith device.
  int estimateMFlops(int deviceId);

  //  Function: loadKernelSource
  //  Loads and compiles MUDA kernel code from source file.
  MUDAProgram loadKernelSource(const char *filename, int nheaders,
                               const char **headers, const char *options);

  //  Function: loadKernelBinary
  //  Loads precompiled MUDA binary from the file.
  MUDAProgram loadKernelBinary(const char *filename);

  MUDAKernel createKernel(const MUDAProgram program, const char *functionName);

  //  Function getModule
  //  Get compiled binary kernel module.
  bool getModule(MUDAProgram program, std::vector<char>& binary);

  MUDAMemory alloc(MUDAMemoryType memType, MUDAMemoryAttrib memAttrib,
                   size_t memSize);

  // MUDAMemory allocImage(MUDAMemoryType memType, MUDAMemoryAttrib memAttrib,
  //              size_t width, size_t height, int components);

  bool free(MUDAMemory mem);

  bool write(int ID, MUDAMemory mem, size_t size, const void *ptr);

  // bool writeImage(int ID, MUDAMemory mem, size_t width, size_t height, const
  // void *ptr);

  bool read(int deviceID, MUDAMemory mem, size_t size, void *ptr);

  bool bindMemoryObject(MUDAKernel kernel, int argNum, MUDAMemory mem);
  bool setArg(MUDAKernel kernel, int argNum, size_t size, size_t align,
              void *arg);

  //  Function: execute
  //  Executes MUDA kernel.
  bool execute(int deviceID, MUDAKernel kernel, int dimension, size_t sizeX,
               size_t sizeY, size_t sizeZ, size_t localSizeX, size_t localSizeY,
               size_t localSizeZ);

  // compileSource();

private:
  MUDADeviceImpl *impl;
};

//
// MDUA Device Interface class.
//
class MUDADeviceImpl {
public:
  MUDADeviceImpl() {};
  ~MUDADeviceImpl() {};

  //  Function: initialize
  //  Initializes MUDA device(s).
  virtual bool initialize(int platformID, int preferredDeviceID,
                          bool verbosity) = 0;

  virtual int getNumDevices() = 0;

  //  Function: estimateMFlops
  //  Returns the Mflops of ith device.
  virtual int estimateMFlops(int deviceId) = 0;

  //  Function: shutdown
  //  Free all MU related resources, except for memory objects.
  virtual bool shutdown() = 0;

  //  Function: loadKernelSource
  //  Loads and compiles MUDA kernel from source file.
  virtual MUDAProgram loadKernelSource(const char *filename, int nheaders,
                                       const char **headers,
                                       const char *options) = 0;

  //  Function: loadKernelBinary
  //  Loads precompiled MUDA binary from the file.
  virtual MUDAProgram loadKernelBinary(const char *filename) = 0;

  //  Function: createKernel
  //  Creates kernel object from kernel module.
  //  You should call loadKernelSource() before calling createKernel().
  virtual MUDAKernel createKernel(const MUDAProgram program,
                                  const char *functionName) = 0;

  //  Function getModule
  //  Get compiled binary kernel module.
  virtual bool getModule(MUDAProgram program, std::vector<char>& binary) = 0;

  //  Function: alloc
  //  Allocates MUDA device memory.
  virtual MUDAMemory alloc(MUDAMemoryType memType, MUDAMemoryAttrib memAttrib,
                           size_t memSize) = 0;

  // virtual MUDAMemory allocImage(MUDAMemoryType memType, MUDAMemoryAttrib
  // memAttrib,
  //              size_t width, size_t height, int components) = 0;

  //  Function: free
  //  Frees MUDA device memory.
  virtual bool free(MUDAMemory mem) = 0;

  //  Function: bindMemoryObject
  //  Binds memory object to the MUDA kernel.
  virtual bool bindMemoryObject(MUDAKernel kernel, int argNum,
                                MUDAMemory mem) = 0;

  // Function: setKernelArg
  // Sets any object to the kernel.
  virtual bool setArg(MUDAKernel kernel, int argNum, size_t size, size_t align,
                      void *arg) = 0;

  //  Function: execute
  //  Executes MUDA kernel.
  virtual bool execute(int deviceID, MUDAKernel kernel, int dimension,
                       size_t sizeX, size_t sizeY, size_t sizeZ,
                       size_t localSizeX, size_t localSizeY,
                       size_t localSizeZ) = 0;

  //  Function: read
  //  Reads data from MUDA buffer.
  virtual bool read(int deviceID, MUDAMemory mem, size_t size, void *ptr) = 0;

  //  Function: write
  //  Writes data to MUDA buffer.
  //  This function does not return until actual memory copy is finished
  //  (bloking operation).
  virtual bool write(int deviceID, MUDAMemory mem, size_t size,
                     const void *ptr) = 0;

  //  Function: writeImage
  //  Writes image to MUDA buffer.
  //  This function does not return until actual memory copy is finished
  //  (bloking operation).
  // virtual bool writeImage(int deviceID, MUDAMemory mem, size_t width, size_t
  // height,
  //                   const void *ptr) = 0;
private:
};

// OpenCL
class MUDADeviceOCL : public MUDADeviceImpl {
public:
  MUDADeviceOCL(MUDADeviceTarget target);
  ~MUDADeviceOCL();

  //  Function: initialize
  //  Initializes OpenCL device(s).
  bool initialize(int platformID = 0, int preferredDeviceID = 0,
                  bool verbosity = false);

  int getNumDevices();

  //  Function: estimateMFlops
  //  Returns the Mflops of ith device.
  int estimateMFlops(int deviceId);

  //  Function: shutdown
  //  Free all CL related resources, except for memory objects.
  bool shutdown();

  //  Function: loadKernelSource
  //  Loads and compiles OpenCL kernel from source file.
  MUDAProgram loadKernelSource(const char *filename, int nheaders,
                               const char **headers, const char *options);

  //  Function: loadKernelBinary
  //  Loads precompiled MUDA binary from the file.
  MUDAProgram loadKernelBinary(const char *filename);

  //  Function: createKernel
  //  Creates CL kernel object from CL program.
  //  You should call loadKernelSource() before calling createKernel().
  MUDAKernel createKernel(const MUDAProgram program, const char *functionName);

  //  Function getModule
  //  Get compiled binary kernel module. 
  bool getModule(MUDAProgram program, std::vector<char>& binary);

  //  Function: alloc
  //  Allocates OpenCL device memory.
  MUDAMemory alloc(MUDAMemoryType memType, MUDAMemoryAttrib memAttrib,
                   size_t memSize);

  //  Function: allocImage
  //  Allocates OpenCL image memory.
  // MUDAMemory allocImage(MUDAMemoryType memType, MUDAMemoryAttrib memAttrib,
  //              size_t width, size_t height, int components);

  //  Function: free
  //  Frees OpenCL device memory.
  bool free(MUDAMemory mem);

  //  Function: bindMemoryObject
  //  Binds memory object to the OpenCL kernel.
  bool bindMemoryObject(MUDAKernel kernel, int argNum, MUDAMemory mem);

  // Function: setKernelArg
  // Sets any object to the kernel.
  bool setArg(MUDAKernel kernel, int argNum, size_t size, size_t align,
              void *arg);

  //  Function: execute
  //  Executes OpenCL kernel.
  bool execute(int deviceID, MUDAKernel kernel, int dimension, size_t sizeX,
               size_t sizeY, size_t sizeZ, size_t localSizeX, size_t localSizeY,
               size_t localSizeZ);

  //  Function: read
  //  Reads data from OpenCL buffer.
  bool read(int deviceID, MUDAMemory mem, size_t size, void *ptr);

  //  Function: write
  //  Writes data to OpenCL buffer.
  //  This function does not return until actual memory copy is finished
  //  (bloking operation).
  bool write(int deviceID, MUDAMemory mem, size_t size, const void *ptr);

  //  Function: writeImage
  //  Writes image to OpenCL buffer.
  //  This function does not return until actual memory copy is finished
  //  (bloking operation).
  // bool writeImage(int deviceID, MUDAMemory mem, size_t width, size_t height,
  // const void *ptr);

  // Returns size of OpenCL memory object.
  const size_t getMemoryObjectSize() const;

private:
  bool useCPU;
  bool debug;
  bool measureProfile;
  bool verb;

  std::vector<MUDAProgram> programs;

#ifdef HAVE_OPENCL
  cl_context context;
  std::vector<cl_device_id> devices; // array
  int currentDeviceID;

  std::vector<cl_command_queue> commandQueues;
  std::vector<cl_kernel> kernels;
// std::vector<MUDAMemory>   memObjs;
#endif
};

} // namespace muda

#endif // MUDA_RUNTIME_H
