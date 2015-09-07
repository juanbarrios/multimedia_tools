############################
# Parameters:
#  BUILD_DIR     absolute path
#  DOC_DIR       absolute path
#  INSTALL_DIR   absolute path
#  VERSION_NAME  name of this compilation
#  SUFFIX_LIB    it must be defined as ".dll" on windows and as ".so" on linux 
#  SUFFIX_EXE    it must be defined as ".exe" on windows and as "" on linux 
############################

PROJECT_NAME := myutilsimage
VERSION_NAME := 0.0.0
BUILD_DIR    := $(realpath $(PROJECT_NAME)_$(VERSION_NAME)/build)
DOC_DIR      := $(realpath $(PROJECT_NAME)_$(VERSION_NAME)/doc)
INSTALL_DIR  := $(realpath $(PROJECT_NAME)_$(VERSION_NAME)/install)
EXECUTABLE   := $(BUILD_DIR)/lib$(PROJECT_NAME)$(SUFFIX_LIB)

##########

CFLAGS   += 
LDFLAGS  += 

##########

ifneq ($(shell find . -maxdepth 0),.)
    $(error command "find" is incorrect, please add GNU's find command to PATH.)
endif

SOURCE_DIR  := src
H_FILES     := $(shell find $(SOURCE_DIR) -type f -name \*.h)
C_FILES     := $(shell find $(SOURCE_DIR) -type f -name \*.c)
C_O_FILES   := $(addprefix $(BUILD_DIR)/c/,$(C_FILES:.c=.o))
HPP_FILES   := $(shell find $(SOURCE_DIR) -type f -name \*.hpp)
CPP_FILES   := $(shell find $(SOURCE_DIR) -type f -name \*.cpp)
CPP_O_FILES := $(addprefix $(BUILD_DIR)/cpp/,$(CPP_FILES:.cpp=.o))

##########

PKG_DIR           := $(INSTALL_DIR)/lib/pkgconfig
PKG_FILE          := $(PKG_DIR)/$(PROJECT_NAME).pc
PKG_LIBNAME       := $(PROJECT_NAME)
PKG_LIBDEPS       := -l$(PROJECT_NAME)
PKG_REQUIRES      :=
PKG_REQUIRES_PRIV := myutils opencv vlfeat

#### RULES ####

.PHONY: all install clean doc

all: $(EXECUTABLE)

$(EXECUTABLE): $(C_O_FILES) $(CPP_O_FILES)
	mkdir -p $(BUILD_DIR)
	g++ -shared -o $(EXECUTABLE) $(C_O_FILES) $(CPP_O_FILES) $(LDFLAGS)

$(BUILD_DIR)/c/%.o: %.c $(H_FILES)
	mkdir -p $(@D)
	gcc -std=gnu99 -c $(CFLAGS) -o $@ $<
	
$(BUILD_DIR)/cpp/%.o: %.cpp $(HPP_FILES) $(H_FILES)
	mkdir -p $(@D)
	g++ -std=gnu++11 -c $(CFLAGS) -o $@ $<

install:
	install -d $(INSTALL_DIR)/lib
	install $(EXECUTABLE) $(INSTALL_DIR)/lib
	install -d $(INSTALL_DIR)/include
	cd $(SOURCE_DIR) && find * -type f \( -name '*.h' -o -name '*.hpp' \) -exec install -D {} $(INSTALL_DIR)/include/{} \;
	install -d $(PKG_DIR)
	@echo 'prefix=$(INSTALL_DIR)'                          >  '$(PKG_FILE)'
	@echo 'exec_prefix=$${prefix}'                         >> '$(PKG_FILE)'
	@echo 'libdir=$${exec_prefix}/lib'                     >> '$(PKG_FILE)'
	@echo 'includedir=$${prefix}/include'                  >> '$(PKG_FILE)'
	@echo ''                                               >> '$(PKG_FILE)'
	@echo 'Name: $(PKG_LIBNAME)'                           >> '$(PKG_FILE)'
	@echo 'Description: $(PKG_LIBNAME)'                    >> '$(PKG_FILE)'
	@echo 'Version: $(VERSION_NAME)'                            >> '$(PKG_FILE)'
	@echo 'Requires: $(PKG_REQUIRES)'                      >> '$(PKG_FILE)'
	@echo 'Requires.private: $(PKG_REQUIRES_PRIV)'         >> '$(PKG_FILE)'
	@echo 'Libs: -L$${libdir} $(PKG_LIBDEPS)'              >> '$(PKG_FILE)'
	@echo 'Libs.private:'                                  >> '$(PKG_FILE)'
	@echo 'Cflags: -I$${includedir}'                       >> '$(PKG_FILE)'

clean:
	rm -rf $(BUILD_DIR)

doc:
	rm -rf $(DOC_DIR)
	sleep 1
	doxygen Doxyfile
