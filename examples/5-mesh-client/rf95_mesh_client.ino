// rf95_mesh_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, routed reliable messaging client
// with the RHMesh class.
// It is designed to work with the other examples rf22_mesh_server*
// Hint: you can simulate other network topologies by setting the 
// RH_TEST_NETWORK define in RHRouter.h

// Mesh has much greater memory requirements, and you may need to limit the
// max message length to prevent wierd crashes
#define RH_MESH_MAX_MESSAGE_LEN 50

#include <RHMesh.h>
#include <RH_RF95.h>
#include <SPI.h>

// In this small artifical network of 4 nodes,
#define CLIENT_ADDRESS 1
#define SERVER1_ADDRESS 2
#define SERVER2_ADDRESS 3
#define SERVER3_ADDRESS 4

SYSTEM_MODE(MANUAL);

//Define pins for the RFM9x:
#define RFM95_CS D4
#define RFM95_RST D3
#define RFM95_INT D2
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
// RH_RF95 driver(D6, D2);
RH_RF95 driver(RFM95_CS, RFM95_INT);


// Class to manage message delivery and receipt, using the driver declared above
RHMesh manager(driver, CLIENT_ADDRESS);

// Need this on Arduino Zero with SerialUSB port (eg RocketScream Mini Ultra Pro)
//#define Serial SerialUSB

void setup() 
{
	Serial.begin(9600);

	// Wait for a USB serial connection for up to 15 seconds
	waitFor(Serial.isConnected, 15000);

	if (!manager.init())
		Serial.println("init failed");
	// Defaults after init are 434.0MHz, 0.05MHz AFC pull-in, modulation FSK_Rb2_4Fd36

	// Setup ISM frequency
  // Frequency is typically 868.0 or 915.0 in the Americas, or 433.0 in the EU
	driver.setFrequency(RF95_FREQ);

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

uint8_t data[] = "Hello World!";
// Dont put this on the stack:
uint8_t buf[RH_MESH_MAX_MESSAGE_LEN];

void loop()
{
	Serial.println("Sending to manager_mesh_server1");

	// Send a message to manager_server
  // A route to the destination will be automatically discovered.
	 if (manager.sendtoWait(data, sizeof(data), SERVER1_ADDRESS) == RH_ROUTER_ERROR_NONE)
	{
    // It has been reliably delivered to the next node.
    // Now wait for a reply from the ultimate server
    uint8_t len = sizeof(buf);
    uint8_t from;     
    if (manager.recvfromAckTimeout(buf, &len, 3000, &from))
    {
			buf[len] = 0;
			Serial.printlnf("got reply from 0x%02x rssi=%d %s", from, driver.lastRssi(), (char *) buf);
		}
		else
		{
			Serial.println("No reply, is rf95_mesh_server1, rf95_mesh_server2 and rf95_mesh_server3 running?");
		}
	}
	else
		Serial.println("sendtoWait failed. Are the intermediate mesh servers running?");
	delay(3000);
}

