sources = {
   "muda_impl.h",
   "muda_device_ocl.cc",
   "OptionParser.cpp",
   "main.cc",
   "third_party/clew/src/clew.c",
   }

-- premake4.lua
solution "OCLCSolution"
   configurations { "Release", "Debug" }

   if (os.is("windows")) then
      platforms { "x64" }
   else
      platforms { "native", "x32", "x64" }
   end

   -- A project defines one build target
   project "OCLC"
      kind "ConsoleApp"
      language "C++"
      files { sources }

      includedirs {
         "./",
         "./third_party/clew/include",
      }

      defines { 'HAVE_OPENCL' }

      -- MacOSX. Guess we use gcc.
      configuration { "macosx", "gmake" }

         defines { '_LARGEFILE_SOURCE', '_FILE_OFFSET_BITS=64' }

      -- Windows specific
      configuration { "windows", "gmake" }

         defines { '__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' } -- c99

         links { "stdc++", "msvcrt", "ws2_32", "winmm" }

      configuration { "windows", "vs2013", "x64" }

         includedirs { "./compat" }

         defines { 'NOMINMAX', '_LARGEFILE_SOURCE', '_FILE_OFFSET_BITS=64' }

      -- Linux specific
      configuration {"linux", "gmake"}
         defines { '__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' } -- c99

         links { "dl" }

      configuration "Debug"
         defines { "DEBUG" } -- -DDEBUG
         symbols "On"
         targetname "oclc_d"

      configuration "Release"
         -- defines { "NDEBUG" } -- -NDEBUG
         symbols "On"
         targetname "oclc"
