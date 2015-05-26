@echo off

rem ...........................................................
rem enviroment:

call %WDKROOT%\bin\setenv.bat %WDKROOT% no_oacr %*

set BUILD_ALT_DIR=
set OBJDIR=obj\amd64
if %_BUILDARCH%==x86 set OBJDIR=obj\i386

rem ...........................................................
rem build:

cd /d %SRC%
if not exist %OBJDIR% md %OBJDIR%
build -c -w -jpath %OBJDIR%
set > %OBJDIR%\env.log
