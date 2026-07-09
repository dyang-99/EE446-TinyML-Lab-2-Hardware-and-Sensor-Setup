#include <Arduino_APDS9960.h>
#include <Arduino_HS300x.h> 
#include <Arduino_BMI270_BMM150.h>
#include <cmath>
#include <string>


//threshold values
static float t_temp = 0.2;
static float t_humid = 5;
static int t_color_light = 100;
static int t_mag = 30;

//------------------------------------------------
// Void Setup
//------------------------------------------------
void setup() { 
  //Serial setup
  Serial.begin(115200); 
  delay(1500);

  //Temp&humidity Setup----------------------
  if (!HS300x.begin()) { 
    Serial.println("Failed to initialize humidity/temperature sensor."); 
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

  Serial.println("Setup completed");
} 


float temp, temp_p = 0, humidity, humidity_p = 0; //temp and humidity
float x, y, z, mag, mag_p = 0; //magnetic field
int r, r_p = 0, g, g_p = 0, b, b_p = 0, c, c_p = 0;//ambient light value

//------------------------------------------------
// Void Loop
//------------------------------------------------
void loop() { 
  bool f_temp = false, f_humid = false, f_light_color = false, f_mag = false; //flags 
  std::string event;
  
  // Humidity and Temp loop-----------------
  temp = HS300x.readTemperature();
  humidity = HS300x.readHumidity();

  //Magnetic Field Loop---------------------
  if (IMU.magneticFieldAvailable()) { 
    IMU.readMagneticField(x, y, z); 
    mag = std::sqrt(x*x + y*y + z*z); //get the magnitude of magnetic field
  }

  //ambient loop----------------------------
  if (APDS.colorAvailable()) { 
    APDS.readColor(r, g, b, c); 
  }

  //Print raw data--------------------------
  Serial.print("raw,rh=");
  Serial.print(humidity);

  Serial.print(",temp=");
  Serial.print(temp);

  Serial.print(",mag=");
  Serial.print(mag);

  Serial.print(",r=");
  Serial.print(r);
  Serial.print(",g=");
  Serial.print(g);
  Serial.print(",b=");
  Serial.print(b);
  Serial.print(",clear=");
  Serial.print(c);

  //flags----------------------------------
  if ((temp-temp_p) > t_temp){
    f_temp = true;
  }
  if (humidity-humidity_p > t_humid){
    f_humid = true;
  }
  if (abs(mag-mag_p) > t_mag){
    f_mag = true;
  }
  int color_light_change = abs(r-r_p) + abs(g-g_p) + abs(b-b_p) + abs(c-c_p);
  if (color_light_change > t_color_light){
    f_light_color = true;
  }

  Serial.print("\nflags,humid_jump=");
  Serial.print(f_humid);

  Serial.print(",temp_rise=");
  Serial.print(f_temp);

  Serial.print(",mag_shift=");
  Serial.print(f_mag);

  Serial.print(",light_or_color_change=");
  Serial.print(f_light_color);

  //state-----------------------------------
  if (f_humid | f_temp){
    event = "BREATH_OR_WARM_AIR_EVENT";
  }else if (f_mag){
    event = "MAGNETIC_DISTURBANCE_EVENT";
  }else if (f_light_color){
    event = "LIGHT_OR_COLOR_CHANGE_EVENT";
  }else{
    event = "BASELINE_NORMAL";
  }
  Serial.println("\nevent,");
  Serial.print(event.c_str());
  Serial.print("\n");

  temp_p = temp;
  humidity_p = humidity;
  r_p = r;
  g_p = g;
  b_p = b;
  c_p = c;
  mag_p = mag;
  delay(2000);
}