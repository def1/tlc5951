#include "tlc5951.h"
#include <SPI.h>

#define bit_set(p,m) ((p) |= (m)) 
#define bit_clear(p,m) ((p) &= ~(m)) 
#define bit_write(c,p,m) (c ? bit_set(p,m) : bit_clear(p,m)) 

void TLC5951::init(int gssin, int gssck, int gslat, int xblnk, int gsckrgb) {
	_gssin = gssin;
	_gssck = gssck;
	_gslat = gslat;
	_xblnk = xblnk;
	_gsckrgb = gsckrgb;
	
	_bufferCount = 0;
	
	pinMode(_gssin, OUTPUT);
	pinMode(_gssck, OUTPUT);
	pinMode(_gslat, OUTPUT);
	pinMode(_xblnk, OUTPUT);
	pinMode(_gsckrgb, OUTPUT);
	
	digitalWrite(_gssin, LOW);
	digitalWrite(_gssck, LOW);
	digitalWrite(_gslat, LOW);
	digitalWrite(_xblnk, LOW);
	analogWrite(_gsckrgb, 127);
	
	SPI.begin();
    SPI.setBitOrder(LSBFIRST);
	SPI.setClockDivider(SPI_CLOCK_DIV2);
}

void TLC5951::init(int gssin, int gssck, int gslat, int xblnk, int gsckrgb, int grayscale) {
	_gssin = gssin;
	_gssck = gssck;
	_gslat = gslat;
	_xblnk = xblnk;
	_gsckrgb = gsckrgb;
	
	_bufferCount = 0;
	_debug = 0;
	
	pinMode(_gssin, OUTPUT);
	pinMode(_gssck, OUTPUT);
	pinMode(_gslat, OUTPUT);
	pinMode(_xblnk, OUTPUT);
	pinMode(_gsckrgb, OUTPUT);
	
	digitalWrite(_gssin, LOW);
	digitalWrite(_gssck, LOW);
	digitalWrite(_gslat, LOW);
	digitalWrite(_xblnk, LOW);
	analogWrite(_gsckrgb, 127);
	
	setAllGSData(grayscale);
	
	SPI.begin();
    SPI.setBitOrder(LSBFIRST);
	SPI.setClockDivider(SPI_CLOCK_DIV2);
}

void TLC5951::setAllGSData(int gsvalue) {
	for(int a = 0; a < 8; a++) {
		for(int b = 0; b < 3; b++) {
			_gsData[a][b] = gsvalue;
		}
	}
}

void TLC5951::updateGS() {
	_bufferCount = 0;
	Serial.println("GS Begin");
	
	digitalWrite(_xblnk, LOW); // Turn off the LED's since we're clocking in data
	digitalWrite(_gslat, LOW); // GS Latch low, so it goes into the GS data latch
	
	for(int a = 7; a >= 0; a--) { // We have 8 LED's. Start at the last since thats how we clock data out
		for(int b = 2; b >= 0; b--) { // Each with 3 colors
			for(int c = 11; c >= 0; c--) { // each consiting of 12 bits
				setBuffer((_gsData[a][b] & (1<<c)));
			}
		}
	}
	
	delay(1);
	
	digitalWrite(_gslat, HIGH);
	digitalWrite(_gslat, LOW);
	
	digitalWrite(_xblnk, HIGH);
	Serial.println("GS END");
}

void TLC5951::setLED(unsigned int led, unsigned int red, unsigned int green, unsigned int blue) {
	_gsData[led][2] = blue;
	_gsData[led][1] = green;
	_gsData[led][0] = red;
}

void TLC5951::setFunctionData(byte data) {
	_functionData = data;
}

void TLC5951::setBrightness(byte red, byte green, byte blue) {
	_brightRed = red;
	_brightGreen = green;
	_brightBlue = blue;
}

void TLC5951::setAllDCData(int dcvalue) {
	for(int a = 0; a < 8; a++) {
		for(int b = 0; b < 3; b++) {
			_dcData[a][b] = dcvalue;
		}
	}
}


void TLC5951::updateControl() {
	_bufferCount = 0;
	
	digitalWrite(_xblnk, LOW);
	digitalWrite(_gslat, HIGH);	
		
	// 89 blank bits to get to correct position for DC/DC/FC (Disregard UD) data
	digitalWrite(_gssin, LOW);
	for(int a = 0; a < 89; a++) {
		setBuffer(0);
	}
	
	// 7-bit Function Data
	for(int a = 6; a >= 0; a--) {
		setBuffer((_functionData & (1<<a)));
	}
	
	// Blue Brightness
	for(int a = 7; a >= 0; a--) {
		setBuffer((_brightBlue & (1<<a)));
	}
	
	// Green Brightness
	for(int a = 7; a >= 0; a--) {
		setBuffer((_brightGreen & (1<<a)));
	}
	
	// Red Brightness
	for(int a = 7; a >= 0; a--) {
		setBuffer((_brightRed & (1<<a)));
	}
	
	// Dot Correctness data
	for(int a = 7; a >= 0; a--) {
		for(int b = 2; b >= 0; b--) {
			for(int c = 6; c >= 0; c--) {
				setBuffer(_dcData[a][b] & (1<<c));
			}
		}
	}
	
	digitalWrite(_gslat, LOW);
	delayMicroseconds(10);
	
	digitalWrite(_gslat, HIGH);
	digitalWrite(_gslat, LOW);
	
	digitalWrite(_xblnk, HIGH);		
}

void TLC5951::update() {
	updateControl();
	updateGS();
}

void TLC5951::setBuffer(bool bit){
	bitWrite(_buffer, _bufferCount, bit);
	Serial.println(bit);
	_bufferCount++;
	if(_bufferCount == 8)
	{
		SPI.transfer(_buffer);
		_debug++;
		Serial.print("Debug: ");
		Serial.print(_debug);
		Serial.print("Buffer: ");
		Serial.println(_buffer);
		_bufferCount = 0;
	}
}
