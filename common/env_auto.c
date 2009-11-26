/*
 * (C) Copyright 2004
 * Jian Zhang, Texas Instruments, jzhang@ti.com.

 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>

 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* #define DEBUG */

#include <common.h>

#if defined(CFG_ENV_IS_IN_AUTO) /* Environment is in Nand Flash */

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <nand.h>
#include <movi.h>
#include <regs.h>

#if defined(CONFIG_CMD_ENV) || defined(CONFIG_CMD_NAND) || defined(CONFIG_CMD_MOVINAND) || defined(CONFIG_CMD_ONENAND)
#define CMD_SAVEENV
#endif

/* info for NAND chips, defined in drivers/nand/nand.c */
extern nand_info_t nand_info[];

#if defined(CONFIG_CMD_ONENAND)
/* info for OneNAND chips */
extern struct mtd_info onenand_info[];
#endif
/* references to names in env_common.c */
extern uchar default_environment[];
extern int default_environment_size;

char * env_name_spec = "SMDK bootable device";

#ifdef ENV_IS_EMBEDDED
extern uchar environment[];
env_t *env_ptr = (env_t *)(&environment[0]);
#else /* ! ENV_IS_EMBEDDED */
env_t *env_ptr = 0;
#endif /* ENV_IS_EMBEDDED */


/* local functions */
#if !defined(ENV_IS_EMBEDDED)
static void use_default(void);
#endif

DECLARE_GLOBAL_DATA_PTR;

uchar env_get_char_spec (int index)
{
	return ( *((uchar *)(gd->env_addr + index)) );
}


/* this is called before nand_init()
 * so we can't read Nand to validate env data.
 * Mark it OK for now. env_relocate() in env_common.c
 * will call our relocate function which will does
 * the real validation.
 *
 * When using a NAND boot image (like sequoia_nand), the environment
 * can be embedded or attached to the U-Boot image in NAND flash. This way
 * the SPL loads not only the U-Boot image from NAND but also the
 * environment.
 */
int env_init(void)
{
#if defined(ENV_IS_EMBEDDED)
	ulong total;
	int crc1_ok = 0, crc2_ok = 0;
	env_t *tmp_env1, *tmp_env2;

	total = CFG_ENV_SIZE;

	tmp_env1 = env_ptr;
	tmp_env2 = (env_t *)((ulong)env_ptr + CFG_ENV_SIZE);

	crc1_ok = (crc32(0, tmp_env1->data, ENV_SIZE) == tmp_env1->crc);
	crc2_ok = (crc32(0, tmp_env2->data, ENV_SIZE) == tmp_env2->crc);

	if (!crc1_ok && !crc2_ok)
		gd->env_valid = 0;
	else if(crc1_ok && !crc2_ok)
		gd->env_valid = 1;
	else if(!crc1_ok && crc2_ok)
		gd->env_valid = 2;
	else {
		/* both ok - check serial */
		if(tmp_env1->flags == 255 && tmp_env2->flags == 0)
			gd->env_valid = 2;
		else if(tmp_env2->flags == 255 && tmp_env1->flags == 0)
			gd->env_valid = 1;
		else if(tmp_env1->flags > tmp_env2->flags)
			gd->env_valid = 1;
		else if(tmp_env2->flags > tmp_env1->flags)
			gd->env_valid = 2;
		else /* flags are equal - almost impossible */
			gd->env_valid = 1;
	}

	if (gd->env_valid == 1)
		env_ptr = tmp_env1;
	else if (gd->env_valid == 2)
		env_ptr = tmp_env2;
#else /* ENV_IS_EMBEDDED */
	gd->env_addr  = (ulong)&default_environment[0];
	gd->env_valid = 1;
#endif /* ENV_IS_EMBEDDED */

	return (0);
}

#ifdef CMD_SAVEENV
/*
 * The legacy NAND code saved the environment in the first NAND device i.e.,
 * nand_dev_desc + 0. This is also the behaviour using the new NAND code.
 */
 
// Modified saveenv_nand() to saveenv_nand_adv() (WC Jang)
int saveenv_nand(void)
{
#ifdef CONFIG_NAND
	size_t total;
	int ret = 0;

	puts("#1 Erasing Nand...");
	if (nand_erase(&nand_info[0], CFG_ENV_OFFSET, CFG_ENV_SIZE)) {
		puts("failed2-nand_write\n");
		return 1;
	}

	puts("#2 Writing to Nand... ");
	total = CFG_ENV_SIZE;

	ret = nand_write(&nand_info[0], CFG_ENV_OFFSET, &total, (u_char*) env_ptr);
	if (ret || total != CFG_ENV_SIZE) {
		puts("failed2-nand_write\n");
		return 1;
	}

	puts("done\n");
	return ret;
#endif
}


// Modified saveenv_nand_adv() to saveenv_nand() (WC Jang)
int saveenv_nand_adv(void)
{
#ifdef CONFIG_NAND
	size_t total;
	int ret = 0;

	u_char *tmp;
	total = CFG_ENV_OFFSET;

	tmp = (u_char *) malloc(total);
	if (tmp == NULL) {
		puts("malloc is failed\n");
		return 1;
	}
	ret = nand_read(&nand_info[0], 0x0, &total, (u_char *) tmp);
	if (ret || total != CFG_ENV_OFFSET) {
		puts("failed1-nand_read\n");
		free(tmp);
		return 1;
	}

	puts("#1 Erasing Nand...");
	if (nand_erase(&nand_info[0], 0x0, CFG_ENV_OFFSET + CFG_ENV_SIZE)) {
		puts("failed2-nand_erase\n");
		free(tmp);
		return 1;
	}

	puts("#2 Writing to Nand... ");
	ret = nand_write(&nand_info[0], 0x0, &total, (u_char *) tmp);
	if (ret) {
		puts("failed3-nand_write\n");
		free(tmp);
		return 1;
	}
	total = CFG_ENV_SIZE;
	ret = nand_write(&nand_info[0], CFG_ENV_OFFSET, &total, (u_char *) env_ptr);
	if (ret || total != CFG_ENV_SIZE) {
		puts("failed4-nand_write\n");
		free(tmp);
		return 1;
	}

	puts("done\n");
	free(tmp);

	return ret;
#endif
}

int saveenv_movinand(void)
{
#if! defined(CONFIG_MOVINAND)
	movi_write_env(virt_to_phys((ulong)env_ptr));
#else if defined(CONFIG_S5PC100)
	movi_write_env(virt_to_phys((ulong)env_ptr));
#endif
	puts("done\n");

	return 1;
}

int saveenv_onenand(void)
{
#if defined(CONFIG_SMDKC100)
	size_t total;
	int ret = 1;
	struct mtd_info *onenand = &onenand_info[0];
	struct erase_info instr;
	u32 erasebase;
	u32 erasesize = onenand->erasesize;
	u32 writesize = erasesize;
	u_char *data = NULL;
	total = CFG_ENV_OFFSET;

	/* If the value of CFG_ENV_OFFSET is not a OneNAND block boundary, the
	 * OneNAND erase operation will fail. So first check if the CFG_ENV_OFFSET
	 * is equal to a NAND block boundary
	 */
	if ((CFG_ENV_OFFSET % (erasesize - 1)) != 0 ) {
		/* CFG_ENV_OFFSET is not equal to block boundary address.
		 * So, read the OneNAND block (in which ENV has to be stored),
		 * and copy the ENV data into the copied block data.
		 */

		/* Step 1: Find out the starting address of the OneNAND block to
		 * be erased. Also allocate memory whose size is equal to tbe
		 * OneNAND block size (OneNAND erasesize).
		 */
		erasebase = (CFG_ENV_OFFSET / erasesize) * erasesize;
		data = (uint8_t*)malloc(erasesize);
		if (data == NULL) {
			printf("Could not save enviroment variables\n");
		return 1;
	}

		/* Step 2: Read the OneNAND block into which the ENV data has
		 * to be copied
		 */
		ret = onenand->read(onenand, erasebase, erasesize, &total, (u_char *) data);
		if (ret || total != erasesize) {
			printf("Could not save enviroment variables %d\n",ret);
			goto err;
		}

		/* Step 3: Copy the ENV data into the local copy of the block
		 * contents.
		 */
		memcpy((data + (CFG_ENV_OFFSET - erasebase)), (void*) env_ptr, CFG_ENV_SIZE);
	} else {
		/* CFG_ENV_OFFSET is equal to a OneNAND block boundary. So
		 * no special care is required when erasing and writing OneNAND
		 * block
		 */
		data = env_ptr;
		erasebase = CFG_ENV_OFFSET;
		writesize = CFG_ENV_SIZE;
	}

	/* Erase the OneNAND block which will hold the ENV data */
	instr.mtd = onenand;
	instr.addr = erasebase;
	instr.len = erasesize;
	instr.callback = 0;
	puts("Erasing OneNAND...\n");
	if (onenand->erase(onenand, &instr)) {
		printf("Could not save enviroment variables\n");
		goto err;
	}

	/* Write the ENV data to the OneNAND block */
	puts("Writing to OneNAND...\n");
	ret = onenand->write(onenand, erasebase, writesize, &total, (u_char *) data);
	if (ret || total != erasesize) {
		printf("Could not save enviroment variables\n");
		goto err;
	}

	puts("Saved enviroment variables\n");
	ret = 0;
err:
	free(data);
	return ret;
#else
	printf("OneNAND does not support the saveenv command\n");
	return 1;
#endif
}

int saveenv(void)
{
#if !defined(CONFIG_SMDK6440)
/*	//------------------- To solve the effect which dosen't save a modified enviroment value. 
	if (INF_REG3_REG == 2 || INF_REG3_REG == 3)
	{
		printf("#0-0 saveenv\n");
		saveenv_nand();
	}
*/
#if! defined(CONFIG_MOVINAND)
	printf("INF_REG3_REG : 0x%x\n", INF_REG3_REG);
	if (INF_REG3_REG == 3)
	{
		printf("#0-0 saveenv\n");
		saveenv_nand();
	}
	else if (INF_REG3_REG == 2 || INF_REG3_REG == 4 || INF_REG3_REG == 5 || INF_REG3_REG == 6)
	{
		printf("#0-1 saveenv\n");
		saveenv_nand_adv();
	}
	else if (INF_REG3_REG == 0 || INF_REG3_REG == 7)
	{
		printf("#0-2 saveenv\n");	
		saveenv_movinand();
	}
	else if (INF_REG3_REG == 1)
	{
		printf("#0-3 saveenv\n");
		saveenv_onenand();
	}
	else
		printf("Unknown boot device\n");
#else
		saveenv_movinand();
#endif

#else
	if (INF_REG3_REG == 3)
	{
		printf("#1-0 saveenv\n");
		saveenv_nand();
	}
	else if (INF_REG3_REG == 4 || INF_REG3_REG == 5 || INF_REG3_REG == 6)
	{
		printf("#1-1 saveenv\n");	
		saveenv_nand_adv();
	}
	else if (INF_REG3_REG == 0 || INF_REG3_REG == 1 || INF_REG3_REG == 7)
	{
		printf("#1-2 saveenv\n");		
		saveenv_movinand();
	}
	else
		printf("Unknown boot device\n");
	
#endif

	return 0;
}

#endif /* CMD_SAVEENV */

/*
 * The legacy NAND code saved the environment in the first NAND device i.e.,
 * nand_dev_desc + 0. This is also the behaviour using the new NAND code.
 */
void env_relocate_spec_nand(void)
{
#ifdef CONFIG_NAND
#if !defined(ENV_IS_EMBEDDED)
	size_t total;
	int ret;

	total = CFG_ENV_SIZE;
	ret = nand_read(&nand_info[0], CFG_ENV_OFFSET, &total, (u_char*)env_ptr);
  	if (ret || total != CFG_ENV_SIZE)
		return use_default();

	if (crc32(0, env_ptr->data, ENV_SIZE) != env_ptr->crc)
		return use_default();

#endif /* ! ENV_IS_EMBEDDED */
#endif
}

void env_relocate_spec_movinand(void)
{
#if !defined(ENV_IS_EMBEDDED)
	uint *magic = (uint*)(PHYS_SDRAM_1);

	if ((0x24564236 != magic[0]) || (0x20764316 != magic[1]))
		movi_read_env(virt_to_phys((ulong)env_ptr));
	if (crc32(0, env_ptr->data, ENV_SIZE) != env_ptr->crc)
		return use_default();
#endif /* ! ENV_IS_EMBEDDED */
}

void env_relocate_spec_onenand(void)
{
#if defined(CONFIG_CMD_ONENAND)
#if !defined(ENV_IS_EMBEDDED)
	size_t total;
	int ret;
	struct mtd_info *onenand = &onenand_info[0];

	total = CFG_ENV_SIZE;
	ret = onenand->read(onenand, CFG_ENV_OFFSET, CFG_ENV_SIZE, &total, (u_char*)env_ptr);
	if (ret || total != CFG_ENV_SIZE)
		return use_default();

	if (crc32(0, env_ptr->data, ENV_SIZE) != env_ptr->crc)
		return use_default();
#endif /* ! ENV_IS_EMBEDDED */
#endif
}

void env_relocate_spec(void)
{
#if defined(CONFIG_SMDKC100) || defined(CONFIG_HKDKC100) || defined(CONFIG_JUDKC100)
#if! defined(CONFIG_MOVINAND)
	if (INF_REG3_REG == 1)
		env_relocate_spec_onenand();
	else if (INF_REG3_REG == 2)
		env_relocate_spec_nand();
	else if (INF_REG3_REG == 3)
		env_relocate_spec_movinand();
#else
		env_relocate_spec_movinand();
#endif
#elif !defined(CONFIG_SMDK6440)
	if (INF_REG3_REG >= 2 && INF_REG3_REG <= 6)
		env_relocate_spec_nand();
	else if (INF_REG3_REG == 0 || INF_REG3_REG == 7)
		env_relocate_spec_movinand();
	else if (INF_REG3_REG == 1)
		env_relocate_spec_onenand();
	else
		printf("Unknown boot device\n");
#else
	if (INF_REG3_REG >= 3 && INF_REG3_REG <= 6)
		env_relocate_spec_nand();
	else if (INF_REG3_REG == 0 || INF_REG3_REG == 1 || INF_REG3_REG == 7)
		env_relocate_spec_movinand();
#endif
}

#if !defined(ENV_IS_EMBEDDED)
static void use_default()
{
	puts("*** Warning - using default environment\n\n");

	if (default_environment_size > CFG_ENV_SIZE) {
		puts("*** Error - default environment is too large\n\n");
		return;
	}

	memset (env_ptr, 0, sizeof(env_t));
	memcpy (env_ptr->data,
			default_environment,
			default_environment_size);
	env_ptr->crc = crc32(0, env_ptr->data, ENV_SIZE);
	gd->env_valid = 1;

}
#endif

#endif /* CFG_ENV_IS_IN_NAND */
