#include <common.h>
#include <malloc.h>
#include <spi.h>
#include <spi_flash.h>
#include <watchdog.h>
#include "dev_spi_ds31400.h"

static int spi_read_write(struct spi_slave *spi,
				const u8 *cmd, size_t cmd_len,
				const u8 *data_out, u8 *data_in,
				size_t data_len)
{
	unsigned long flags = SPI_XFER_BEGIN;
	int ret;

	if (data_len == 0)
		flags |= SPI_XFER_END;

	ret = spi_xfer(spi, cmd_len * 8, cmd, NULL, flags);
	if (ret) {
		debug("SF: Failed to send command (%zu bytes): %d\n",
				cmd_len, ret);
	} else if (data_len != 0) {
		ret = spi_xfer(spi, data_len * 8, data_out, data_in, SPI_XFER_END);
		if (ret)
			debug("SF: Failed to transfer %zu bytes of data: %d\n",
					data_len, ret);
	}

	return ret;
}


int ds31400_read(struct spi_slave *spi, unsigned short addr, void *data, size_t data_len)
{

	uint8_t buf[2];
	//uint8_t reg;
	//int ret;
	unsigned short address;
	//reg = cmd[0];
  //  buf[0] = ((reg >> 7) & 0x7f) | 0x80; /* bit 7 is set for read */
  //  buf[1] = (reg & 0x7f) << 1; /* bit 0 is clear for single read */
	//cmd = buf;
	uint8_t cmd_len = 2;

	address = (addr << 1) & 0x7ffe;

	buf[0] = (unsigned char)((address >> 8) & 0xff);
	buf[1] = (unsigned char)((address) & 0xff);

	/* MSB must be '1' to read */
	buf[0] |= 0x80;
	/* LSB must be '1' to burst read */
	if (data_len > 1)
		buf[1] |= 0x01;

	return  spi_read_write(spi, buf, cmd_len, NULL, data, data_len);
}

int ds31400_write(struct spi_slave *spi, unsigned short addr,
		const void *data, size_t data_len)
{
	unsigned short address;
	unsigned char buf[2];
	uint8_t cmd_len = 2;
	
	address = (addr << 1) & 0x7ffe;

	buf[0] = (unsigned char)((address >> 8) & 0xff);
	buf[1] = (unsigned char)((address) & 0xff);

	/* MSB must be '0' to write */
	buf[0] &= ~0x80;
	/* LSB must be '1' to burst write */
	if (data_len > 1)
		buf[1] |= 0x01;
	return spi_read_write(spi, buf, cmd_len, data, NULL, data_len);
}


int ds31400_write_byte(struct spi_slave *spi, unsigned short addr, uint8_t data)
{
	size_t data_len = 1;
	return ds31400_write(spi, addr,&data, data_len);
}
struct spi_slave *spi_slave_init(void)
{

	struct spi_slave *slave;
	unsigned int	bus = 0;
	unsigned int	cs = 1;
	unsigned int	mode = SPI_MODE_0;

	slave = spi_setup_slave(bus, cs, 1000000, mode);
	if (!slave) {
		printf("Invalid device %d:%d\n", bus, cs);
		return 1;
	}

	spi_claim_bus(slave);

	return slave;
}


int spi_slave_free(struct spi_slave *slave)
{
	spi_release_bus(slave);
	spi_free_slave(slave);
}


void dpll_init_pre(struct spi_slave *spi)
{
    printf("dpll_init...");
#if 0
    /* reset MCPLL */
    ds31400_write_byte(spi, 0x03DE, 0xF0);
    ds31400_write_byte(spi, 0x03DF, 0x40);
    ds31400_write_byte(spi, 0x03E0, 0x3F);
    ds31400_write_byte(spi, 0x03E1, 0x8F);
    ds31400_write_byte(spi, 0x03E2, 0x80);
    ds31400_write_byte(spi, 0x03E3, 0x08);
    ds31400_write_byte(spi, 0x03E4, 0x98);
    ds31400_write_byte(spi, 0x03E5, 0x04);
    ds31400_write_byte(spi, 0x03E5, 0x00);

    /* Output APLL1 */
    ds31400_write_byte(spi, 0xC0, 0x01);
    /* Enable Noise Shapping */
    ds31400_write_byte(spi, 0xD3, 0x03);
    
    ds31400_write_byte(spi, 0xD4, 0x40);
    ds31400_write_byte(spi, 0xD6, 0x3F);
    ds31400_write_byte(spi, 0xD7, 0x0F);
    ds31400_write_byte(spi, 0xD8, 0x87);
    ds31400_write_byte(spi, 0xD9, 0x08);

    /* Output APLL2 */
    ds31400_write_byte(spi, 0xC0, 0x02);
    /* Enable Noise Shapping */
    ds31400_write_byte(spi, 0xD3, 0x03);
    
    ds31400_write_byte(spi, 0xD4, 0x40);
    ds31400_write_byte(spi, 0xD6, 0x3F);
    ds31400_write_byte(spi, 0xD7, 0x0F);
    ds31400_write_byte(spi, 0xD8, 0x87);
    ds31400_write_byte(spi, 0xD9, 0x08);

    /* Output APLL3 */
    ds31400_write_byte(spi, 0xC0, 0x03);
    /* Enable Noise Shapping */
    ds31400_write_byte(spi, 0xD3, 0x03);
    
    ds31400_write_byte(spi, 0xD4, 0x40);
    ds31400_write_byte(spi, 0xD6, 0x3F);
    ds31400_write_byte(spi, 0xD7, 0x0F);
    ds31400_write_byte(spi, 0xD8, 0x87);
    ds31400_write_byte(spi, 0xD9, 0x08);

    /* Configure the DS31400 to use a 19.44MHz MCLKOSC
       Pins OSCFREQ[2:0] must be configured to 100  */
    ds31400_write_byte(spi, 0x03DD, 0x83);
    ds31400_write_byte(spi, 0x03E1, 0x0F);
    ds31400_write_byte(spi, 0x03E4, 0x18);
    ds31400_write_byte(spi, 0x03DE, 0x4F);
    ds31400_write_byte(spi, 0x03E5, 0x04);
    ds31400_write_byte(spi, 0x03E5, 0x00);


/************************************************************/
/*
  ;-----------------------------------
  ; Begin OC1 Register Bank
  ;-----------------------------------
*/
/*    
  

*/
    /*new 
	./ds31400_write_byte(spi,  00C0	01
	./ds31400_write_byte(spi,  00C1	C0
	./ds31400_write_byte(spi,  00C3 	80		                       
	./ds31400_write_byte(spi,  00C4	80	
	./ds31400_write_byte(spi,  00C8	01

	./ds31400_write_byte(spi,  00C9	00
	./ds31400_write_byte(spi,  00CA	00
	./ds31400_write_byte(spi,  00CB	00
*/
    ds31400_write_byte(spi, 0x00C0, 0x01);
    ds31400_write_byte(spi, 0x00C1, 0xc0);
    ds31400_write_byte(spi, 0x00C3, 0x80);
    ds31400_write_byte(spi, 0x00C4, 0x80);
    ds31400_write_byte(spi, 0x00C8, 0x01);

    ds31400_write_byte(spi, 0x00C9, 0x00);
    ds31400_write_byte(spi, 0x00CA, 0x00);
    ds31400_write_byte(spi, 0x00CB, 0x00);
    
/*    
    
        ./ds31400_write_byte(spi,  00D0	88
	./ds31400_write_byte(spi,  00D2	13
	./ds31400_write_byte(spi,  00D8	02

	./ds31400_write_byte(spi,  00E0	00
	./ds31400_write_byte(spi,  00E1	00
	./ds31400_write_byte(spi,  00E2	00
	./ds31400_write_byte(spi,  00E3	00
	./ds31400_write_byte(spi,  00E4	00
	./ds31400_write_byte(spi,  00E5	00
	./ds31400_write_byte(spi,  00E6	00
	./ds31400_write_byte(spi,  00E7	00
	./ds31400_write_byte(spi,  00E8	00
	./ds31400_write_byte(spi,  00E9	50

	./ds31400_write_byte(spi,  00F2	2C
*/


    ds31400_write_byte(spi, 0x00D0, 0x88);
    ds31400_write_byte(spi, 0x00D2, 0x13);
    ds31400_write_byte(spi, 0x00D8, 0x02);

    ds31400_write_byte(spi, 0x00E0, 0x00);
    ds31400_write_byte(spi, 0x00E1, 0x00);
    ds31400_write_byte(spi, 0x00E2, 0x00);
    ds31400_write_byte(spi, 0x00E3, 0x00);
    ds31400_write_byte(spi, 0x00E4, 0x00);
    ds31400_write_byte(spi, 0x00E5, 0x00);
    ds31400_write_byte(spi, 0x00E6, 0x00);
    ds31400_write_byte(spi, 0x00E7, 0x00);
    ds31400_write_byte(spi, 0x00E8, 0x00);
    ds31400_write_byte(spi, 0x00E9, 0x50);

    ds31400_write_byte(spi, 0x00F2, 0x2C);




/*
  ;-----------------------------------
  ; Begin OC2Register Bank
  ;-----------------------------------
*/
/*    
  

*/
    /*new 
	./ds31400_write_byte(spi,  00C0	01
	./ds31400_write_byte(spi,  00C1	C0
	./ds31400_write_byte(spi,  00C3 	80		                       
	./ds31400_write_byte(spi,  00C4	80	
	./ds31400_write_byte(spi,  00C8	01

	./ds31400_write_byte(spi,  00C9	00
	./ds31400_write_byte(spi,  00CA	00
	./ds31400_write_byte(spi,  00CB	00
*/
    ds31400_write_byte(spi, 0x00C0, 0x02);
    ds31400_write_byte(spi, 0x00C1, 0xc0);
    ds31400_write_byte(spi, 0x00C3, 0x80);
    ds31400_write_byte(spi, 0x00C4, 0x80);
    ds31400_write_byte(spi, 0x00C8, 0x01);

    ds31400_write_byte(spi, 0x00C9, 0x00);
    ds31400_write_byte(spi, 0x00CA, 0x00);
    ds31400_write_byte(spi, 0x00CB, 0x00);
    
/*    
    
        ./ds31400_write_byte(spi,  00D0	88
	./ds31400_write_byte(spi,  00D2	13
	./ds31400_write_byte(spi,  00D8	02

	./ds31400_write_byte(spi,  00E0	00
	./ds31400_write_byte(spi,  00E1	00
	./ds31400_write_byte(spi,  00E2	00
	./ds31400_write_byte(spi,  00E3	00
	./ds31400_write_byte(spi,  00E4	00
	./ds31400_write_byte(spi,  00E5	00
	./ds31400_write_byte(spi,  00E6	00
	./ds31400_write_byte(spi,  00E7	00
	./ds31400_write_byte(spi,  00E8	00
	./ds31400_write_byte(spi,  00E9	50

	./ds31400_write_byte(spi,  00F2	2C
*/


    ds31400_write_byte(spi, 0x00D0, 0x88);
    ds31400_write_byte(spi, 0x00D2, 0x13);
    ds31400_write_byte(spi, 0x00D8, 0x02);

    ds31400_write_byte(spi, 0x00E0, 0x00);
    ds31400_write_byte(spi, 0x00E1, 0x00);
    ds31400_write_byte(spi, 0x00E2, 0x00);
    ds31400_write_byte(spi, 0x00E3, 0x00);
    ds31400_write_byte(spi, 0x00E4, 0x00);
    ds31400_write_byte(spi, 0x00E5, 0x00);
    ds31400_write_byte(spi, 0x00E6, 0x00);
    ds31400_write_byte(spi, 0x00E7, 0x00);
    ds31400_write_byte(spi, 0x00E8, 0x00);
    ds31400_write_byte(spi, 0x00E9, 0x50);

    ds31400_write_byte(spi, 0x00F2, 0x2C);
#else 

    ds31400_write_byte(spi, 0x0006, 0x80);
    udelay(1000000);
    ds31400_write_byte(spi, 0x0006, 0x00);
/*#reset MCPLL*/
    ds31400_write_byte(spi, 0x03DE, 0xF0);
    ds31400_write_byte(spi, 0x03DF, 0x40);
    ds31400_write_byte(spi, 0x03E0, 0x3F);
    ds31400_write_byte(spi, 0x03E1, 0x8F);
    ds31400_write_byte(spi, 0x03E2, 0x80);
    ds31400_write_byte(spi, 0x03E3, 0x08);
    ds31400_write_byte(spi, 0x03E4, 0x98);
    ds31400_write_byte(spi, 0x03E5, 0x04);
    ds31400_write_byte(spi, 0x03E5, 0x00);

/*# Output APLL1*/
    ds31400_write_byte(spi, 0x00C0, 0x01);
/*# Enable Noise Shapping*/
    ds31400_write_byte(spi, 0x00D3, 0x03);
    ds31400_write_byte(spi, 0x00D4, 0x40);
    ds31400_write_byte(spi, 0x00D6, 0x3F);
    ds31400_write_byte(spi, 0x00D7, 0x0F);
    ds31400_write_byte(spi, 0x00D8, 0x87);
    ds31400_write_byte(spi, 0x00D9, 0x08);

/*# Output APLL2*/
    ds31400_write_byte(spi, 0x00C0, 0x02);
/*# Enable Noise Shapping*/
    ds31400_write_byte(spi, 0x00D3, 0x03);
    ds31400_write_byte(spi, 0x00D4, 0x40);
    ds31400_write_byte(spi, 0x00D6, 0x3F);
    ds31400_write_byte(spi, 0x00D7, 0x0F);
    ds31400_write_byte(spi, 0x00D8, 0x87);
    ds31400_write_byte(spi, 0x00D9, 0x08);

/*# Output APLL3*/
    ds31400_write_byte(spi, 0x00C0, 0x03);
    
/*# Enable Noise Shapping*/
    ds31400_write_byte(spi, 0x00D3, 0x03);
    ds31400_write_byte(spi, 0x00D4, 0x40);
    ds31400_write_byte(spi, 0x00D6, 0x3F);
    ds31400_write_byte(spi, 0x00D7, 0x0F);
    ds31400_write_byte(spi, 0x00D8, 0x87);
    ds31400_write_byte(spi, 0x00D9, 0x08);

/*## Configure the DS31400 to use a 19.44MHz MCLKOSC
# Pins OSCFREQ[2:0] must be configured to 100*/
    ds31400_write_byte(spi, 0x03DD, 0x83);
    ds31400_write_byte(spi, 0x03E1, 0x0F);
    ds31400_write_byte(spi, 0x03E4, 0x18);
    ds31400_write_byte(spi, 0x03DE, 0x4F);
    ds31400_write_byte(spi, 0x03E5, 0x04);
    ds31400_write_byte(spi, 0x03E5, 0x00);
    
/*# init priority to 0 (disable) ic7 = 1 highest*/
    ds31400_write_byte(spi, 0x0080, 0x00);

/*# dpll 2 - force select local clock ic3*/
    ds31400_write_byte(spi, 0x0080, 0x01);
    ds31400_write_byte(spi, 0x0081, 0x03);

/*# dpll 1 - force select local clock ic3*/
    ds31400_write_byte(spi, 0x0080, 0x00);
    ds31400_write_byte(spi, 0x0081, 0x03);
  
/*# ic1  64k*/
    ds31400_write_byte(spi, 0x0060, 0x01);
    ds31400_write_byte(spi, 0x0061, 0x82);
    ds31400_write_byte(spi, 0x0062, 0x01);
	
/*# ic2  64k*/
    ds31400_write_byte(spi, 0x0060, 0x02);
    ds31400_write_byte(spi, 0x0061, 0x82);
    ds31400_write_byte(spi, 0x0062, 0x01);

/*##ic3 ic4 5 6 7  156.25M*/
    ds31400_write_byte(spi, 0x0060, 0x03);
    ds31400_write_byte(spi, 0x0061, 0x9C);
    ds31400_write_byte(spi, 0x0062, 0x01);
    ds31400_write_byte(spi, 0x0064, 0x00);
    ds31400_write_byte(spi, 0x0065, 0x00);
    ds31400_write_byte(spi, 0x0066, 0x04);
    ds31400_write_byte(spi, 0x0067, 0x00);
    ds31400_write_byte(spi, 0x0068, 0x00);
    ds31400_write_byte(spi, 0x0069, 0x00);
    
    ds31400_write_byte(spi, 0x0060, 0x04);
    ds31400_write_byte(spi, 0x0061, 0x9C);
    ds31400_write_byte(spi, 0x0062, 0x01);
    ds31400_write_byte(spi, 0x0064, 0x00);
    ds31400_write_byte(spi, 0x0065, 0x00);
    ds31400_write_byte(spi, 0x0066, 0x04);
    ds31400_write_byte(spi, 0x0067, 0x00);
    ds31400_write_byte(spi, 0x0068, 0x00);
    ds31400_write_byte(spi, 0x0069, 0x00);
    
    ds31400_write_byte(spi, 0x0060, 0x05);
    ds31400_write_byte(spi, 0x0061, 0x9C);
    ds31400_write_byte(spi, 0x0062, 0x01);
    ds31400_write_byte(spi, 0x0064, 0x00);
    ds31400_write_byte(spi, 0x0065, 0x00);
    ds31400_write_byte(spi, 0x0066, 0x04);
    ds31400_write_byte(spi, 0x0067, 0x00);
    ds31400_write_byte(spi, 0x0068, 0x00);
    ds31400_write_byte(spi, 0x0069, 0x00);
    
    ds31400_write_byte(spi, 0x0060, 0x06);
    ds31400_write_byte(spi, 0x0061, 0x9C);
    ds31400_write_byte(spi, 0x0062, 0x01);
    ds31400_write_byte(spi, 0x0064, 0x00);
    ds31400_write_byte(spi, 0x0065, 0x00);
    ds31400_write_byte(spi, 0x0066, 0x04);
    ds31400_write_byte(spi, 0x0067, 0x00);
    ds31400_write_byte(spi, 0x0068, 0x00);
    ds31400_write_byte(spi, 0x0069, 0x00);

/*##ic7 for 2000 is 64K*/
    ds31400_write_byte(spi, 0x0060, 0x07);
    ds31400_write_byte(spi, 0x0061, 0x82);
    ds31400_write_byte(spi, 0x0062, 0x01);

/*##ic8 64k*/
    ds31400_write_byte(spi, 0x0060, 0x08);
    ds31400_write_byte(spi, 0x0061, 0x82);
    ds31400_write_byte(spi, 0x0062, 0x01);

/*# oc1 cmos 25MHz, cml 25MHz*/
    ds31400_write_byte(spi, 0x00C0, 0x01);
    ds31400_write_byte(spi, 0x00C1, 0xC1);
    ds31400_write_byte(spi, 0x00C2, 0x10);
    ds31400_write_byte(spi, 0x00C3, 0x80);
    ds31400_write_byte(spi, 0x00C4, 0x80);
    ds31400_write_byte(spi, 0x00C8, 0x00);
    ds31400_write_byte(spi, 0x00C9, 0x00);
    ds31400_write_byte(spi, 0x00CA, 0x00);
    ds31400_write_byte(spi, 0x00CB, 0x00);
    ds31400_write_byte(spi, 0x00D0, 0x98);
    ds31400_write_byte(spi, 0x00D1, 0x13);
    ds31400_write_byte(spi, 0x00D2, 0x13);
    ds31400_write_byte(spi, 0x00D8, 0x02);
    ds31400_write_byte(spi, 0x00E0, 0x00);
    ds31400_write_byte(spi, 0x00E1, 0x00);
    ds31400_write_byte(spi, 0x00E2, 0x00);
    ds31400_write_byte(spi, 0x00E3, 0x00);
    ds31400_write_byte(spi, 0x00E4, 0x00);
    ds31400_write_byte(spi, 0x00E5, 0x00);
    ds31400_write_byte(spi, 0x00E6, 0x00);
    ds31400_write_byte(spi, 0x00E7, 0x00);
    ds31400_write_byte(spi, 0x00E8, 0x00);
    ds31400_write_byte(spi, 0x00E9, 0x50);
    ds31400_write_byte(spi, 0x00F2, 0x2C);

/*# oc2 cmos 25MHz, cml 25MHz */
    ds31400_write_byte(spi, 0x00C0, 0x02);
    ds31400_write_byte(spi, 0x00C1, 0xC1);
    ds31400_write_byte(spi, 0x00C2, 0x10);
    ds31400_write_byte(spi, 0x00C3, 0x80);
    ds31400_write_byte(spi, 0x00C4, 0x80);
    ds31400_write_byte(spi, 0x00C8, 0x00);
    ds31400_write_byte(spi, 0x00C9, 0x00);
    ds31400_write_byte(spi, 0x00CA, 0x00);
    ds31400_write_byte(spi, 0x00CB, 0x00);
    ds31400_write_byte(spi, 0x00D0, 0x98);
    ds31400_write_byte(spi, 0x00D1, 0x13);
    ds31400_write_byte(spi, 0x00D2, 0x13);
    ds31400_write_byte(spi, 0x00D8, 0x02);
    ds31400_write_byte(spi, 0x00E0, 0x00);
    ds31400_write_byte(spi, 0x00E1, 0x00);
    ds31400_write_byte(spi, 0x00E2, 0x00);
    ds31400_write_byte(spi, 0x00E3, 0x00);
    ds31400_write_byte(spi, 0x00E4, 0x00);
    ds31400_write_byte(spi, 0x00E5, 0x00);
    ds31400_write_byte(spi, 0x00E6, 0x00);
    ds31400_write_byte(spi, 0x00E7, 0x00);
    ds31400_write_byte(spi, 0x00E8, 0x00);
    ds31400_write_byte(spi, 0x00E9, 0x50);
	 
/*# oc3 cmos 25MHz, cml 25MHz */
    ds31400_write_byte(spi, 0x00C0, 0x03);
    ds31400_write_byte(spi, 0x00C1, 0xC1);
    ds31400_write_byte(spi, 0x00C2, 0x10);
    ds31400_write_byte(spi, 0x00C3, 0x80);
    ds31400_write_byte(spi, 0x00C4, 0x80);
    ds31400_write_byte(spi, 0x00C8, 0x00);
    ds31400_write_byte(spi, 0x00C9, 0x00);
    ds31400_write_byte(spi, 0x00CA, 0x00);
    ds31400_write_byte(spi, 0x00CB, 0x00);
    ds31400_write_byte(spi, 0x00D0, 0x98);
    ds31400_write_byte(spi, 0x00D1, 0x13);
    ds31400_write_byte(spi, 0x00D2, 0x13);
    ds31400_write_byte(spi, 0x00D8, 0x02);
    ds31400_write_byte(spi, 0x00E0, 0x00);
    ds31400_write_byte(spi, 0x00E1, 0x00);
    ds31400_write_byte(spi, 0x00E2, 0x00);
    ds31400_write_byte(spi, 0x00E3, 0x00);
    ds31400_write_byte(spi, 0x00E4, 0x00);
    ds31400_write_byte(spi, 0x00E5, 0x00);
    ds31400_write_byte(spi, 0x00E6, 0x00);
    ds31400_write_byte(spi, 0x00E7, 0x00);
    ds31400_write_byte(spi, 0x00E8, 0x00);
    ds31400_write_byte(spi, 0x00E9, 0x50);

/*# Begin OC4 Register Bank  63.536 k*/
    ds31400_write_byte(spi, 0x00C0, 0x04);
    ds31400_write_byte(spi, 0x00B0, 0x74);
    ds31400_write_byte(spi, 0x00B1, 0x9B);
    ds31400_write_byte(spi, 0x00B2, 0xD5);
    ds31400_write_byte(spi, 0x00B3, 0xE9);
    ds31400_write_byte(spi, 0x00B4, 0x51);
    ds31400_write_byte(spi, 0x00B5, 0x44);
    ds31400_write_byte(spi, 0x00B6, 0x83);
    ds31400_write_byte(spi, 0x00B7, 0xAF);
    ds31400_write_byte(spi, 0x00B8, 0x01);
    ds31400_write_byte(spi, 0x00BC, 0xFF);
    ds31400_write_byte(spi, 0x00BD, 0x7F);
    ds31400_write_byte(spi, 0x00BE, 0x7D);
    ds31400_write_byte(spi, 0x00C1, 0x40);
    ds31400_write_byte(spi, 0x00C3, 0x80);
    ds31400_write_byte(spi, 0x00C8, 0xE7);
    ds31400_write_byte(spi, 0x00C9, 0x03);
    ds31400_write_byte(spi, 0x00CA, 0x00);
    ds31400_write_byte(spi, 0x00CB, 0x00);
    ds31400_write_byte(spi, 0x00D3, 0x5C);

/*# oc5 cmos and cml 25MHz */
    ds31400_write_byte(spi, 0x00C0, 0x05);
    ds31400_write_byte(spi, 0x00C1, 0xC0);
    ds31400_write_byte(spi, 0x00C3, 0x80);
    ds31400_write_byte(spi, 0x00C4, 0x80);
    ds31400_write_byte(spi, 0x00C8, 0x01);
    ds31400_write_byte(spi, 0x00C9, 0x00);
    ds31400_write_byte(spi, 0x00CA, 0x00);
    ds31400_write_byte(spi, 0x00CB, 0x00);
    ds31400_write_byte(spi, 0x00CC, 0x01);
    ds31400_write_byte(spi, 0x00CD, 0x00);
    ds31400_write_byte(spi, 0x00CE, 0x00);
    ds31400_write_byte(spi, 0x00CF, 0x00);

/*# Begin OC6 Register Bank 1hz*/
    ds31400_write_byte(spi, 0x00C0, 0x06);
    ds31400_write_byte(spi, 0x00C1, 0xC0);
    ds31400_write_byte(spi, 0x00C3, 0x80);
    ds31400_write_byte(spi, 0x00C8, 0x7F);
    ds31400_write_byte(spi, 0x00C9, 0xF0);
    ds31400_write_byte(spi, 0x00CA, 0xFA);
    ds31400_write_byte(spi, 0x00CB, 0x02);
/*# End OC6 Register Bank

# Begin OC7 Register Bank  63.536 k*/
    ds31400_write_byte(spi, 0x00C0, 0x07);
    ds31400_write_byte(spi, 0x00B0, 0x74);
    ds31400_write_byte(spi, 0x00B1, 0x9B);
    ds31400_write_byte(spi, 0x00B2, 0xD5);
    ds31400_write_byte(spi, 0x00B3, 0xE9);
    ds31400_write_byte(spi, 0x00B4, 0x51);
    ds31400_write_byte(spi, 0x00B5, 0x44);
    ds31400_write_byte(spi, 0x00B6, 0x83);
    ds31400_write_byte(spi, 0x00B7, 0xAF);
    ds31400_write_byte(spi, 0x00B8, 0x01);
    ds31400_write_byte(spi, 0x00BC, 0xFF);
    ds31400_write_byte(spi, 0x00BD, 0x7F);
    ds31400_write_byte(spi, 0x00BE, 0x7D);
    ds31400_write_byte(spi, 0x00C1, 0x40);
    ds31400_write_byte(spi, 0x00C3, 0x80);
    ds31400_write_byte(spi, 0x00C8, 0xE7);
    ds31400_write_byte(spi, 0x00C9, 0x03);
    ds31400_write_byte(spi, 0x00CA, 0x00);
    ds31400_write_byte(spi, 0x00CB, 0x00);
    ds31400_write_byte(spi, 0x00D3, 0x5C);
/*# End OC7 Register Bank*/
    ds31400_write_byte(spi, 0x03E5, 0x04);
    ds31400_write_byte(spi, 0x03E5, 0x00);

/*# relock apll 1*/
    ds31400_write_byte(spi, 0x00C0, 0x01);
    ds31400_write_byte(spi, 0x00D0, 0x18);
    ds31400_write_byte(spi, 0x00D0, 0x98);
/*# relock apll 2 */
    ds31400_write_byte(spi, 0x00C0, 0x02);
    ds31400_write_byte(spi, 0x00D0, 0x18);
    ds31400_write_byte(spi, 0x00D0, 0x98);
/*# relock apll 3*/ 
    ds31400_write_byte(spi, 0x00C0, 0x03);
    ds31400_write_byte(spi, 0x00D0, 0x18);
    ds31400_write_byte(spi, 0x00D0, 0x98);
/*# lock the dpll register*/

#endif
    printf("done!\n\r");
}


