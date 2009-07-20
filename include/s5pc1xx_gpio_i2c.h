//[*]----------------------------------------------------------------------------------------------[*]
//
//
// 
//  Aesop s5pc100 gpio i2c driver (charles.park)
//  2009.07.19
//  /driver/i2c/s5pc1xx_gpio_i2c.c
//
//[*]----------------------------------------------------------------------------------------------[*]
#ifndef	_S5PC1XX_GPIO_I2C_H_
#define	_S5PC1XX_GPIO_I2C_H_

//[*]----------------------------------------------------------------------------------------------[*]
extern	void			gpio_i2c_start				(void);
extern	void			gpio_i2c_stop				(void);
                                        	
extern	void			gpio_i2c_send_ack			(void);
extern	void			gpio_i2c_send_noack			(void);
extern	unsigned char	gpio_i2c_chk_ack			(void);
                                        	
extern	void 			gpio_i2c_byte_write			(unsigned char wdata);
extern	void			gpio_i2c_byte_read			(unsigned char *rdata);

//[*]----------------------------------------------------------------------------------------------[*]
#endif	//_S5PC1XX_GPIO_I2C_H_
//[*]----------------------------------------------------------------------------------------------[*]
