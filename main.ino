#include <SPI.h>
#include <SD.h>

#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"

#include "Fonts/TomThumb.h"

#include <math.h>

#include <Ethernet.h>
#include <EthernetUdp.h>

#include <MFRC522.h>

#define SETTINGS_FILE_NAME "settings.cfg"
#define MAX_SETTINGS 10

File myFile;
EthernetUDP Udp;

// global variables
char version[10] = "0.20.0";

int16_t localPort = 8888;
int16_t remotePort = 5555;

uint8_t autoBroadcasting = 1;
uint8_t transitionLevels = 1;

uint8_t machineId = 0;

bool loggedIn = false;
char userName[15];
uint8_t target;
uint8_t completed;

byte macAddress[6] = {0, 0, 0, 0, 0, 0};
byte ipAddress[4] = {192, 168, 0, 177};

uint16_t lastADCValue;

#define RA8875_INT 3
#define RA8875_CS 6
#define RA8875_RESET 9

Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);

#define SS_PIN 53
#define RST_PIN 8

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

byte nuidPICC[4] = {0x80, 0xFD, 0xB3, 0x1B};

void setup()
{

    // Open serial communications and wait for port to open:
    Serial.begin(9600);
    while (!Serial)
    {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    //*************

    if (!tft.begin(RA8875_800x480))
    {
        Serial.println("RA8875 Not Found!");
        while (1)
            ;
    }

    // init pot
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);

    // init display
    tft.displayOn(true);
    tft.GPIOX(true);                              // Enable TFT - display enable tied to GPIOX
    tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
    tft.PWM1out(255);

    /* Switch to text mode */
    tft.textMode();
    tft.setFont(&TomThumb);
    tft.cursorBlink(32);

    /* Set the cursor location (in pixels) */
    tft.textSetCursor(10, 10);

    /* Render some text! */
    char string[15] = "Hello, World! ";

    printLoggedOutDisplay();

    for (byte i = 0; i < 6; i++)
    {
        key.keyByte[i] = 0xFF;
    }

    rfid.PCD_Init(); // Init MFRC522

    // //**********

    readConfigFile();
    initializeUDP();
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
        char *packetBuffer;
        packetBuffer = (char *)calloc(UDP_TX_PACKET_MAX_SIZE, sizeof(char)); // allocate heap for buffer and set to 0

        /* read the packet into packetBufffer */
        Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);

        if (packetBuffer[3] != ':')
        {
            sendUPDString("ERR:invalid_cmd_format");
        }
        else
        {
            char *key = (char *)calloc(4, sizeof(char));
            char *val = (char *)calloc(packetSize - 5, sizeof(char));
            strncpy(key, packetBuffer, 3);
            strncpy(val, packetBuffer + 4, packetSize - 6);

            if (!strcmp(key, "LGN")) // Enable or Disable Autobroadcasting
            {

                char *tgt = (char *)malloc(sizeof(char));
                char *cmp = (char *)malloc(sizeof(char));

                uint8_t lastValIndex = 0;
                uint8_t valCount = 0;

                for (uint8_t i = 0; i <= strlen(val); i++)
                {
                    if (val[i] == ',' || val[i] == '\0')
                    {

                        if (valCount == 0)
                        {
                            strncpy(userName, val + lastValIndex, i - lastValIndex);
                            Serial.print("Username is: ");
                            Serial.println(userName);
                        }
                        else if (valCount == 1)
                        {
                            strncpy(tgt, val + lastValIndex, i - lastValIndex);
                            target = atoi(tgt);
                        }
                        else if (valCount == 2)
                        {
                            strncpy(cmp, val + lastValIndex, i - lastValIndex);
                            completed = atoi(cmp);
                        }
                        else
                        {
                            sendUPDString("LGN:ERR:0");
                        }
                        lastValIndex = i + 1;
                        valCount++;
                    }
                }
                loggedIn = 1;

                printLoggedInDisplay();
                digitalWrite(2, HIGH);
                free(tgt);
                free(cmp);
            }
            else if (!strcmp(key, "LGO")) // Enable or Disable Autobroadcasting
            {
                if (loggedIn)
                {
                    loggedIn = 0;
                    sendUPDString("LGO:1");
                    digitalWrite(2, LOW);
                    printLoggedOutDisplay();
                }
                else
                {
                    sendUPDString("LGO:ERR,0");
                }
            }
            else if (!strcmp(key, "TAR")) // Enable or Disable Autobroadcasting
            {
                if (isValueValid(0, 255, val))
                {
                    target = atoi(val);
                    printLoggedInDisplay();
                    sendUPDString(packetBuffer);
                }
                else
                {
                    sendUPDString("TAR:ERR,0");
                }
            }
            else if (!strcmp(key, "CMP")) // Enable or Disable Autobroadcasting
            {
                if (isValueValid(0, 255, val))
                {
                    completed = atoi(val);
                    printLoggedInDisplay();
                    sendUPDString(packetBuffer);
                }
                else
                {
                    sendUPDString("CMP:ERR,0");
                }
            }
            else if (!strcmp(key, "SAB")) // Enable or Disable Autobroadcasting
            {
                if (isValueValid(0, 1, val))
                {
                    autoBroadcasting = atoi(val);
                    sendUPDString(packetBuffer);
                }
                else
                {
                    sendUPDString("SAB:ERR,0");
                }
            }
            else if (!strcmp(key, "STL")) // Set Num Transition Levels
            {
                if (isValueValid(1, 100, val))
                {
                    transitionLevels = atoi(val);
                    sendUPDString(packetBuffer);
                }
                else
                {
                    sendUPDString("STL:ERR,0");
                }
            }
            else if (!strcmp(key, "RST")) // Set Num Transition Levels
            {
                sendUPDString("RST:1");
                // TODO
            }
            else if (packetBuffer[0] == 'V' && packetBuffer[1] == 'A' && packetBuffer[2] == 'L') // Get Transition level value
            {
                // int16_t adc;
                // double volts;

                // adc = ads0.readADC_SingleEnded(0);
                // volts = ads0.computeVolts(adc);
                // double currentLevel = ceil((double)(volts * transitionLevels) / 5);
                // char cmdOut[6];
                // sprintf(cmdOut, "VAL:%d", (int)currentLevel);
                // sendUPDString(cmdOut);
            }
            else
            {
                sendUPDString("ERR:invalid_cmd");
            }
            free(key);
            free(val);
        }
        free(packetBuffer);
    }

    if (autoBroadcasting && loggedIn)
    {

        uint16_t currentADCValue;

        currentADCValue = analogRead(0);
        double currentLevel = ceil((double)(currentADCValue * transitionLevels) / 1023);
        if (ceil((double)(lastADCValue * transitionLevels) / 1023) != currentLevel)
        {
            lastADCValue = currentADCValue;

            char cmdOut[6];
            sprintf(cmdOut, "SCH:%d", (int)currentLevel);

            sendUPDString(cmdOut);
        }
    }
    else if (!loggedIn)
    {

        if (!rfid.PICC_IsNewCardPresent())
            return;

        // Verify if the NUID has been readed
        if (!rfid.PICC_ReadCardSerial())
            return;

        Serial.print(F("PICC type: "));
        MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
        Serial.println(rfid.PICC_GetTypeName(piccType));

        // Check is the PICC of Classic MIFARE type
        if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
            piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
            piccType != MFRC522::PICC_TYPE_MIFARE_4K)
        {
            Serial.println(F("Your tag is not of type MIFARE Classic."));
            return;
        }

        Serial.println(F("Read card."));
        char loginString[20];
        sprintf(loginString, "LGN:%x:%x:%x:%x,%d", rfid.uid.uidByte[3], rfid.uid.uidByte[2], rfid.uid.uidByte[1], rfid.uid.uidByte[0], machineId);
        sendUPDString(loginString);

        // Halt PICC
        rfid.PICC_HaltA();

        // Stop encryption on PCD
        rfid.PCD_StopCrypto1();
    }
    // delay(10);
}

int initializeUDP()
{

    IPAddress ip(ipAddress[0], ipAddress[1], ipAddress[2], ipAddress[3]);
    Ethernet.begin(macAddress, ip);

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

    Udp.begin(localPort);
}

// reads all values from settings file, returns 0 on success, -1 on error.
int readConfigFile()
{

    Serial.print("Initializing SD card...");

    if (!SD.begin(4))
    {
        Serial.println("initialization failed!");
    }

    Serial.println("initialization done.");

    char *fileBuffer;                        // Declare a pointer to your buffer.
    myFile = SD.open(F(SETTINGS_FILE_NAME)); // Open file for reading.
    if (myFile)
    {
        unsigned int fileSize = myFile.size();     // Get the file size.
        fileBuffer = (char *)malloc(fileSize + 1); // Allocate memory for the file and a terminating null char.
        myFile.read(fileBuffer, fileSize);         // Read the file into the buffer.
        fileBuffer[fileSize] = '\0';               // Add the terminating null char.
        // Serial.println(fileBuffer);                // Print the file to the serial monitor.
        myFile.close(); // Close the file.
        SD.end();
    }

    uint8_t newLineIndex = 0; // newLineIndex index is the character index of the first character of a new line (not \n)
    for (uint8_t i = 0; i <= strlen(fileBuffer); i++)
    {

        if (fileBuffer[i] == (char)13 || fileBuffer[i] == '\0') // has reached the end of a line or the end of the buffer
        {
            if (fileBuffer[newLineIndex] != '/' && fileBuffer[newLineIndex] != char(13) && fileBuffer[newLineIndex] != ' ') // check if the value at the beginning of the line is a comment or a blank line
            {
                if (fileBuffer[newLineIndex + 3] != '=')
                {
                    Serial.println("Error: Incorrect file formatting");
                    return -1;
                }

                char *key;
                char *value;
                key = (char *)malloc(4);
                value = (char *)malloc(20);

                strlcpy(key, fileBuffer + newLineIndex, 4);
                strlcpy(value, fileBuffer + newLineIndex + 4, i - (newLineIndex + 4) + 1);
                if (assignValues(key, value) == -1)
                {
                    Serial.println("Error: Unknown key:value pair");
                    return -1;
                }

                free(key);
                free(value);
            }
            newLineIndex = i + 2;
        }
    }

    free(fileBuffer);
    return 0;
}

void printLoggedOutDisplay()
{

    tft.fillScreen(RA8875_BLACK);
    tft.textTransparent(RA8875_WHITE);
    tft.textEnlarge(4);
    tft.textSetCursor(80, 50);
    tft.textWrite("MFI Sewing Assistant");
    tft.textSetCursor(80, 120);
    tft.textWrite(version);
    tft.textEnlarge(1);
    tft.textSetCursor(80, 220);
    tft.textWrite("Please place your card aganst the NFC");
    tft.textSetCursor(80, 250);
    tft.textWrite("reader to log in.");
    tft.textSetCursor(1000, 1000);
}

void printLoggedInDisplay()
{

    char tar[4];
    sprintf(tar, "%d", target);

    char comp[4];
    sprintf(comp, "%d", completed);

    tft.fillScreen(RA8875_BLACK);
    tft.textTransparent(RA8875_WHITE);
    tft.textEnlarge(4);
    tft.textSetCursor(80, 50);
    tft.textWrite("Welcome: ");
    tft.textWrite(userName);
    tft.textSetCursor(80, 120);
    tft.textEnlarge(2);
    tft.textSetCursor(80, 220);
    tft.textWrite("Target: ");
    tft.textWrite(tar);
    tft.textSetCursor(80, 270);
    tft.textWrite("Completed: ");
    tft.textWrite(comp);
    tft.textSetCursor(1000, 1000);
}

uint8_t assignValues(char *key, char *val)
{

    if (!strcmp(key, "ABC"))
    {
        autoBroadcasting = atoi(val);
        return 0;
    }
    else if (!strcmp(key, "STL"))
    {
        transitionLevels = atoi(val);
        return 0;
    }
    else if (!strcmp(key, "IPA"))
    {
        parseBytes(val, '.', ipAddress, 4, 10);
        return 0;
    }
    else if (!strcmp(key, "MAC"))
    {
        parseBytes(val, ':', macAddress, 6, 16);
        return 0;
    }
    else if (!strcmp(key, "MID"))
    {
        machineId = atoi(val);
    }
    else
    {
        return -1;
    }
}

// converts any symbol separated value in string form to an array of bytes
void parseBytes(const char *str, char sep, byte *bytes, int maxBytes, int base)
{
    for (int i = 0; i < maxBytes; i++)
    {
        bytes[i] = strtoul(str, NULL, base); // Convert byte
        str = strchr(str, sep);              // Find next separator
        if (str == NULL || *str == '\0')
        {
            break; // No more separators, exit
        }
        str++; // Point to next character after separator
    }
}

void sendUPDString(char *string)
{
    Udp.beginPacket(Udp.remoteIP(), remotePort);
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

// checks if an incoming value in key value pair is valid.
bool isValueValid(int begin, int end, char *value)
{
    int val = atoi(value);

    // check if the beggining of the value is valid
    if (value[4] == ' ' || value[4] == 13)
    {
        return 0;
    }

    if (val > end || val < begin)
    {
        return 0;
    }
    return 1;
}