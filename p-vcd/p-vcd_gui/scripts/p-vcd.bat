@echo off

rem Retrieving installation path
set "INSTALL_PATH=%~dp0"
rem The gui jar path
set "JAR_FILE=%INSTALL_PATH%\lib\p-vcd_gui.jar"

FOR %%F IN ("%JAR_FILE%") DO (
	if not exist %%F goto errReq
)

set JAVA_BIN=java.exe
set JAVAW_BIN=javaw.exe
if "%JAVA_HOME%" NEQ "" goto setJavaHome
goto testBitness

:setJavaHome
set "JAVA_BIN=%JAVA_HOME%\bin\java.exe"
set "JAVAW_BIN=%JAVA_HOME%\bin\javaw.exe"

:testBitness
"%JAVA_BIN%" -d64 -version > NUL 2>&1
IF %ERRORLEVEL% EQU 0 goto runBit64

"%JAVA_BIN%" -d32 -version > NUL 2>&1
IF %ERRORLEVEL% EQU 0 goto runBit32

goto errJava

:runBit32
set BITS=32
goto run

:runBit64
set BITS=64
goto run

:run
start "P-VCD" "%JAVAW_BIN%" -jar "%JAR_FILE%" "%BITS%" "%INSTALL_PATH%"
goto endOk

:errJava
echo Cannot find a valid JRE 32 bit nor 64 bits on this machine.
echo Please go to http://www.java.com/ and install the latest JRE.
goto endError

:errReq
echo Cannot find all the required resources to run P-VCD.
echo Searched path: %JAR_FILE%
echo Please re-install the program.
goto endError

:endError
pause

:endOk
