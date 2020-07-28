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
if not exist %DEV_ROOT%\lib mkdir %DEV_ROOT%\lib
if not exist lib\nul mklink /d lib %DEV_ROOT%\lib

pushd "%~dp0include"
if not exist utpp\nul (
    call :getrepo utpp git@github.com:neacsum/utpp.git
    mklink /d utpp %DEV_ROOT%\utpp\include\utpp
  )
if not exist utf8\nul (
    call :getrepo utf8 git@github.com:neacsum/utf8.git
    mklink /d utf8 %DEV_ROOT%\utf8\include\utf8
  )
popd
goto :EOF

rem
rem Clone or pull a linked repo
rem Usage:
rem   call :getrepo <folder> <git repo>
rem
rem Folder is relative to DEV_ROOT
rem
:getrepo
pushd %DEV_ROOT%
if not exist %1\nul (
    git clone %2 %1
    cd %1
    goto :repodone
  )
rem meake sure linked repo is up to date
cd %1
git pull

:repodone
popd
goto :EOF
