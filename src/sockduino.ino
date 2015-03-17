#include <Wire.h>
#include <RCSwitch.h>

#define SLAVE_ADDRESS 0x23
#define TRANSMIT_PIN 10

// Initialize the rc switch library
RCSwitch sockSwitch = RCSwitch();

void setup()
{
    // Begin serial communication at 9600 bauds for debugging
    Serial.begin(9600);

    // Begin i2c communication as a slave with an arbitrary address
    Wire.begin(SLAVE_ADDRESS);
    // Register a callback for incoming data
    Wire.onReceive(receiveData);
    // Register a callback for requested data
    Wire.onRequest(sendData);

    // Set the pin connected to the data input of the transmitter
    sockSwitch.enableTransmit(TRANSMIT_PIN);
    // Set the pulse length (adjusting this value can increase the range)
    sockSwitch.setPulseLength(360);
}

void loop()
{
    // The main loop is not of interest, as all code execution depends
    // on i2c commands
    delay(1000);
}

void receiveData(int byteCount)
{
    if (byteCount == 5)
    {
        // First code type to switch the sockets: "type a with dip switches"
        // 11 characters are sent over i2c:
        // byte 0: 1 or 0, a 1 indicates to turn the socket on, a 0
        //         indicates to turn the socket off
        // byte 1: 5 bit value giving the system code
        // byte 2: 5 bit value giving the unit code
        // byte 3 to 4: 16 bit pulse length, byte 11 is the high byte,
        //              byte 12 is the low byte

        char newState = 0x00;
        char systemCode[6] = { 0x00, };
        char unitCode[6] = { 0x00, };

        // Byte 0 is the new state of the socket
        newState = Wire.read();

        // Byte 1 is the system code of the socket
        char systemCodeBits = Wire.read();

        // Convert the binary code to a string
        for (int i = 0; i < 5; i++)
        {
            systemCode[i] = '0' + ((systemCodeBits >> (4 - i)) & 0x01);
        }

        // Byte 2 is the unit code of the socket
        char unitCodeBits = Wire.read();

        // Convert the binary code to a string
        for (int i = 0; i < 5; i++)
        {
            unitCode[i] = '0' + ((unitCodeBits >> (4 - i)) & 0x01);
        }

        // Byte 3 is the high byte of the pulse length
        char pulseHigh = Wire.read();
        // Byte 4 is the low byte of the pulse length 
        char pulseLow = Wire.read();

        // Construct the 16 bit value and set the pulse length
        sockSwitch.setPulseLength((pulseHigh << 8) | pulseLow);

        if (newState == 0)
        {
            Serial.println("Switching socket off.");

            // New state is a '0', so turn the socket off
            sockSwitch.switchOff(systemCode, unitCode);
        }
        else if (newState == 1)
        {
            Serial.println("Switching socket on.");

            // New state is a '1', so turn the socket on
            sockSwitch.switchOn(systemCode, unitCode);
        }
    }
    else if (byteCount == 4)
    {
        // Second code type to switch the sockets: "decimal"
        // One decimal value (16 bit) is sent over i2c, that
        // determines which socket is addressed as well as if
        // it will be turned off or on

        // First byte sent is the high byte
        char valueHigh = Wire.read();
        // Second byte sent is the low byte
        char valueLow = Wire.read();

        // Third byte is the high byte of the pulse length
        char pulseHigh = Wire.read();
        // Fourth is the low byte of the pulse length 
        char pulseLow = Wire.read();

        // Construct the 16 bit value and set the pulse length
        sockSwitch.setPulseLength((pulseHigh << 8) | pulseLow);
        // Construct the 16 bit value and begin the transmission
        sockSwitch.send((valueHigh << 8) | valueLow, 24);
    }
    else
    {
        Serial.println("I need 5/4 bytes to operate properly.");

        while (Wire.available())
        {
            // Clear the buffer if there is too much to read
            Wire.read();
        }
    }
}

void sendData()
{
    // Just respond something if data is requested
    Wire.write("hi.");
}
