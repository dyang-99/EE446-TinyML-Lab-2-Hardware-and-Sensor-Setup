#include <Arduino_APDS9960.h>
#include <PDM.h>
#include <Arduino_BMI270_BMM150.h>
#include <cmath>
#include <iostream>
#include <string>


//threshold values
static int t_mic = 60;
static float t_IMU = 1;
static int t_light = 100;
static int t_prox = 200;


short sampleBuffer [256]; 
volatile int samplesRead = 0;

void onPDMdata() { 
  int bytesAvailable = PDM.available(); 
  PDM.read(sampleBuffer, bytesAvailable); 
  samplesRead = bytesAvailable / 2;
}


//------------------------------------------------
// Void Setup
//------------------------------------------------
void setup() { 
  //Serial setup
  Serial.begin(115200); 
  delay(1500);

  //Microphone Setup----------------------
  PDM.onReceive(onPDMdata);

  if (!PDM.begin(1, 16000)){
    Serial.println("Failed to start PDM microphone.");
    while (1);
  }

  //IMU setup-----------------------------
  if (!IMU.begin()) { 
    Serial.println("Failed to initialize IMU."); 
    while (1); 
  }

  //Prixomiy &ambient light setup----------
  if (!APDS.begin() ) { 
    Serial.println("Failed to initialize APDS9960 sensor."); 
    while (1); 
  }

  Serial.println("Ambient light and color test started"); 
  Serial.println("r,g,b, clear"); 
} 



//------------------------------------------------
// Void Loop
//------------------------------------------------
void loop() { 
  int level; //mic value
  int r, g, b, c;//ambient light value
  float x, y, z, IMU_mag;//IMU values
  int proximity; //proximity
  bool f_sound = false, f_dark = false, f_moving = false, f_near = false; //flags for 
  std::string situation;
  
  // Microphone loop-----------------------
  if (samplesRead) { 
    long sum = 0;
    for (int i = 0; i < samplesRead; i++) { 
      sum += abs(sampleBuffer[i]); 
    }
    level = sum / samplesRead; 
    samplesRead = 0;
  }

  //IMU Loop--------------------------------
  if (IMU.accelerationAvailable()) { 
    IMU. readAcceleration(x, y, z); 
    IMU_mag = std::sqrt(x*x + y*y + z*z); //get the magnitude of acceleration
  }

  //Proximity Loop--------------------------
  if (APDS.proximityAvailable()) { 
    proximity = APDS.readProximity(); 
  }
  //ambient loop----------------------------
  if (APDS.colorAvailable()) { 
    APDS.readColor(r, g, b, c); 
  }


  //Print raw data--------------------------
  Serial.print("raw,mic=");
  Serial.print(level);

  Serial.print(",clear=");
  Serial.print(c);

  Serial.print(",motion=");
  Serial.print(IMU_mag);

  Serial.print(",prox=");
  Serial.print(proximity);

  //flags----------------------------------
  if (level > t_mic){
    f_sound = true;
  }
  if (IMU_mag > t_IMU){
    f_moving = true;
  }
  if (c < t_light){
    f_dark = true;
  }
  if (proximity < t_prox){
    f_near = true;
  }
  Serial.print("\nflags,sound=");
  Serial.print(f_sound);

  Serial.print(",dark=");
  Serial.print(f_dark);

  Serial.print(",moving=");
  Serial.print(f_moving);

  Serial.print(",near=");
  Serial.print(f_near);

  //state-----------------------------------
  if (f_sound & !f_dark & !f_moving & !f_near){
    situation = "NOISY_BRIGHT_STEADY_FAR";
  }else if (f_sound & !f_dark & f_moving & f_near){
    situation = "NOISY_BRIGHT_MOVING_NEAR";
  }else if (!f_sound & !f_dark & !f_moving & !f_near){
    situation = "QUIET_BRIGHT_STEADY_FAR";
  }else if (!f_sound & f_dark & !f_moving & f_near){
    situation = "QUIET_DARK_STEADY_NEAR";
  }
  Serial.println("\nstate,");
  Serial.print(situation.c_str());
  Serial.print("\n");
  delay(1000);
}