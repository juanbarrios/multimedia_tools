#!/bin/bash

#####################################################################
#
# This script compiles and install all the projects.
#
# It requires a GNU's compilation environment.
# Configuration for dependencies paths must be modified below.
# 
# Run the script with no parameters to print help:
#   bash compileInstallAll.sh
# 
#
# WINDOWS
#    Requires a MinGW compiler and MSYS tools (including commands: make, find).
#    Usage example:
#        ./compileInstallAll.sh release win64 'c:/multimedia_tools/'
#
# LINUX
#    Requires GNU compiler and build-tools.
#    Usage example:
#        ./compileInstallAll.sh release linux64 /opt/multimedia_tools
#
#####################################################################

#Configure dependencies and compile options
function setExternalDependencies {
	#Configure warnings
	CFLAGS+=" -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-missing-field-initializers"
	#Options used for a release
	#CFLAGS+=" -pedantic -Werror"

	#Configure dependency with OPENCV. See http://opencv.org/
	#Option 1: set the NO_OPENCV flag, which disables all methods requiring OPENCV.
	#CFLAGS+=" -DNO_OPENCV"
	#Option 2: Use the pkg-config tool (if it is available).
	CFLAGS+=" "`pkg-config --cflags opencv`
	LDFLAGS+=" "`pkg-config --libs opencv`
	#Option 3: Manually set the paths to headers and libraries.
	#CFLAGS+=" -I/usr/opencv/include"
	#LDFLAGS+=" -L/usr/opencv/lib -lopencv_core249 -lopencv_highgui249 -lopencv_imgproc249 -lopencv_contrib249 -lopencv_features2d249 -lopencv_nonfree249"

	#Configure dependency with FLANN. See http://www.cs.ubc.ca/research/flann/
	#Option 1: set the NO_FLANN flag, which disables all methods requiring FLANN.
	#CFLAGS+=" -DNO_FLANN"
	#Option 2: Use the pkg-config tool (if it is available).
	CFLAGS+=" "`pkg-config --cflags flann`
	LDFLAGS+=" "`pkg-config --libs flann`
	#Option 3: Manually set the paths to headers and libraries.
	#CFLAGS+=" -I/usr/flann/include"
	#LDFLAGS+=" -L/usr/flann/lib -lflann -lflann_cpp"

	#Configure dependency with VLFEAT. See http://www.vlfeat.org/
	#Option 1: set the NO_VLFEAT flag, which disables all methods requiring VLFEAT.
	#CFLAGS+=" -DNO_VLFEAT"
	#Option 2: Use the pkg-config tool (if it is available).
	CFLAGS+=" "`pkg-config --cflags vlfeat`
	LDFLAGS+=" "`pkg-config --libs vlfeat`
	#Option 3: Manually set the paths to headers and libraries.
	#CFLAGS+=" -I/usr/vlfeat/include"
	#LDFLAGS+=" -L/usr/vlfeat/lib -lvl"

	#Options for the make tool. -j compiles sources in parallel threads.
	MAKE_OPTIONS="-j 8"
}

function printHelp {
	echo "Use:"
	echo "  $0 [ACTION] [OS] [INSTALL_PATH] [VERSION_NAME] [SOURCE_PATH]"
	echo ""
	echo "    ACTION        Mandatory. Possible values: release, debug, doc, clean."
	echo "                      release = compiles sources with optimization flags and install binaries to [INSTALL_PATH]."
	echo "                      debug   = compiles sources with debug flags and install binaries to [INSTALL_PATH]."
	echo "                      doc     = generates the documentation for all projects."
	echo "                      clean   = deletes all generated files."
	echo "    OS            Possible values: win32, win64, linux32, linux64. default=auto select."
	echo "    INSTALL_PATH  The path to install the compiled binaries. default=install_[OS]."
	echo "    VERSION_NAME  The name of the generated compilation. default=current time."
	echo "    SOURCE_PATH   The path where sources are located. default=current folder."
}
function testAction {
	if [[ "$ACTION" != "release" && "$ACTION" != "debug" && "$ACTION" != "doc" && "$ACTION" != "clean" ]]; then
		printHelp
		echo ""
		echo "Please enter a valid action: release, debug, doc, clean."
		exit 1
	fi
}
function testOS {
	if [[ "$OS" == "" || "$OS" == "auto" ]]; then
		VAL=`g++ -dumpmachine`
		if [[ "$VAL" == "i686-w64-mingw32" ]]; then
			OS="win32"
		elif [[ "$VAL" == "x86_64-w64-mingw32" ]]; then
			OS="win64"
		elif [[ "$VAL" == "i686-linux-gnu" ]]; then
			OS="linux32"
		elif [[ "$VAL" == "x86_64-linux-gnu" ]]; then
			OS="linux64"
		else
			echo "OS auto-detection failed!"
			echo "Please enter a valid OS: win32, win64, linux32, linux64."
			exit 1
		fi
	fi
	if [[ "$OS" != "win32" && "$OS" != "win64" && "$OS" != "linux32" && "$OS" != "linux64" ]]; then
		printHelp
		echo ""
		echo "Please enter a valid OS: win32, win64, linux32, linux64."
		exit 1
	fi
}
function testSourcePath {
	#default source folder is current folder
	if [[ "$SOURCE_PATH" == "" ]]; then
		SOURCE_PATH="$PWD"
	fi
	#test the existence of projects folders
	if [[ ! -d "$SOURCE_PATH/myutils/myutils_lib"
		  || ! -d "$SOURCE_PATH/myutils/myutilsimage_lib"
		  || ! -d "$SOURCE_PATH/metricknn/metricknn_lib"
		  || ! -d "$SOURCE_PATH/p-vcd/p-vcd_lib" ]]; then
		echo "Path $SOURCE_PATH does not contain source for projects myutils, metricknn and p-vcd."
		exit 1
	fi
}
function testInstallPath {
	#default install path
	if [[ "$INSTALL_PATH" == "" ]]; then
		INSTALL_PATH="multimedia_tools_$OS"
	fi
	#convert relative to absolute path
	if [[ "${INSTALL_PATH:0:1}" != "/" ]]; then
		INSTALL_PATH="$PWD/$INSTALL_PATH"
	fi
}
function testVersion {
	#default version name
	if [[ "$VERSION_NAME" == "" ]]; then
		DATE=`date +%Y%m%d_%H%M%S`
		VERSION_NAME="${ACTION}_${OS}_${DATE}"
	fi
}
function updateCompileFlags {
	if [[ "$OS" == "win32" || "$OS" == "win64" ]]; then
		#the warnings for printf formats do not work well on windows
		CFLAGS+=" -Wno-format"
		SUFFIX_LIB=".dll"
		SUFFIX_EXE=".exe"
	elif [[ "$OS" == "linux32" || "$OS" == "linux64" ]]; then
		CFLAGS+=" -fPIC"
		SUFFIX_LIB=".so"
		SUFFIX_EXE=""
	fi

	if [[ "$OS" == "win32" || "$OS" == "linux32" ]]; then
		CFLAGS+=" -m32"
	elif [[ "$OS" == "win64" || "$OS" == "linux64" ]]; then
		CFLAGS+=" -m64"
	fi

	if [[ "$ACTION" == "debug" ]]; then
		#debug options
		CFLAGS+=" -O0 -ggdb"
	elif [[ "$ACTION" == "release" ]]; then
		#release options
		if [[ "$OS" == "win32" || "$OS" == "win64" ]]; then
			CFLAGS+=" -O2"
		elif [[ "$OS" == "linux32" || "$OS" == "linux64" ]]; then
			CFLAGS+=" -O3"
		fi
	fi


	CFLAGS+=" -I$INSTALL_PATH/include"
	if [[ -d "$INSTALL_PATH/lib" ]]; then
		LDFLAGS+=" -L$INSTALL_PATH/lib"
	fi
	if [[ -f "$INSTALL_PATH/lib/libmyutils$SUFFIX_LIB" ]]; then
		LDFLAGS+=" -lmyutils"
	fi
	if [[ -f "$INSTALL_PATH/lib/libmetricknn$SUFFIX_LIB" ]]; then
		LDFLAGS+=" -lmetricknn"
	fi
	if [[ -f "$INSTALL_PATH/lib/libmyutilsimage$SUFFIX_LIB" ]]; then
		LDFLAGS+=" -lmyutilsimage"
	fi
	if [[ -f "$INSTALL_PATH/lib/libpvcd$SUFFIX_LIB" ]]; then
		LDFLAGS+=" -lpvcd"
	fi
	LDFLAGS+=" -lpthread -lm"
	setExternalDependencies
}

function compileProject {
	local DIR="$1"
	if [[ ! -d "$DIR" ]]; then return; fi
	local OLD_CFLAGS="$CFLAGS"
	local OLD_LDFLAGS="$LDFLAGS"
	updateCompileFlags
	export CFLAGS
	export LDFLAGS
	export SUFFIX_LIB
	export SUFFIX_EXE
	echo \
	make $MAKE_OPTIONS -f Makefile -C "${SOURCE_PATH}/$DIR" "BUILD_DIR=${ACTION}_${OS}" "VERSION_NAME=${VERSION_NAME}" all && \
	make $MAKE_OPTIONS -f Makefile -C "${SOURCE_PATH}/$DIR" "BUILD_DIR=${ACTION}_${OS}" "VERSION_NAME=${VERSION_NAME}" all
	if [[ $? -ne 0 ]]; then exit 1; fi
	echo \
	make $MAKE_OPTIONS -f Makefile -C "${SOURCE_PATH}/$DIR" "BUILD_DIR=${ACTION}_${OS}" "VERSION_NAME=${VERSION_NAME}" "INSTALL_DIR=${INSTALL_PATH}" install && \
	make $MAKE_OPTIONS -f Makefile -C "${SOURCE_PATH}/$DIR" "BUILD_DIR=${ACTION}_${OS}" "VERSION_NAME=${VERSION_NAME}" "INSTALL_DIR=${INSTALL_PATH}" install
	if [[ $? -ne 0 ]]; then exit 1; fi
	CFLAGS="$OLD_CFLAGS"
	LDFLAGS="$OLD_LDFLAGS"
	export CFLAGS
	export LDFLAGS
}
function docProject {
	local DIR="$1"
	if [[ ! -d "$DIR" ]]; then return; fi
	echo \
	make $MAKE_OPTIONS -f Makefile -C "${SOURCE_PATH}/$DIR" "DOC_DIR=generated_doc" doc && \
	make $MAKE_OPTIONS -f Makefile -C "${SOURCE_PATH}/$DIR" "DOC_DIR=generated_doc" doc
	if [[ $? -ne 0 ]]; then exit 1; fi
}
function cleanProject {
	local DIR="$1"
	if [[ ! -d "$DIR" ]]; then return; fi
	make $MAKE_OPTIONS -f Makefile -C "${SOURCE_PATH}/$DIR" "BUILD_DIR=release_${OS}" clean
	if [[ $? -ne 0 ]]; then exit 1; fi
	make $MAKE_OPTIONS -f Makefile -C "${SOURCE_PATH}/$DIR" "BUILD_DIR=debug_${OS}" clean
	if [[ $? -ne 0 ]]; then exit 1; fi
}

#Read command line parameters
ACTION="$1"
OS="$2"
INSTALL_PATH="$3"
SOURCE_PATH="$4"
VERSION_NAME="$5"

testAction
testOS
testSourcePath
testInstallPath
testVersion

if [[ "$ACTION" == "debug" || "$ACTION" == "release" ]]; then
	compileProject "myutils/myutils_lib"
	compileProject "metricknn/metricknn_lib"
	compileProject "metricknn/metricknn_cli"
	compileProject "myutils/myutilsimage_lib"
	compileProject "myutils/myutilsimage_cli"
	compileProject "p-vcd/p-vcd_lib"
	compileProject "p-vcd/p-vcd_cli"
	#on windows copy all the generaed dll to the exe folder
	if [[ "$OS" == "win32" || "$OS" == "win64" ]]; then
		install "${INSTALL_PATH}/lib/"*"${SUFFIX_LIB}" "${INSTALL_PATH}/bin/"
	fi
elif [[ "$ACTION" == "doc" ]]; then
	docProject "myutils/myutils_lib"
	docProject "myutils/myutilsimage_lib"
	docProject "metricknn/metricknn_lib"
	docProject "metricknn/metricknn_cli"
elif [[ "$ACTION" == "clean" ]]; then
	cleanProject "myutils/myutils_lib"
	cleanProject "metricknn/metricknn_lib"
	cleanProject "metricknn/metricknn_cli"
	cleanProject "myutils/myutilsimage_lib"
	cleanProject "myutils/myutilsimage_cli"
	cleanProject "p-vcd/p-vcd_lib"
	cleanProject "p-vcd/p-vcd_cli"
fi
