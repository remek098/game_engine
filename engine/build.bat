@REM REM Building script for engine
@REM @ECHO OFF
@REM SetLocal EnableDelayedExpansion

@REM REM Get list of all the .cpp files
@REM SET cppFileNames=
@REM FOR /R %%f in (*.cpp) do (
@REM     SET cppFileNames=!cppFileNames! %%f
@REM )

@REM REM echo "Files:" %cFileNames%

@REM SET assembly=engine
@REM SET compilerFlags=-g -shared -Wvarargs -Wall -Werror -std=c++20
@REM REM -Wall -Werror
@REM SET includeFlags=-Isrc -I%VULKAN_SDK%/Include
@REM SET linkerFlags=-luser32 -lvulkan-1 -L%VULKAN_SDK%/Lib -lkernel32 -Xlinker /NODEFAULTLIB:libcmt msvcrtd.lib
@REM SET defines=-D_DEBUG -DKEXPORT -D_CRT_SECURE_NO_WARNINGS

@REM ECHO "Building %assembly%%..."
@REM clang++ %cppFileNames% %compilerFlags% -o ../bin/%assembly%.dll %defines% %includeFlags% %linkerFlags%

@REM @ECHO OFF
@REM SetLocal EnableDelayedExpansion

@REM REM Build settings
@REM SET assembly=engine
@REM SET compilerFlags=-std=c++20 -g -Wall -Werror -shared -D_DEBUG -DKEXPORT -D_CRT_SECURE_NO_WARNINGS
@REM SET includeFlags=-Isrc -I%VULKAN_SDK%/Include
@REM SET linkerFlags=-luser32 -lkernel32 -lvulkan-1 -L%VULKAN_SDK%/Lib ^
@REM     -Xlinker /NODEFAULTLIB:libcmt ^
@REM     -Xlinker /DEFAULTLIB:msvcrtd

@REM REM Collect .cpp files
@REM SET cppFileNames=
@REM FOR /R %%f in (*.cpp) do (
@REM     SET cppFileNames=!cppFileNames! "%%f"
@REM )

@REM ECHO Building %assembly%...
@REM clang++ %compilerFlags% %cppFileNames% -o ../bin/%assembly%.dll %includeFlags% %linkerFlags%


@REM @ECHO OFF
@REM SetLocal EnableDelayedExpansion

@REM REM Build settings
@REM SET assembly=engine

@REM REM Replace -D_DLL with -D_MT
@REM SET compilerFlags=-std=c++20 -g -Wall -Werror -shared -D_DEBUG -D_MT -DKEXPORT -D_CRT_SECURE_NO_WARNINGS
@REM SET includeFlags=-Isrc -I%VULKAN_SDK%/Include

@REM REM msvcrtd.lib = Debug DLL CRT
@REM SET linkerFlags=-luser32 -lkernel32 -lvulkan-1 -L%VULKAN_SDK%/Lib ^
@REM     -Xlinker /NODEFAULTLIB:msvcrtd ^
@REM     -Xlinker /DEFAULTLIB:libcmtd

@REM REM Collect .cpp files
@REM SET cppFileNames=
@REM FOR /R %%f in (*.cpp) do (
@REM     SET cppFileNames=!cppFileNames! "%%f"
@REM )

@REM ECHO Building %assembly%...
@REM clang++ %compilerFlags% %cppFileNames% -o ../bin/%assembly%.dll %includeFlags% %linkerFlags%

@ECHO OFF
SetLocal EnableDelayedExpansion

SET assembly=engine
SET compilerFlags=-std=c++20 -g -Wall -Werror -shared -D_DEBUG -D_DLL -DKEXPORT -D_CRT_SECURE_NO_WARNINGS
SET includeFlags=-Isrc -I%VULKAN_SDK%/Include

SET linkerFlags=-luser32 -lkernel32 -lvulkan-1 -L%VULKAN_SDK%/Lib ^
    -Xlinker /NODEFAULTLIB:libcmtd ^
    -Xlinker /NODEFAULTLIB:libcmt ^
    -Xlinker /NODEFAULTLIB:msvcrt ^
    -Xlinker /DEFAULTLIB:msvcrtd ^
    -Xlinker /DEFAULTLIB:ucrtd ^
    -Xlinker /DEFAULTLIB:vcruntimed

REM Collect .cpp files
SET cppFileNames=
FOR /R %%f in (*.cpp) do (
    SET cppFileNames=!cppFileNames! "%%f"
)

ECHO Building %assembly%...
clang++ %compilerFlags% %cppFileNames% -o ../bin/%assembly%.dll %includeFlags% %linkerFlags%