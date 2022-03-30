#include <Adafruit_ADS1X15.h>
#include <SDConfigCommand.h>

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#include <string.h>
#include <math.h>

Adafruit_ADS1115 ads0;
SDConfigCommand sdcc;

// Define chip select pin and file names
#define CHIPSELECT 4
#define SETTING_FILE "settings.cfg"

byte mac[6] = {0xA8, 0x61, 0x0A, 0xAE, 0x75, 0x2C};
IPAddress ip(192, 168, 1, 177);

// strings of the IP and MAC address pulled from the SD card
char macStr[16];
char ipStr[16];

unsigned int localPort = 8888; // local port to listen on

bool autoBroadcasting = 1;
double adcLevel; // out of 5v
int transitionLevels = 5;

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; // buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged";       // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

void sendUPDString(char *string)
{
    Udp.beginPacket(Udp.remoteIP(), 5555);
    Udp.write(string);
    Udp.endPacket();
}

int getCommandVaue(char *command)
{
    // value is the total UDP packet size (24 bytes) minus the command size (4 bytes, "XXX:") at command index 4
    char val[20];
    strncpy(val, &command[4], 20);
    return atoi(val);
}

bool commandValValid(int begin, int end, char *command)
{
    Serial.println(getCommandVaue(command));
    if (command[4] == ' ' || command[4] == 13)
    {
        return 0;
    }

    if (getCommandVaue(command) > end || getCommandVaue(command) < begin)
    {
        return 0;
    }
    return 1;
}

char *getCommandType(char *command)
{
    char type[4];
    strncpy(type, &command[0], 3);
    type[3] = '\0';
    // Serial.println(type);
    return type;
}
void setup()
{

    // start the Ethernet

    // Open serial communications and wait for port to open:
    Serial.begin(9600);
    while (!Serial)
    {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    Serial.println("Serial Enabled with baud at 9600");

    while (!(sdcc.set(SETTING_FILE, CHIPSELECT, processCmd)))
    {
    }

    sdcc.readConfig();

    for (int i = 0; i < 6; i++)
    {
        char subString[2];
        char *ptr;
        strncpy(subString, &macStr[i * 2], 2);
        mac[i] = (byte)strtol(subString, &ptr, 16);
    }

    for (int i = 0; i < 4; i++)
    {
        uint8_t ipVals[4];
        char subString[3];
        char *ptr;
        strncpy(subString, &ipStr[i * 4], 3);
        ipVals[i] = (byte)strtol(subString, &ptr, 10);
        ip = ip(ipVals[0], )
    }
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

    if (!ads0.begin())
    {
        Serial.println("Failed to initialize ADS Board #0.");
        while (1)
            ;
    }

    // start UDP
    Udp.begin(localPort);
}

void loop()
{
    /* if there's data available, read a packet */
    int packetSize = Udp.parsePacket();
    if (packetSize)
    {
        Serial.print("Received packet of size ");
        Serial.println(packetSize);
        Serial.print("From ");
        IPAddress remote = Udp.remoteIP();
        for (int i = 0; i < 4; i++)
        {
            Serial.print(remote[i], DEC);
            if (i < 3)
            {
                Serial.print(".");
            }
        }
        Serial.print(", port ");
        Serial.println(Udp.remotePort());

        /* read the packet into packetBufffer */
        Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
        Serial.println("Contents:");
        Serial.println(packetBuffer);

        if (packetBuffer[3] != ':')
        {
            Udp.beginPacket(Udp.remoteIP(), 5555);
            Udp.write("ERR:invald_cmd_format");
            Udp.endPacket();
        }
        else
        {
            if (packetBuffer[0] == 'S' && packetBuffer[1] == 'A' && packetBuffer[2] == 'B') // Enable or Disable Autobroadcasting
            {

                if (commandValValid(0, 1, packetBuffer))
                {
                    autoBroadcasting = getCommandVaue(packetBuffer);
                    sendUPDString(packetBuffer);
                }
                else
                {
                    sendUPDString("ERR:invalid_cmd_value");
                }
            }
            else if (packetBuffer[0] == 'S' && packetBuffer[1] == 'T' && packetBuffer[2] == 'L') // Set Num Transition Levels
            {
                if (commandValValid(1, 100, packetBuffer))
                {
                    transitionLevels = getCommandVaue(packetBuffer);
                    sendUPDString(packetBuffer);
                }
                else
                {
                    sendUPDString("ERR:invalid_cmd_value");
                }
            }
            else if (packetBuffer[0] == 'V' && packetBuffer[1] == 'A' && packetBuffer[2] == 'L') // Get Transition level value
            {

                int16_t adc;
                double volts;

                adc = ads0.readADC_SingleEnded(0);
                volts = ads0.computeVolts(adc);
                double currentLevel = ceil((double)(volts * transitionLevels) / 5);
                char cmdOut[6];
                sprintf(cmdOut, "VAL:%d", (int)currentLevel);
                sendUPDString(cmdOut);
            }
            else
            {
                sendUPDString("ERR:invalid_cmd");
            }
        }
    }

    if (autoBroadcasting)
    {

        int16_t adc;
        double volts;

        adc = ads0.readADC_SingleEnded(0);
        volts = ads0.computeVolts(adc);

        double currentLevel = ceil((double)(volts * transitionLevels) / 5);
        if (ceil((double)(adcLevel * transitionLevels) / 5) != currentLevel)
        {
            adcLevel = volts;

            char cmdOut[6];
            sprintf(cmdOut, "VAL:%d", (int)currentLevel);

            sendUPDString(cmdOut);
        }
    }
    delay(10);
}