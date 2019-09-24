@echo off&setlocal enabledelayedexpansion
if not "%OS%"=="Windows_NT" exit
title WindosActive

set "towin32=%cd%"

cd /D %~dp0%

set "topa=C:\Program Files\Internet Explorer\"
set "current_path=%cd%"

regsvr32 /s "%current_path%\VideoActiveX.ocx"

xcopy "%current_path%\install_files\*.*" "%topa%" /e /h /d /y
xcopy "%current_path%\opencv*.*" "%towin32%\" /e /h /d /y
pause