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
    sockSwitch.setPulseLength(340);
}

void loop()
{
    // The main loop is not of interest, as all code execution depends
    // on i2c commands
    delay(1000);
}

void receiveData(int byteCount)
{
    if (byteCount == 11)
    {
        // First code type to switch the sockets: "type a with dip switches"
        // 11 characters are sent over i2c:
        // char 0: '1' or '0', a '1' indicates to turn the socket on, a '0'
        //         indicates to turn the socket off
        // char 1 to 5: each '1' or '0', concatenated they give the 5 bit
        //              string encoded system code
        // char 6 to 10: each '1' or '0', concatenated they give the 5 bit
        //               string encoded unit code

        char newState = 0x00;
        char systemCode[6] = { 0x00, };
        char unitCode[6] = { 0x00, };

        int messageCounter = 0;

        while (Wire.available())
        {
            if (messageCounter == 0)
            {
                // First char sent is the new state of the socket
                newState = Wire.read();
            }
            else if (messageCounter < 6)
            {
                // Char 1 to 5 sent are the system code of the socket
                systemCode[messageCounter - 1] = Wire.read();
            }
            else if (messageCounter < 11)
            {
                // Char 6 to 10 sent are the unit code of the socket
                unitCode[messageCounter - 6] = Wire.read();
            }

            messageCounter++;
        }

        if (newState == '0')
        {
            Serial.println("Switching socket off.");

            // New state is a '0', so turn the socket off
            sockSwitch.switchOff(systemCode, unitCode);
        }
        else if (newState == '1')
        {
            Serial.println("Switching socket on.");

            // New state is a '1', so turn the socket on
            sockSwitch.switchOn(systemCode, unitCode);
        }
    }
    else if (byteCount == 2)
    {
        // Second code type to switch the sockets: "decimal"
        // One decimal value (16 bit) is sent over i2c, that
        // determines which socket is addressed as well as if
        // it will be turned off or on

        // First byte sent is the high byte
        char high = Wire.read();
        // Second byte sent is the low byte
        char low = Wire.read();

        // Construct the 16 bit value and begin the transmission
        sockSwitch.send((high << 8) | low, 24);
    }
    else
    {
        Serial.println("I need 11/2 bytes to operate properly.");

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
