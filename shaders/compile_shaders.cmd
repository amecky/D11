@echo off
rem THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
rem ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
rem THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
rem PARTICULAR PURPOSE.
rem
rem Copyright (c) Microsoft Corporation. All rights reserved.

setlocal
set error=0

set FX_PATH=C:\devtools\DirectX\Utilities\bin\x86

call :CompileShader%1 Sprite Sprite vs VS_Main
call :CompileShader%1 Sprite Sprite ps PS_Main
call :CompileShader%1 Sprite Sprite gs GS_Main

rem GrayFade effect
call :CompileShader%1 postprocess\GrayFade GrayFade vs VS_Main
call :CompileShader%1 postprocess\GrayFade GrayFade ps PS_Main

rem ScreenShake effect
call :CompileShader%1 postprocess\ScreenShake ScreenShake vs VS_Main
call :CompileShader%1 postprocess\ScreenShake ScreenShake ps PS_Main

echo.

if %error% == 0 (
    echo Shaders compiled ok
) else (
    echo There were shader compilation errors!
)

endlocal
exit /b

:CompileShader
set fxc=%FX_PATH%\fxc /nologo %1.fx /T%3_5_0 /Zi /Zpc /Qstrip_reflect /Qstrip_debug /E%4 /Fh%1_%4.inc /Vn%2_%4
echo.
echo %fxc%
%fxc% || set error=1
exit /b
