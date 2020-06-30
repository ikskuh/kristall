@ECHO OFF

REM This script builds Kristall for windows and creates a deployment

SET "DEPLOY=%CD%\kristall"
SET "ROOT=%CD%"
CALL "C:\Qt\5.15.0\mingw81_64\bin\qtenv2.bat"
CD /D "%ROOT%"

IF NOT EXIST build MKDIR build || GOTO ERR
CD build || GOTO ERR

ECHO Compiling...
qmake ..\..\src\kristall.pro || GOTO ERR
mingw32-make || GOTO ERR
cd ..

RMDIR /S /Q "%DEPLOY%" || GOTO ERR
MKDIR "%DEPLOY%" || GOTO ERR

ECHO Deployment...
COPY build\release\kristall.exe "%DEPLOY%\kristall.exe" || GOTO ERR
COPY "C:\Qt\Tools\OpenSSL\Win_x64\bin\libcrypto-1_1-x64.dll" "%DEPLOY%\libcrypto-1_1-x64.dll" || GOTO ERR
COPY "C:\Qt\Tools\OpenSSL\Win_x64\bin\libssl-1_1-x64.dll" "%DEPLOY%\libssl-1_1-x64.dll" || GOTO ERR

windeployqt %DEPLOY%\kristall.exe || GOTO ERR

CD "%ROOT% || GOTO ERR

ECHO Creating package...
DEL /Q kristall-nightly.zip || GOTO ERR
.\7z\7za.exe a kristall-nightly.zip %DEPLOY% || GOTO ERR

EXIT /B 0

:ERR
ECHO There was a error building kristall!
PAUSE
EXIT /B 1