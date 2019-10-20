workspace "routing"


    configurations
    {
        "Debug",
        "Release",
    }

project "routing"
    kind "ConsoleApp"
    language"C++"
    targetdir ("../bin/")
    objdir("../bin-init/")
    files{
        "../src/**.h",
        "../src/**.cpp",        
    }
    toolset("clang")
    filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"
      
  
   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
        
    flags { cppdialect "C++17" }
    
    configuration { "windows", "gmake" }
        buildoptions { "-fpermissive" }

    configuration { "linux", "gmake" }
        buildoptions { "-fpermissive","-Ofast" }
    configuration { "linux", "gmake" }
        
    configuration { "windows", "gmake" }
        buildoptions { "-fpermissive","-Ofast" }

    configuration{"windows","vs2019"}
        buildoptions{"/O2","/Ot"}

    configuration "linux"
        links {"pthread" }

