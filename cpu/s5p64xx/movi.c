#include <common.h>
#include <movi.h>
#include <asm/io.h>
#include <regs.h>

uint movi_hc = 0;

void movi_set_capacity(void)
{
	if (MOVI_HIGH_CAPACITY & 0x1)
		movi_hc = 1;
}

int movi_set_ofs(uint last)
{
	int changed = 0;

	if (INF_REG3_REG == 1) {	/* eMMC_4.3 */
		/* last of boot partition */
		ofsinfo.last 	= EMMC_BL_BLKCNT + EMMC_KERNEL_BLKCNT + EMMC_ROOTFS_BLKCNT;
		ofsinfo.bl1	= 0;
		ofsinfo.bl2	= 0;
		ofsinfo.env	= ofsinfo.bl2 + EMMC_UBOOT_BLKCNT;
		ofsinfo.kernel	= ofsinfo.bl2 + EMMC_BL_BLKCNT;
		ofsinfo.rootfs	= ofsinfo.kernel + EMMC_KERNEL_BLKCNT;
		changed = 1;
	} else {			/* SD/MMC */
		if (ofsinfo.last != last) {
			ofsinfo.last 	= last - (eFUSE_SIZE / MOVI_BLKSIZE);
			ofsinfo.bl1	= ofsinfo.last - MOVI_BL1_BLKCNT;
			ofsinfo.env	= ofsinfo.bl1 - MOVI_ENV_BLKCNT;
			ofsinfo.bl2	= ofsinfo.bl1 - (MOVI_BL2_BLKCNT + MOVI_ENV_BLKCNT);
			ofsinfo.kernel	= ofsinfo.bl2 - MOVI_ZIMAGE_BLKCNT;
			ofsinfo.rootfs	= ofsinfo.kernel - MOVI_ROOTFS_BLKCNT;
			changed = 1;
		}
	}

	return changed;
}

int movi_init(void)
{
	int ret = 0;

	hsmmc_set_gpio();
	hsmmc_set_base();
	hsmmc_reset();

	if (INF_REG3_REG == 1)	/* eMMC_4.3 */
		ret = emmc_init();
	else			/* SD/MMC */
		ret = hsmmc_init();

	if (ret) {
		printf("\nCard Initialization failed(%d)\n", ret);
		return -1;
	}

	return 1;
}

void movi_write_env(ulong addr)
{
	if (INF_REG3_REG == 1) {	/* eMMC_4.3 */
		emmc_write((uint)addr, ofsinfo.env, EMMC_ENV_BLKCNT);
	} else {			/* SD/MMC */
		movi_write((uint)addr, ofsinfo.env, MOVI_ENV_BLKCNT);
	}
}

void movi_read_env(ulong addr)
{
	if (INF_REG3_REG == 1) {	/* eMMC_4.3 */
		emmc_read((uint)addr, ofsinfo.env, EMMC_ENV_BLKCNT);
	} else {			/* SD/MMC */
		movi_read((uint)addr, ofsinfo.env, MOVI_ENV_BLKCNT);
	}
}

void movi_bl2_copy(void)
{
	int movi_ch;

	if (INF_REG3_REG & 0x1)
		movi_ch = 1;
	else
		movi_ch = 0;

	if (INF_REG3_REG == 1) {
		/* Device copy function for eMMC */
		COPY_eMMC_to_MEM(0, (1024-14), (uint *)(BL2_BASE+0x1c00), 4);
		COPY_eMMC_to_MEM(0, EMMC_KERNEL_BLKCNT, (uint *)(0x20008000), 4);
		COPY_eMMC_to_MEM(0, EMMC_ROOTFS_BLKCNT, (uint *)(0x20800000), 4);

		memcpy(0x27e00000, 0xd0020000, 7*1024);
	} else {
		/* Device copy function for SD/MMC */
		CopyMovitoMem(movi_ch, MOVI_BL2_POS, MOVI_BL2_BLKCNT, (uint *)BL2_BASE, MOVI_INIT_REQUIRED);
	}
}
