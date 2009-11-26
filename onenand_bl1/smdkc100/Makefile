#
# BL1 of U-Boot for OneNAND
#

CROSS_COMPILE	= /usr/local/arm/4.2.2-eabi/usr/bin/arm-linux-

AS				= $(CROSS_COMPILE)as
LD				= $(CROSS_COMPILE)ld
CC              = $(CROSS_COMPILE)gcc
AR              = $(CROSS_COMPILE)ar
NM              = $(CROSS_COMPILE)nm
STRIP           = $(CROSS_COMPILE)strip
OBJCOPY         = $(CROSS_COMPILE)objcopy
OBJDUMP         = $(CROSS_COMPILE)objdump

# Load generated board configuration
sinclude ../../include/autoconf.mk

MKBL1			= mkbl1

TARGET          = BL1.bin
O_TARGET		= BL1.elf
OBJS			= Init.o 
OBJS            += Main.o 

TEXT_BASE       = 0xD0034000

ifdef CONFIG_SMDKC100_POP_AC
CFLAGS			= -I../../include -Os -DCONFIG_SMDKC100_POP_AC
endif

ifdef CONFIG_SMDKC100_POP_D
CFLAGS			= -I../../include -Os -DCONFIG_SMDKC100_POP_D
endif

#
all: $(TARGET) $(MKBL1) $(TARGET).padding

$(TARGET):      $(OBJS)
	$(LD) -T BL1.lds -Ttext $(TEXT_BASE) -o $(O_TARGET) -Map BL1.map $(OBJS)
	$(OBJCOPY) --gap-fill=0xFF --pad-to=0x1000 -O binary $(O_TARGET) $(TARGET)

$(TARGET).padding:	$(TARGET) $(MKBL1)
	./$(MKBL1) $(TARGET) $(TARGET).padding

u-boot:			$(TARGET) $(MKBL1) ../u-boot.bin
	./$(MKBL1)
	cat BL1.bin.padding ../../u-boot.bin > ../../u-boot-onenand.bin

$(MKBL1):		mkbl1.c
	gcc -o $(MKBL1) mkbl1.c	

Main.o:			Main.c
	$(CC) $(CFLAGS) -c -o Main.o Main.c  

Init.o:			Init.S
	$(CC) $(CFLAGS) -c -o Init.o Init.S

dep:
	gccmakedep $(OBJS:.o=.c)(OBJS:.o=.s)

clean   :
	rm -rf $(OBJS) $(TARGET) $(O_TARGET) BL1.map $(TARGET).padding $(MKBL1)

# DO NOT DELETE

