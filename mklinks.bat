@echo off
rem
rem  Create symbolic links for included projects and output directories.
rem  Environment variable DEV_ROOT points to the root of development tree.
rem
rem  This script should be run as administrator
rem

pushd "%~dp0"
mklink /d lib %DEV_ROOT%\lib

cd include
mklink /d utpp %DEV_ROOT%\utpp\include\utpp
popd
