  /* ARBikeJapan
  Designed for Santa Cruz V10 bike
 Version by Greg Brown, PTC Japan
 June 2015
 
 Hardware:
 Raspberry Pi B+
 Arduberry
 Grove Sensor Shield
 Grove 10DOF IMU (connected on i2c) 
 Grove LED Bar (connected on Digital 8,9)
 Grove IR Relective sensors (x3)  (connected on Digital 2,3,4)
 Grove Ultrasonic Ranger (Digital 7)
 10k potentiometer (A0)
 */
 
  #include <PinChangeInt.h>
  #include <Ultrasonic.h>
  #include <Wire.h>
  #include "I2Cdev.h"
  #include "MPU9250.h"
  #include "BMP180.h"
  #include <Grove_LED_Bar.h> // eye candy ;
  
  Grove_LED_Bar bar(9, 8, 0);  // Clock pin, Data pin, Orientation
  MPU9250 accelgyro;
  Ultrasonic ultrasonic(7);  // Grove Ultrasonic Ranger is on Digital 7
  BMP180 Barometer;
  
  // Arduino port mapping
  #define FRONTWHEEL     2 
  #define REARWHEEL      3 
  #define PEDALS         4  
  #define FORK           A0  
  #define SUSPENSIONLINK A2 // TO DO!!! NOT USED YET
  
  // Sensor constants
  const int FRW_TICKS = 12; // specific to number of reflective points on rotor
  const int CRK_TICKS = 8;
  const int RRW_TICKS = 12;
  const float FORK_OFFSET = 250.0; // specific each fork-tire distance
  
  // variables for MPU-9250 9-DOF IMU
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  int16_t mx, my, mz;
  
  // variables for BMP180 pressure/temperature sensor
  float temperature;
  float pressure;
  float atm;
  float altitude;      
    
  // variables for wheel sensors
  volatile unsigned long currentFrontWheelTick = 0;
  volatile unsigned long currentRearWheelTick = 0;
  volatile unsigned long currentPedalsTick = 0;
  volatile unsigned long lastFrontWheelTick = 0;
  volatile unsigned long lastRearWheelTick = 0;
  volatile unsigned long lastPedalsTick = 0;
  
  // variables for Grove Ultrasonic Ranger (fork sensor)
  float frontShockDisplacement = 0.0;
  unsigned long lastpingmillis = 0;
  
  // INTERRUPT ROUTINES
  void interruptFrontWheel() {
    lastFrontWheelTick = currentFrontWheelTick;
    currentFrontWheelTick = millis();
  }
  
  void interruptRearWheel() {
    lastRearWheelTick = currentRearWheelTick;
    currentRearWheelTick = millis();
  }
  
  void interruptPedals() {
    lastPedalsTick = currentPedalsTick;
    currentPedalsTick = millis();
  }
    
  // Function to compute rpm of wheels/crank
  int computeRPM(unsigned long lastTime, unsigned long currentTime, int ticksperturn) {
    unsigned long deltaTime = currentTime - lastTime;
    if (deltaTime >= 60000/ticksperturn || deltaTime == 0 || (millis() - currentTime) >= 1000)
      return 0;
    else 
      return 60000/(ticksperturn*deltaTime);
  }
  
  
  void setup() {
    Serial.begin(19200);
    Wire.begin();
    
    // setup the interrupts for IR sensor on wheels/crank
    pinMode(PEDALS, INPUT);
    pinMode(FRONTWHEEL, INPUT);
    pinMode(REARWHEEL, INPUT);
    attachPinChangeInterrupt(PEDALS, interruptPedals,FALLING);
    attachPinChangeInterrupt(REARWHEEL, interruptRearWheel, FALLING);
    attachPinChangeInterrupt(FRONTWHEEL, interruptFrontWheel, FALLING);
    
    // initialize IMU ... MPU-9250 device
    Serial.println("Initializing I2C devices...");
    accelgyro.initialize();
    // verify connection
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU9250 connection successful" : "MPU9250 connection failed");

    Barometer.init();

    //Start the LED bargraph
    bar.begin();
  } 
  
  // Main loop
  void loop() {
     
    bar.setLevel(0);
    
    int frontWheelRPM = computeRPM(lastFrontWheelTick,currentFrontWheelTick,FRW_TICKS);
    int rearWheelRPM  = computeRPM(lastRearWheelTick,currentRearWheelTick,RRW_TICKS); 
    int pedalsRPM = computeRPM(lastPedalsTick,currentPedalsTick,CRK_TICKS); 
  
    //FORK PING WAS PREVIOUSLY HERE, TRYING IT INSIDE THE "S" LOOP
   
    if(Serial.read()=='s')  {
//    if (1) {  // for testing only. Comment out the line above and use this one to run continously
    
    bar.setLevel(1);  
      
    // Calculate fork extension using Ultrasonic range sensor  
    if (millis() - lastpingmillis >=35) { 
        ultrasonic.MeasureInCentimeters();
        frontShockDisplacement = (ultrasonic.RangeInCentimeters*10.0-FORK_OFFSET);     
        lastpingmillis = millis();
        bar.setLevel(2);
    }     
      
    // read the input on analog pins:
    int forkRotarySensor = analogRead(FORK);
    bar.setLevel(3);
    int suspensionLinkRotarySensor = analogRead(SUSPENSIONLINK);
    bar.setLevel(4);
     
    Serial.print(frontWheelRPM);  Serial.print("  ");
    Serial.print(rearWheelRPM);   Serial.print("  ");
    Serial.print(pedalsRPM);      Serial.print("  ");   
    
    // map the analog sensor reading to range of motion (270 degrees i.e. +/- 135 deg) of potentiometer
    // Through inspection,the digital range of 10k pot turns out to be 0 to 709
    Serial.print(`map(forkRotarySensor,0,709,-135,135));  Serial.print("  "); 

//    NOT USING REAR SUSPENSION SENSOR YET   
//    Serial.print(suspensionLinkRotarySensor/1023.0*184.25 - 30.07344);  Serial.print("  ");
    Serial.print(0.0);  Serial.print("  ");   
    
    if(frontShockDisplacement >= -7.0) {
    Serial.print("0.0");   Serial.print("  ");    
    }
    else {
    Serial.print(frontShockDisplacement);   Serial.print("  ");
    }
//  Serial.print(frontShockDisplacement);   Serial.print("  ");

    // read raw accel/gyro measurements from device
    accelgyro.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);
    float pitch = asin(-((double) ax / 16384));
    float roll = asin(((double) ay / 16384) / cos(pitch));
    
    // Poor man's roll/pitch not using Fusion yet...
    Serial.print(roll*(180/PI)); Serial.print("  ");
    Serial.print(pitch*(180/PI)); Serial.print("  ");
    Serial.print(gz); Serial.print("  ");
    bar.setLevel(5);
    
    //Accelerations
    Serial.print(ax/16160.0);   Serial.print("  ");
    Serial.print(ay/16160.0);   Serial.print("  ");
    Serial.print(az/16160.0);   Serial.print("  ");
    bar.setLevel(6);
    
    // Digital compass
    Serial.print(mx);  Serial.print("  ");
    Serial.print(my);  Serial.print("  ");
    Serial.print(mz);  Serial.print("  ");
    bar.setLevel(7);
   
    temperature = Barometer.bmp180GetTemperature(Barometer.bmp180ReadUT()); //Get the temperature, bmp180ReadUT MUST be called first
    //pressure = Barometer.bmp180GetPressure(Barometer.bmp180ReadUP());//Get the temperature
    //altitude = Barometer.calcAltitude(pressure); //Uncompensated caculation - in Meters
    Serial.print(temperature); Serial.println("  ");  
    bar.setLevel(8);
    
    }
  
  }
