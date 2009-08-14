#
# (C) Copyright 2009
# Hakjoo Kim, Hard Kernel, <ruppi.kim@hardkernel.com>
# HARDKERNEL HKDKC100 board with mDirac3 (ARM-CortexA8) cpu
#
# see http://www.hardkernel.com/ for more information on HARDKERNEL
#

#
# SMDKC100 has a 128 MB SDR SDRAM
#
#
# Linux-Kernel is expected to be at 5000'8000, entry 5000'8000
# optionally with a ramdisk at 5080'0000
#
# we load ourself to 57e0'0000 without MMU
# with MMU, load address is changed to 0xc7e0_0000
#
# download area is 5000'0000
#


ifndef TEXT_BASE
TEXT_BASE = 0xc7e00000
//TEXT_BASE = 0x27e00000
endif

