#include <Arduino_FreeRTOS.h>
#include <Servo.h>
#include "FastLED.h"

#include "ultrasonido.h"

#define PIN_RBGLED 4
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

// Ultrasound
#define TRIG_PIN 13
#define ECHO_PIN 12
UltraSoundClass ultrasound(ECHO_PIN, TRIG_PIN);

// Infra-red
#define PIN_ITR20001_LEFT   A2
#define PIN_ITR20001_MIDDLE A1
#define PIN_ITR20001_RIGHT  A0

// Motors
#define PIN_Motor_STBY 3

// A Group Motors Right Side
#define PIN_Motor_RIN_1 7
#define PIN_Motor_PWMR 5

// B Group Motors Left Side
#define PIN_Motor_LIN_1 8
#define PIN_Motor_PWML 6

/*-----------------------------------------------------------------------------*/

// PD

float Kp = 0.05; // Proportional gain
float Kd = 0.25; // Derivative gain

// Internal variables of PD
int error = 0;
int lastError = 0;
int P = 0;
int D = 0;

/*-----------------------------------------------------------------------------*/

// Task rates
#define PERIODIC_T1 200   // ULTRASOUND
#define PERIODIC_T2 50    // INFRARED (PD Loop)
#define PERIODIC_T3 4000   // COMS

/*-----------------------------------------------------------------------------*/

// Robot parameters
// Velocities
volatile int MAXIMUM_SPEED = 220; // Max speed allowed
volatile int BASE_SPEED = 190;     // Base speed
int vel_turn = 160; // Vel for turning

// To check the side where the line was last seen
float prev_right = -1;
float prev_left = -1;

/* 
* Infrared counter indicates how many measures the infrared took in total and 
* also another counter to measure how many times the black line was detected
*/
/* Readable from different tasks */
volatile float infrared_total = 0;
volatile float infrared_detected = 0;

int start_time = 0;


bool lost = false;

volatile bool finished = false;

/*-----------------------------------------------------------------------------*/

#define COMPUTATION_TIME_ON_T1 50

#define DIST_STOP 10 // With a bit of gap because the intertia of movement
#define DIST_SLOW 15 // Distance where the robot is going to star slowing the speed

/*-----------------------------------------------------------------------------*/

// LED configuration
uint32_t Color(uint8_t r, uint8_t g, uint8_t b)
{
  return (((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

// Calculates difference of time given two measures
float get_time_diff(long t0, long t1) {
  if (t0 < 0 || t1 < 0)
    return -1;
  else
    return t1 - t0; // return in millis
}

void setup()
{
  Serial.begin(9600);
  
  pinMode(PIN_Motor_STBY, OUTPUT);
  digitalWrite(PIN_Motor_STBY, HIGH);

  // Right motor
  pinMode(PIN_Motor_RIN_1, OUTPUT);
  pinMode(PIN_Motor_PWMR, OUTPUT);
  digitalWrite(PIN_Motor_RIN_1, HIGH);
  analogWrite(PIN_Motor_PWMR, 0);

  // Left motor
  pinMode(PIN_Motor_LIN_1, OUTPUT);
  pinMode(PIN_Motor_PWML, OUTPUT);
  digitalWrite(PIN_Motor_LIN_1, HIGH);
  analogWrite(PIN_Motor_PWML, 0);

  // LED
  FastLED.addLeds<NEOPIXEL, PIN_RBGLED>(leds, NUM_LEDS);
  FastLED.setBrightness(20);
  
  // Wait until the ESP is ready. Will recive a start signal though Serial port
  while(1) {
    if (Serial.available()) {
      char c = Serial.read();
      
      if (c == 'r')  {
        start_time = millis(); // Start the timer
        String start_lap = "{sl}";  // Send start lap when signal received
        Serial.print(start_lap);
        break;
      }
    }
  }
  
  // Once it receives start signal create tasks and start execution
  xTaskCreate(MyTask1, "TaskUltra", 256, NULL, 2, NULL); // Ultrasound
  xTaskCreate(MyTask2, "TaskPID", 256, NULL, 3, NULL); // PID
  xTaskCreate(MyTask3, "TaskCom", 100, NULL, 1, NULL);   // Ping
}

void loop() {} // Void for freeRTOS

// Ultrasound task
static void MyTask1(void* pvParameters)
{
  TickType_t xLastWakeTime, aux;
  xLastWakeTime = xTaskGetTickCount();
  
  while(1)
  {
    aux = xLastWakeTime;
      
    float dist = ultrasound.get_dist();

    // Filter wrong measures (0) and check if obstacle is close enough to slow the vel
    if (dist <= (DIST_SLOW + DIST_STOP) && dist > 0.1 ) {
      MAXIMUM_SPEED = 100;
      BASE_SPEED = 75;
    }
    
    // Filter wrong measures (0) and check if obstacle is close enough
    if (dist <= DIST_STOP && dist > 0.1) { 
      digitalWrite(PIN_Motor_STBY, LOW); // STOP

      // Calculate percentage of lost line
      float percentage = (infrared_detected / infrared_total) * 100;

      // Calculate total lap duration
      long duration = get_time_diff(start_time, millis());

      // Send statistics of how much the robot saw the line during circuit
      String  visible_line = "{vl" + String(percentage) + "}";

      // duration of whole program
      String end_lap = "{el" + String(duration) + "}";
      
      // To send right distance measure
      delay(1000);
      dist = ultrasound.get_dist(); // Check dist with the robot full stoped
      String obst_det = "{od" + String(dist) + "}"; // Send distance from object when detected

      // Send the msg
      Serial.print(visible_line);
      Serial.print(obst_det);
      Serial.print(end_lap);

      finished = true;
      vTaskDelete(NULL); // kill task

    }

    while ( (aux - xLastWakeTime)*portTICK_PERIOD_MS < COMPUTATION_TIME_ON_T1) {
      aux = xTaskGetTickCount();
    }

    xTaskDelayUntil( &xLastWakeTime, ( PERIODIC_T1 / portTICK_PERIOD_MS ) );
  }
}

// Infrared and PD task
static void MyTask2(void* pvParameters)
{
  TickType_t xLastWakeTime, aux;

  xLastWakeTime = xTaskGetTickCount();
  while(1)
  {
    // When finished = true end the task
    if (finished) {
      vTaskDelete(NULL);
    }
    aux = xLastWakeTime;

    // Read infrareds
    int left = analogRead(PIN_ITR20001_LEFT);
    int middle = analogRead(PIN_ITR20001_MIDDLE);
    int right = analogRead(PIN_ITR20001_RIGHT);

    infrared_total++;

    // If line was lost
    if (left <= 600 && middle <= 600 && right <= 600) {
      // Only send this messages on first notice of being lost
      if (lost == false) {
        lost = true;
        String lost_line = "{ll}"; // Send that it lost the black line
        Serial.print(lost_line);
        String line_search = "{ls}"; // Send that it will start looking for the line
        Serial.print(line_search);
      }

      // Configure LED color to red
      FastLED.showColor(Color(255, 0, 0));

      /* Find line protocol, if last seen in left sensor, turn left and viceversa */
      if (prev_right > 600) {
        analogWrite(PIN_Motor_PWML, vel_turn);
        analogWrite(PIN_Motor_PWMR, 0);

      } else if (prev_left > 600) {
        analogWrite(PIN_Motor_PWML, 0);
        analogWrite(PIN_Motor_PWMR, vel_turn);
      }

    // If it detects line in any of the infrareds
    } else {
      // Only send this messages on first notice of finding the line
      if (lost == true) {
        lost = false;
        String stop_search = "{ss}";  // it stopped searching for the line because it found it
        Serial.print(stop_search);
        String line_found = "{lf}"; // send that it found the line again only after losing it (if condition)
        Serial.print(line_found);
      }

      // Increase black line detection counter
      infrared_detected++;

      // PD logic
      error = right - left;

      P = error;
      D = error - lastError;

      float pid_output = (Kp * P) + (Kd * D);

      lastError = error; // Save last error to compare after

      // Calculate velocities
      int speedLeft = BASE_SPEED + pid_output;
      int speedRight = BASE_SPEED - pid_output;

      // Clamp to adequate values
      if (speedLeft > MAXIMUM_SPEED) {
        speedLeft = MAXIMUM_SPEED;
      }
      if (speedLeft < 0) {
        speedLeft = 0;
      }

      if (speedRight > MAXIMUM_SPEED) {
        speedRight = MAXIMUM_SPEED;
      }
      if (speedRight < 0) {
        speedRight = 0;
      }

      // Apply in motors
      digitalWrite(PIN_Motor_LIN_1, HIGH);
      digitalWrite(PIN_Motor_RIN_1, HIGH);
      
      analogWrite(PIN_Motor_PWML, speedLeft);
      analogWrite(PIN_Motor_PWMR, speedRight);

      // Save infrared measures for finding line protocol
      prev_right = right;
      prev_left = left;
      
      FastLED.showColor(Color(0, 255, 0));
    }

    xTaskDelayUntil( &xLastWakeTime, ( PERIODIC_T2 / portTICK_PERIOD_MS ) );
  }
}

// Send ping every 4 s
static void MyTask3(void* pvParameters)
{
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  
  while(1)
  {
    if (finished) {
      vTaskDelete(NULL);
    }

    long ping_time = get_time_diff(start_time, millis());
    String ping_message = "{pm" + String(ping_time) + "}";
    Serial.print(ping_message); // Send ping message every 4s with low priority
       
    xTaskDelayUntil( &xLastWakeTime, ( PERIODIC_T3 / portTICK_PERIOD_MS ) );
  }
}
