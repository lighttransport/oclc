//
// MUDA device for OpenCL.
//
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <iterator>

#include "muda_runtime.h"
#include "muda_impl.h"

#include <sys/stat.h>

using namespace std;

#define CL_CHECK(ret)                                                          \
  {                                                                            \
    if ((ret) != CL_SUCCESS) {                                                 \
      fprintf(stderr, "CL operation failed at %s:%d (%d)\n", __FILE__,         \
              __LINE__, ret);                                                  \
    }                                                                          \
  }

namespace muda {

MUDADeviceOCL::MUDADeviceOCL(MUDADeviceTarget target) : MUDADeviceImpl() {
  assert(target == ocl_cpu || target == ocl_gpu || target == ocl_accel);

  //
  // At this time there is support for ocl_accel type.
  //

  if (target == ocl_cpu) {
    this->useCPU = true;
  } else {
    this->useCPU = false;
  }

  this->debug = false;
  this->measureProfile = false;

#ifdef HAVE_OPENCL

  this->context = 0;
  this->kernels.clear();
  this->commandQueues.clear();

#endif
}

MUDADeviceOCL::~MUDADeviceOCL() {
#ifdef HAVE_OPENCL
  {
    std::vector<cl_command_queue>::iterator it;
    for (it = this->commandQueues.begin(); it != this->commandQueues.end();
         it++) {
      clReleaseCommandQueue(*it);
    }
  }

// {
//     std::vector<cl_kernel>::iterator it;
//     for (it = this->kernels.begin();
//          it != this->kernels.end();
//          it++) {
//         clReleaseKernel(*it);
//     }
// }

// clReleaseProgram(this->program);

#endif
}

bool MUDADeviceOCL::initialize(int reqPlatformID, int preferredDeviceID,
                               bool verbosity) {

  verb = verbosity;

#ifdef HAVE_OPENCL

  //
  // Query platform
  //
  cl_platform_id platform_ids[32];
  cl_platform_id platform_id;
  {
    cl_int errCode = CL_SUCCESS;

    cl_uint numPlatforms;
    errCode = clGetPlatformIDs(0, 0, &numPlatforms);
    if (errCode != CL_SUCCESS) {
      fprintf(stdout, "[OCL] clGetPlatformIDs failed.\n");
      return false;
    };

    assert(numPlatforms >= 1);
    assert(numPlatforms < 32);
    assert(reqPlatformID < (int)numPlatforms);
    if (verbosity)
      printf("[OCL] Num platforms: %d\n", numPlatforms);
    errCode = clGetPlatformIDs(numPlatforms, platform_ids, 0);
    if (errCode != CL_SUCCESS) {
      fprintf(stdout, "[OCL] clGetPlatformIDs failed.\n");
      return false;
    }

    if (verbosity) {
      char buffer[2048];
      for (int i = 0; i < (int)numPlatforms; i++) {
        clGetPlatformInfo(platform_ids[i], CL_PLATFORM_NAME, sizeof(buffer),
                          buffer, NULL);
        printf("CL_PLATFORM_NAME:    %s\n", buffer);
        clGetPlatformInfo(platform_ids[i], CL_PLATFORM_PROFILE, sizeof(buffer),
                          buffer, NULL);
        printf("CL_PLATFORM_PROFILE: %s\n", buffer);
        clGetPlatformInfo(platform_ids[i], CL_PLATFORM_VERSION, sizeof(buffer),
                          buffer, NULL);
        printf("CL_PLATFORM_VERSION: %s\n", buffer);
        clGetPlatformInfo(platform_ids[i], CL_PLATFORM_VENDOR, sizeof(buffer),
                          buffer, NULL);
        printf("CL_PLATFORM_VENDOR:  %s\n", buffer);
        clGetPlatformInfo(platform_ids[i], CL_PLATFORM_EXTENSIONS,
                          sizeof(buffer), buffer, NULL);
        printf("CL_PLATFORM_EXTENSIONS: %s\n", buffer);
      }
    }

    int max_devices = 32;
    cl_device_id devices[32];

    // this->devices.resize(max_devices);
    cl_uint num_devices;
    platform_id = platform_ids[reqPlatformID];
    errCode = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, max_devices,
                             devices, &num_devices);
    CL_CHECK(errCode);

    assert(num_devices >= 1);

    if (verbosity)
      printf("[MUDA] [OCL] # of devices = %d\n", num_devices);

    this->devices.clear();
    for (int i = 0; i < (int)num_devices; i++) {
      this->devices.push_back(devices[i]);
    }

    this->currentDeviceID = 0;

    if (verbosity) {
      char buffer[8192];
      for (int i = 0; i < (int)num_devices; i++) {
        printf("==> Device [%d] ========================\n", i);
        size_t size_ret;
        clGetDeviceInfo(this->devices[i], CL_DEVICE_NAME, sizeof(buffer),
                        &buffer, &size_ret);
        printf("CL_DEVICE_NAME: %s\n", buffer);
        clGetDeviceInfo(this->devices[i], CL_DEVICE_VENDOR, sizeof(buffer),
                        &buffer, (size_t *)buffer);
        printf("CL_DEVICE_VENDOR: %s\n", buffer);
        clGetDeviceInfo(this->devices[i], CL_DEVICE_OPENCL_C_VERSION,
                        sizeof(buffer), &buffer, &size_ret);
        printf("CL_DEVICE_OPENCL_C_VERSION: %s\n", buffer);
        clGetDeviceInfo(this->devices[i], CL_DEVICE_PROFILE, sizeof(buffer),
                        &buffer, &size_ret);
        printf("CL_DEVICE_PROFILE: %s\n", buffer);
        clGetDeviceInfo(this->devices[i], CL_DEVICE_VERSION, sizeof(buffer),
                        &buffer, &size_ret);
        printf("CL_DEVICE_VERSION: %s\n", buffer);
        clGetDeviceInfo(this->devices[i], CL_DRIVER_VERSION, sizeof(buffer),
                        &buffer, &size_ret);
        printf("CL_DRIVER_VERSION: %s\n", buffer);
        clGetDeviceInfo(this->devices[i], CL_DEVICE_EXTENSIONS, sizeof(buffer),
                        &buffer, &size_ret);
        printf("CL_DEVICE_EXTENSIONS: %s\n", buffer);

        cl_uint uival;
        clGetDeviceInfo(this->devices[i], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
                        sizeof(uival), &uival, &size_ret);
        printf("CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS: %d\n", uival);

        cl_bool bval;
        clGetDeviceInfo(this->devices[i], CL_DEVICE_IMAGE_SUPPORT,
                        sizeof(cl_bool), &bval, &size_ret);
        printf("CL_DEVICE_IMAGE_SUPPORT: %d\n", bval);
        if (bval) {
          size_t val;
          clGetDeviceInfo(this->devices[i], CL_DEVICE_IMAGE2D_MAX_WIDTH,
                          sizeof(size_t), &val, &size_ret);
          printf("CL_DEVICE_IMAGE2D_MAX_WIDTH: %d\n", (int)val);
          clGetDeviceInfo(this->devices[i], CL_DEVICE_IMAGE2D_MAX_HEIGHT,
                          sizeof(size_t), &val, &size_ret);
          printf("CL_DEVICE_IMAGE2D_MAX_HEIGHT: %d\n", (int)val);
        }

        {
          cl_ulong uval;
          clGetDeviceInfo(this->devices[i], CL_DEVICE_GLOBAL_MEM_SIZE,
                          sizeof(cl_uint), &uval, &size_ret);
          printf("CL_DEVICE_GLOBAL_MEM_SIZE: %lld\n", uval);
        }

        size_t sz;
        clGetDeviceInfo(this->devices[i], CL_DEVICE_MAX_WORK_GROUP_SIZE,
                        sizeof(size_t), &sz, &size_ret);
        printf("CL_DEVICE_MAX_WORK_GROUP_SIZE: %d\n", (int)sz);

        {
          cl_uint uval;
          clGetDeviceInfo(this->devices[i], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
                          sizeof(cl_uint), &uval, &size_ret);
          printf("CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS: %d\n", (int)uval);
        }

        size_t sz3[3];
        clGetDeviceInfo(this->devices[i], CL_DEVICE_MAX_WORK_ITEM_SIZES,
                        3 * sizeof(size_t), sz3, &size_ret);
        printf("CL_DEVICE_MAX_WORK_ITEM_SIZES: %d, %d, %d\n", (int)sz3[0],
               (int)sz3[1], (int)sz3[2]);
      }
    }
  }

  //
  // Create CL context.
  //
  if (preferredDeviceID < (int)this->devices.size()) {
    this->currentDeviceID = preferredDeviceID;
  }
  if (verbosity)
    printf("[MUDA] [OCL] Use device: %d\n", this->currentDeviceID);

  {
    cl_int err;
    context = clCreateContext(NULL, 1, &this->devices[this->currentDeviceID],
                              NULL, NULL, &err);

    if (err != CL_SUCCESS) {

      cout << "[OCL] Failed to create CL context.\n";

      if (!this->useCPU) {
        cout << "[OCL] Unsupported GPU card?.\n";
        cout << "[OCL] Try to use CPU version.\n";
      } else {
        cout << "[OCL] CPU version doesn't work.\n";
      }

      exit(-1); // force exit.

      return false;
    }
  }

  return true;
#else

  cout << "OpenCL device target is not supported in this build."
       << "\n";
  return false;

#endif // HAVE_OPENCL
}

int MUDADeviceOCL::getNumDevices() {
#if HAVE_OPENCL
  return this->devices.size();
#else
  cout << "OpenCL device target is not supported in this build."
       << "\n";
  return 1;
#endif
}

int MUDADeviceOCL::estimateMFlops(int deviceId) {
#if HAVE_OPENCL
  cl_device_id clDeviceId = this->devices[deviceId];
  cl_uint ncompute_units;
  clGetDeviceInfo(clDeviceId, CL_DEVICE_MAX_COMPUTE_UNITS,
                  sizeof(ncompute_units), &ncompute_units, NULL);

  cl_uint nclock_freq;
  clGetDeviceInfo(clDeviceId, CL_DEVICE_MAX_CLOCK_FREQUENCY,
                  sizeof(nclock_freq), &nclock_freq, NULL);

  return 3 * 8 * ncompute_units * nclock_freq;

#else
  cout << "OpenCL device target is not supported in this build."
       << "\n";
  return 1;
#endif
}

bool MUDADeviceOCL::shutdown() {

#if HAVE_OPENCL
  return true;
#else

  cout << "OpenCL device target is not supported in this build."
       << "\n";
  return false;

#endif // HAVE_OPENCL
}

MUDAProgram MUDADeviceOCL::loadKernelSource(const char *filename, int nheaders,
                                            const char **headers,
                                            const char *options) {
#if HAVE_OPENCL

  assert(this->context != NULL);

  size_t len;
  cl_int err;

  char path[4096];
  sprintf(path, "%s", filename);

  if (verb) {
    cout << "[OCL] Read CL kernel: " << path << "\n";
  }


  std::ifstream clsrc(path);
  std::istreambuf_iterator<char> vdataBegin(clsrc);
  std::istreambuf_iterator<char> vdataEnd;
  std::string clstr(vdataBegin, vdataEnd);

  std::vector<const char*> args;
  std::vector<size_t> lengths;
  for (int i = 0; i < nheaders; i++) {
    args.push_back(headers[i]);
    lengths.push_back(strlen(headers[i]));
  }
  args.push_back(clstr.c_str());
  lengths.push_back(clstr.size());

  int n = args.size();

  MUDAProgram program = new _MUDAProgram;
  program->progObjOCL = clCreateProgramWithSource(
      this->context, n, &args.at(0), &lengths.at(0), &err);
  CL_CHECK(err);

  err = clBuildProgram(program->progObjOCL, 1,
                       &this->devices[this->currentDeviceID], options, NULL,
                       NULL);

  ///< @fixme { make the code NV independent. }
  if (err != CL_SUCCESS) {
    fprintf(stdout, "[OCL] clBuildProgram failed. err = %d\n", err);

    size_t len;
    std::vector<unsigned char> buffer;

    // Get err str length
    clGetProgramBuildInfo(program->progObjOCL,
                          this->devices[this->currentDeviceID],
                          CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
    buffer.resize(len);

    clGetProgramBuildInfo(program->progObjOCL,
                          this->devices[this->currentDeviceID],
                          CL_PROGRAM_BUILD_LOG, len, &buffer.at(0), NULL);
    // printf("len = %d\n", (int)len);
    printf("err: %s\n", &buffer.at(0));
    exit(1);
  }

//
// Setup command queues.
//
#if 0
  {
    for (int i = 0; i < this->devices.size(); i++) {

      cl_command_queue cmdq;

      cmdq = clCreateCommandQueue(this->context, this->devices[i],
                                  0, //CL_QUEUE_PROFILING_ENABLE,
                                  &err);
      if (err != CL_SUCCESS) {
          cout << "[OCL] Failed to create command queue.\n";
          exit(-1);
      }

      this->commandQueues.push_back(cmdq);

    }
  }
#else
  {
    cl_command_queue cmdq;

    cmdq = clCreateCommandQueue(this->context,
                                this->devices[this->currentDeviceID],
                                0, // CL_QUEUE_PROFILING_ENABLE,
                                &err);
    if (err != CL_SUCCESS) {
      cout << "[OCL] Failed to create command queue.\n";
      exit(-1);
    }

    this->commandQueues.push_back(cmdq);
  }
#endif

  return program;
#else

  cout << "OpenCL device target is not supported in this build."
       << "\n";
  return NULL;

#endif
}

MUDAProgram MUDADeviceOCL::loadKernelBinary(const char *filename) {
#if HAVE_OPENCL
  assert(this->context != NULL);

  size_t len;
  cl_int err;

  char path[4096];
  sprintf(path, "%s.clbin", filename);

  cout << "[OCL] Load CL kernel: " << path << "\n";

  struct stat st;
  // If this source file does not include <sys/stat.h>, the below line occurs a
  // compile error.
  stat(path, &st);

  len = (size_t)st.st_size;
  FILE *fp = fopen(path, "rb");
  size_t lens[] = {len};
  unsigned char *bins[] = {new unsigned char[len]};
  size_t lenRead = fread(bins[0], 1, len, fp);
  assert(lenRead == len);

  MUDAProgram program = new _MUDAProgram;

  program->progObjOCL = clCreateProgramWithBinary(
      this->context, this->devices.size(),
      &this->devices[this->currentDeviceID], lens,
      const_cast<const unsigned char **>(bins), NULL, &err);
  CL_CHECK(err);

  err = clBuildProgram(program->progObjOCL, this->devices.size(),
                       &this->devices[this->currentDeviceID], NULL, NULL, NULL);
  delete[] bins[0];

  ///< @fixme { make the code NV independent. }
  if (err != CL_SUCCESS) {
    size_t len;
    char buffer[2048];

    clGetProgramBuildInfo(program->progObjOCL,
                          this->devices[this->currentDeviceID],
                          CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
    printf("%s\n", buffer);
    exit(1);
  }

  //
  // Setup command queues.
  //
  {
    for (int i = 0; i < (int)this->devices.size(); i++) {
      printf("[OCL] OpenCL Device %d Info:\n", i);
      // oclPrintDevInfo(LOGBOTH, this->devices[i]);

      cl_command_queue cmdq;

      cmdq = clCreateCommandQueue(this->context, this->devices[i], 0, &err);
      if (err != CL_SUCCESS) {
        cout << "[OCL] Failed to create command queue.\n";
        exit(-1);
      }

      this->commandQueues.push_back(cmdq);
    }
  }

  return program;
#else
  cout << "OpenCL device target is not supported in this build."
       << "\n";
  return NULL;
#endif
}

MUDAMemory MUDADeviceOCL::alloc(MUDAMemoryType memType,
                                MUDAMemoryAttrib memAttrib, size_t memSize) {
#if HAVE_OPENCL

  assert(this->context != NULL);

  MUDAMemory mem = new _MUDAMemory;

  cl_int flag = 0;

  if (memAttrib == muda::ro) {
    flag |= CL_MEM_READ_ONLY;
  } else if (memAttrib == muda::wo) {
    flag |= CL_MEM_WRITE_ONLY;
  } else if (memAttrib == muda::rw) {
    flag |= CL_MEM_READ_WRITE;
  }

  cl_int err;
  cl_mem memObj = clCreateBuffer(this->context, flag, memSize, NULL, &err);
  CL_CHECK(err);

  mem->memObjOCL = memObj;
  mem->size = memSize;
  mem->ptr = NULL; // OCL target don't use this member.

  return mem;

#else

  cout << "OpenCL device target is not supported in this build."
       << "\n";
  return NULL;

#endif
}

#if 0
MUDAMemory
MUDADeviceOCL::allocImage(
    MUDAMemoryType   memType,
    MUDAMemoryAttrib memAttrib,
    size_t           width,
    size_t           height,
    int              components)
{
#if HAVE_OPENCL

    assert(this->context != NULL);

    MUDAMemory mem = new _MUDAMemory;

    cl_int flag = 0;

    if (memAttrib == muda::ro) {
        flag |= CL_MEM_READ_ONLY;
    } else if (memAttrib == muda::wo) {
        flag |= CL_MEM_WRITE_ONLY;
    } else if (memAttrib == muda::rw) {
        flag |= CL_MEM_READ_WRITE;
    }

    assert(components == 4);
    cl_image_format format;
    // @fixme
    format.image_channel_order = CL_RGBA;
    format.image_channel_data_type = CL_UNSIGNED_INT32;
    cl_int err;
    cl_mem memObj = clCreateImage2D(this->context, flag, &format, width, height, 0, 0, &err);
    CL_CHECK(err);

    mem->memObjOCL = memObj;
    mem->size      = width * height * sizeof(float) * 4;
    mem->ptr       = NULL;       // OCL target don't use this member.

    return mem;

#else

    cout << "OpenCL device target is not supported in this build." << "\n";
    return NULL;

#endif
}
#endif

bool MUDADeviceOCL::free(MUDAMemory mem) {
#if HAVE_OPENCL

  clReleaseMemObject(mem->memObjOCL);

  delete mem;
  return true;

#else

  cout << "OpenCL device target is not supported in this build."
       << "\n";
  return false;

#endif
}

MUDAKernel MUDADeviceOCL::createKernel(const MUDAProgram program,
                                       const char *functionName) {
#if HAVE_OPENCL

  assert(this->context != NULL);

  MUDAKernel kernel = new _MUDAKernel;

  cl_int err;
  kernel->kernObjOCL = clCreateKernel(program->progObjOCL, functionName, &err);

  if (err != CL_SUCCESS) {

    cout << "[OCL] Failed to create kernel. function name = " << functionName
         << "\n";

    delete kernel;

    exit(-1);
  }

  return kernel;

#else

  cout << "OpenCL device target is not supported in this build."
       << "\n";
  return false;

#endif
}

bool MUDADeviceOCL::read(int deviceID, MUDAMemory mem, size_t size, void *ptr) {
#ifdef HAVE_OPENCL

  assert(this->context != NULL);

  cl_event event;

  if (this->debug) {
    cout << "[OCL] read operation started.\n";
  }

  // blocking read.

  clEnqueueReadBuffer(this->commandQueues[deviceID], mem->memObjOCL, CL_FALSE,
                      0, size, ptr, 0, NULL, &event);

  clWaitForEvents(1, &event);
  cl_int err = clReleaseEvent(event);
  CL_CHECK(err);

  if (this->debug) {
    cout << "[OCL] read operation ended.\n";
  }

  return (err == CL_SUCCESS ? true : false);
#else

  cout << "OpenCL device target is not supported in this build."
       << "\n";
  return false;

#endif
}

bool MUDADeviceOCL::write(int deviceID, MUDAMemory mem, size_t size,
                          const void *ptr) {
#ifdef HAVE_OPENCL

  assert(this->context != NULL);

  if (this->debug) {
    cout << "[OCL] write operation started.\n";
  }

  cl_event event;

  assert(this->commandQueues[deviceID]);
  assert(mem->memObjOCL);

  cl_int err =
      clEnqueueWriteBuffer(this->commandQueues[deviceID], mem->memObjOCL,
                           CL_TRUE, 0, size, ptr, 0, NULL, &event);
  CL_CHECK(err);

  clWaitForEvents(1, &event);
  err = clReleaseEvent(event);
  CL_CHECK(err);

  if (this->debug) {
    cout << "[OCL] write operation ended.\n";
  }

  return (err == CL_SUCCESS ? true : false);

#else

  cout << "OpenCL device target is not supported in this build."
       << "\n";
  return false;

#endif
}

#if 0
bool
MUDADeviceOCL::writeImage(
    int     deviceID,
    MUDAMemory mem,
    size_t  width,
    size_t  height,
    const void   *ptr)
{
#ifdef HAVE_OPENCL

    assert(this->context != NULL);

    if (this->debug) {
        cout << "[OCL] write operation started.\n";
    }

    cl_event event;

    assert(this->commandQueues[deviceID]);
    assert(mem->memObjOCL);

    size_t origin[] = {0,0,0}; // Defines the offset in pixels in the image from where to write.
    size_t region[] = {width, height, 1}; // Size of object to be transferred

    size_t row_pitch = width * 4 * sizeof(unsigned int);
    cl_int err = clEnqueueWriteImage(this->commandQueues[deviceID],
                                      mem->memObjOCL,
                                      CL_TRUE,
                                      origin, region,
                                      row_pitch, 0,
                                      ptr,
                                      0, NULL, &event);
	CL_CHECK(err);

    clWaitForEvents(1, &event);
    err = clReleaseEvent(event);
    CL_CHECK(err);

    if (this->debug) {
        cout << "[OCL] write operation ended.\n";
    }
    
    return (err == CL_SUCCESS ? true : false);

#else

    cout << "OpenCL device target is not supported in this build." << "\n";
    return false;

#endif
}
#endif

bool MUDADeviceOCL::bindMemoryObject(MUDAKernel kernel, int argNum,
                                     MUDAMemory mem) {
#if HAVE_OPENCL

  cl_int err;
  err = clSetKernelArg(kernel->kernObjOCL, argNum, sizeof(cl_mem),
                       &mem->memObjOCL);
  CL_CHECK(err);

  return (err == CL_SUCCESS ? true : false);

#else

  cout << "OpenCL device target is not supported in this build."
       << "\n";
  return false;

#endif
}

bool MUDADeviceOCL::setArg(MUDAKernel kernel, int argNum, size_t size,
                           size_t align, void *arg) {
#if HAVE_OPENCL

  cl_int err;
  err = clSetKernelArg(kernel->kernObjOCL, argNum, size, arg);
  CL_CHECK(err);

  return (err == CL_SUCCESS ? true : false);

#else

  cout << "OpenCL device target is not supported in this build."
       << "\n";
  return false;

#endif
}

bool MUDADeviceOCL::execute(int deviceID, MUDAKernel kernel, int dimension,
                            size_t sizeX, size_t sizeY, size_t sizeZ,
                            size_t localSizeX, size_t localSizeY,
                            size_t localSizeZ) {

#if HAVE_OPENCL

  assert(this->context != NULL);

  size_t sizes[3];

  sizes[0] = sizeX;
  sizes[1] = sizeY;
  sizes[2] = sizeZ;

  // @fixme {}
  assert((sizeX % 8) == 0);
  // assert((sizeY % 8) == 0);

  size_t local_sizes[3];
  local_sizes[0] = localSizeX;
  local_sizes[1] = localSizeY;
  local_sizes[2] = 1;
  // printf("dim = %d, sz = %d, %d, %d\n", dimension, sizes[0], sizes[1],
  // sizes[2]);
  // printf("dim = %d, local sz = %d, %d, %d\n", dimension, local_sizes[0],
  // local_sizes[1], local_sizes[2]);

  // Blocking operation.

  cl_int err;
  cl_event event;

  err = clEnqueueNDRangeKernel(this->commandQueues[deviceID],
                               kernel->kernObjOCL, dimension, NULL, sizes,
                               local_sizes, 0, NULL, &event);
  CL_CHECK(err);

  err = clWaitForEvents(1, &event);
  CL_CHECK(err);

  cl_ulong start, end;
  // clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START,
  // sizeof(cl_ulong), &start, NULL);
  // clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong),
  // &end, NULL);
  // END-START gives you hints on kind of “pure HW execution time”
  // the resolution of the events is 1e-09 sec
  // cl_double timeMs = (cl_double)(end - start)*(cl_double)(1e-06);

  err = clReleaseEvent(event);
  CL_CHECK(err);
  // if (err == CL_SUCCESS) {
  //  cout << "[OCL] Kernel exec: " << timeMs << " msec(s)\n";
  //}
  return (err == CL_SUCCESS ? true : false);

#else

  cout << "OpenCL device target is not supported in this build."
       << "\n";
  return false;

#endif
}
}
