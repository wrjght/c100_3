/*
 *	Main: Copy U-Boot Image from OneNAND to DRAM
 */
/* This code support both 2KB page and 4KB page */

#include <linux/mtd/onenand_regs.h>

#define ONENAND_MAP00_BASE	0xB0000000
#define ONENAND_MAP11_BASE	0xBC000000

#if defined(CONFIG_SMDKC100_POP_AC)
#define ONENAND_PAGE_SIZE	2048
#elif defined(CONFIG_SMDKC100_POP_D)
#define ONENAND_PAGE_SIZE	4096
#endif

#define UBOOT_PHY_BASE		0x27E00000

#define READ_INTERRUPT()	\
	onenand_readl(ONENAND_MAP11_BASE + (ONENAND_REG_INTERRUPT<<1))

#define onenand_block_address(block)		(block)
#define onenand_sector_address(page)		(page << ONENAND_FPA_SHIFT)
#define onenand_buffer_address()			((1 << 3) << 8)
//#define onenand_buffer_address()			((0 << 3) << 8)	
#define onenand_bufferram_address(block)	(0)

inline unsigned int onenand_readl (unsigned int addr)
{
	return *(unsigned int*)addr;
}

inline void onenand_writel (unsigned int value, unsigned int addr)
{
	*(unsigned int*)addr = value;
}

void(*run_uboot)(void) = UBOOT_PHY_BASE;

//void ONENAND_WriteReg()
//
void ONENAND_ReadPage(
	void* base, unsigned int block, unsigned int page)
{
	int len;
	unsigned int* dest;
	unsigned int* src;

	// Block Number
	onenand_writel(onenand_block_address(block),
			ONENAND_MAP11_BASE + (ONENAND_REG_START_ADDRESS1<<1));

	// BufferRAM
	onenand_writel(onenand_bufferram_address(block),
			ONENAND_MAP11_BASE + (ONENAND_REG_START_ADDRESS2<<1));

	// Page (Sector) Number Set: FPA, FSA
	onenand_writel(onenand_sector_address(page),
			ONENAND_MAP11_BASE + (ONENAND_REG_START_ADDRESS8<<1));

	// BSA, BSC
	onenand_writel(onenand_buffer_address(),
			ONENAND_MAP11_BASE + (ONENAND_REG_START_BUFFER<<1));

	// Interrupt clear
	onenand_writel(ONENAND_INT_CLEAR,
			ONENAND_MAP11_BASE + (ONENAND_REG_INTERRUPT<<1));

	onenand_writel(ONENAND_CMD_READ,
			ONENAND_MAP11_BASE + (ONENAND_REG_COMMAND<<1));

#if 1
	while (!(READ_INTERRUPT() & ONENAND_INT_READ))
		continue;
#else
	while (!(READ_INTERRUPT() & ONENAND_INT_MASTER))
		continue;
#endif

	len = ONENAND_PAGE_SIZE >> 2;
	dest = (unsigned short *)(base);
	src = (unsigned short *)(ONENAND_MAP00_BASE + ONENAND_DATARAM);
	while (len-- > 0)
	{
		*dest++ = *src++;
	}
}

/*
 * Copy U-Boot from OneNAND to DRAM (368KB)
 */
void Main(void)
{
	volatile unsigned int base = UBOOT_PHY_BASE;
	int block, page;

#if defined(CONFIG_SMDKC100_POP_AC)
	for (block=0, page = 2; page < 64; page++) {
		ONENAND_ReadPage((void *)base, block, page);
		base += ONENAND_PAGE_SIZE;
	}

	for (block=1, page = 0; page < 64; page++) {
		ONENAND_ReadPage((void *)base, block, page);
		base += ONENAND_PAGE_SIZE;
	}

	for (block=2, page = 0; page < 64; page++) {
		ONENAND_ReadPage((void *)base, block, page);
		base += ONENAND_PAGE_SIZE;
	}
#elif defined(CONFIG_SMDKC100_POP_D)
	for (block=0, page = 1; page < 64; page++) {
		ONENAND_ReadPage((void *)base, block, page);
		base += ONENAND_PAGE_SIZE;
	}

	for (block=1, page = 0; page < 32; page++) {
		ONENAND_ReadPage((void *)base, block, page);
		base += ONENAND_PAGE_SIZE;
	}
#endif

	run_uboot();
}

