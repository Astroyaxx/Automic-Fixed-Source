@ECHO OFF

SET PATH=%PATH%;C:\Program Files\7-Zip;C:\android-ndk-r16b;C:\platform-tools
ECHO Copying library...
REM Copy the so file from libs/armeabi-v7a to gt's lib dir
COPY libs\armeabi-v7a\libnightfarm.so gt\lib\armeabi-v7a\libnightfarm.so

ECHO Packaging the library...
REM delete the original apk
DEL growtopia.apk

REM package the apk itself
CD gt
7Z a -tzip growtopia.apk *
CD ..
MOVE gt\growtopia.apk growtopia.apk

ECHO Signing the apk...
REM sign the apk
java -jar apksigner.jar sign --min-sdk-version 20 --key apkeasytool.pk8 --cert apkeasytool.pem  --v4-signing-enabled false growtopia.apk