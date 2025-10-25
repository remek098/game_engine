@REM REM Building script for testbed
@REM @ECHO OFF
@REM SetLocal EnableDelayedExpansion

@REM REM Get list of all the .cpp files
@REM SET cppFileNames=
@REM FOR /R %%f in (*.cpp) do (
@REM     SET cppFileNames=!cppFileNames! %%f
@REM )

@REM REM echo "Files:" %cFileNames%

@REM SET assembly=testbed
@REM SET compilerFlags=-g -std=c++20
@REM REM -Wall -Werror
@REM SET includeFlags=-Isrc -I../engine/src
@REM SET linkerFlags=-L../bin/ -lengine.lib
@REM SET defines=-D_DEBUG -DKEXPORT

@REM ECHO "Building %assembly%%..."
@REM clang %cppFileNames% %compilerFlags% -o ../bin/%assembly%.exe %defines% %includeFlags% %linkerFlags%

@ECHO OFF
SetLocal EnableDelayedExpansion

SET assembly=testbed
SET compilerFlags=-std=c++20 -g -Wall -Werror -D_DEBUG -D_DLL -DKEXPORT -Wno-vla-cxx-extension
SET includeFlags=-Isrc -I../engine/src

SET linkerFlags=-L../bin/ -lengine.lib ^
    -Xlinker /NODEFAULTLIB:libcmtd ^
    -Xlinker /NODEFAULTLIB:libcmt ^
    -Xlinker /NODEFAULTLIB:msvcrt ^
    -Xlinker /DEFAULTLIB:msvcrtd ^
    -Xlinker /DEFAULTLIB:ucrtd ^
    -Xlinker /DEFAULTLIB:vcruntimed ^
    -Xlinker /SUBSYSTEM:CONSOLE

REM Get list of all .cpp files
SET cppFileNames=
FOR /R %%f in (*.cpp) do (
    SET cppFileNames=!cppFileNames! "%%f"
)

ECHO Building %assembly%...
clang++ %compilerFlags% %cppFileNames% -o ../bin/%assembly%.exe %includeFlags% %linkerFlags%