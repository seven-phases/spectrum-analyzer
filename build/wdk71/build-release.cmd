@echo off

set THIS=%~dp0
set PRJ=%THIS:build\wdk71\=%

set SRC=%PRJ%src
set DST=%PRJ%release
set DST32=%DST%\x86
set DST64=%DST%\x64

set NAME=Spectrum Analyzer
set NAME_SHORT=spha

rem ...........................................................

if exist paths.cmd (
    call paths.cmd
) else (
    echo Error: 'paths.cmd' not found, creating one from template:
    copy paths-template.cmd paths.cmd
)

if not defined WDKROOT (
    echo Error: WDKROOT is not set. Specify WDK path in 'paths.cmd'
    goto :exit
)

if not defined VSTSDKROOT (
    echo Error: VSTSDKROOT is not set. Specify VST3 SDK path in 'paths.cmd'
    goto :exit
)

set PATH=%SystemRoot%\System32

echo ..........................................................
cmd /c target_ fre x86 wxp
echo.
echo ..........................................................
cmd /c target_ fre x64 win7
echo.

echo ..........................................................
echo copying binaries:

if not exist %DST32% md %DST32%
if not exist %DST64% md %DST64%
if exist "%DST32%\%NAME%.dll" del "%DST32%\%NAME%.dll"
if exist "%DST64%\%NAME%.dll" del "%DST64%\%NAME%.dll"
echo on
@copy %SRC%\obj\i386\%NAME_SHORT%.dll  "%DST32%\%NAME%.dll"
@copy %SRC%\obj\amd64\%NAME_SHORT%.dll "%DST64%\%NAME%.dll"
@echo off

echo done.

rem ...........................................................
:exit
