BUILD_DIR = ./object
SRC_DIR = ./src

CFILES = $(wildcard $(SRC_DIR)/*.c)
OFILES = $(CFILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles
LDFLAGS = -nostdlib -nostartfiles

all: $(BUILD_DIR)/kernel8.img #run

$(BUILD_DIR)/start.o: $(SRC_DIR)/start.S
	aarch64-none-elf-gcc $(GCCFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	aarch64-none-elf-gcc $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel8.img: $(BUILD_DIR)/start.o $(OFILES)
	aarch64-none-elf-ld $(LDFLAGS) $(BUILD_DIR)/start.o $(OFILES) -T $(SRC_DIR)/link.ld -o $(BUILD_DIR)/kernel8.elf
	aarch64-none-elf-objcopy -O binary $(BUILD_DIR)/kernel8.elf kernel8.img

clean:
	del .\object\kernel8.elf .\object\*.o kernel8.img

run1:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio

run0:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
run:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio
	
test1: all run1
test0: all run0