newoption {
	trigger = "link",
	value = "type",
	description = "Choose a linking result",
	allowed = {
		{ "shared",  "Shared library (.dll, .so)" },
		{ "static",  "Static library (.lib, .a)" },
	},
	default = "shared"
}

local localBinLocation = "%{wks.location}/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}"
local localBinIntLocation = "%{wks.location}/bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}"

if (outputBinDirectory ~= nil) then localBinLocation = outputBinDirectory .. "/%{prj.name}" end
if (outputInterDirectory ~= nil) then localBinIntLocation = outputInterDirectory .. "/%{prj.name}" end

project "Core"
    language "C++"
    cppdialect "C++23"
    allmodulespublic "off"
    scanformoduledependencies "off"
    exceptionhandling "off"
    multiprocessorcompile "on"

    targetdir (localBinLocation)
    objdir    (localBinIntLocation)

    characterset "Unicode"
            
    files
    {
        -- Include
        "Include/**.h",
        "Include/**.hpp",
        "Include/**.cppm",
        "Include/**.inl",
        
        -- Source
        "Source/**.h",
        "Source/**.hpp",
        "Source/**.inl",
        "Source/**.cpp",
        "Source/**.cppm"
    }

    includedirs 
    {
        "%{wks.location}/Dependencies/spdlog/include",
        "%{wks.location}/Dependencies/fmt/include",
        "%{wks.location}/Dependencies/xxHash",
        "%{wks.location}/Include",
        "%{wks.location}/Source",
    }

    links { "xxHash" }

    -- System configurations
    filter "system:windows"
        systemversion "latest"
        links {}
    -- Compiler configurations
    filter "action:vs*"
        buildoptions { "/utf-8" }
    filter "action:clang"
        --buildoptions { "-fmodules-ts", "-fimplicit-modules", "-fimplicit-module-maps" }

    -- Option configurations
    filter { "options:link=shared" }
        kind "SharedLib"
        staticruntime "off"
        defines 
        {
           "SPDLOG_BUILD_SHARED",
           "MOON_CORE_LIBRARY_BUILD", 
           "_WINDLL",
        }

    filter { "options:link=static" }
        kind "StaticLib"
        staticruntime "on"
        defines { "MOON_CORE_STATIC_LIB" }
        
    -- Files configurations
    filter "files:**.cppm"
        compileas "Module"
    filter "files:**.cpp"
        compileas "C++"

    -- Build configurations
    filter "configurations:Debug"
        defines "MC_DEBUG"
        symbols "on"
        runtime "Debug"
    filter "configurations:Release"
        defines  "MC_RELEASE"
        optimize "on"
        runtime  "Release"

    group "Dependencies"
        include "Dependencies/xxHash"