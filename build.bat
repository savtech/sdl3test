@echo off
cls

:: Edit this path to point to your vcvars script
set "__local_vcvars_path=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

if not defined VSCMD_ARG_TGT_ARCH (
    if exist "%__local_vcvars_path%" (
        call "%__local_vcvars_path%"
    ) else (
        echo Unable to locate the specified vcvars script: %__local_vcvars_path%
        exit /b 1
    )
)

setlocal ENABLEDELAYEDEXPANSION

set "executable_name=s.exe"
set "compiler_options=/EHsc /nologo /W4 /external:W0 /MP /MTd /std:c++20"
set "debug_options=/Od /Zi"
set "release_options=/O2"
set "includes=/IC:\Dev\_libraries\sdl3\include /IC:\Dev\_libraries\sdl\include /IC:\Dev\_libraries\orshlib\include /external:IC:\Dev\_libraries\stbimage"
set build_result=0

:: Regular build path
echo Building !executable_name!...

set "build_options="
if "%~1"=="-r" (
    echo Release configuration
	set "build_options=!release_options!"
) else (
    echo Debug configuration
	set "build_options=!debug_options!"
)

echo [!compiler_options! !build_options!]
cl !compiler_options! !build_options! !includes! src\main.cpp /Fe!executable_name! /link C:\Dev\_libraries\sdl3\lib\x64\SDL3.lib

if !ERRORLEVEL! neq 0 (
	set build_result=1
)

:end

if !build_result! equ 0 (
    echo Build successful.
) else (
    echo Build failed.
)

endlocal