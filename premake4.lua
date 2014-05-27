sources = {
   "muda_impl.h",
   "muda_device_ocl.cc",
   "OptionParser.cpp",
   "main.cc",
   }

-- premake4.lua
solution "OCLCSolution"
   configurations { "Release", "Debug" }

   if (os.is("windows")) then
      platforms { "native", "x32", "x64" }
   else
      platforms { "native", "x32", "x64" }
   end

   -- A project defines one build target
   project "OCLC"
      kind "ConsoleApp"
      language "C++"
      files { sources }

      includedirs {
         "./"
      }

      -- MacOSX. Guess we use gcc.
      configuration { "macosx", "gmake" }

         defines { '_LARGEFILE_SOURCE', '_FILE_OFFSET_BITS=64' }

         defines { "HAVE_OPENCL" }
         links { "OpenCL.framework" }

      -- Windows specific
      configuration { "windows", "gmake" }

         defines { '__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' } -- c99

         links { "stdc++", "msvcrt", "ws2_32", "winmm" }

      -- Windows specific
      configuration { "windows", "vs2008", "x64" }

         includedirs { "./compat" }

         defines { 'NOMINMAX', '_LARGEFILE_SOURCE', '_FILE_OFFSET_BITS=64' }


      configuration { "windows", "vs2008", "x32" }

      	includedirs { "./compat" }

         defines { 'NOMINMAX', '_LARGEFILE_SOURCE', '_FILE_OFFSET_BITS=64' }

      -- Linux specific
      configuration {"linux", "gmake"}
         defines { '__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' } -- c99

         defines { "HAVE_OPENCL" }
         links { "OpenCL" }

      configuration "Debug"
         defines { "DEBUG" } -- -DDEBUG
         flags { "Symbols" }
         targetname "oclc_d"

      configuration "Release"
         -- defines { "NDEBUG" } -- -NDEBUG
         flags { "Symbols", "Optimize" }
         targetname "oclc"
