#define SHIFT_REGISTER DDRB
#define SHIFT_PORT PORTB
#define DATA (1<<DDRB5)		//MOSI (SI)
#define LATCH (1<<DDRB4)		//SS   (RCK)
#define CLOCK (1<<DDRB7)		//SCK  (SCK)

void init_IO(void){
	SHIFT_REGISTER |= (DATA | LATCH | CLOCK);	//Set control pins as outputs
	SHIFT_PORT &= ~(DATA | LATCH | CLOCK);		//Set control pins low
}

void init_SPI(void){
	//Setup SPI
	SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0);	//Start SPI as Master
}

void spi_send(unsigned char byte){
	SPDR = byte;			//Shift in some data
	while(!(SPSR & (1<<SPIF)));
}


//display all colors at once
void update_display(unsigned char green_data, unsigned char blue_data, unsigned char red_data, unsigned char i) {
	SHIFT_PORT &= ~LATCH;
	spi_send(0x01 << i);
	spi_send(~green_data);
	spi_send(~blue_data);
	spi_send(~red_data);
	SHIFT_PORT |= LATCH;
}


// The below can be used together to display stuff like yellow but updating one at a time on my LED.  I ended up not using it.  I believe before I accidentally
// connected my LED directly to 5v and damaged it that the above code worked to display all 7 colors, but ever since that accident the LED matrix hasn't been the same.

void update_green(unsigned char green_data, unsigned char i) {
	SHIFT_PORT &= ~LATCH;
	spi_send(0x01 << i);
	spi_send(~green_data);
	spi_send(0xFF);
	spi_send(0xFF);
	SHIFT_PORT |= LATCH;
}

void update_red(unsigned char red_data, unsigned char i) {
	SHIFT_PORT &= ~LATCH;
	spi_send(0x01 << i);
	spi_send(0xFF);
	spi_send(0xFF);
	spi_send(~red_data);
	SHIFT_PORT |= LATCH;
}


void update_blue(unsigned char blue_data, unsigned char i) {
	SHIFT_PORT &= ~LATCH;
	spi_send(0x01 << i);
	spi_send(0xFF);
	spi_send(~blue_data);
	spi_send(0xFF);
	SHIFT_PORT |= LATCH;
}



