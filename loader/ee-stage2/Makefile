EE_BIN_DIR = bin/
EE_OBJS_DIR = obj/
EE_SRC_DIR = src/

EE_TARGET=ee-stage2
EE_BIN = $(EE_BIN_DIR)$(EE_TARGET).elf
EE_BIN_RAW = $(EE_BIN_DIR)$(EE_TARGET).bin
EE_BIN_STRIPPED = $(EE_BIN_DIR)$(EE_TARGET).stripped.elf

EE_OBJS = main.o
EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

EE_LINKFILE = linkfile
EE_LDFLAGS += -nostartfiles -Wl,--gc-sections

EE_INCS += -I../lib
EE_CFLAGS += -Os \
    -DEE_STAGE_3_ADDR=${EE_STAGE_3_ADDR} \
    -DEE_STAGE_3_SIZE=${EE_STAGE_3_SIZE} \
    -DEE_STAGE_3_CHECKSUM=${EE_STAGE_3_CHECKSUM}

.PHONY: all clean

all:
	$(if $(EE_STAGE_2_ADDR), $(), $(error EE_STAGE_2_ADDR not specified))
	$(if $(EE_STAGE_3_ADDR), $(), $(error EE_STAGE_3_ADDR not specified))
	$(if $(EE_STAGE_3_SIZE), $(), $(error EE_STAGE_3_SIZE not specified))
	$(if $(EE_STAGE_3_CHECKSUM), $(), $(error EE_STAGE_3_CHECKSUM not specified))
	$(MAKE) $(EE_BIN_RAW)

# rm'ing EE_LINKFILE might result in deleting the linkfile from SDK, if local variable is unset
clean:
	rm -rf $(EE_OBJS_DIR) $(EE_BIN_DIR) linkfile

linkfile:
	cp ../linkfile.in linkfile
	sed -i s/@LOAD_ADDR@/${EE_STAGE_2_ADDR}/ linkfile

$(EE_BIN): linkfile

$(EE_BIN_STRIPPED): $(EE_BIN)
	$(EE_STRIP) -s -R .comment -R .gnu.version --strip-unneeded -o $@ $<

$(EE_BIN_RAW): $(EE_BIN_STRIPPED)
	$(EE_OBJCOPY) -O binary -v $< $@

$(EE_BIN_DIR):
	@mkdir -p $@

$(EE_OBJS_DIR):
	@mkdir -p $@

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.c | $(EE_OBJS_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
