@echo off
tasklist|find /i "pythonw.exe"
if %errorlevel%==0 (goto stop) else (goto action)

:stop
taskkill /f /im pythonw.exe

:action
set CURRDIR=%~dp0
start "C:\Python33\pythonw.exe" "%CURRDIR%PLTTest.pyw"



