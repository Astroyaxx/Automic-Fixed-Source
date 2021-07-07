@ECHO OFF
SET PATH=%PATH%;C:\Program Files\7-Zip;C:\android-ndk-r16b;
CALL COMPILE.BAT
SET PATH=%PATH%;C:\Program Files\7-Zip;C:\android-ndk-r16b;C:\platform-tools
ECHO Copying library...
REM Copy the so file from libs/armeabi-v7a to gt's lib dir
COPY libs\armeabi-v7a\libinfernal.so gt\lib\armeabi-v7a\libnightfarm.so
