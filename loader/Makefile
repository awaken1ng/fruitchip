EE_STAGE_1_ADDR = 0x001001B4
EE_STAGE_2_ADDR = 0x000E0000
EE_STAGE_3_ADDR = 0x00090000

EE_STAGE_1 = ee-stage1.bin
EE_STAGE_2 = ee-stage2.bin

all: $(EE_STAGE_2) $(EE_STAGE_1)

clean:
	$(MAKE) -C ee-stage2 clean
	$(MAKE) -C ee-stage1 clean
	rm -rf bin

fletcher16:
	mkdir -p bin
	gcc -I./lib tools/fletcher16.c -o bin/fletcher16

$(EE_STAGE_2): fletcher16
	$(if $(EE_STAGE_3_FILE), $(), $(error EE_STAGE_3_FILE not specified))
	$(MAKE) -C ee-stage2 \
		EE_STAGE_2_ADDR=$(EE_STAGE_2_ADDR) \
		EE_STAGE_3_ADDR=$(EE_STAGE_3_ADDR) \
		EE_STAGE_3_SIZE=$(shell stat --dereference --format=%s $(EE_STAGE_3_FILE)) \
		EE_STAGE_3_CHECKSUM=$(shell ./bin/fletcher16 $(EE_STAGE_3_FILE))
	mips64r5900el-ps2-elf-objdump -S --disassemble ee-stage2/bin/ee-stage2.elf > ee-stage2.objdump

$(EE_STAGE_1): fletcher16 $(EE_STAGE_2)
	$(MAKE) -C ee-stage1 \
		EE_STAGE_1_ADDR=$(EE_STAGE_1_ADDR) \
		EE_STAGE_2_ADDR=$(EE_STAGE_2_ADDR) \
		EE_STAGE_2_SIZE=$(shell stat --dereference --format=%s ee-stage2/bin/$(EE_STAGE_2)) \
		EE_STAGE_2_CHECKSUM=$(shell ./bin/fletcher16 ee-stage2/bin/$(EE_STAGE_2))
	mips64r5900el-ps2-elf-objdump -S --disassemble ee-stage1/bin/ee-stage1.elf > ee-stage1.objdump
