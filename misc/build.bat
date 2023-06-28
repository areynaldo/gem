@echo off
if not exist build (mkdir build)
pushd build
set code_path=..\src\
:: GENERAL COMPILER FLAGS
set compiler=               -nologo &:: Supress startup banner 
set compiler=%compiler%     -Oi     &:: Use assembly intrinsics where possible
set compiler=%compiler%     -MT     &:: Include CRT library in the executable (static link)
set compiler=%compiler%     -Gm-    &:: Disable minimal rebuild
set compiler=%compiler%     -GR-    &:: Disable runtime type info (C++)
set compiler=%compiler%     -EHa-   &:: Disable exception handling (C++)
set compiler=%compiler%     -W4     &:: Display warnings up to level 4
set compiler=%compiler%     -WX     &:: Treat all warnings as errors
:: IGNORE WARNINGS
set compiler=%compiler%     -wd4201 &:: Nameless struct/union
set compiler=%compiler%     -wd4100 &:: Unusued function parameter
set compiler=%compiler%     -wd4189 &:: Local variable not referenced
:: DEBUG VARIABLES
set debug=                  -FC     &:: Produce the full path of the source code file
set debug=%debug%           -Z7     &:: Produce debug information 
:: WIN32 LINKER SWITCHES
set win32_link= -opt:ref            &:: Remove unused functions
:: CROSS_PLATFORM DEFINES
set defines=                -DGEM_INTERNAL=1
set defines=%defines%       -DGEM_SLOW=1

:: No optimizations (slow): -Od; all optimizations (fast): -O2
cl -Od %compiler% -DGEM_WIN32=1 %defines% %debug% -Fmgem.map %code_path%gem.c /link %win32_link%

popd