#!/bin/bash

#Retrieving installation path
CURR=`pwd`
cd `dirname "$0"`
INSTALL_PATH=`pwd`
cd "$CURR"
#The gui jar path
JAR_FILE="$INSTALL_PATH/lib/p-vcd_gui.jar"

if [[ ! -f "$JAR_FILE" ]]; then
	echo "Cannot find all the required resources to run P-VCD."
	echo "Searched path: $JAR_FILE"
	echo "Please re-install the program."
	exit 1
fi

JAVA_BIN="java"
if [[ "$JAVA_HOME" != "" ]]; then
	JAVA_BIN="$JAVA_HOME/bin/java"
fi

BITS=""
"$JAVA_BIN" -d64 -version > /dev/null 2>&1
if [[ $? -eq 0 ]]; then
	BITS="64"
else
	"$JAVA_BIN" -d32 -version > /dev/null 2>&1
	if [[ $? -eq 0 ]]; then
		BITS="32"
	else
		echo "Cannot find a valid JRE 32 bit nor 64 bits on this machine."
		echo "Please install the latest JRE."
		exit 1
	fi
fi

"$JAVA_BIN" -jar "$JAR_FILE" "$BITS" "$INSTALL_PATH"
if [[ $? -ne 0 ]]; then
	echo An error occurred at starting P-VCD.
	exit 1
fi
