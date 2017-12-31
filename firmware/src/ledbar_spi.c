#include "spi_interface.h"
#include "ledbar_spi.h"

void ledbar_spi_init(void) {
	/*
	 * Initialize SPI hardware
	 * CPHA = 0, data valid on rising edge
	 * CPOL = 1, inverted clock due to hardware inverter (3.3V --> 5V converter)
	 */
	SpiAttr attr;
	attr.mode = SpiMode_Master;
	attr.subMode = SpiSubMode_2;
	attr.speed = SpiSpeed_2MHz;
	attr.bitOrder = SpiBitOrder_MSBFirst;
	SPIInit(SpiNum_HSPI, &attr);

	// Reverse SPI byte order
	SET_PERI_REG_MASK(SPI_USER(SpiNum_HSPI), SPI_WR_BYTE_ORDER);

	/*
	 * Initialize GPIO
	 * GPIO13 (MTCK) = MOSI = ~LEDDAT
	 * GPIO14 (MTMS) = SCK = ~LEDCLK
	 * Setting PERIPHS_IO_MUX to 0x105 effectively clears bit 9 which makes sure
	 * that the SPI clock is not directly derived from the 80MHz CPU clock, but
	 * scaled down to respect the attr.speed setting.
	 */
	WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2);
}

/*
 * The ESP8266 defines SPI transmissions with command and address fields,
 * which we don't really need. Instead, use maximum command and address length and
 * distribute data on these fields as well to make use of maximum hardware FIFO size.
 *
 * Unfortunately the FIFO size is only 2 cmd bytes + 4 addr bytes + 64 data bytes = 70 bytes
 * which is not enough for the LED bar (22 LEDs, LEDBAR_PIXELS). In order to nicely align LED
 * data, we only use 2 bytes for the address.
 */
#define PART1_PIXELS 16
#define ENDFRAME_WORDS 1
void ledbar_spi_send_data_part1(Color *pixels, uint8_t brightness) {
	SpiData spidata;

	// 2 byte command, 2 byte address (start frame) + 64 byte data length
	spidata.cmdLen = 2;
	spidata.addrLen = 2;
	spidata.dataLen = PART1_PIXELS * 4;

	// APA102 start frame, 4 times 0x00 (inverted due to hardware inverter)
	spidata.cmd = ~0x0000;
	uint32_t addr = ~0x00000000;
	spidata.addr = &addr;

	uint32_t data[PART1_PIXELS];
	uint8_t i;
	for (i = 0; i < PART1_PIXELS; ++i) {
		data[i] = 0xe0000000;
		data[i] |= 0x1f000000 & (brightness << 24);
		data[i] |= (pixels[i].b) << 16;
		data[i] |= (pixels[i].g) << 8;
		data[i] |= (pixels[i].r);

		// Invert data bits since MOSI is inverted by hardware inverter
		// (3.3V --> 5V converter)
		data[i] = ~data[i];
	}
	spidata.data = data;

	SPIMasterSendData(SpiNum_HSPI, &spidata);
}

void ledbar_spi_send_data_part2(Color *pixels, uint8_t brightness) {
	SpiData spidata;

	// Don't use command and addr, just put everything into the 64 byte data field
	spidata.cmdLen = 0;
	spidata.addrLen = 0;
	spidata.dataLen = (LEDBAR_PIXELS - PART1_PIXELS + ENDFRAME_WORDS) * 4;

	uint32_t data[LEDBAR_PIXELS - PART1_PIXELS + ENDFRAME_WORDS];
	uint8_t i;
	for (i = PART1_PIXELS; i < LEDBAR_PIXELS; ++i) {
		data[i - PART1_PIXELS] = 0xe0000000;
		data[i - PART1_PIXELS] |= 0x1f000000 & (brightness << 24);
		data[i - PART1_PIXELS] |= (pixels[i].b) << 16;
		data[i - PART1_PIXELS] |= (pixels[i].g) << 8;
		data[i - PART1_PIXELS] |= (pixels[i].r);

		// Invert data bits since MOSI is inverted by hardware inverter
		// (3.3V --> 5V converter)
		data[i - PART1_PIXELS] = ~data[i - PART1_PIXELS];
	}
	spidata.data = data;

	// End frame, ENDFRAME_WORDS times 0xffffffff (inverted due to hardware inverter)
	for (i = LEDBAR_PIXELS - PART1_PIXELS; i < LEDBAR_PIXELS - PART1_PIXELS + ENDFRAME_WORDS; ++i)
		data[i] = ~0xffffffff;

	SPIMasterSendData(SpiNum_HSPI, &spidata);
}
