#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino_FreeRTOS.h>
#include <SoftwareSerial.h>

// Initialize the LCD with I2C address 0x27 and 16x2 size
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial playerSerial(2, 3); // RX, TX
//Defein pin port
#define greenLED  8
#define redLED  13
#define isRunningButton  11
#define isPausedButton  12
#define echoPin  9
#define trigPin  10
#define lightSensorPin  A0 
//State of different sensor
volatile bool isRunning = false;
volatile bool isTiming = false;
volatile bool isPasuing = true;


int seconds = 0;
int distance = 0;
int lightLevel = 0;
// Tasks handle
TaskHandle_t WorkTaskHandle;
TaskHandle_t LCDTaskHandle;
TaskHandle_t IsRunningButtonTaskHandle;
TaskHandle_t IsPausedButtonTaskHandle;
TaskHandle_t UltrasonicTaskHandle;
TaskHandle_t LightSensorTaskHandle;
TaskHandle_t RestTaskHandle;

//Working state task
void WorkingTask(void *pvParameters){
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  for (;;) {
    // control red and green led
    if (isRunning) {
      digitalWrite(greenLED, HIGH); // turn on green
      digitalWrite(redLED, LOW);   // 
    } else {
      digitalWrite(greenLED, LOW); // 
      digitalWrite(redLED, HIGH); // turn on grern
    }
    vTaskDelay(pdMS_TO_TICKS(100)); // check every 100ms
  }
}

// isRunning Butoon Task: dectect the button state and control timer
void IsRunningButtonTask(void *pvParameters) {
  int lastButtonState = LOW;
  pinMode(isRunningButton, INPUT);
  
  for (;;) {
    int buttonState = digitalRead(isRunningButton);
    if (buttonState == HIGH && lastButtonState == LOW) {
      isRunning = !isRunning;
      isTiming = !isTiming;
      if (isPasuing){
        isPasuing = !isPasuing;
      }
      delay(100);  // debounce 
    }
    lastButtonState = buttonState;
    vTaskDelay(100 / portTICK_PERIOD_MS); 
  }
}

void IsPausedButtonTask(void *pvParameters) {
  int lastButtonState = LOW;
  pinMode(isPausedButton, INPUT);

  for (;;) {
    int buttonState = digitalRead(isPausedButton);

    // check button
    if (buttonState == HIGH && lastButtonState == LOW && isRunning) {
      isTiming = !isTiming; // 
      isPasuing  = !isPasuing;
      delay(100);           
    }
    lastButtonState = buttonState;

    vTaskDelay(100 / portTICK_PERIOD_MS); 
  }
}
//distance detection
void UltrasonicTask(void *pvParameters) {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  for (;;) {
    if (isRunning && isPasuing == false) { 
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      
      long duration = pulseIn(echoPin, HIGH);
      distance = duration * 0.0344 / 2;

      if (distance < 40) {
        playFile("/screen.mp3", 4000);
      }
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

// light  detection use LDR
void LightSensorTask(void *pvParameters) {
  for (;;) {
    if (isRunning && isPasuing == false) { // only running in running status
      lightLevel = analogRead(lightSensorPin);
      if (lightLevel < 500) {
         playFile("/light.mp3", 9000);
      }
    }
    vTaskDelay(500 / portTICK_PERIOD_MS); 
  }
}

//Study continuously for an hour,need to have a break
void RestTask(void *pvParameters){
 for(;;){
  unsigned long tempSeconds = seconds;
  if (tempSeconds % 3600 == 0 && tempSeconds != 0){
     playFile("/time.mp3", 7000);
  }
  vTaskDelay(500 / portTICK_PERIOD_MS); 
 }
}

// LCD Task: updates the LCD display
bool hasFinishedDisplayed = false;

void LCDTask(void *pvParameters) {
    lcd.init();        // init lcd
    lcd.backlight();   //back light
    lcd.clear();       // clear lcd
    lcd.print("Study Time:");
    vTaskDelay(1000 / portTICK_PERIOD_MS);  

    for (;;) {
        lcd.setCursor(0, 1);  

        if (isRunning) {
            // timer running
            if (isTiming) {
                unsigned long hours = seconds / 3600;           // hours
                unsigned long minutes = (seconds % 3600) / 60; // mins
                unsigned long displaySeconds = seconds % 60;   // seconds

                // format "HH:MM:SS"
                if (hours < 10) lcd.print("0");
                lcd.print(hours);
                lcd.print(":");
                if (minutes < 10) lcd.print("0");
                lcd.print(minutes);
                lcd.print(":");
                if (displaySeconds < 10) lcd.print("0");
                lcd.print(displaySeconds);
                lcd.print("        ");
                seconds++; 
            } else {
                // stop timiing, display paused
                lcd.setCursor(0, 1);
                lcd.print("                ");//clear time display 
                lcd.setCursor(0, 1);
                lcd.print("PAUSED   ");
            }
        } else {
            if (seconds == 0 ) {
                lcd.print("Ready to start");
            } else if (seconds == 0 && !hasFinishedDisplayed) {
                lcd.setCursor(0, 1);
                lcd.print("Ready to start");
                hasFinishedDisplayed = true;
            } else if (seconds != 0) {
                unsigned long tempSeconds = seconds; // race condition 
                hasFinishedDisplayed = false;
                lcd.setCursor(0, 1);
                lcd.print("                ");
                lcd.setCursor(0, 1);
                lcd.print("Finished");
                isTiming = false;
                isPasuing = true;
                if (tempSeconds < 60) {
                    playFile("/" + String(tempSeconds) + "s.mp3", 4000);
                } else if (tempSeconds < 3600) {
                    playFile("/" + String((tempSeconds / 60 + ((tempSeconds % 60 > 30) ? 1 : 0))*60) + "s.mp3", 4000);
                } else {
                    playFile("/" + String(convertToRoundedSeconds(tempSeconds)) + "s.mp3", 4000);
                }

                seconds = 0; 
            }
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS); //upate 1 s
        Serial.print("Distance: ");
        Serial.println(distance);
        Serial.print("Light Level: ");
        Serial.println(lightLevel);
    }
}




void setup() {
  Serial.begin(9600);
  playerSerial.begin(9600);
  delay(1000);
  sendCommand("AT+PLAYMODE=3");
  sendCommand("AT+LED=OFF");
  sendCommand("AT+VOL=13");
  // Create the LCD task
  xTaskCreate(
    LCDTask,          // Task function
    "LCDTask",        // Task name
    128,              // Stack size
    NULL,             // Task parameters 
    4,                // Task priority
    &LCDTaskHandle    // Task handle to manage the task
  );

  xTaskCreate( 
    IsRunningButtonTask,
    "isRunningButtonTask", 
    128, 
    NULL, 
    4, 
    &IsRunningButtonTaskHandle
  );

  xTaskCreate( 
    IsPausedButtonTask,
    "isPausedButtonTask", 
    128, 
    NULL, 
    4, 
    &IsPausedButtonTaskHandle
  );

   xTaskCreate(
   WorkingTask, 
   "LED Control", 
   128, 
   NULL, 
   4, 
   &WorkTaskHandle);

   xTaskCreate(
    UltrasonicTask, 
    "UltrasonicTask", 
    128, 
    NULL, 
    2, 
    &UltrasonicTaskHandle
    );

    xTaskCreate(LightSensorTask, 
    "LightSensorTask", 
    128, 
    NULL, 
    2, 
    &LightSensorTaskHandle
    );

    xTaskCreate(RestTask, 
    "RestTask", 
    128, 
    NULL, 
    2, 
    &RestTaskHandle
    );


  // Start the FreeRTOS scheduler
  vTaskStartScheduler();
}

void loop() {
  // Empty loop since all functionality is managed by FreeRTOS tasks
}

void playFile(String fileName, unsigned long duration) {
  sendCommand("AT+PLAYFILE=" + fileName);
  delay(duration);
}

void sendCommand(String command) {
  playerSerial.print(command + "\r\n");
  Serial.println("Sent: " + command);
}
//convert to seconds
int convertToRoundedSeconds(int seconds) {
    int hours = seconds / 3600;
    int minutes = (seconds % 3600 + 30) / 60;
    // trnasfer to total seconds
    int total_seconds = hours * 3600 + minutes * 60;
    return total_seconds;
}