#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads0;
Adafruit_ADS1115 ads1;

#define ADDR_1 (0b1001001)

void setup(void)
{
    Serial.begin(9600);
    Serial.println("Hello!");

    Serial.println("Getting single-ended readings from AIN0..3");
    Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");

    // The ADC input range (or gain) can be changed via the following
    // functions, but be careful never to exceed VDD +0.3V max, or to
    // exceed the upper and lower limits if you adjust the input range!
    // Setting these values incorrectly may destroy your ADC!
    //                                                                ADS1015  ADS1115
    //                                                                -------  -------
    // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
    // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
    // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
    // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
    // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
    // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

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
}
int8_t adc_number = 0;

void loop(void)
{

    int16_t adc;
    float volts;

    if (adc_number == 0)
    {

        adc = ads0.readADC_SingleEnded(0);
        volts = ads0.computeVolts(adc);

        Serial.print("A0:");
        Serial.print(volts);
        Serial.print("v            ");

        adc_number = 1;
    }
    else if (adc_number == 1)
    {

        adc = ads1.readADC_SingleEnded(0);
        volts = ads1.computeVolts(adc);

        Serial.print("A1:");
        Serial.print(volts);
        Serial.println("v");

        adc_number = 0;
    }

    delay(25);
}