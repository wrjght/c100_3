//[*]----------------------------------------------------------------------------------------------[*]
//
//
// 
//  Aesop s5pc100 gpio i2c driver (charles.park)
//  2009.07.19
// 
//
//[*]----------------------------------------------------------------------------------------------[*]
#include <common.h>
#include <asm/io.h>
#include <s5pc100.h>

//[*]----------------------------------------------------------------------------------------------[*]
#include "s5pc1xx_gpio_i2c.h"

//[*]----------------------------------------------------------------------------------------------[*]
#define	GPIO_I2C_SDA_CON_PORT	(*(unsigned long *)GPDCON)
#define	GPIO_I2C_SDA_DAT_PORT	(*(unsigned long *)GPDDAT)
#define	GPIO_SDA_PIN			3

#define	GPIO_I2C_CLK_CON_PORT	(*(unsigned long *)GPDCON)
#define	GPIO_I2C_CLK_DAT_PORT	(*(unsigned long *)GPDDAT)
#define	GPIO_CLK_PIN			4

//[*]----------------------------------------------------------------------------------------------[*]
#define	GPIO_CON_PORT_MASK		0xF
#define	GPIO_CON_PORT_OFFSET	0x4

#define	GPIO_CON_INPUT			0x0
#define	GPIO_CON_OUTPUT			0x1

#define	DELAY_TIME				1000	// us value

#define	HIGH					1
#define	LOW						0

#define	DEBUG_GPIO_I2C			// debug enable flag
#define	DEBUG_MSG(x)			printf(x)

//[*]----------------------------------------------------------------------------------------------[*]
//	static function prototype
//[*]----------------------------------------------------------------------------------------------[*]
static	void			gpio_i2c_sda_port_control	(unsigned char inout);
static	void			gpio_i2c_clk_port_control	(unsigned char inout);

static	unsigned char	gpio_i2c_get_sda			(void);
static	void			gpio_i2c_set_sda			(unsigned char hi_lo);
static	void			gpio_i2c_set_clk			(unsigned char hi_lo);
                                        	
//[*]----------------------------------------------------------------------------------------------[*]
//	extern function prototype
//[*]----------------------------------------------------------------------------------------------[*]
void					gpio_i2c_start				(void);
void					gpio_i2c_stop				(void);
                                        	
void					gpio_i2c_send_ack			(void);
void					gpio_i2c_send_noack			(void);
unsigned char			gpio_i2c_chk_ack			(void);
                		                        	
void 					gpio_i2c_byte_write			(unsigned char wdata);
void					gpio_i2c_byte_read			(unsigned char *rdata);
		        		
//[*]----------------------------------------------------------------------------------------------[*]
static	void			gpio_i2c_sda_port_control	(unsigned char inout)
{
	GPIO_I2C_SDA_CON_PORT &=  (unsigned long)(~(GPIO_CON_PORT_MASK << (GPIO_SDA_PIN * GPIO_CON_PORT_OFFSET)));
	GPIO_I2C_SDA_CON_PORT |=  (unsigned long)( (inout << (GPIO_SDA_PIN * GPIO_CON_PORT_OFFSET)));
}

//[*]----------------------------------------------------------------------------------------------[*]
static	void			gpio_i2c_clk_port_control	(unsigned char inout)
{
	GPIO_I2C_CLK_CON_PORT &=  (unsigned long)(~(GPIO_CON_PORT_MASK << (GPIO_CLK_PIN * GPIO_CON_PORT_OFFSET)));
	GPIO_I2C_CLK_CON_PORT |=  (unsigned long)( (inout << (GPIO_CLK_PIN * GPIO_CON_PORT_OFFSET)));
}

//[*]----------------------------------------------------------------------------------------------[*]
static	unsigned char	gpio_i2c_get_sda		(void)
{
	return	GPIO_I2C_SDA_DAT_PORT & (HIGH << GPIO_SDA_PIN) ? 1 : 0;
}

//[*]----------------------------------------------------------------------------------------------[*]
static	void			gpio_i2c_set_sda		(unsigned char hi_lo)
{
	if(hi_lo)	GPIO_I2C_SDA_DAT_PORT |=  (HIGH << GPIO_SDA_PIN);
	else		GPIO_I2C_SDA_DAT_PORT &= ~(HIGH << GPIO_SDA_PIN);
}

//[*]----------------------------------------------------------------------------------------------[*]
static	void			gpio_i2c_set_clk		(unsigned char hi_lo)
{
	if(hi_lo)	GPIO_I2C_CLK_DAT_PORT |=  (HIGH << GPIO_CLK_PIN);
	else		GPIO_I2C_CLK_DAT_PORT &= ~(HIGH << GPIO_CLK_PIN);
}

//[*]----------------------------------------------------------------------------------------------[*]
void			gpio_i2c_start			(void)
{
	gpio_i2c_sda_port_control(GPIO_CON_OUTPUT);
	gpio_i2c_clk_port_control(GPIO_CON_OUTPUT);

	// Setup SDA, CLK output High
	gpio_i2c_set_sda(HIGH);
	gpio_i2c_set_clk(HIGH);
	
	udelay(DELAY_TIME);

	// SDA low before CLK low
	gpio_i2c_set_sda(LOW);	udelay(DELAY_TIME);
	gpio_i2c_set_clk(LOW);	udelay(DELAY_TIME);
}

//[*]----------------------------------------------------------------------------------------------[*]
void			gpio_i2c_stop			(void)
{
	// Setup SDA, CLK output low
	gpio_i2c_set_sda(LOW);
	gpio_i2c_set_clk(LOW);
	
	udelay(DELAY_TIME);
	
	// SDA high after CLK high

	gpio_i2c_set_clk(HIGH);	udelay(DELAY_TIME);
	gpio_i2c_set_sda(HIGH);	udelay(DELAY_TIME);
}

//[*]----------------------------------------------------------------------------------------------[*]
void			gpio_i2c_send_ack		(void)
{
	// SDA Low
	gpio_i2c_set_sda(LOW);	udelay(DELAY_TIME);
	gpio_i2c_set_clk(HIGH);	udelay(DELAY_TIME);
	gpio_i2c_set_clk(LOW);	udelay(DELAY_TIME);
}

//[*]----------------------------------------------------------------------------------------------[*]
void			gpio_i2c_send_noack		(void)
{
	// SDA Low
	gpio_i2c_set_sda(HIGH);	udelay(DELAY_TIME);
	gpio_i2c_set_clk(HIGH);	udelay(DELAY_TIME);
	gpio_i2c_set_clk(LOW);	udelay(DELAY_TIME);
}

//[*]----------------------------------------------------------------------------------------------[*]
unsigned char	gpio_i2c_chk_ack		(void)
{
	unsigned char	count = 0, ret = 0;

	gpio_i2c_set_sda(LOW);							udelay(DELAY_TIME);
	gpio_i2c_sda_port_control(GPIO_CON_INPUT);		udelay(DELAY_TIME);
	gpio_i2c_set_clk(HIGH);							udelay(DELAY_TIME);

	while(gpio_i2c_get_sda())	{
		if(count++ > 100)	{	ret = 1;	break;	}
		else					udelay(DELAY_TIME);	
	}

	gpio_i2c_set_clk(LOW);							udelay(DELAY_TIME);
	gpio_i2c_sda_port_control(GPIO_CON_OUTPUT);		udelay(DELAY_TIME);
	
	#if defined(DEBUG_GPIO_I2C)
		if(ret)		DEBUG_MSG(("%s (%d): no ack!!\n",__FUNCTION__, ret));
		else		DEBUG_MSG(("%s (%d): ack !! \n" ,__FUNCTION__, ret));
	#endif

	return	ret;
}

//[*]----------------------------------------------------------------------------------------------[*]
void 			gpio_i2c_byte_write		(unsigned char wdata)
{
	unsigned char	cnt, mask;
	
	for(cnt = 0, mask = 0x80; cnt < 8; cnt++, mask >>= 1)	{
		if(wdata & mask)		gpio_i2c_set_sda(HIGH);
		else					gpio_i2c_set_sda(LOW);
			
		gpio_i2c_set_clk(HIGH);		udelay(DELAY_TIME);
		gpio_i2c_set_clk(LOW);		udelay(DELAY_TIME);
	}
}

//[*]----------------------------------------------------------------------------------------------[*]
void			gpio_i2c_byte_read		(unsigned char *rdata)
{
	unsigned char	cnt, mask;

	gpio_i2c_sda_port_control(GPIO_CON_INPUT);		udelay(DELAY_TIME);

	for(cnt = 0, mask = 0x80, *rdata = 0; cnt < 8; cnt++, mask >>= 1)	{
		gpio_i2c_set_clk(HIGH);		udelay(DELAY_TIME);
		
		if(gpio_i2c_get_sda())		*rdata |= mask;
		
		gpio_i2c_set_clk(LOW);		udelay(DELAY_TIME);
		
	}
}

//[*]----------------------------------------------------------------------------------------------[*]
//[*]----------------------------------------------------------------------------------------------[*]


