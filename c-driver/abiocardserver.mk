
# The compiler and linker options are set for a release build. If you want to
# build a debug version, apply the following settings:
# CFLAGS: specify -O0 -g instead of -O2.
# LDFLAGS: leave out -s.

TARGET     := abiocardserver
BASE_DIR   := .
SOURCE_DIR := $(BASE_DIR)
BUILD_DIR  := $(BASE_DIR)/build/$(TARGET)
BIN_DIR    := .
CFLAGS     := -O2 -std=gnu89 -I$(SOURCE_DIR)
LDFLAGS    := -s
SRCS       := $(SOURCE_DIR)/abiocardserver.c \
              $(SOURCE_DIR)/bcm2835_detect.c \
              $(SOURCE_DIR)/bsc.c \
              $(SOURCE_DIR)/bsc_i2cbus.c \
              $(SOURCE_DIR)/i2cdev_i2cbus.c \
              $(SOURCE_DIR)/abiocard.c
OBJS       := $(patsubst $(SOURCE_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

all: $(BIN_DIR)/$(TARGET)

.PHONY: clean

clean:
	rm -f $(OBJS)
	rm -f $(TARGET)

# Target .o for each .c
$(foreach src,$(SRCS),$(eval $(patsubst $(SOURCE_DIR)/%.c,$(BUILD_DIR)/%.o,$(src)): $(src)))

# C compilation rule
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c -o $@ $(CFLAGS) $<

# Link rule
$(BIN_DIR)/$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $+

