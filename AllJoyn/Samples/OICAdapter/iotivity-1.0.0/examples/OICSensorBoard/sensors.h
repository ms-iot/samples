//******************************************************************
//
// Copyright 2014 Intel Corporation.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <stdlib.h>
#include <math.h>
#include "mraa.h"

#define ONBOARD_LED_PIN 13
#define TEMPERATURE_AIO_PIN 0
#define LIGHT_SENSOR_AIO_PIN 2
#define SAMPLE_NUM 5

namespace Sensors
{
mraa_gpio_context led_gpio = NULL;
mraa_aio_context tmp_aio = NULL;
mraa_aio_context light_aio = NULL;

inline void SetupPins()
{
    led_gpio = mraa_gpio_init(ONBOARD_LED_PIN); // Initialize pin 13
    if (led_gpio != NULL)
        mraa_gpio_dir(led_gpio, MRAA_GPIO_OUT); // Set direction to OUTPUT
    tmp_aio = mraa_aio_init(TEMPERATURE_AIO_PIN);   // initialize pin 0
    light_aio = mraa_aio_init(LIGHT_SENSOR_AIO_PIN);   // initialize pin 2
}

inline void ClosePins()
{
    mraa_gpio_close(led_gpio);
    mraa_aio_close(tmp_aio);
    mraa_aio_close(light_aio);
}

inline void SetOnboardLed(int on)
{
    if (led_gpio == NULL)
    {
        led_gpio = mraa_gpio_init(ONBOARD_LED_PIN); // Initialize pin 13
        if (led_gpio != NULL)
            mraa_gpio_dir(led_gpio, MRAA_GPIO_OUT); // Set direction to OUTPUT
    }
    if (led_gpio != NULL)
        mraa_gpio_write(led_gpio, on); // Writes into GPIO
}

inline float GetAverageTemperatureRaw()
{
    if (tmp_aio == NULL)
    {
        tmp_aio = mraa_aio_init(TEMPERATURE_AIO_PIN); // initialize pin 0
    }
    
    uint16_t adc_value = 0;
    for (int i=0; i< SAMPLE_NUM; i++)
        adc_value += mraa_aio_read(tmp_aio);           // read the raw value
    
    float average = (float)adc_value/SAMPLE_NUM;
    cout << "Temperature reading raw ..."  << average << endl;
    
    return average;
}

inline float GetTemperatureInC()
{
    // Temperature calculation using simpilfy Steinhart-Hart equation
    //
    //          1/T = 1/T0 + 1/beta*ln (R/R0)
    //
    // where T0 = 25C room temp, R0 = 10000 ohms
    //
    float beta = 4090.0;            //the beta of the thermistor, magic number
    float t_raw = GetAverageTemperatureRaw();
    float R = 1023.0/t_raw -1;      // 
    R = 10000.0/R;                  // 10K resistor divider circuit
        
    float T1 = log(R/10000.0)/beta; // natural log 
    float T2 = T1 + 1.0/298.15;     // room temp 25C= 298.15K
    float ret = 1.0/T2 - 273.0;
 
    return ret;
}

// This function returns light level between 0 and 4095
inline int GetLightLevel()
{
    uint16_t adc_value = 0;
    if (light_aio == NULL)
        light_aio = mraa_aio_init(LIGHT_SENSOR_AIO_PIN); // initialize pin 2
    if (light_aio != NULL)
        adc_value = mraa_aio_read(light_aio); // read the raw value
    return adc_value;
}
}
