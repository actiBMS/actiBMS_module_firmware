#include "Steinhart.h"

//The thermistor is connected in series with another 47k resistor
//and across the 2.048V reference giving 50:50 weighting

//We can calculate the  Steinhart-Hart Thermistor Equation based on the B Coefficient of the thermistor
// at 25 degrees C rating

//https://arduinodiy.wordpress.com/2015/11/10/measuring-temperature-with-ntc-the-steinhart-hart-formula/
//https://learn.adafruit.com/thermistor/using-a-thermistor
//Temp = log(Temp);
//Temperature in Kelvin = 1 / {A + B[ln(R)] + C[ln(R)]^3}
//Temp = 1.0 / (A + (B*Temp) + (C * Temp * Temp * Temp ));


float Steinhart::rawToCelcius(uint16_t beta, uint16_t reading) {

  // If we get zero its likely the ADC is connected to ground
  if (reading > 0) {
    float steinhart = RESISTOR_SERIES * (1023.0F / (float)reading - 1.0F);
    
    steinhart = steinhart / THERMISTOR_NOMINAL;     // (R/Ro)
    steinhart = log(steinhart);                  // ln(R/Ro)
    steinhart /= beta;                   // 1/B * ln(R/Ro)
    steinhart += 1.0 / (THERMISTOR_TEMPERATURE + 273.15); // + (1/To)
    steinhart = 1.0 / steinhart;                 // Invert
    steinhart -= 273.15;                         // convert to C

    return steinhart;
  }

  return -273.15;
}
