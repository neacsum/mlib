@echo off
rem
rem  Create symbolic links for included projects and output directories.
rem  Environment variable DEV_ROOT points to the root of development tree.
rem
rem  This script should be run as administrator
rem

if defined DEV_ROOT goto MAKELINKS
echo Environment variable DEV_ROOT is not set!
echo Cannot create symlinks.
goto :EOF

:MAKELINKS
if exist lib\nul goto LIBDONE
mklink /d lib %DEV_ROOT%\lib

:LIBDONE
pushd "%~dp0include"
if exist utpp\nul goto UTPPDONE
mklink /d utpp %DEV_ROOT%\utpp\include\utpp

:UTPPDONE
if exist utf8\nul goto UTF8DONE
mklink /d utf8 %DEV_ROOT%\utf8\include\utf8

:UTF8DONE
popd
