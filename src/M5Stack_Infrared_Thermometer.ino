#include <M5Stack.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "DHT12.h"
#include <Wire.h> 
#include <TinyGPS++.h>
#include "Adafruit_Sensor.h"
#include <Adafruit_BMP280.h>
#include <Adafruit_MLX90614.h>
#include <String.h>
#include <CStringBuilder.h>
#include "ESPDateTime.h"
#include <EspMQTTClient.h>

#define BLYNK_PRINT Serial

DHT12 dht12; 
TinyGPSPlus gps; // GPS NMEA parser
HardwareSerial ss(1);  // The serial connection to the GPS device
BlynkTimer timer; // Timer  for the Blynk connection
WidgetMap myMap(V0);  // V0 for virtual pin of Map Widget
Adafruit_BMP280 bme; // Pressure sensor
Adafruit_MLX90614 mlx = Adafruit_MLX90614(); // Infared temperature sensor
char buff[169];
CStringBuilder sb(buff, sizeof(buff)); // String builder setup for 
char buff2[150];
CStringBuilder sb2(buff2, sizeof(buff2)); // String builder setup

char auth[] = "BLYNK-AUTH-TOKEN"; // Blynk auth token
char ssid[] = "WIFI-SSID"; // Wifi ssid
char pass[] = "WIFI-PASSWORD"; // Wifi password

// MQTT client details
EspMQTTClient client(
  "WIFI-SSID",
  "WIFI-PASSWORD",
  "MQTT-BROKER-IP",  // MQTT Broker server ip
  "USERNAME",   // Can be omitted if not needed
  "PASSWORD",   // Can be omitted if not needed
  "DEVICE-ID",      // Client name that uniquely identify your device
  1883
);

int timeout = 0;
int wifi = 1;
int flag = 1;
bool GPS_output = 0;
bool sendSMSOK = 0; //Set to one if there is data to be sent through SMS
float spd;       //Variable  to store the speed
float sats;      //Variable to store no. of satellites response
String bearing;  //Variable to store orientation or direction of GPS
double MLXtemp;  //Variable to store the object temperature read from the MLX sensor
double lat_data; //Variable to store the latitude
double lng_data;  //Vriable to store the longitude
float tmp;  // Variable to store the environment temperature
float hum;  // Variable to store the humidity
float pressure; // Variable to store the pressure
int batteryLevel; // Variable to store the battery level
int Lcdsleep = 0; // Set to 1 if LCD is in sleep mode

// Timer from the Blynk server
void myTimerEvent() {
    //float tmp = dht12.readTemperature();
    //float hum = dht12.readHumidity();
    //Serial.printf("Timer: Temperatura: %2.2f*C  Humedad: %0.2f%%\r\n", tmp, hum);

    //Blynk.virtualWrite(V0, tmp);
    //Blynk.virtualWrite(V1, hum);
    //M5.Lcd.setCursor(0, 0);
    batteryLevel = M5.Power.getBatteryLevel();
    Serial.printf("BatLevel: %d \r\n",batteryLevel);
}

//Checks if the GPS is OK or not
void checkGPS(){
  if (gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    Blynk.virtualWrite(V4, "GPS ERROR");  // Value Display widget  on V4 if GPS not detected
  }
}

// Gets the GPS signals and displays/uploads the positioning data
void displayInfo()
{ 
  if (gps.location.isValid() ) 
  {
    
    lat_data = (gps.location.lat());     //Storing the Lat. and Lon. 
    lng_data = (gps.location.lng()); 
    
    Serial.print("LAT:  ");
    Serial.println(lat_data, 6);  // float to x decimal places
    Serial.print("LONG: ");
    Serial.println(lng_data, 6);
    Blynk.virtualWrite(V1, String(lat_data, 6));   
    Blynk.virtualWrite(V2, String(lng_data, 6));  
    myMap.location(1, lat_data, lng_data, "GPS_Location");
    spd = gps.speed.kmph();               //get speed
    //Blynk.virtualWrite(V3, spd);
       
    sats = gps.satellites.value();    //get number of satellites
    Blynk.virtualWrite(V8, sats);

    bearing = TinyGPSPlus::cardinal(gps.course.value()); // get the direction
    //Blynk.virtualWrite(V5, bearing);                  
  }
  Serial.println();
}

//MQTT connection callback
void onConnectionEstablished() {
  Serial.print("Connected");  
}

// Sends the SMS
void sendSMS(){
  Serial2.println("AT");
  delay(100);
  Serial2.println("AT+CMGF=1");
  delay(100);
  Serial2.println("AT+CSCS=\"GSM\"");
  delay(100);
  Serial2.println("AT+CMGS=\"151********\"");
  delay(100);
  Serial2.println(buff);
  delay(100);
  Serial2.write(0x1A);
  delay(100);
  Serial2.println();
}

void setupDateTime() {
  // setup this after wifi connected
  // you can use custom timeZone,server and timeout
  // DateTime.setTimeZone(-4);
  DateTime.setServer("cn.pool.ntp.org");
  //   DateTime.begin(15 * 1000);
  DateTime.setTimeZone(8);
  DateTime.begin();
  if (!DateTime.isTimeValid()) {
    Serial.println("Failed to get time from server.");
  }
}

//Prints the default screen with no data
void printDefaultScreen(){  
  M5.Lcd.clear(WHITE);
  M5.Lcd.setTextColor(TFT_BLACK,WHITE);
  M5.Lcd.setCursor(5, 5);
  M5.Lcd.setTextSize(1);
  M5.Lcd.println("Temperature");
  M5.Lcd.progressBar(270,5,20,8,M5.Power.getBatteryLevel());
  M5.Lcd.setCursor(295, 5);
  M5.Lcd.println(M5.Power.getBatteryLevel());

  M5.Lcd.setCursor(5, 110);
  M5.Lcd.setTextColor(TFT_DARKGREY,WHITE);
  M5.Lcd.print("Measurement taken at");
  M5.Lcd.setTextColor(TFT_BLACK,WHITE);
  
  M5.Lcd.drawFastHLine(0,120,M5.Lcd.width(),TFT_DARKGREEN);
  M5.Lcd.setCursor(5, 125);
  M5.Lcd.setTextSize(1);
  M5.Lcd.print("Env. Temp");
  M5.Lcd.setCursor(100, 125);
  M5.Lcd.print("Humidity");
  M5.Lcd.setCursor(195, 125);
  M5.Lcd.print("Pressure");

  M5.Lcd.drawFastHLine(0,180,M5.Lcd.width(),TFT_DARKGREEN);
  M5.Lcd.setCursor(5, 185);
  M5.Lcd.setTextSize(1);
  M5.Lcd.println("Lat.");
  M5.Lcd.setCursor(145, 185);
  M5.Lcd.println("Lon.");
}



//----------------SETUP & MAIN LOOP----------------------------------------


void setup() {
    // Debug console    
    Serial.begin(115200);
    //Blynk start
    Blynk.begin(auth, ssid, pass, "BLYNK-SERVER-IP",8080);
    // M5 start
    M5.begin();
    // I2C start
    Wire.begin();
    //gps Serial start
    ss.begin(9600,SERIAL_8N1,36,26);
    // MLX start
    mlx.begin();
    //GSM Serial start
    Serial2.begin(115200, SERIAL_8N1, 16, 17); 
    //SD start
    SD.begin();
    //dateTime setup
    setupDateTime();
    client.enableDebuggingMessages(); 
      
    M5.Lcd.println("Starting up!");
    // Setup a function to be called every second
    timer.setInterval(20000, myTimerEvent);
    M5.Lcd.setBrightness(50);
    M5.Lcd.setTextColor(BLACK, WHITE);

    if (!SD.begin()) {
      Serial.println("Card failed, or not present");
      M5.Lcd.println("Card failed, or not present");
    }
    while (!bme.begin(0x76)){  
      Serial.println("Could not find a valid BMP280 sensor, check wiring!");
      M5.Lcd.println("Could not find a valid BMP280 sensor, check wiring!");
      delay(1000);
    }

    while (!mlx.begin()){  
      Serial.println("Could not find a valid MLX90614 sensor, check wiring!");
      M5.Lcd.println("Could not find a valid MLX90614 sensor, check wiring!");
      delay(1000);
    }
    M5.Lcd.println("Done!");
    delay(500);
    printDefaultScreen();
    //M5.setWakeupButton(BUTTON_A_PIN);
}

void loop() {
    M5.update();
    if(M5.BtnA.wasPressed()){
      if(!Lcdsleep){
        M5.Lcd.sleep();
        M5.Lcd.setBrightness(0);
        Lcdsleep = 1;
      } else{
        M5.Lcd.wakeup();
        M5.Lcd.setBrightness(50);
        Lcdsleep = 0;
      }     
    }

    if(M5.BtnB.wasPressed()){
      //M5.Lcd.clear(WHITE);
      printDefaultScreen();
      sb.reset();
      sb.print("Measurement taken at: ");
      sb2.reset();
      M5.Lcd.setCursor(70, 55);
      M5.Lcd.setTextSize(2);
      M5.Lcd.println("..Reading..");
      M5.Speaker.tone(1000, 200);
      delay(100);
      M5.Speaker.mute();

      //----------time-------------
      M5.Lcd.setCursor(5,110);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(TFT_DARKGREY,WHITE);
      DateTime.begin();
      if (!DateTime.isTimeValid()) {
        M5.Lcd.println("Failed to get time from server.");
        M5.Lcd.setTextColor(TFT_BLACK,WHITE);
        sb.println("Time NULL");
        sb2.println("Time NULL ");
      }else{
        M5.Lcd.print("Measurement taken at");
        M5.Lcd.setCursor(135, 110);
        M5.Lcd.setTextColor(TFT_BLUE,WHITE);
        M5.Lcd.print(DateTime.toString().c_str());
        Blynk.virtualWrite(V7, DateTime.toString().c_str());
        M5.Lcd.setTextColor(TFT_BLACK,WHITE);
        sb.println(DateTime.toString().c_str());
        sb2.printf("%s ",DateTime.toString().c_str());
      }
      
      //-------------MLX90614 Temp----------------------
      delay(2000);  // Delay to give sensor some time to get the temperature
      MLXtemp = mlx.readObjectTempC(); //Get temperature
      M5.Lcd.setCursor(45, 40); // Displaying and sending temp. data
      M5.Lcd.setTextSize(6);
      if(MLXtemp > 37.7){
        M5.Lcd.setTextColor(TFT_RED,WHITE);
      }else{
        M5.Lcd.setTextColor(TFT_NAVY,WHITE);
      }  
      M5.Lcd.print(MLXtemp); 
      M5.Lcd.setTextSize(3);
      M5.Lcd.println(" *C");
      M5.Lcd.setTextColor(TFT_BLACK,WHITE);
      Serial.print("Object = "); Serial.print(MLXtemp); Serial.println("*C");
      Blynk.virtualWrite(V6, MLXtemp);
      sb.printf("Temperature: %2.2f*C\r\n", MLXtemp);
      sb2.printf("%2.2f*C ", MLXtemp);
      M5.Speaker.tone(1047, 200); // A tune that indicates a finished measuring
      delay(50);
      M5.Speaker.tone(2093, 200);
      delay(50);
      M5.Speaker.mute();

      //-------------Env Temp---------------------------
      tmp = dht12.readTemperature(); //getting the data
      hum = dht12.readHumidity();
      pressure = bme.readPressure();
      
      Serial.printf("Temperature: %2.2f*C  Humidity: %0.2f%%  Pressure: %0.2fPa\r\n", tmp, hum, pressure);
      Blynk.virtualWrite(V3, tmp);
      Blynk.virtualWrite(V4, hum);
      Blynk.virtualWrite(V5, pressure);
      //M5.Lcd.printf("Temperature: %2.2f*C  Humidity: %0.2f%%  Pressure: %0.2fPa\r\n", tmp, hum, pressure);
      sb.printf("Env. Temp: %2.2f*C\r\nHumidity: %0.2f%%\r\nPressure: %0.2fPa\r\n", tmp, hum, pressure);
      sb2.printf("%2.2f*C %0.2f%% %0.2fPa ", tmp, hum, pressure);
      M5.Lcd.setCursor(5, 147);
      M5.Lcd.setTextSize(2);
      M5.Lcd.printf("%2.2f", tmp);
      M5.Lcd.setTextSize(1);
      M5.Lcd.print("*C");
      M5.Lcd.setCursor(100, 147);
      M5.Lcd.setTextSize(2);
      M5.Lcd.printf("%0.2f%%", hum);
      M5.Lcd.setCursor(195, 147);
      M5.Lcd.printf("%0.2f", pressure);
      M5.Lcd.setTextSize(1);
      M5.Lcd.print("Pa");
      
      // ---------------------GPS-------------------------------------
      while (ss.available() == 0 && flag == 1)  //Check whether there is a GPS available
      {
        timeout ++;
        delay(100);
        if(timeout > 15)
        {
          timeout = 0;
          flag = 0;
          Serial.println("GPS not available");   
          M5.Lcd.setCursor(0, 100);
          M5.Lcd.setTextColor(RED, WHITE);
          M5.Lcd.setTextSize(3);
          M5.Lcd.printf("GPS not available"); 
          M5.Lcd.setTextColor(BLACK, WHITE);        
        }
      }
      flag = 1;
      timeout = 0;
      while ((ss.available() > 0 && GPS_output == 0) || (gps.location.isUpdated()==0 && GPS_output == 0 && flag == 1)) 
      {
        if (gps.encode(ss.read())) //Reading and parsing the data
        {
          if (gps.location.isUpdated())
          {
            displayInfo();
            M5.Lcd.setTextColor(BLACK, WHITE);
            M5.Lcd.setTextSize(2);
            M5.Lcd.setCursor(5, 205);
            //lat_data = gps.location.lat();
            //lng_data = gps.location.lng();
            M5.Lcd.printf("%2.6f", lat_data);   
            M5.Lcd.setCursor(145, 205); 
            M5.Lcd.printf("%3.6f", lng_data);   
            sb.printf("Latitude: %2.6f\r\nLongitude: %3.6f", lat_data, lng_data);       
            sb2.printf("%2.6f %3.6f \r\n", lat_data, lng_data);          
            GPS_output = 1; 
          }
          else
          {
            timeout ++;
            delay(100);
            Serial.println("Not updated");
            if(timeout > 15)
            {
              timeout = 0;
              flag = 0;
              lat_data = -1;
              lng_data = -1;
              Serial.println("GPS update timeout");   
              M5.Lcd.setCursor(5, 205);
              M5.Lcd.setTextColor(RED, WHITE);
              M5.Lcd.setTextSize(2);
              M5.Lcd.printf("GPS update timeout");  
              sb.print("No GPS");  
              sb2.printf("No GPS \r\n"); 
              Blynk.virtualWrite(V1, "No GPS");   
              Blynk.virtualWrite(V2, "No GPS");
              M5.Lcd.setTextColor(BLACK, WHITE);    
            }          
          }
        }
      }
      
      //-----------------Log data to TF card------------------------
      File f = SD.open("/data_log.txt", FILE_APPEND);
        if (f) {
          f.print(buff2);
          f.close();
      }
      
      //------------------MQTT data publish-------------------------
      client.publish("data",buff2);
      
      //------------------Flags-------------------------------------
      sendSMSOK = 1;
      GPS_output = 0;
      flag = 1;
      timeout = 0;
    }

    if(M5.BtnC.wasPressed()){
      //------------------Send SMS----------------------------------
      if(!sendSMSOK){
        Serial.println("No data to be sent through SMS, measure first!");
        M5.Lcd.println("No data");
      }else{
        M5.Speaker.tone(1047, 200);
        delay(100);
        M5.Speaker.tone(1319, 200);
        delay(100);
        M5.Speaker.tone(1568, 200);
        delay(100);
        M5.Speaker.tone(2093, 200);
        delay(100);
        sendSMS();
      }
    }
    
    client.loop();  //MQTT client loop
    Blynk.run();   //Blynk client loop
    timer.run(); // Initiates BlynkTimer
}
