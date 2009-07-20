#include <common.h>

#include <command.h>
#include <movi.h>
#include <nand.h>

struct movi_offset_t ofsinfo;
extern nand_info_t nand_info[];


#define readl(addr) (*(volatile unsigned int*)(addr))
#define writel(b,addr) ((*(volatile unsigned int *) (addr)) = (b))


#if 0
int do_testhsmmc(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	uint start_blk, blknum;
	start_blk = simple_strtoul(argv[1], NULL, 10);
	blknum = simple_strtoul(argv[2], NULL, 10);

	if(argc < 3)
		return 0;

	printf("COMMAND --> start_blk = 0x%x, blknum = %d\n", start_blk, blknum);
	test_hsmmc(4, 1, start_blk, blknum);
	return 1;
}

U_BOOT_CMD(
	testhsmmc,	4,	0,	do_testhsmmc,
	"testhsmmc - hsmmc testing write/read\n",
	NULL
);
#endif


#if !defined(CONFIG_SMDK6440)
int do_movi(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	char *cmd;
	unsigned int addr, blk, blocks;
	uint last_blkpos, bytes;

	cmd = argv[1];

	if (strcmp(cmd, "init") == 0) {
		if (argc < 4)
			goto usage;
		
		last_blkpos = (uint) simple_strtoul(argv[2], NULL, 10);
		movi_hc = (uint) simple_strtoul(argv[3], NULL, 10);

		if (movi_set_ofs(last_blkpos))
			movi_init();
	} else {
		if (argc == 4 || (strcmp(argv[2], "rootfs") == 0))
			addr = (ulong) simple_strtoul(argv[3], NULL, 16);
		else if (argc == 5)
			addr = (ulong) simple_strtoul(argv[4], NULL, 16);
		else
			goto usage;

		if (addr >= 0xc0000000)
			addr = virt_to_phys(addr);

		if (ofsinfo.last == 0) {
			printf("### Execute 'movi init' command first!\n");
			return -1;
		}
		
		if (strcmp(cmd, "read") == 0) {
			if (argc == 4 || (strcmp(argv[2], "rootfs") == 0)) {
				if (strcmp(argv[2], "u-boot") == 0) {
					printf("Reading bootloader.. ");
					movi_read((uint) addr, ofsinfo.bl2, MOVI_BL2_BLKCNT);
					printf("completed\n");

				} else if (strcmp(argv[2], "kernel") == 0) {
					printf("Reading kernel from sector %d (%d sectors).. ", ofsinfo.kernel, MOVI_ZIMAGE_BLKCNT);
					movi_read((uint) addr, ofsinfo.kernel, MOVI_ZIMAGE_BLKCNT);
					printf("completed\n");

				} else if (strcmp(argv[2], "rootfs") == 0) {
					bytes = simple_strtoul(argv[4], NULL, 16);
					
					if (argc == 5)
						blocks = (ulong) (bytes / MOVI_BLKSIZE);
					else
						blocks = MOVI_ROOTFS_BLKCNT;

					if (bytes % MOVI_BLKSIZE > 0)
						blocks++;

					blk = ofsinfo.rootfs;

					printf("Reading rootfs from sector %d (%d sectors).. ", blk, blocks);

					while (blocks > 0) {
						if (blocks < MOVI_RW_MAXBLKS) {
							movi_read((uint) addr, blk, blocks);
							blocks = 0;
						} else {
							movi_read((uint) addr, blk, MOVI_RW_MAXBLKS);
							addr += (MOVI_RW_MAXBLKS * MOVI_BLKSIZE);
							blk += MOVI_RW_MAXBLKS;
							blocks -= MOVI_RW_MAXBLKS;
						}
					}
					printf("completed\n");
				} else
					goto usage;
			} else {
				blk = (uint) simple_strtoul(argv[2], NULL, 10);
				bytes = (uint) simple_strtoul(argv[3], NULL, 16);

				if (blk >= 0 && blk <= ofsinfo.last) {
					blocks = (int) (bytes / MOVI_BLKSIZE);

					if (bytes % MOVI_BLKSIZE > 0)
						blocks++;

					printf("Reading data from sector %d (%d sectors).. ", blk, blocks);

					while (blocks > 0) {
						if (blocks < MOVI_RW_MAXBLKS) {
							movi_read((uint) addr, blk, blocks);
							blocks = 0;
						} else {
							movi_read((uint) addr, blk, MOVI_RW_MAXBLKS);
							addr += (MOVI_RW_MAXBLKS * MOVI_BLKSIZE);
							blk += MOVI_RW_MAXBLKS;
							blocks -= MOVI_RW_MAXBLKS;
						}
					}
					printf("completed\n");
				} else
					goto usage;
			}
		} else if (strcmp(cmd, "write") == 0) {
			if (argc == 4 || (strcmp(argv[2], "rootfs") == 0)) {
				if (strcmp(argv[2], "u-boot") == 0) {
					printf("Writing 1st bootloader to sector %d (%d sectors).. ", ofsinfo.bl1, MOVI_BL1_BLKCNT);
					movi_write((uint) addr, ofsinfo.bl1, MOVI_BL1_BLKCNT);
					printf("completed\nWriting 2nd bootloader to sector %d (%d sectors).. ", ofsinfo.bl2, MOVI_BL2_BLKCNT);
					movi_write((uint) addr, ofsinfo.bl2, MOVI_BL2_BLKCNT);
					printf("completed\n");

				} else if (strcmp(argv[2], "kernel") == 0) {
					printf("Writing kernel to sector %d (%d sectors).. ", ofsinfo.kernel, MOVI_ZIMAGE_BLKCNT);
					movi_write((uint) addr, ofsinfo.kernel, MOVI_ZIMAGE_BLKCNT);
					printf("completed\n");

				} else if (strcmp(argv[2], "env") == 0) {
					printf("Writing env to sector %d (%d sectors).. ", ofsinfo.env, MOVI_ENV_BLKCNT);
					movi_write((uint) addr, ofsinfo.env, MOVI_ENV_BLKCNT);
					printf("completed\n");

				} else if (strcmp(argv[2], "rootfs") == 0) {
					blk = ofsinfo.rootfs;
					bytes = simple_strtoul(argv[4], NULL, 16);
					
					if (argc == 5)
						blocks = (ulong) (bytes / MOVI_BLKSIZE);
					else
						blocks = MOVI_ROOTFS_BLKCNT;

					if (bytes % MOVI_BLKSIZE > 0)
						blocks++;

					printf("Writing rootfs to sector %d (%d sectors).. ", blk, blocks);

					while (blocks > 0) {
						if (blocks < MOVI_RW_MAXBLKS) {
							movi_write((uint) addr, blk, blocks);
							blocks = 0;
						} else {
							movi_write((uint) addr, blk, MOVI_RW_MAXBLKS);
							addr += (MOVI_RW_MAXBLKS * MOVI_BLKSIZE);
							blk += MOVI_RW_MAXBLKS;
							blocks -= MOVI_RW_MAXBLKS;
						}
					}
					printf("completed\n");
				} else
					goto usage;
			} else {
				blk = (uint) simple_strtoul(argv[2], NULL, 10);
				bytes = (uint) simple_strtoul(argv[3], NULL, 16);

				if (blk >= 0 && blk <= ofsinfo.last) {
					blocks = (int) (bytes / MOVI_BLKSIZE);

					if (bytes % MOVI_BLKSIZE > 0)
						blocks++;

					printf("Writing data to sector %d (%d sectors).. ", blk, blocks);

					while (blocks > 0) {
						if (blocks < MOVI_RW_MAXBLKS) {
							movi_write((uint) addr, blk, blocks);
							blocks = 0;
						} else {
							movi_write((uint) addr, blk, MOVI_RW_MAXBLKS);
							addr += (MOVI_RW_MAXBLKS * MOVI_BLKSIZE);
							blk += MOVI_RW_MAXBLKS;
							blocks -= MOVI_RW_MAXBLKS;
						}
					}
					printf("completed\n");
				} else
					goto usage;
			}
		} else {
			goto usage;
		}
	}

	return 1;

usage:
	printf("Usage:\n%s\n", cmdtp->usage);
	return -1;
}
#else
int do_movi(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	char *cmd;
	unsigned int addr, blk, blocks;
	uint last_blkpos, bytes;

	cmd = argv[1];

	if (strcmp(cmd, "init") == 0) {
		if (argc < 4)
			goto usage;
		
		last_blkpos = (uint) simple_strtoul(argv[2], NULL, 10);
		movi_hc = (uint) simple_strtoul(argv[3], NULL, 10);

		if (movi_set_ofs(last_blkpos))
			movi_init();
	} else {
		if (argc == 4 || (strcmp(argv[2], "rootfs") == 0))
			addr = (ulong) simple_strtoul(argv[3], NULL, 16);
		else if (argc == 5)
			addr = (ulong) simple_strtoul(argv[4], NULL, 16);
		else
			goto usage;

		if (addr >= 0xc0000000)
			addr = virt_to_phys(addr);

		if ((ofsinfo.last == 0) && (movi_emmc ==0)) {
			printf("### Execute 'movi init' command first!\n");
			return -1;
		}
		
		if (strcmp(cmd, "read") == 0) {
			if (argc == 4 || (strcmp(argv[2], "rootfs") == 0)) {
				if (strcmp(argv[2], "u-boot") == 0) {
					if (movi_emmc == 1) {	/* eMMC_4.3 */
						printf("eMMC Reading bootloader.. from sector %d (%d sectors)..", ofsinfo.bl2, EMMC_BL_BLKCNT);
						emmc_read((uint) addr, ofsinfo.bl2, EMMC_BL_BLKCNT);
					} else {
						printf("Reading bootloader.. ");
						movi_read((uint) addr, ofsinfo.bl2, MOVI_BL2_BLKCNT);
					}
					printf("completed\n");

				} else if (strcmp(argv[2], "kernel") == 0) {
					if (movi_emmc == 1) {	/* eMMC_4.3 */
						printf("eMMC Reading kernel from sector %d (%d sectors).. ", ofsinfo.kernel, EMMC_KERNEL_BLKCNT);
						emmc_read((uint) addr, ofsinfo.kernel, EMMC_KERNEL_BLKCNT);
					} else {
						printf("Reading kernel from sector %d (%d sectors).. ", ofsinfo.kernel, MOVI_ZIMAGE_BLKCNT);
						movi_read((uint) addr, ofsinfo.kernel, MOVI_ZIMAGE_BLKCNT);
					}
					printf("completed\n");

				} else if (strcmp(argv[2], "rootfs") == 0) {
					bytes = simple_strtoul(argv[4], NULL, 16);
					
					if (argc == 5)
						blocks = (ulong) (bytes / MOVI_BLKSIZE);
					else
						blocks = MOVI_ROOTFS_BLKCNT;

					if (bytes % MOVI_BLKSIZE > 0)
						blocks++;

					blk = ofsinfo.rootfs;

					if (movi_emmc == 1) {	/* eMMC_4.3 */
						printf("eMMC Reading rootfs from sector %d (%d sectors).. ", ofsinfo.rootfs, EMMC_ROOTFS_BLKCNT);
						emmc_read((uint) addr, ofsinfo.rootfs, EMMC_ROOTFS_BLKCNT);
					} else {
						printf("Reading rootfs from sector %d (%d sectors).. ", blk, blocks);

						while (blocks > 0) {
							if (blocks < MOVI_RW_MAXBLKS) {
								movi_read((uint) addr, blk, blocks);
								blocks = 0;
							} else {
								movi_read((uint) addr, blk, MOVI_RW_MAXBLKS);
								addr += (MOVI_RW_MAXBLKS * MOVI_BLKSIZE);
								blk += MOVI_RW_MAXBLKS;
								blocks -= MOVI_RW_MAXBLKS;
							}
						}
					}
					printf("completed\n");
				} else
					goto usage;
			} else {
				blk = (uint) simple_strtoul(argv[2], NULL, 10);
				bytes = (uint) simple_strtoul(argv[3], NULL, 16);

				if (blk >= 0 && blk <= ofsinfo.last) {
					blocks = (int) (bytes / MOVI_BLKSIZE);

					if (bytes % MOVI_BLKSIZE > 0)
						blocks++;

					if (movi_emmc == 1) {	/* eMMC_4.3 */
						if ((blk + blocks) <= ofsinfo.last ) {
							printf("eMMC Reading data from sector %d (%d sectors).. ", blk, blocks);
							emmc_read((uint) addr, blk, blocks);
						}
					} else {
						printf("Reading data from sector %d (%d sectors).. ", blk, blocks);

						while (blocks > 0) {
							if (blocks < MOVI_RW_MAXBLKS) {
								movi_read((uint) addr, blk, blocks);
								blocks = 0;
							} else {
								movi_read((uint) addr, blk, MOVI_RW_MAXBLKS);
								addr += (MOVI_RW_MAXBLKS * MOVI_BLKSIZE);
								blk += MOVI_RW_MAXBLKS;
								blocks -= MOVI_RW_MAXBLKS;
							}
						}
					}
					printf("completed\n");
				} else
					goto usage;
			}
		} else if (strcmp(cmd, "write") == 0) {
			if (argc == 4 || (strcmp(argv[2], "rootfs") == 0)) {
				if (strcmp(argv[2], "u-boot") == 0) {
					if (movi_emmc == 1) {	/* eMMC_4.3 */
						printf("eMMC Writing bootloader to sector %d (%d sectors).. ", ofsinfo.bl2, EMMC_BL_BLKCNT);
						emmc_write((uint) addr, ofsinfo.bl2, EMMC_BL_BLKCNT);
					} else {		/* SD/MMC */
						printf("Writing 1st bootloader to sector %d (%d sectors).. ", ofsinfo.bl1, MOVI_BL1_BLKCNT);
						movi_write((uint) addr, ofsinfo.bl1, MOVI_BL1_BLKCNT);
						printf("completed\nWriting 2nd bootloader to sector %d (%d sectors).. ", ofsinfo.bl2, MOVI_BL2_BLKCNT);
						movi_write((uint) addr, ofsinfo.bl2, MOVI_BL2_BLKCNT);
					}
					printf("completed\n");

				} else if (strcmp(argv[2], "kernel") == 0) {
					if (movi_emmc == 1) {	/* eMMC_4.3 */
						printf("eMMC Writing kernel to sector %d (%d sectors).. ", ofsinfo.kernel, EMMC_KERNEL_BLKCNT);
						emmc_write((uint) addr, ofsinfo.kernel, EMMC_KERNEL_BLKCNT);
					} else {		/* SD/MMC */
						printf("Writing kernel to sector %d (%d sectors).. ", ofsinfo.kernel, MOVI_ZIMAGE_BLKCNT);
						movi_write((uint) addr, ofsinfo.kernel, MOVI_ZIMAGE_BLKCNT);
					}
					printf("completed\n");

				} else if (strcmp(argv[2], "env") == 0) {
					if (movi_emmc == 1) {	/* eMMC_4.3 */
						printf("eMMC Writing env to sector %d (%d sectors).. ", ofsinfo.env, EMMC_ENV_BLKCNT);
						emmc_write((uint) addr, ofsinfo.env, EMMC_ENV_BLKCNT);
					} else {		/* SD/MMC */
						printf("Writing env to sector %d (%d sectors).. ", ofsinfo.env, MOVI_ENV_BLKCNT);
						movi_write((uint) addr, ofsinfo.env, MOVI_ENV_BLKCNT);
					}
					printf("completed\n");

				} else if (strcmp(argv[2], "rootfs") == 0) {
					if (movi_emmc == 1) {	/* eMMC_4.3 */
						printf("eMMC Writing rootfs to sector %d (%d sectors).. ", ofsinfo.rootfs, EMMC_ROOTFS_BLKCNT);
						emmc_write((uint) addr, ofsinfo.rootfs, EMMC_ROOTFS_BLKCNT);
					} else {		/* SD/MMC */
						blk = ofsinfo.rootfs;
						bytes = simple_strtoul(argv[4], NULL, 16);
						
						if (argc == 5)
							blocks = (ulong) (bytes / MOVI_BLKSIZE);
						else
							blocks = MOVI_ROOTFS_BLKCNT;

						if (bytes % MOVI_BLKSIZE > 0)
							blocks++;

						printf("Writing rootfs to sector %d (%d sectors).. ", blk, blocks);

						while (blocks > 0) {
							if (blocks < MOVI_RW_MAXBLKS) {
								movi_write((uint) addr, blk, blocks);
								blocks = 0;
							} else {
								movi_write((uint) addr, blk, MOVI_RW_MAXBLKS);
								addr += (MOVI_RW_MAXBLKS * MOVI_BLKSIZE);
								blk += MOVI_RW_MAXBLKS;
								blocks -= MOVI_RW_MAXBLKS;
							}
						}
					}
					printf("completed\n");
				} else
					goto usage;
			} else {
				blk = (uint) simple_strtoul(argv[2], NULL, 10);
				bytes = (uint) simple_strtoul(argv[3], NULL, 16);

				if (blk >= 0 && blk <= ofsinfo.last) {
					blocks = (int) (bytes / MOVI_BLKSIZE);

					if (bytes % MOVI_BLKSIZE > 0)
						blocks++;

					if (movi_emmc == 1) {	/* eMMC_4.3 */
						if ((blk + blocks) <= ofsinfo.last ) {
							printf("eMMC Writing data to sector %d (%d sectors).. ", blk, blocks);
							emmc_write((uint) addr, blk, blocks);
						}
					} else {		/* SD/MMC */
						printf("Writing data to sector %d (%d sectors).. ", blk, blocks);

						while (blocks > 0) {
							if (blocks < MOVI_RW_MAXBLKS) {
								movi_write((uint) addr, blk, blocks);
								blocks = 0;
							} else {
								movi_write((uint) addr, blk, MOVI_RW_MAXBLKS);
								addr += (MOVI_RW_MAXBLKS * MOVI_BLKSIZE);
								blk += MOVI_RW_MAXBLKS;
								blocks -= MOVI_RW_MAXBLKS;
							}
						}
					}
					printf("completed\n");
				} else
					goto usage;
			}
		} else {
			goto usage;
		}
	}

	return 1;

usage:
	printf("Usage:\n%s\n", cmdtp->usage);
	return -1;
}
#endif

U_BOOT_CMD(
	movi,	5,	0,	do_movi,
	"movi\t- moviNAND sub-system\n",
	"init {total sectors} {hc(0/1)} - Initialize moviNAND\n"
	"movi read  {u-boot | kernel} {addr} - Read data from moviNAND\n"
	"movi write {u-boot | kernel} {addr} - Write data to moviNAND\n"
	"movi read  rootfs {addr} [bytes(hex)] - Read rootfs data from moviNAND by size\n"
	"movi write rootfs {addr} [bytes(hex)] - Write rootfs data to moviNAND by size\n"
	"movi read  {sector#} {bytes(hex)} {addr} - Read data from moviNAND sector#\n"
	"movi write {sector#} {bytes(hex)} {addr} - Write data to moviNAND sector#\n"
);

#if defined(CONFIG_CMD_NAND)
int do_insnand(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	size_t total = CFG_ENV_OFFSET;
	ulong addr = PHYS_SDRAM_1;

	movi_read((uint) addr, ofsinfo.bl2, MOVI_BL2_BLKCNT);
	nand_erase(&nand_info[0], 0x0, CFG_ENV_OFFSET + CFG_ENV_SIZE);
	nand_write(&nand_info[0], 0x0, &total, (u_char *) addr);

	printf("Done\n");

	return 1;
}



int do_insAndroidUboot(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int ret = 0;

	nand_write_options_t opts;

	memset(&opts, 0, sizeof(opts));
	opts.pad = 1;
	opts.blockalign = 1;
	opts.writeoob = 0;
	opts.autoplace = 1;
//------------------------------------------------------------------
	printf("Start android uboot installation. \n");
	

	opts.offset = 0x0;
	opts.length = readl(0x27FFFFA8);
	opts.buffer = readl(0x27FFFFA0);


	if(opts.length > 0x500000)	opts.quiet  = 0;
	else				opts.quiet = 1;

	
	nand_erase(&nand_info[0], opts.buffer, &opts.length);
	ret = nand_write_opts(&nand_info[0], &opts);
	printf("Writed uBoot \n");


	printf("Installed Android uboot image. \n");

	return ret == 0 ? 0 : 1;

	return 1;
}




int do_insAndroidzImage(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int ret = 0;

	nand_write_options_t opts;

	memset(&opts, 0, sizeof(opts));
	opts.pad = 1;
	opts.blockalign = 1;
	opts.writeoob = 0;
	opts.autoplace = 1;
//------------------------------------------------------------------
	printf("Start android zImage installation. \n");
	opts.offset = 0x200000;
	opts.length = readl(0x27FFFFB8);
	opts.buffer = readl(0x27FFFFB0);


	if(opts.length > 0x500000)	opts.quiet  = 0;
	else				opts.quiet = 1;

	nand_erase(&nand_info[0], opts.buffer, &opts.length);
	ret = nand_write_opts(&nand_info[0], &opts);
	printf("Writed zImage \n");


	printf("Installed Android zImage image. \n");

	return ret == 0 ? 0 : 1;

	return 1;
}




int do_insAndroidRamdisk(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int ret = 0;

	nand_write_options_t opts;

	memset(&opts, 0, sizeof(opts));
	opts.pad = 1;
	opts.blockalign = 1;
	opts.writeoob = 0;
	opts.autoplace = 1;
//------------------------------------------------------------------
	printf("Start android Ramdisk installation. \n");

	opts.offset = 0x600000;
	opts.length = readl(0x27FFFFC8);
	opts.buffer = readl(0x27FFFFC0);


	if(opts.length > 0x500000)	opts.quiet  = 0;
	else				opts.quiet = 1;

	
	nand_erase(&nand_info[0], opts.buffer, &opts.length);
	ret = nand_write_opts(&nand_info[0], &opts);
	printf("Writed Ramdisk \n");


	printf("Installed Android Ramdisk image. \n");

	return ret == 0 ? 0 : 1;

	return 1;
}




int do_insAndroidSystemImg(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int ret = 0;

	nand_write_options_t opts;

	memset(&opts, 0, sizeof(opts));
//------------------------------------------------------------------
	opts.pad = 0;
	opts.blockalign = 1;
	opts.writeoob = 1;
	opts.autoplace = 1;
	
	opts.offset = 0xA00000;
	opts.length = readl(0x27FFFFD8);
	opts.buffer = readl(0x27FFFFD0);

	if(opts.length > 0x500000)	opts.quiet  = 0;
	else				opts.quiet = 1;

	printf("Start Addr : %X, Size :  %X \n",opts.buffer,opts.length);

	nand_erase(&nand_info[0], opts.buffer, &opts.length);
	ret = nand_write_opts(&nand_info[0], &opts);
	printf("Writed System Image \n");


	printf("Installed Android System Image image. \n");

	return ret == 0 ? 0 : 1;

	return 1;
}




int do_insAndroidUserImage(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int ret = 0;

	nand_write_options_t opts;

	memset(&opts, 0, sizeof(opts));
//------------------------------------------------------------------
	opts.pad = 0;
	opts.blockalign = 1;
	opts.writeoob = 1;
	opts.autoplace = 1;
	opts.offset = 0x9000000;
	opts.length = readl(0x27FFFFE8);
	opts.buffer = readl(0x27FFFFE0);

	if(opts.length > 0x500000)	opts.quiet = 0;
	else				opts.quiet = 1;

	printf("Start Addr : %X, Size :  %X \n",opts.buffer,opts.length);

	nand_erase(&nand_info[0], opts.buffer, &opts.length);
	ret = nand_write_opts(&nand_info[0], &opts);
	printf("Writed User Data \n");


	printf("Installed Android UserData Image image. \n");

	return ret == 0 ? 0 : 1;

	return 1;
}




int do_insAndroid(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int ret = 0;

	nand_write_options_t opts;

	memset(&opts, 0, sizeof(opts));
	opts.pad = 1;
	opts.blockalign = 1;
	opts.writeoob = 0;
	opts.autoplace = 1;
//------------------------------------------------------------------
	printf("Start android installation. \n");
	
	opts.offset = 0x0;
	opts.length = readl(0x27FFFFA8);
	opts.buffer = readl(0x27FFFFA0);
	if(opts.length > 0x500000)	opts.quiet  = 0;
	else				opts.quiet = 1;
	
	nand_erase(&nand_info[0], opts.buffer, &opts.length);
	ret = nand_write_opts(&nand_info[0], &opts);
	printf("Writed uBoot \n");


	opts.offset = 0x200000;
	opts.length = readl(0x27FFFFB8);
	opts.buffer = readl(0x27FFFFB0);

	if(opts.length > 0x500000)	opts.quiet  = 0;
	else				opts.quiet = 1;
	nand_erase(&nand_info[0], opts.buffer, &opts.length);
	ret = nand_write_opts(&nand_info[0], &opts);
	printf("Writed zImaget \n");


	opts.offset = 0x600000;
	opts.length = readl(0x27FFFFC8);
	opts.buffer = readl(0x27FFFFC0);
	if(opts.length > 0x500000)	opts.quiet  = 0;
	else				opts.quiet = 1;


	nand_erase(&nand_info[0], opts.buffer, &opts.length);
	ret = nand_write_opts(&nand_info[0], &opts);
	printf("Writed Ramdisk Image \n");

//------------------------------------------------------------------
	udelay (1000);
	
	opts.pad = 0;
	opts.blockalign = 1;
	opts.writeoob = 1;
	opts.autoplace = 1;
	
	opts.offset = 0xA00000;
	opts.length = readl(0x27FFFFD8);
	opts.buffer = readl(0x27FFFFD0);
	if(opts.length > 0x500000)	opts.quiet  = 0;
	else				opts.quiet = 1;


	printf("Start Addr : %X, Size :  %X \n",opts.buffer,opts.length);

	nand_erase(&nand_info[0], opts.buffer, &opts.length);
	ret = nand_write_opts(&nand_info[0], &opts);
	printf("Writed System Image \n");

	udelay (1000);

	opts.offset = 0x9000000;
	opts.length = readl(0x27FFFFE8);
	opts.buffer = readl(0x27FFFFE0);
	if(opts.length > 0x500000)	opts.quiet  = 0;
	else				opts.quiet = 1;

	printf("Start Addr : %X, Size :  %X \n",opts.buffer,opts.length);

	nand_erase(&nand_info[0], opts.buffer, &opts.length);
	ret = nand_write_opts(&nand_info[0], &opts);
	printf("Writed User Data \n");

	printf("Android Installation OK. \n");


	return ret == 0 ? 0 : 1;

	return 1;
}



U_BOOT_CMD(
	insnand, 1, 0, do_insnand,
	"insnand - install SD/MMC bootloader image to NAND flash\n",
	": install SD/MMC bootloader to NAND flash\n"
);



U_BOOT_CMD(
	insAndUboot, 1, 0, do_insAndroidUboot,
	"insAndUboot - install Android Uboot image from DRAM to NAND flash\n",
	": install Android Uboot Image from DRAM to NAND flash\n"
);


U_BOOT_CMD(
	insAndZimage, 1, 0, do_insAndroidzImage,
	"insAndZimage - install Android ZImage image from DRAM to NAND flash\n",
	": install Android ZImage Image from DRAM to NAND flash\n"
);


U_BOOT_CMD(
	insAndRamdisk, 1, 0, do_insAndroidRamdisk,
	"insAndRamdisk - install Android Ramdisk image from DRAM to NAND flash\n",
	": install Android Ramdisk Image from DRAM to NAND flash\n"
);


U_BOOT_CMD(
	insAndSystemImg, 1, 0, do_insAndroidSystemImg,
	"insAndSystemImg - install Android System image from DRAM to NAND flash\n",
	": install Android System Image from DRAM to NAND flash\n"
);


U_BOOT_CMD(
	insAndUserdata, 1, 0, do_insAndroidUserImage,
	"insAndUserdata - install Android Userdata image from DRAM to NAND flash\n",
	": install Android Userdata Image from DRAM to NAND flash\n"
);

U_BOOT_CMD(
	insAndroid, 1, 0, do_insAndroid,
	"insAndroid - install Android all image from DRAM to NAND flash\n",
	": install Android all Images from DRAM to NAND flash\n"
);
#endif

