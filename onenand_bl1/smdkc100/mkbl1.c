/*
 *	1. Compute checksum for S5PC100 iROM  code
 *	2. Pad 0xFFFFFFFF
 *
 *	djpark (2009.10.13)
 */
#include <stdio.h>

#define BL1_LENGTH			(4*1024)
#define BL1_PAD_LENGTH		BL1_LENGTH 
#define BL1_PAD2_LENGTH		(4*1024)

#define DEFAULT_IN_FILE		"BL1.bin"
#define DEFAULT_OUT_FILE	"BL1.bin.padding"

#define PADDING_DATA		0xFFFFFFFF

//#define DEBUG_MSG

int make_image(char* input_file, char* output_file)
{
	FILE *fp_read = NULL;
	FILE *fp_write = NULL;
	int ret = 0;
	int length = 0;
	unsigned int data;
	unsigned int checksum = 0;
	int i;

	fp_read = fopen(input_file, "rb");
	if (fp_read == NULL)
	{
		printf("File open error! - %s\n", input_file);
		goto err;
	}

	fp_write = fopen(output_file, "wb");
	if (fp_write == NULL)
	{
		printf("File open error! - %s\n", output_file);
		goto err;
	}

	while (ret = fread(&data, sizeof(unsigned int), 1, fp_read))
	{
		length += 4;
		fwrite(&data, sizeof(unsigned int), 1, fp_write);

		checksum += ((data >> 0) & 0xff);
		checksum += ((data >> 8) & 0xff);
		checksum += ((data >> 16) & 0xff);
		checksum += ((data >> 24) & 0xff);
	}

#ifdef DEBUG_MSG
	printf("Input File Length: %d Bytes\n", length);
#endif

	data = PADDING_DATA;
	for (; length < (BL1_LENGTH-4); length += 4)
	{
		fwrite(&data, sizeof(unsigned int), 1, fp_write);

		checksum += ((data >> 0) & 0xff);
		checksum += ((data >> 8) & 0xff);
		checksum += ((data >> 16) & 0xff);
		checksum += ((data >> 24) & 0xff);
	}

#ifdef DEBUG_MSG
	printf("Checksum: 0x%08X\n", checksum);
#endif

	data = PADDING_DATA;
	for (; length < (BL1_PAD_LENGTH-4); length += 4)
	{
		fwrite(&data, sizeof(unsigned int), 1, fp_write);
	}

	data = checksum;
	fwrite(&data, sizeof(unsigned int), 1, fp_write);
	length += 4;

	data = PADDING_DATA;
	for (; length < (BL1_PAD2_LENGTH); length += 4)
	{
		fwrite(&data, sizeof(unsigned int), 1, fp_write);
	}

err:
	if (fp_read != NULL)	fclose(fp_read);
	if (fp_write != NULL)	fclose(fp_write);

	return 0;
}

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		printf("Default input/output files are used!\n");
		return make_image(DEFAULT_IN_FILE, DEFAULT_OUT_FILE);
	}
	else if (argc == 3)
	{
		return make_image(argv[1], argv[2]);
	}
	else
	{
		printf("Error: Unsupported input parameter!\n");
		printf("usage: %s [<input_file>] [<output_file>]\n", argv[0]);
	return 1;
	}
}
