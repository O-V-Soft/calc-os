ARCH ?= x86

ifeq ($(ARCH),x86)
    AS      := nasm
    CC      := gcc
    LD      := ld
    OBJCOPY := objcopy
    RM      := rm -f
    ASFLAGS_BIN := -f bin
    ASFLAGS_ELF := -f elf32
    CFLAGS      := -m32 -ffreestanding -fno-stack-protector -fno-leading-underscore \
            	-mgeneral-regs-only -mno-red-zone -I./include -c \
               -fno-pic -fno-asynchronous-unwind-tables -fno-strict-aliasing -fpack-struct \
			   -mno-sse -O2 -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable \
			   -Wno-unused-but-set-variable -Wno-unused-value -Wno-missing-field
    LDFLAGS     := -m elf_i386 -T linker.ld --nostdlib --static

    OBJ := init.o kernel.o cmos.o screen.o mouse_asm.o utils.o keyboard.o font.o inout.o \
           mouse.o wrappers.o idt.o isr.o task.o ata.o fat.o read.o write.o \
           sound.o pci_x86.o rtl8139.o mm.o forth.o syscalls.o sys_exit.o \
           sys_getpid.o sys_open.o sys_read.o sys_time.o sys_uname.o \
           sys_write.o sys_close.o sys_exec.o sys_getuid.o paging.o vfs.o \
           casm.o manual.o signal.o desktop.o string.o memory.o convert.o \
           keyboard_x86.o pci.o manual_x86.o calc3d.o edges.o nodes.o \
		   rotate.o project.o fill.o screen_x86.o

else ifeq ($(ARCH),riscv)
    AS      := riscv64-unknown-elf-gcc
    CC      := riscv64-unknown-elf-gcc
    LD      := riscv64-unknown-elf-ld
    OBJCOPY := riscv64-unknown-elf-objcopy
    RM      := rm -f
    ASFLAGS_ELF := -c -march=rv32ima_zicsr -mabi=ilp32
    CFLAGS      := -march=rv32ima_zicsr -mabi=ilp32 -ffreestanding -fno-stack-protector \
                   -ffunction-sections -I./include -c -fno-pic 
    LDFLAGS     := -m elf32lriscv -T arch/risc-v/linker_riscv.ld --nostdlib --static

    OBJ := boot.o init_riscv.o mm.o uart.o font.o keyboard_riscv.o manual.o \
	       convert.o string.o pci_riscv.o timer.o keyboard.o pci.o manual_riscv.o screen.o \
		   gpu.o screen_riscv.o
endif

vpath %.c kernel/main kernel arch/x86/cpu arch/x86/cpu/idt arch/x86/cpu/idt/tasks mm arch/x86/cpu/paging \
          arch/x86/drivers/cmos drivers/screen drivers/screen/font arch/x86/drivers/mouse \
          drivers/keyboard arch/x86/drivers/ata fs/fat12 arch/x86/drivers/sound \
          arch/x86/drivers/pci arch/x86/drivers/rtl8139 fs/vfs \
          lib forth casm commands arch/x86 arch/risc-v arch/risc-v/drivers/uart \
		  drivers/keybrd  arch/risc-v/drivers/pci arch/risc-v/drivers/keyboard \
		  arch/risc-v/cpu/timer fs arch/x86/drivers/keyboard drivers/pci \
		  arch/x86/commands arch/risc-v/commands Calc3D arch/risc-v/drivers/virtio \
	      arch/x86/drivers/screen arch/risc-v/drivers/screen

vpath %.asm arch/x86/boot arch/x86/io arch/x86/drivers/mouse/asm arch/x86/cpu/idt/asm
vpath %.S arch/risc-v/boot 

.PHONY: all clean cleane run boch help push test re rer dd rea rear

all: build_target

build_target:
	@if [ "$(ARCH)" = "x86" ]; then $(MAKE) os-image.img; else $(MAKE) KERNEL.SYS; fi

os-image.img: boot.bin KERNEL.SYS LS.BIN DEVICES.BIN STATUS.BIN \
			  BEHAVE.BIN REBOOT.BIN MOUNT.BIN UMOUNT.BIN WHOAMI.BIN

	dd if=/dev/zero of=$@ bs=512 count=2880
	mformat -i $@ -f 1440 ::
	dd if=$< of=$@ conv=notrunc bs=512 count=1
	mcopy -i $@ KERNEL.SYS ::KERNEL.SYS
	mcopy -i $@ LS.BIN ::LS.BIN
	mcopy -i $@ DEVICES.BIN ::DEVICES.BIN
	mcopy -i $@ STATUS.BIN ::STATUS.BIN
	mcopy -i $@ BEHAVE.BIN ::BEHAVE.BIN
	mcopy -i $@ REBOOT.BIN ::REBOOT.BIN
	mcopy -i $@ MOUNT.BIN ::MOUNT.BIN
	mcopy -i $@ UMOUNT.BIN ::UMOUNT.BIN
	mcopy -i $@ WHOAMI.BIN ::WHOAMI.BIN

boot.bin: arch/x86/boot/entry.asm
	$(AS) $(ASFLAGS_BIN) $< -o $@

KERNEL.SYS: $(OBJ)
	$(LD) $(LDFLAGS) -o kernel.elf $(OBJ)
	$(OBJCOPY) -O binary kernel.elf $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS_ELF) $< -o $@

%.o: %.S
	$(AS) $(ASFLAGS_ELF) $< -o $@

ls.o: commands/ls.c
	$(CC) $(CFLAGS) $< -o $@

devices.o: commands/devices.c
	$(CC) $(CFLAGS) $< -o $@

status.o: commands/status.c
	$(CC) $(CFLAGS) $< -o $@

behave.o: commands/behave.c
	$(CC) $(CFLAGS) $< -o $@

reboot.o: commands/reboot.c
	$(CC) $(CFLAGS) $< -o $@

umount.o: commands/umount.c
	$(CC) $(CFLAGS) $< -o $@

mount.o: commands/mount.c
	$(CC) $(CFLAGS) $< -o $@

whoami.o: commands/whoami.c
	$(CC) $(CFLAGS) $< -o $@

LS.BIN: ls.o app.ld
	$(LD) -m elf_i386 -T app.ld ls.o -o $@

DEVICES.BIN: devices.o app.ld
	$(LD) -m elf_i386 -T app.ld devices.o -o $@

STATUS.BIN: status.o app.ld
	$(LD) -m elf_i386 -T app.ld status.o -o $@

BEHAVE.BIN: behave.o app.ld
	$(LD) -m elf_i386 -T app.ld behave.o -o $@

REBOOT.BIN: reboot.o app.ld
	$(LD) -m elf_i386 -T app.ld reboot.o -o $@

UMOUNT.BIN: umount.o app.ld
	$(LD) -m elf_i386 -T app.ld umount.o -o $@

MOUNT.BIN: mount.o app.ld
	$(LD) -m elf_i386 -T app.ld mount.o -o $@

WHOAMI.BIN: whoami.o app.ld
	$(LD) -m elf_i386 -T app.ld whoami.o -o $@

clean:
	$(RM) *.o *.bin *.elf KERNEL.SYS *.BIN

cleane:
	$(RM) *.o *.bin *.elf *.img *.vdi KERNEL.SYS *.BIN arch/x86/*.o arch/riscv/*.o
	$(RM) traffic.pcap
	touch traffic.pcap

run: os-image.img
	qemu-system-i386 -drive file=os-image.img,format=raw \
	-netdev user,id=net0 \
    -object filter-dump,id=f1,netdev=net0,file=traffic.pcap \
    -device rtl8139,netdev=net0

ahci:
	qemu-system-i386 -device ich9-ahci,id=ahci \
    -drive file=os-image.img,format=raw,if=none,id=drv0 \
    -device ide-hd,bus=ahci.0,drive=drv0 \
    -netdev user,id=net0 \
    -object filter-dump,id=f1,netdev=net0,file=traffic.pcap \
    -device rtl8139,netdev=net0

boch:
	bochs -f bochsrc.txt

help:
	@echo boch - start with bochs
	@echo run - start x86
	@echo clean - clear files
	@echo cleane - clear all files
	@echo push - push to github
	@echo test - test pcap file
	@echo re - rebuild x86
	@echo rer - rebuild and run x86
	@echo dd - write image to sd card
	@echo rea - rebuild riscv
	@echo rear - rebuild and run riscv

push:
	git add .
	-git commit -m "CalcOS"
	git push origin main --force

test:
	tcpdump -XX -r traffic.pcap

rer:
	$(MAKE) cleane
	$(MAKE) run

re:
	$(MAKE) cleane
	$(MAKE)

dd:
	-sudo umount /dev/mmcblk0*
	sudo dd if=os-image.img of=/dev/mmcblk0 status=progress conv=fsync

rea:
	$(MAKE) clean
	$(MAKE) ARCH=riscv

rear:
	$(MAKE) clean
	$(MAKE) ARCH=riscv
	qemu-system-riscv32 -M virt -m 128M \
		-bios none -kernel kernel.elf \
		-device virtio-gpu-device \
		-serial stdio
