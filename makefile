CC := gcc

PROGRAM := main

SOURCE_DIR := ./src
OUTPUT_DIR := ./build

SOURCE_FILES = $(SOURCE_DIR)/main.c
SOURCE_FILES += $(SOURCE_DIR)/renderer_structs/renderer_structs.c
SOURCE_FILES += $(SOURCE_DIR)/asset_loader/asset_loader.c
SOURCE_FILES += $(SOURCE_DIR)/utils/parallel_unit.c
SOURCE_FILES += $(SOURCE_DIR)/utils/timerino.c
SOURCE_FILES += $(SOURCE_DIR)/utils/polly.c
SOURCE_FILES += $(SOURCE_DIR)/renderer/renderer.c
SOURCE_FILES += $(SOURCE_DIR)/transform_functions/transform_functions.c
SOURCE_FILES += $(SOURCE_DIR)/camera_transform/camera_transform.c

CFLAGS += -funroll-loops -Ofast -I$(SOURCE_DIR)
CFLAGS += -mavx2 -mfma
#CFLAGS += -march=native
#CFLAGS += -fopenmp
#CFLAGS += -funroll-loops
CFLAGS += -fprefetch-loop-arrays
#CFLAGS += -fno-exceptions

LDFLAGS += -lm

OBJS = $(SOURCE_FILES:%.c=%.o)
OBJS_OUTPUT = $(OBJS:$(SOURCE_DIR)/%=$(OUTPUT_DIR)/%)

all: $(PROGRAM)

debug: CFLAGS += -g3
debug: clean all

$(PROGRAM): $(OBJS_OUTPUT) makefile
	$(CC) $(OBJS_OUTPUT) -o $(PROGRAM) $(LDFLAGS)

$(OUTPUT_DIR)/%.o : $(SOURCE_DIR)/%.c | $(OUTPUT_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

clean:
	rm -rf $(PROGRAM) $(OUTPUT_DIR)

.PHONY: all clean debug
