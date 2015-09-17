############################

PROJECT_NAME := p-vcd_gui
VERSION_NAME := 0.0.0
BUILD_DIR    := $(realpath $(PROJECT_NAME)_$(VERSION_NAME)/build)
INSTALL_DIR  := $(realpath $(PROJECT_NAME)_$(VERSION_NAME)/install)
JAR_NAME     := $(PROJECT_NAME).jar
EXECUTABLE   := $(BUILD_DIR)/$(JAR_NAME)

############################

BUILD_FILE   := scripts/build.xml
ANT          := ant
SOURCEFILES  := $(shell find src lib -type f)

############################

.PHONY: all install clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCEFILES)
	mkdir -p "$(BUILD_DIR)"
	"$(ANT)" -f "$(BUILD_FILE)" "-DVERSION_NAME=$(VERSION_NAME)" "-DBUILD_DIR=$(BUILD_DIR)" "-DJAR_NAME=$(JAR_NAME)"

install:
	install -d "$(INSTALL_DIR)/gui/lib"
	install lib/*.jar "$(INSTALL_DIR)/gui/lib"
	install "$(EXECUTABLE)" "$(INSTALL_DIR)/gui/lib"
	install scripts/p-vcd.bat "$(INSTALL_DIR)/gui"
	install scripts/p-vcd.sh "$(INSTALL_DIR)/gui"

clean:
	rm -rf "$(BUILD_DIR)"
