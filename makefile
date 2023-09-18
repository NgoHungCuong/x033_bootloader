CC = g++

PRJ = x033_bootloader
BUILD_DIR = build

SRC = main.cpp
SRC += nhc_usb.cpp
SRC += nhc_hex_file.cpp

OBJ = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRC))

ifeq ($(OS),Windows_NT)
	LIB = -lwinusb -lsetupapi
else
	LIB = -lusb-1.0
endif

all: $(OBJ)
	$(CC) $^ $(LIB) -o $(BUILD_DIR)/$(PRJ)

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CC) -c $< -o $@

$(BUILD_DIR):
	mkdir $@

clean:
	-rm -fR $(BUILD_DIR)
