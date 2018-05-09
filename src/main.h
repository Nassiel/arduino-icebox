
#include "Arduino.h"

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

#define PELTIER_DIR 4
#define PUMP_ENABLE 5

#define FAN_PWM 6
#define PELTIER_PWM 7
// reserve pin 8 to use it with the same timer (4) in the future
#define INNER_SENSOR 9
#define HOTSIDE_SENSOR 10
#define COLDSIDE_SENSOR 11

#define LED_PELTIER_ON 12

//Peltier working modes
#define COLD_MODE 0x0
#define HOT_MODE 0x1


#define OFF LOW
#define ON HIGH
#define MAX 0xFF

#define COLD_LIMIT 40.0f
#define WARM_LIMIT 10.0f

void executeActions();
bool getTemperature();
void evaluateVariables();
void stablishVariables();
void restoreVariables();

void ColdMode();
void HotMode();
void CalculatePwm(float tTemp, float lastTTemp);

#define HMIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define HMAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define REACH_LIMIT(temp) (((temp) > (COLD_LIMIT)) || ((temp) < (WARM_LIMIT)))
#define GET_MODE(temp) (((temp) < (WARM_LIMIT)) ? (HOT_MODE) : (COLD_MODE))