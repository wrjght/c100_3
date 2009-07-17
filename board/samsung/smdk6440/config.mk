#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
# David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
#
# SAMSUNG SMDK6440 board with (ARM1176) cpu
#
# see http://www.samsung.com/ for more information on SAMSUNG
#

#
# SMDK6440 has a 128 MB SDR SDRAM
#
# 2000'0000 to 2800'0000
#
#
# Linux-Kernel is expected to be at 2000'8000, entry 2000'8000
# optionally with a ramdisk at 2080'0000
#
# we load ourself to 27e0'0000 without MMU
# with MMU, load address is changed to 0xc7e0_0000
#
# download area is 2000'0000
#


ifndef TEXT_BASE
TEXT_BASE = 0xc7e00000
endif

