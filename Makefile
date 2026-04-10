BUILD = build
SRC = src
DRIVER = drivers
MIDDLEWARE = middleware
CORE = core


ARM = arm-none-eabi-
CC = $(ARM)gcc
AS = $(ARM)as
LD = $(ARM)ld
GDB = $(ARM)gdb
OBJCPY = $(ARM)objcopy

#flags
INCLUDES = -Iinclude/cmsis -Iinclude/device -Iinclude

FLAGS = -mcpu=cortex-m4 -mthumb -o2	-ffreestanding
# frestandin is important as we need to tell the compiler that there is 
# no os or libs gurentee startup provided by system or syscalls
# it remove some assumption that compiler makes

FLASH_BASE = 0x08000000

C_SRC_FILES = $(shell find $(SRC) -name "*.c")
AS_SRC_FILES = $(shell find $(SRC) -name "*.s")

C_OBJ_FILES =		$(patsubst $(SRC)/%.c, $(BUILD)/%_c_.o, $(C_SRC_FILES))
AS_OBJ_FILES =		$(patsubst $(SRC)/%.s, $(BUILD)/%_as_.o, $(AS_SRC_FILES))

LINKER_SCRIPT =  linkerscript.ld 

ELF = $(BUILD)/firmware.elf 
BIN = $(BUILD)/firmware.bin

OPENOCD = openocd -f interface/stlink.cfg -f target/stm32f4x.cfg

default : all

all : build Makefile $(shell find include -name "*.h") $(ELF) $(BIN)

build :
	mkdir -p $(BUILD)/$(MIDDLEWARE) $(BUILD)/$(DRIVER) $(BUILD)/$(CORE)

flash : $(BIN)
	$(OPENOCD) -c "program $(BIN) $(FLASH_BASE) verify reset exit"

start_debug_server : $(ELF)
	$(OPENOCD)
start_debug_client : $(ELF)
	$(GDB) $(ELF)

$(BIN) : $(ELF)
	$(OBJCPY) -O binary $(ELF) $(BIN)

$(ELF) : $(LINKER_SCRIPT) $(C_OBJ_FILES) $(AS_OBJ_FILES)
	$(CC) -nostartfiles -Wl,--gc-sections  -T $(LINKER_SCRIPT) $(C_OBJ_FILES) $(AS_OBJ_FILES) -o $(ELF)

$(BUILD)/%_c_.o : $(SRC)/%.c 
		
	$(CC) -c $(FLAGS) $(INCLUDES) -g3 -fno-inline $< -o $@

$(BUILD)/%_as_.o : $(SRC)/%.s
	
	$(AS) -g $< -o $@


clean : 
	@rm -rf $(BUILD)

	


