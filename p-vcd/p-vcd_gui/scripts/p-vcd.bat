@echo off
set "INI_FILE=%cd%\p-vcd.ini"
set "JAR_FILE=%cd%\lib\p-vcd_gui.jar"

FOR %%F IN ("%INI_FILE%" "%JAR_FILE%") DO (
	if not exist %%F goto errReq
)

set JAVA_BIN=java.exe
set JAVAW_BIN=javaw.exe
if "%JAVA_HOME%" NEQ "" goto setJavaHome
goto testBitness

:setJavaHome
set "JAVA_BIN=%JAVA_HOME%\bin\java.exe"
set "JAVAW_BIN=%JAVA_HOME%\bin\javaw.exe"

FOR %%F IN ("%JAVA_BIN%" "%JAVAW_BIN%") DO (
	if not exist %%F goto errJava
)

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
start "P-VCD" "%JAVAW_BIN%" -jar "%JAR_FILE%" "%BITS%" "%INI_FILE%"
goto endOk

:errJava
echo Cannot find a valid JRE 32 bit nor 64 bits on this machine.
echo Please go to http://www.java.com/ and install the latest version.
goto endError

:errReq
echo Cannot find all the required resources to run P-VCD.
echo Please re-install the program.
goto endError

:errStart
echo An error occurred at starting P-VCD.
goto endError

:endError
pause

:endOk
