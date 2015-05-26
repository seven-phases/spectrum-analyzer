@echo off
cd /d %~dp0

cd ..\..

rmdir /q /s src\obj
rmdir /q /s release\x86
rmdir /q /s release\x64
