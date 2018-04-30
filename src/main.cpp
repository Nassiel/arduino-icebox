/**
 * Blink
 *
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */

#include "Arduino.h"
#include "main.h"
#include "DHT.h"

int myPrescaler = 1; //Time pre-scaler

int pumpEnable = OFF; //By default, off
int peltierMode = COLD_MODE; //By default, get inside colder
int ledPeltierOn = OFF; //By default, off
int ledBuiltIn = OFF; //By default, off

int fanPwm = OFF; //By default, off
int peltierPwm = OFF; //By default, off

DHT* boxSensor = NULL;
DHT* coldSideSensor = NULL;
DHT* hotSideSensor = NULL;

float boxTemp = 0;
float coldSideTemp = 0;
float hotSideTemp = 0;

float lastBoxTemp = 0;
float lastColdSideTemp = 0;
float lastHotSideTemp = 0;

void setup()
{
  // configure digital outputs  
  pinMode(PUMP_ENABLE, OUTPUT);
  pinMode(PELTIER_DIR, OUTPUT);
  pinMode(LED_PELTIER_ON, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // configure analog outputs
  pinMode(FAN_PWM, OUTPUT);
  pinMode(PELTIER_PWM, OUTPUT);

  // configure anagog inputs
  boxSensor = new DHT(INNER_SENSOR, DHT11);
  coldSideSensor = new DHT(HOTSIDE_SENSOR, DHT11);
  hotSideSensor = new DHT(COLDSIDE_SENSOR, DHT11);

  // timers prescale configuration reference for mega2560 
  // https://forum.arduino.cc/index.php?topic=72092.msg541587#msg541587 
  // set timer 4 (pins 6, 7, 8)
  TCCR4B &= ~7; //111 : set all to 0
  TCCR4B |= myPrescaler;

  // initialize temperature & humidity lectures
  boxSensor->begin();
  coldSideSensor->begin();
  hotSideSensor->begin();

  // initialize variables
  getTemperature();
  stablishVariables();
}

void executeActions(){  
  // initialize digital outputs
  digitalWrite(PUMP_ENABLE, pumpEnable);
  digitalWrite(PELTIER_DIR, peltierMode);
  digitalWrite(LED_PELTIER_ON, ledPeltierOn);
  digitalWrite(LED_BUILTIN, ledBuiltIn);

  // initialize analog outputs
  analogWrite(PELTIER_PWM, peltierPwm);
  analogWrite(FAN_PWM, fanPwm);  
}

void restoreVariables(){
  boxTemp = lastBoxTemp;
  coldSideTemp = lastColdSideTemp;
  hotSideTemp = lastHotSideTemp;
}

bool getTemperature(){
  boxTemp = boxSensor->readTemperature();
  coldSideTemp = coldSideSensor->readTemperature();
  hotSideTemp = hotSideSensor->readTemperature();

  if(isnan(boxTemp) || isnan(coldSideTemp) || isnan(hotSideTemp)){
    restoreVariables();
    return false;
  }
  else 
    return true;
}

void HotMode(){ //The temperature is bellow 10 degrees
  float tTemp = boxTemp * -1.0f + 10.0f; //10 become 0 and everything bellow get more and more possitive.
  float lastTTemp = lastBoxTemp * -1.0f + 10.0f;
  CalculatePwm(tTemp, lastTTemp);
}

void ColdMode(){
  float tTemp = boxTemp - 40.0f; //40 become 0 and everything above get more possitive
  float lastTTemp = lastBoxTemp - 40.0f;
  CalculatePwm(tTemp, lastTTemp);
}

void CalculatePwm(float tTemp, float lastTTemp){
  float diff = tTemp - lastTTemp;
  int runningPeltierPwm = peltierPwm;
  int runningFanPwm = fanPwm;

  int gradientValue = (int)(255.0 * (1.0-pow((lastTTemp / tTemp), (diff))));
  int linearValue = (int)(5.1 * tTemp - 204.0);
  peltierPwm = MAX(gradientValue, linearValue);

  //While i don't have the water temperature, it'll be the same "power"
  fanPwm = peltierPwm;
}

void evaluateVariables(){
  peltierMode = GET_MODE(boxTemp);

  if(REACH_LIMIT(boxTemp)){
    pumpEnable = ON;
    ledPeltierOn = ON;
    if(peltierMode == HOT_MODE) HotMode();
    else ColdMode();
  }
  else{
    pumpEnable = OFF;
    ledPeltierOn = OFF;
    peltierPwm = 0;
    fanPwm = 0;
  }
}

void stablishVariables(){
  lastBoxTemp = boxTemp;
  lastColdSideTemp = coldSideTemp;
  lastHotSideTemp = hotSideTemp;
}

void loop()
{
  if(getTemperature()){
    evaluateVariables();
    executeActions();
    stablishVariables();
    delay(5000); //DHT11 5 second recomendation
  }
  else
    delay(500); // we should control that errors doesn't happen to many times
}