// rf95_reliable_datagram_server.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging server
// with the RHReliableDatagram class, using the RH_RF95 driver to control a RF95 radio.
// It is designed to work with the other example rf95_reliable_datagram_client
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with the RFM95W 

#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

SYSTEM_THREAD(ENABLED)

// Singleton instance of the radio driver
RH_RF95 driver(D6, D2);

// Frequency is typically 868.0 or 915.0 in the Americas, or 433.0 in the EU
float frequency = 868.0;

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, SERVER_ADDRESS);

// Need this on Arduino Zero with SerialUSB port (eg RocketScream Mini Ultra Pro)
//#define Serial SerialUSB

void setup() 
{
	// Rocket Scream Mini Ultra Pro with the RFM95W only:
	// Ensure serial flash is not interfering with radio communication on SPI bus
	//  pinMode(4, OUTPUT);
	//  digitalWrite(4, HIGH);

	Serial.begin(9600);

	// Wait for a USB serial connection for up to 15 seconds
	waitFor(Serial.isConnected, 15000);

	if (!manager.init())
		Serial.println("init failed");

	// Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

	// Setup ISM frequency
	driver.setFrequency(frequency);

	// The default transmitter power is 13dBm, using PA_BOOST.
	// If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
	// you can set transmitter powers from 5 to 23 dBm:
	driver.setTxPower(23, false);

	// If you are using Modtronix inAir4 or inAir9,or any other module which uses the
	// transmitter RFO pins and not the PA_BOOST pins
	// then you can configure the power transmitter power for -1 to 14 dBm and with useRFO true.
	// Failure to do that will result in extremely low transmit powers.
	//  driver.setTxPower(14, true);
	// You can optionally require this module to wait until Channel Activity
	// Detection shows no activity on the channel before transmitting by setting
	// the CAD timeout to non-zero:
	//  driver.setCADTimeout(10000);
}

// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop()
{
	if (manager.available())
	{
		// Wait for a message addressed to us from the client
		uint8_t len = sizeof(buf);
		uint8_t from;
		if (manager.recvfromAck(buf, &len, &from))
		{
			buf[len] = 0;
			Serial.printlnf("got packet from 0x%02x rssi=%d %s", from, driver.lastRssi(), (char *)buf);

			int request = 0;
			char *cp = strchr((char *)buf, '=');
			if (cp) {
				request = atoi(cp + 1);
			}

			snprintf((char *)buf, sizeof(buf), "request=%d rssi=%d", request, driver.lastRssi());

			// Send a reply back to the originator client
			if (!manager.sendtoWait(buf, strlen((char *)buf), from))
				Serial.println("sendtoWait failed");
		}
	}
}

