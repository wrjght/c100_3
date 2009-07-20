//[*]----------------------------------------------------------------------------------------------[*]
//
//
// 
//  Aesop s5pc100 gpio i2c driver (charles.park)
//  2009.07.19
//	max8698 control driver
//
//[*]----------------------------------------------------------------------------------------------[*]
#include <common.h>
#include <asm/io.h>
#include <s5pc100.h>

//[*]----------------------------------------------------------------------------------------------[*]
//	GPIO I2C Control function
//[*]----------------------------------------------------------------------------------------------[*]
#include <s5pc1xx_gpio_i2c.h>
#include <max8698.h>

//[*]----------------------------------------------------------------------------------------------[*]
// 	MAX8698 I2C ADDRESS DEFINE
//[*]----------------------------------------------------------------------------------------------[*]
#define	I2C_WRITE_ADDR			0xCC
#define	I2C_READ_ADDR			0xCD

//[*]----------------------------------------------------------------------------------------------[*]
#define	DEBUG_MAX8698			// debug enable flag
#define	DEBUG_MSG(x)			printf(x)

//[*]----------------------------------------------------------------------------------------------[*]
unsigned char max8698_write	(unsigned char reg, unsigned char wdata);
unsigned char max8698_read	(unsigned char reg, unsigned char *rdata);

//[*]----------------------------------------------------------------------------------------------[*]
unsigned char max8698_write	(unsigned char reg, unsigned char wdata)
{
	unsigned char ack;
	
	// start
	gpio_i2c_start();
	
	gpio_i2c_byte_write(I2C_WRITE_ADDR);	// i2c address

	if((ack = gpio_i2c_chk_ack()))	{
		#if defined(DEBUG_MAX8698)
			DEBUG_MSG(("%s [write address] : no ack\n",__FUNCTION__));
		#endif

		goto	write_stop;
	}
	
	gpio_i2c_byte_write(reg);	// register
	
	if((ack = gpio_i2c_chk_ack()))	{
		#if defined(DEBUG_MAX8698)
			DEBUG_MSG(("%s [write register] : no ack\n",__FUNCTION__));
		#endif

		goto	write_stop;
	}

	gpio_i2c_byte_write(wdata);	// value
	
	if((ack = gpio_i2c_chk_ack()))	{
		#if defined(DEBUG_MAX8698)
			DEBUG_MSG(("%s [write data] : no ack\n",__FUNCTION__));
		#endif

		goto	write_stop;
	}

write_stop:
	gpio_i2c_stop();

	return	ack;
}

//[*]----------------------------------------------------------------------------------------------[*]
unsigned char max8698_read	(unsigned char reg, unsigned char *rdata)
{
	unsigned char ack;

	// start
	gpio_i2c_start();
	
	gpio_i2c_byte_write(I2C_WRITE_ADDR);	// i2c address

	if((ack = gpio_i2c_chk_ack()))	{
		#if defined(DEBUG_MAX8698)
			DEBUG_MSG(("%s [write address] : no ack\n",__FUNCTION__));
		#endif

		goto	read_stop;
	}
	
	gpio_i2c_byte_write(reg);	// register
	
	if((ack = gpio_i2c_chk_ack()))	{
		#if defined(DEBUG_MAX8698)
			DEBUG_MSG(("%s [write register] : no ack\n",__FUNCTION__));
		#endif

		goto	read_stop;
	}

	gpio_i2c_stop();

	// restart
	gpio_i2c_start();
	
	gpio_i2c_byte_write(I2C_READ_ADDR);	// i2c address

	if((ack = gpio_i2c_chk_ack()))	{
		#if defined(DEBUG_MAX8698)
			DEBUG_MSG(("%s [read address] : no ack\n",__FUNCTION__));
		#endif

		goto	read_stop;
	}
	
	gpio_i2c_byte_read(*rdata);	// value
	
	gpio_i2c_send_noack();

read_stop:
	gpio_i2c_stop();

	return	ack;
}

//[*]----------------------------------------------------------------------------------------------[*]
//[*]----------------------------------------------------------------------------------------------[*]
