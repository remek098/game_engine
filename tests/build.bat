REM Build script for tests
@ECHO OFF
SetLocal EnableDelayedExpansion

REM Get a list of all .cpp files
SET cppFileNames=
FOR /R %%f in (*.cpp) do (
    SET cppFileNames=!cppFileNames! "%%f"
)

REM echo "Files: " %cppFileNames%

SET assembly=tests
SET compilerFlags=-g -Wno-missing-braces -Wno-vla-cxx-extension
REM -Wall -Werror -save-temps=obj -O0
SET includeFlags=-Isrc -I../engine/src/
SET linkerFlags=-L../bin/ -lengine.lib
SET defines=-D_DEBUG -DKIMPORT

ECHO "Building %assembly%%..."
clang++ %cppFileNames% %compilerFlags% -o ../bin/%assembly%.exe %defines% %includeFlags% %linkerFlags%