@echo off

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

SET includes=/Isrc /I%VULKAN_SDK%/include
SET links=/link /LIBPATH:%VULKAN_SDK%/lib vulkan-1.lib user32.lib
SET defines=/D DEBUG /D WINDOWS_BUILD

cl /EHsc /Z7 %includes% %defines% src/platform/win32_platform.cpp %links%