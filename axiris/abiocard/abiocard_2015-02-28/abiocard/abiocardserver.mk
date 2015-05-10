
# The compiler and linker options are set for a release build. If you want to
# build a debug version, apply the following settings:
# CFLAGS: specify -O0 -g instead of -O2.
# LDFLAGS: leave out -s.

TARGET     := abiocardserver
BASE_DIR   := ./..
SOURCE_DIR := $(BASE_DIR)
BUILD_DIR  := $(BASE_DIR)/build/gcc/$(TARGET)
BIN_DIR    := .
CFLAGS     := -O2 -std=gnu89 \
              -I$(SOURCE_DIR)/common/$(shell gcc -dumpmachine) \
              -I$(SOURCE_DIR)/common \
              -I$(SOURCE_DIR)/common/linux \
              -I$(SOURCE_DIR)/axicat
LDFLAGS    := -s -lrt
SRCS       := $(SOURCE_DIR)/abiocard/abiocardserver.c \
              $(SOURCE_DIR)/abiocard/axicat_i2cbus.c \
              $(SOURCE_DIR)/abiocard/bsc_i2cbus.c \
              $(SOURCE_DIR)/abiocard/i2cdev_i2cbus.c \
              $(SOURCE_DIR)/abiocard/abiocard.c \
              $(SOURCE_DIR)/axicat/axicat_al.c \
              $(SOURCE_DIR)/common/bsc.c \
              $(SOURCE_DIR)/common/rpidetect.c \
              $(SOURCE_DIR)/common/ldllist.c \
              $(SOURCE_DIR)/common/linux/ft245.c \
              $(SOURCE_DIR)/common/linux/osrtl_time.c
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

