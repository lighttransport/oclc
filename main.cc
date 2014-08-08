#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <string>
#include <fstream>
#include <vector>

#ifdef _WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#else
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#endif

#include "muda_runtime.h"
//#include "timerutil.h"
#include "OptionParser.h"

void usage(const char *prog) {
  printf("Usage: %s <options> input.cl\n", prog);
  printf("  <options>\n");
  printf("\n");
  printf("  --verbose           Verbose mode.\n");
  printf("  --platform=N        Specify platform ID.\n");
  printf("  --device=N          Specify device ID.\n");
  printf(
      "  --clopt=STRING      Specify compiler options for OpenCL compiler.\n");
  printf("  --header=FILENAME   Specify custom header file to be included.\n");
  printf("  -c                  Build kernel module.\n");
}

std::string readfile(const char *path) {
  std::ifstream clsrc(path);
  std::istreambuf_iterator<char> vdataBegin(clsrc);
  std::istreambuf_iterator<char> vdataEnd;
  std::string clstr(vdataBegin, vdataEnd);

  return clstr;
}

int main(int argc, char *const argv[]) {

  if (argc < 2) {
    usage(argv[0]);
    exit(1);
  }

  optparse::OptionParser parser = optparse::OptionParser();

  parser.set_defaults("verbosity", "0");
  parser.add_option("--verbose").action("store_true").dest("verbosity");
  parser.add_option("--platform")
      .action("store")
      .type("int")
      .set_default(0)
      .help("default: %default");
  parser.add_option("--device").action("store").type("int").set_default(0).help(
      "default: %default");
  parser.add_option("--header").dest("header");
  parser.add_option("--clopt").action("store").type("string");
  parser.add_option("-c").action("store_true").dest("module");

  optparse::Values &options = parser.parse_args(argc, argv);
  std::vector<std::string> args = parser.args();

  if (args.size() < 1) {
    printf("Needs input OpenCL kernel file.\n");
    usage(argv[0]);
    exit(1);
  }

  bool verb = (bool)options.get("verbosity");
  bool module = (bool)options.get("module");

  int reqPlatformID = (int)options.get("platform");
  int deviceNum = (int)options.get("device");
  const char *headerfilename = options["header"].c_str();

  // printf("Use platform: %d\n", reqPlatformID);

  muda::MUDADeviceOCL *device = new muda::MUDADeviceOCL(muda::ocl_cpu);
  assert(device);

  bool ret = device->initialize(reqPlatformID, deviceNum, verb);
  assert(ret);

  int numDevices = device->getNumDevices();

  std::string kernelfile = args.at(0);

  const char *cloptions = (const char *)options.get("clopt");
  if (verb)
    printf("clopts = %s\n", cloptions);


  std::string headerStr;
  if (headerfilename) {
    if (verb)
      printf("Reading header file: %s\n", headerfilename);
    headerStr = readfile(headerfilename);
  }

  muda::MUDAProgram prog;
  if (headerStr.empty()) {
    prog = device->loadKernelSource(kernelfile.c_str(), 0, NULL, cloptions);
  } else {
    const char *headers[1];
    headers[0] = headerStr.c_str();
    prog = device->loadKernelSource(kernelfile.c_str(), 1, headers, cloptions);
  }

  if (!prog) {
    return -1;
  }

  if (module) {
    std::vector<char> bins;
    bool ret = device->getModule(prog, bins);
    if (!ret) {
      return -1;
    }

    if (bins.size() == 0) {
      return -1;
    }

    FILE* fp = fopen("module.dat", "wb");
    if (!fp) {
      return -1;
    }

    fwrite(&bins.at(0), 1, bins.size(), fp);
    fclose(fp);
  }

  return 0;
}
