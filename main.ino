#include <Adafruit_ADS1X15.h>

#include <SPI.h>
#include <Ethernet.h>

Adafruit_ADS1115 ads0;
Adafruit_ADS1115 ads1;
Adafruit_ADS1115 ads2;
Adafruit_ADS1115 ads3;

#define ADDR_1 (0b1001001)
#define ADDR_2 (0b1001010)
#define ADDR_3 (0b1001011)

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
    0xA8, 0x61, 0x0A, 0xAE, 0x75, 0x2C};
IPAddress ip(192, 168, 1, 177);

int8_t adc_number = 0;
double adcVals[16] = {1.1, 2, 1.5, 3.1, 4, 5, 6, 1.6, 8.4, 1.1, 2, 1, 3.1, 4, 5, 6};

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup()
{
    Serial.begin(9600);
    while (!Serial)
    {
        ; // wait for serial port to connect. Needed for native USB port only
    }
    Serial.println("Ethernet WebServer Example");

    // start the Ethernet connection and the server:
    Ethernet.begin(mac, ip);

    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
        Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
        while (true)
        {
            delay(1); // do nothing, no point running without Ethernet hardware
        }
    }
    if (Ethernet.linkStatus() == LinkOFF)
    {
        Serial.println("Ethernet cable is not connected.");
    }

    // start the server
    server.begin();
    Serial.print("server is at ");
    Serial.println(Ethernet.localIP());

    if (!ads3.begin(ADDR_3))
    {
        Serial.println("Failed to initialize ADS Board #3.");
        while (1)
            ;
    }

    if (!ads2.begin(ADDR_2))
    {
        Serial.println("Failed to initialize ADS Board #2.");
        while (1)
            ;
    }
    if (!ads1.begin(ADDR_1))
    {
        Serial.println("Failed to initialize ADS Board #1.");
        while (1)
            ;
    }
    if (!ads0.begin())
    {
        Serial.println("Failed to initialize ADS Board #0.");
        while (1)
            ;
    }

    // initialize the values into the array
    for (int i = 0; i < 16; i++)
    {
        readADCVal(i, adcVals);
    }
}

void readADCVal(int sensorNumber, double *sensorVals)
{
    int16_t adc;
    double volts;

    if (sensorNumber < 4)
    {
        adc = ads0.readADC_SingleEnded(sensorNumber);
        volts = ads0.computeVolts(adc);
    }
    else if (sensorNumber > 3 && sensorNumber < 8)
    {
        adc = ads1.readADC_SingleEnded(sensorNumber - 4);
        volts = ads1.computeVolts(adc);
    }
    else if (sensorNumber > 7 && sensorNumber < 12)
    {
        adc = ads2.readADC_SingleEnded(sensorNumber - 8);
        volts = ads2.computeVolts(adc);
    }
    else
    {
        adc = ads3.readADC_SingleEnded(sensorNumber - 12);
        volts = ads3.computeVolts(adc);
    }
    sensorVals[sensorNumber] = volts;
}

void loop()
{

    unsigned long time = millis();
    unsigned long deltaTime;

    // for (int i = 0; i < 16; i++)
    // {
    //     readADCVal(i, adcVals);
    // }

        deltaTime = millis() - time;
    Serial.print("Time taken to record was:");
    Serial.println(deltaTime);

    time = millis();

    // listen for incoming clients
    EthernetClient client = server.available();

    if (client)
    {
        Serial.println("new client");
        // an http request ends with a blank line
        boolean currentLineIsBlank = true;
        while (client.connected())
        {
            if (client.available())
            {
                char c = client.read();
                // Serial.write(c);
                //  if you've gotten to the end of the line (received a newline
                //  character) and the line is blank, the http request has ended,
                //  so you can send a reply
                if (c == '\n' && currentLineIsBlank)
                {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: application/json");
                    client.println("Connection: close"); // the connection will be closed after completion of the response
                    client.println("Refresh: 1");        // refresh the page automatically every 1 sec
                    client.println();
                    client.print("[");
                    for (int i = 0; i < 16; i++)
                    {
                        client.print("{\"id\":");
                        client.print(i);
                        client.print(",\"value\":");
                        client.print(adcVals[i]);
                        if (i == 15)
                        {
                            client.print("}");
                        }
                        else
                        {
                            client.print("},");
                        }
                    }
                    client.println("]");
                    break;
                }
                if (c == '\n')
                {
                    // you're starting a new line
                    currentLineIsBlank = true;
                }
                else if (c != '\r')
                {
                    // you've gotten a character on the current line
                    currentLineIsBlank = false;
                }
            }
        }
        // give the web browser time to receive the data
        delay(1);
        // close the connection:
        client.stop();
        Serial.println("client disconnected");

        deltaTime = millis() - time;
        Serial.print("Time taken to print was:");
        Serial.println(deltaTime);
    }
    else
    { // there is not a request from the client for sensor information, record the values
        for (int i = 0; i < 16; i++)
        {
            readADCVal(i, adcVals);
            // time quantum for recording all the adc values and storing them on an array is at least 160ms
        }
    }
}
