# M5Stack_Infrared_Thermometer
M5Stack/ESP32 Infrared Thermometer with Blynk/MQTT/GSM Support

[A participant in the First Internet of Things Competition of Southeast University, author: YZY]

This is a light and portable thermometer that uses M5Stack (ESP32) as its core. An Single and Dual Zone Infra Red Thermometer MLX90614 provides fast and accurate temperature measurements. The M5Stack GPS unit and ENV unit are utilized to provide position data and environment readings that together with the  temperature readings, will help fight the COVID-19 outbreak. A SIM800L module is used to send SMS messages that contain the readings.

## Objectives
- Build a thermometer that is capable of fast and accurate measurements without having to touch the candidate.
- Able to upload data to the Internet and/or other methods of transferring data
- Able to log information that can be valuable to pandemic prevention, such as position data where it can be used to track patients.

## Overview

This project is written using Arduino. There are a couple of libraries that need to be installed before you proceed. The specifics are written below. Be sure to have them installed or the example will not execute correctly.

- M5Stack: This is the main library for M5Stack and it provides a lot of functionality with the hardware.
- Wifi: Used to connect to a WiFi hotspot and send/receive UDP packets.
- Adafruit BMP280 Library: Used to drive the BMP280 pressure and altitude sensor.
- Adafruit MLX90614 Library: Used to obtain and parse temperature readings from the MLX90614 thermometer.
- Adafruit Unified Sensor: Used to drive adafruit sensors.
- Blynk: A library for Blynk in Arduino
- DHT12 : For the DHT12 sensor.
- ESPDateTime: A library that allows you to connect to a time server and get the date and time.
- EspMQTTClient: Allows you to connect to an MQTT broker easily.
- TinyGPS: A compact NMEA parsing library to get the latitude and longitudes from the GPS.

Both the MLX90614 sensor and the ENV sensor communicate with I2C and can use the same set of pins. On the other hand, GPS and the GSM module both use UART, therefore you should take care not to use conflicting pins.

-----------------------

### Blynk support

Another thing that you need to have is a [Blynk](https://blynk.io/) server. Blynk is a IoT platform that offers easy implementation that allows you to receive and control your IoT projects with a smartphone. It is really easy to use and offers great results.

There are servers provided by Blynk themselves and you can choose to use that. I chose to build my own Blynk server as I happen to have a VPS that is handy for the job. 

As Blynk mainly runs on JAVA, the first thing you will need to do is to have Java support on your server. This depends on your environment and installation methods may differ.

Then you have to install Blynk and change some preferences.

For more information and tutorials on how to build your own Blynk Server, visit their Github page [here](https://github.com/blynkkk/blynk-server).

--------------------------

### MQTT support

MQTT is a lightweight protocol that allows you to communicate between devices. 

##### MQTT broker

In order to utilize MQTT, you will need a MQTT broker. It can be built in your own local network, but I chose to implement it into the same VPS as the Blynk one as this gives me access wherever I have an Internet connection.

I chose Mosquitto MQTT as my server MQTT broker appication as it is well-known and there are a lot of tutorials. Visit their homepage [here](https://mosquitto.org/) and their Github page [here](https://github.com/eclipse/mosquitto).

Tools like MQTT.fx and MQTTBox can help you diagnose problems you encounter on the way.

##### MQTT client

I chose to use the PAHO.MQTT library in python to build my own MQTT client that can subscribe and log the data that is being uploaded online. The code can be seen in srcs/MQTT_logger.ipynb

---------------------------------------

### GSM support

Using AT commands we can easily send the data through SMS text messages. This is fairly easy and I suggest looking for some tutorials.

---------------

### TF card logging

Last of all the data can be logged into an SD card. We will use the TF card library in M5Stack to accomplish this. More can be seen [here](https://docs.m5stack.com/#/en/arduino/arduino_api)

## How to use

The Arduino sketch provided in src folder can be directly burned into the M5tack hardware. 

MLX90614 and the Env. Unit should both be connected to Port A, and for the GPS, I remapped its pins to the ones in Port B so that it will not conflict with the GSM module.

After burning, press the middle button to start a measurement. The left button is for shutting down the Lcd screen to save power. The right button is for sending out an SMS.

----------------------

## On the build
![IMG_20200513_055545](https://github.com/AuroralDiffraction/M5Stack_Infrared_Thermometer/blob/master/images/1.jpg)

### Data methods

![image-20200513101227109](https://github.com/AuroralDiffraction/M5Stack_Infrared_Thermometer/blob/master/images/8.png)


### The front

A big blue figure shows the measured temperature in Celsius. A time stamp shows when the measurement was taken. Environment temperature, humidity and pressure is also shown. I was indoors and couldn't get GPS signals, thus the timeout warning.

![IMG_20200513_055457](https://github.com/AuroralDiffraction/M5Stack_Infrared_Thermometer/blob/master/images/2.jpg)

### The main component

The MLX90614 Infrared sensor pocking out at the front of the thermometer. It is the heart of this project. It can measure from -70℃ to 380℃ without even needing to touch the object! For human body temperature range, it can achieve a reasonable accuracy.

The rectangular thing on the top left corner is the antenna of the GPS.

![IMG_20200513_055522](https://github.com/AuroralDiffraction/M5Stack_Infrared_Thermometer/blob/master/images/3.jpg)

### The sides

We can see the GPS module here. My M5Stack GPS unit had a "antenna open" error, so I had to buy another of the same kind. It worked well. To get the strongest signals, you had to be outdoors.

The blue GSM module can also be seen here.

![IMG_20200513_055511](https://github.com/AuroralDiffraction/M5Stack_Infrared_Thermometer/blob/master/images/4.jpg)

### The bottom

Underneath, a single ENV unit sticks at the bottom. It obtains temperature, humidity and pressure readings of the environment and sends it to the core using I2C.

![IMG_20200513_055516](https://github.com/AuroralDiffraction/M5Stack_Infrared_Thermometer/blob/master/images/5.jpg)

 ### Blynk app

Shows all of the data and can be updated almost instantly! The long decimal place is due to calculation precision problems and can be ignored. The position locations are not shown here due to privacy reasons.

![image-20200513094841350](https://github.com/AuroralDiffraction/M5Stack_Infrared_Thermometer/blob/master/images/6.jpg)

### MQTT logger

It subscribes to the broker and the data topic. Then it logs the topic down once it is published.

![image-20200513095221975](https://github.com/AuroralDiffraction/M5Stack_Infrared_Thermometer/blob/master/images/10.png)

### SMS notification

The readings can be sent via SMS to the owner.

![image-20200513095350047](https://github.com/AuroralDiffraction/M5Stack_Infrared_Thermometer/blob/master/images/7.jpg)
