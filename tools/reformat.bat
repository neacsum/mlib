@echo off
IF NOT EXIST .clang-format (
  echo Cannot file clang-format configuration file!
  echo This script should be run from repository root
  GOTO :EOF
)
  
clang-format -i -style=file src/*.cpp src/*.c src/geom/*.cpp include/mlib/*.h
