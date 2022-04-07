// // Example for SDConfigCommand - Read And Write Config File //
// /*
// SDConfigCommand streamlines reading settings from a config file on SD card. SDConfigCommand can read standardised text files stored on a SD card,
// parse and tokenise them into commands and values. The library can also write over existing settings but it is currently slow to do so.
// For every line on the config file this library reads, it will callback a user-specified function. The user can access the current command and values,
// then decide the next action, such as verifying commands and storing values in variables. With regards to writing over existing settings, user can choose one command,
// and the library will search for the command in the config file and replace the whole setting line with a new value. The library does not add or remove settings.

// The config file must have a 8.3 file name. Each setting takes one line, with the command on the left, followed by an equal sign, then the value on the right.
// Comments can be written after two slashes like in the Arduino IDE. Due to the limitation of Arduino, minimising comments and arranging the settings
// to be read sequentially will improve performance. Check out the included example files for a better idea.

// In this example, connect your Arduino to read SD card using a SD card shield or module. To connect Uno to a SD module with LEVEL SHIFTER:
// 5V to VCC, GND to GND, 11 to MOSI, 12 to MISO, 13 to SCK, 4 to Chip Select. *Remember to use level shifters for 5V Arduinos.*

// You will also need setting.cfg, stress.cfg and stress_r.cfg in the root folder of your SD card.
// Upload the sketch and observe the Serial Monitor and take instructions from there.
// */

// #include <SDConfigCommand.h>
// #include <string.h>

// // Define chip select pin and file names
// #define CHIPSELECT 4
// #define SETTING_FILE "settings.cfg"

// // Some variables to store main settings
// int transitionLevels, autoBroadcasting;
// char macStr[16];
// char ipStr[16];
// byte mac[6];
// byte ipVals[4];

// // Create an instance of SDConfigCommand
// SDConfigCommand sdcc;

// void setup()
// {
//     Serial.begin(9600);

//     while (!Serial)
//     {
//     } // wait for serial port to connect. Needed for native USB port only

//     // setting.cfg is the file name, 4 is chip enable pin for SPI, processCmd is the callback function
//     while (!(sdcc.set(SETTING_FILE, CHIPSELECT, processCmd)))
//     {
//     }

//     // Display the setting.cfg in Serial Monitor
//     sdcc.openInSerial();
//     Serial.println();

//     // Read in and store the commands in config file
//     sdcc.readConfig();

//     for (int i = 0; i < 6; i++)
//     {
//         char subString[2];
//         char *ptr;
//         strncpy(subString, &macStr[i * 2], 2);
//         mac[i] = (byte)strtol(subString, &ptr, 16);
//     }

//     for (int i = 0; i < 4; i++)
//     {
//         char subString[3];
//         char *ptr;
//         strncpy(subString, &ipStr[i * 4], 3);
//         ipVals[i] = (byte)strtol(subString, &ptr, 10);
//     }
// }

// void loop()
// {

//     delay(1000);
// }

// void processCmd()
// {
//     // This function will run every time there is a command
//     // You can then check the command and value and dictate the next action

//     if (strcmp(sdcc.getCmd(), "STL") == 0)
//     {
//         transitionLevels = atoi(sdcc.getValue()); // You can manually convert the returned c-string as well
//     }
//     else if (strcmp(sdcc.getCmd(), "ABC") == 0)
//     {
//         autoBroadcasting = atoi(sdcc.getValue()); // You can manually convert the returned c-string as well
//     }
//     else if (strcmp(sdcc.getCmd(), "IPA") == 0)
//     {
//         strcpy(ipStr, sdcc.getValue());
//     }
//     else if (strcmp(sdcc.getCmd(), "MAC") == 0)
//     {
//         strcpy(macStr, sdcc.getValue()); // Use strcpy instead of = for c-string
//     }
//     else
//     {
//         Serial.print(sdcc.getCmd());
//         Serial.println(F(" is an unrecognised command."));
//     }
// }
