@echo off

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

SET includes=/Isrc /I%VULKAN_SDK%/include
SET links=/link /LIBPATH:%VULKAN_SDK%/lib vulkan-1.lib
SET defines=/D DEBUG

cl /EHsc %includes% %defines% src/main.cpp %links%