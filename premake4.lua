-- OpenCL path
newoption {
  trigger     = "opencl-path",
  value       = "PATH",
  description = "Path to OpenCL header and library."
}

-- OpenCL lib path
newoption {
  trigger     = "opencl-libpath",
  value       = "PATH",
  description = "Path to OpenCL library."
}

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
         "./"
      }

      defines { 'HAVE_OPENCL' }

      if _OPTIONS['opencl-path'] then
         includedirs { _OPTIONS['opencl-path'] .. "/include" }
         if _OPTIONS['opencl-libpath'] then
            libdirs { _OPTIONS['opencl-libpath'] }
	      else
            libdirs { _OPTIONS['opencl-path'] .. "/lib/x86_64" } 
         end
	   end

      -- MacOSX. Guess we use gcc.
      configuration { "macosx", "gmake" }

         defines { '_LARGEFILE_SOURCE', '_FILE_OFFSET_BITS=64' }

         links { "OpenCL.framework" }

      -- Windows specific
      configuration { "windows", "gmake" }

         defines { '__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' } -- c99

         links { "stdc++", "msvcrt", "ws2_32", "winmm" }

      configuration { "windows", "vs2013", "x64" }

         includedirs { "./compat" }

         defines { 'NOMINMAX', '_LARGEFILE_SOURCE', '_FILE_OFFSET_BITS=64' }
         links { "OpenCL" }


      -- Linux specific
      configuration {"linux", "gmake"}
         defines { '__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' } -- c99

         links { "OpenCL" }

      configuration "Debug"
         defines { "DEBUG" } -- -DDEBUG
         flags { "Symbols" }
         targetname "oclc_d"

      configuration "Release"
         -- defines { "NDEBUG" } -- -NDEBUG
         flags { "Symbols" }
         targetname "oclc"
