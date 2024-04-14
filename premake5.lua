-- premake5.lua
workspace "FaRayTracingApp"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "FaRayTracingApp"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/WalnutExternal.lua"

include "FaRayTracingApp"