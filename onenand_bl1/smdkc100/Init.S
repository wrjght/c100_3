
.include "s5pc100_reg.inc"

.equ	_STACK_BASEADDRESS,	0x27FF8000

@;/* Pre-defined constants */
.equ	USERMODE,			0x10
.equ	FIQMODE,			0x11
.equ	IRQMODE,			0x12
.equ	SVCMODE,			0x13
.equ	ABORTMODE,			0x17
.equ	UNDEFMODE,			0x1b
.equ	MODEMASK,			0x1f
.equ	NOINT,				0xc0

@;/* The location of stacks */
.equ	UserStack,			(_STACK_BASEADDRESS-0x3800) @;0x83ff4800 ~
.equ	SVCStack,			(_STACK_BASEADDRESS-0x2800) @;0x83ff5800 ~
.equ	UndefStack,			(_STACK_BASEADDRESS-0x2400) @;0x83ff5c00 ~
.equ	AbortStack,			(_STACK_BASEADDRESS-0x2000) @;0x83ff6000 ~
.equ	IRQStack,			(_STACK_BASEADDRESS-0x1000) @;0x83ff7000 ~
.equ	FIQStack,			(_STACK_BASEADDRESS-0x0)    @;0x83ff8000 ~

.globl Main

.section .text, "x"

.globl _start

_start:
	b	ResetHandler
	b	HandlerUndef                    @;handler for Undefined mode
	b	HandlerSWI                      @;handler for SWI interrupt
	b	HandlerPabort                   @;handler for PAbort
	b	HandlerDabort                   @;handler for DAbort
	b	.                               @;reserved
	b	HandlerIRQ                      @;handler for IRQ interrupt
	b	HandlerFIQ                      @;handler for FIQ interrupt
	b	.                               @;by digibuff FEB-23-2004 dummy branch

.ifdef zero_init
.globl _bss_start
_bss_start:
	.word bss_start

.globl _bss_end
_bss_end:
	.word bss_end

.globl _data_start
_data_start:
	.word data_start

.globl _rodata
_rodata:
	.word rodata
.endif

ResetHandler:
	bl	reset

	@;test
	@;bl	dma_init
	
	ldr	r0, =(ELFIN_CLOCK_POWER_BASE+RST_STAT_OFFSET)
	ldr	r1, [r0]
	bic	r1, r1, #0xfffffff7
	cmp	r1, #0x8
	beq	wakeup_reset

	bl	Main
	b	.

reset:
	/*
	 * set the cpu to SVC32 mode and IRQ & FIQ disable
	 */
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0xd3
	msr	cpsr,r0

/*
 *************************************************************************
 *
 * CPU_init_critical registers
 *
 * setup important registers
 * setup memory timing
 *
 *************************************************************************
 */
	/*
	 * we do sys-critical inits only at reboot,
	 * not when booting from ram!
	 */
cpu_init_crit:
	/*
	 * Invalidate L1 I/D
	 */
	mov	r0, #0                  @ set up for MCR
	mcr	p15, 0, r0, c8, c7, 0   @ invalidate TLBs
	mcr	p15, 0, r0, c7, c5, 0   @ invalidate icache

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002000     @ clear bits 13 (--V-)
	bic	r0, r0, #0x00000007     @ clear bits 2:0 (-CAM)
	orr	r0, r0, #0x00000002     @ set bit 1 (--A-) Align
	orr	r0, r0, #0x00000800     @ set bit 12 (Z---) BTB
	mcr	p15, 0, r0, c1, c0, 0

	/*
	 * Go setup Memory and board specific bits prior to relocation.
	 */
lowlevel_init:

	/* IO Retention release */
	ldr	r0, =(ELFIN_CLOCK_POWER_BASE + OTHERS_OFFSET)
	ldr	r1, [r0]
	ldr	r2, =IO_RET_REL
	orr	r1, r1, r2
	str	r1, [r0]

	/* Disable Watchdog */
	ldr	r0, =0xEA200000
	mov	r1, #0
	str	r1, [r0]

	/* CS0 - 16bit SRAM, Enable nBE */
	ldr	r0, =ELFIN_SROM_BASE
	mov	r1, #0x9
	str	r1, [r0]

/*
 * system_clock_init: Initialize core clock and bus clock.
 * void system_clock_init(void)
 */
system_clock_init:

	ldr	r0, =ELFIN_CLOCK_POWER_BASE	@0xe0100000

	mov	r1, #0xe00
	orr	r1, r1, #0x10
	str	r1, [r0, #APLL_LOCK_OFFSET]
	str	r1, [r0, #MPLL_LOCK_OFFSET]
	str	r1, [r0, #EPLL_LOCK_OFFSET]
	str	r1, [r0, #HPLL_LOCK_OFFSET]

	ldr   	r1, [r0, #CLK_DIV0_OFFSET]
	ldr	r2, =CLK_DIV0_MASK
	bic	r1, r1, r2

	ldr	r2, =CLK_DIV0_VAL
	orr	r1, r1, r2
	str	r1, [r0, #CLK_DIV0_OFFSET]

	ldr	r1, =APLL_VAL
	str	r1, [r0, #APLL_CON_OFFSET]

	ldr   	r1, [r0, #CLK_DIV1_OFFSET]
	ldr	r2, =CLK_DIV1_VAL
	orr	r1, r1, r2
	str	r1, [r0, #CLK_DIV1_OFFSET]

	ldr	r1, =MPLL_VAL
	str	r1, [r0, #MPLL_CON_OFFSET]
	ldr	r1, =EPLL_VAL
	str	r1, [r0, #EPLL_CON_OFFSET]
	ldr	r1, =HPLL_VAL
	str	r1, [r0, #HPLL_CON_OFFSET]

	ldr	r1, [r0, #CLK_SRC0_OFFSET]
	ldr	r2, =0x1111
	orr	r1, r1, r2
	str	r1, [r0, #CLK_SRC0_OFFSET]

	mov	r1, #0x10000
1:	subs	r1, r1, #1
	bne	1b

mem_ctrl_asm_init:

	ldr	r0, =APB_DMC_BASE			@APB_DMC_BASE 0xE6000000

	ldr	r1, =PRO_ID_BASE
	ldr	r2, [r1, #PRO_ID_OFFSET]
	bic	r2, #0xfffffdff
	mov	r2, r2, lsr #9
	cmp	r2, #0x1
	beq	onenand_pop

onenand_pop:
#if defined(CONFIG_SMDKC100_POP_AC)
	ldr	r1, =0x50101000				@Phycontrol0 DLL parameter setting
	str	r1, [r0, #DMC_PHYCONTROL0]
#elif defined(CONFIG_SMDKC100_POP_D)
	ldr	r1, =0x50101008				@Phycontrol0 DLL parameter setting
	str	r1, [r0, #DMC_PHYCONTROL0]
#endif

	ldr	r1, =0x000000F4				@Phycontrol1 DLL parameter setting
	str	r1, [r0, #DMC_PHYCONTROL1]

	ldr	r1, =0x00000000				@Phycontrol2 DLL parameter setting
	str	r1, [r0, #DMC_PHYCONTROL2]

#if defined(CONFIG_SMDKC100_POP_AC)
	ldr	r1, =0x50101002				@Dll on
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x50101003				@dll start
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x50101003				@Force Value locking
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x50101001				@Dll off
	str	r1, [r0, #DMC_PHYCONTROL0]
#elif defined(CONFIG_SMDKC100_POP_D)
	ldr	r1, =0x5010100A				@Dll on
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x5010100B				@dll start
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x5010100B				@Force Value locking
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x50101009				@Dll off
	str	r1, [r0, #DMC_PHYCONTROL0]
#endif

	ldr	r1, =0xFF001010				@auto refresh off
	str	r1, [r0, #DMC_CONCONTROL]

	ldr	r1, =0x00212100				@Dll off
	str	r1, [r0, #DMC_MEMCONTROL]

#if defined(CONFIG_SMDKC100_POP_AC)
	ldr	r1, =0x28F80222
	str	r1, [r0, #DMC_MEMCONFIG0]

	ldr	r1, =0x20F80222
	str	r1, [r0, #DMC_MEMCONFIG1]
#elif defined(CONFIG_SMDKC100_POP_D)
	ldr	r1, =0x30F81312				@ 128MB
	str	r1, [r0, #DMC_MEMCONFIG0]

	ldr	r1, =0x20F01322				@ 256MB
	str	r1, [r0, #DMC_MEMCONFIG1]
#endif

	ldr	r1, =0x20000000
	str	r1, [r0, #DMC_PRECHCONFIG]

	ldr	r1, =0x0000050E
	str	r1, [r0, #DMC_TIMINGAREF]

	ldr	r1, =0x0C233287				@TimingRow	@133MHz
	str	r1, [r0, #DMC_TIMINGROW]

	ldr	r1, =0x32330303
	str	r1, [r0, #DMC_TIMINGDATA]

	ldr	r1, =0x04141433				@Timing Power
	str	r1, [r0, #DMC_TIMINGPOWER]

	ldr	r1, =0x07000000				@chip0 Deselect
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x01000000				@chip0 PALL
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x05000000				@chip0 REFA
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x05000000				@chip0 REFA
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00000032				@chip0 MRS
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x07100000				@chip1 Deselect
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x01100000				@chip1 PALL
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x05100000				@chip1 REFA
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x05100000				@chip1 REFA
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00100032				@chip1 MRS
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0xFF002030				@ConControl auto refresh on
	str	r1, [r0, #DMC_CONCONTROL]

	ldr	r1, =0x00100002				@PwrdnConfig
	str	r1, [r0, #DMC_PWRDNCONFIG]

	ldr	r1, =0xFF212100				@MemControl
	str	r1, [r0, #DMC_MEMCONTROL]

_exit_cpu_init:
	mov	pc, lr


wakeup_reset:
	@;all leds on for test
	ldr r0, =ELFIN_GPIO_BASE
	ldr r1, =0x40
	str r1, [r0, #GPH1DAT_OFFSET]

	@;Clear wakeup status register
	ldr r0, =(ELFIN_CLOCK_POWER_BASE+WAKEUP_STAT_OFFSET)
	ldr r1, [r0]
	str r1, [r0]

	@;Load return address and jump to kernel
	ldr r0, =(INF_REG_BASE+INF_REG0_OFFSET)
	ldr r1, [r0]    @; r1 = physical address of

	mov pc, r1      @; Jump to kernel
	nop
	nop


@; Setting for DMA
dma_init:

	ldr r0, =0xE3800000
	mov r1, #0x0
	str r1, [r0]
	mov r1, #0xff
	str r1, [r0, #0x804]
	str r1, [r0, #0x810]

	ldr	r0, =0xE2800000
	str r1, [r0, #0x804]
	str r1, [r0, #0x810]
	str r1, [r0, #0x81C]

	ldr r0, =0xE2900000
	str r1, [r0, #0x804]
	str r1, [r0, #0x810]

	mov pc, lr


@;The LR register wont be valid if the current mode is not SVC mode.
HandlerFIQ:
	b	.
HandlerIRQ:
	b	.
HandlerUndef:
	b	.
HandlerSWI:
	b	.
HandlerDabort:
	b	.
HandlerPabort:
	b	.

.end

