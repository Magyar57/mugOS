# Drivers makefile

SOURCES_C=$(wildcard *.c)
OBJECTS_C=$(patsubst %.c, $(BUILD_SUBDIR)/%.o, $(SOURCES_C))

all: drivers

.PHONY: all drivers

drivers: $(OBJECTS_C)

# Generic object rules

$(BUILD_SUBDIR)/%.o: %.c | $(BUILD_SUBDIR)
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<
