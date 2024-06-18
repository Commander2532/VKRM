export ARCH:=arm
export CROSS_COMPILE:=arm-none-linux-gnueabihf

CC = arm-linux-gnueabihf-gcc

MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(notdir $(patsubst %/,%,$(dir $(MKFILE_PATH))))

PROJ_NAME = $(CURRENT_DIR)
TARGET_NAME = test
INCLUDE_DIR = include
SOURCE_DIR = source
LIB_DIR = lib
GEN_DIR = obj
CFLAGS = --enable-threads=posix

INCLUDE = $(wildcard $(INCLUDE_DIR)/*.h)
SOURCES = $(wildcard $(SOURCE_DIR)/*.c)
OBJECTS = $(patsubst $(SOURCE_DIR)/%.c, $(GEN_DIR)/%.o, $(SOURCES))
LIBS	= $(wildcard $(LIB_DIR)/*.a)
TARGET  = $(PROJ_NAME)

all: printStart $(TARGET) printEnd
default: printStart $(TARGET) printEnd

printStart:
	@echo ''
	@echo '***************************************************'
	@echo 'Compiling project: $(PROJ_NAME)'

printEnd:
	@echo ''
	@echo 'Object files are in the "$(GEN_DIR)" directory'
	@echo 'Finished building project: $(PROJ_NAME)'
	@echo '***************************************************'
	@echo ''

$(TARGET): $(GEN_DIR)/%.o
	@echo ''
	@echo 'Building target: $(TARGET_NAME)'
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET_NAME) -static $(LIBS)
	@echo 'Finished building target: $(TARGET_NAME)'

$(GEN_DIR)/%.o: $(SOURCES)
	@mkdir -p $(GEN_DIR)
	@echo 'Building file: $<'
	@echo 'Invoking compiler: $(CC)'
	$(CC) -c $^ -I$(INCLUDE_DIR)
	@mv *.o ./obj

.PHONY: all clean

clean:
	@echo ''
	@echo '***************************************************'
	@echo 'Cleaning project: $(PROJ_NAME)'
	@echo ''
	@rm -rf -v $(GEN_DIR)
	@rm $(TARGET_NAME)
	@echo ''
	@echo 'Finished cleaning project $(PROJ_NAME)'
	@echo '***************************************************'
	@echo ''
