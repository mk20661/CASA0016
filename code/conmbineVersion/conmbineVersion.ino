#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino_FreeRTOS.h>
#include <SoftwareSerial.h>

// Initialize the LCD with I2C address 0x27 and 16x2 size
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial playerSerial(2, 3); // RX, TX
//Defein pin port
int greenLED = 8;
int redLED = 13; 
int isRunningButton = 11;
int isPausedButton = 12;
int echoPin = 9;
int trigPin = 10;
int lightSensorPin = A0; 
//State of different sensor
volatile bool isRunning = false;
volatile bool isTiming = false;
volatile bool isPasuing = true;
volatile bool isFirstStarting = true;

int seconds = 0;
int distance = 0;
int lightLevel = 0;
// Task handle for the LCD task
TaskHandle_t WorkTaskHandle;
TaskHandle_t LCDTaskHandle;
TaskHandle_t IsRunningButtonTaskHandle;
TaskHandle_t IsPausedButtonTaskHandle;
TaskHandle_t UltrasonicTaskHandle;
TaskHandle_t LightSensorTaskHandle;
TaskHandle_t PlayerTaskHandle;

//Working state task
void WorkingTask(void *pvParameters){
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  for (;;) {
    // 根据 isRunning 状态控制红灯和绿灯
    if (isRunning) {
      digitalWrite(greenLED, HIGH); // 开启绿灯
      digitalWrite(redLED, LOW);   // 关闭红灯
    } else {
      digitalWrite(greenLED, LOW); // 关闭绿灯
      digitalWrite(redLED, HIGH); // 开启红灯
      if (seconds > 0){

      }
    }
    vTaskDelay(pdMS_TO_TICKS(100)); // 每 100ms 检查一次状态
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
      delay(100);  // 防止按钮反弹
    }
    lastButtonState = buttonState;
    vTaskDelay(100 / portTICK_PERIOD_MS);  // 每100ms检测一次按钮
  }
}

void IsPausedButtonTask(void *pvParameters) {
  int lastButtonState = LOW;
  pinMode(isPausedButton, INPUT);

  for (;;) {
    int buttonState = digitalRead(isPausedButton);

    // 检查按钮按下，同时确保 isRunning 为 true（绿灯亮）
    if (buttonState == HIGH && lastButtonState == LOW && isRunning) {
      isTiming = !isTiming; // 切换计时器状态
      isPasuing  = !isPasuing;
      delay(100);           // 防止按钮反弹
    }
    lastButtonState = buttonState;

    vTaskDelay(100 / portTICK_PERIOD_MS); // 每100ms检测一次按钮
  }
}

void UltrasonicTask(void *pvParameters) {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  for (;;) {
    if (isRunning && isPasuing == false) { // 检查 isRunning 状态
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      
      long duration = pulseIn(echoPin, HIGH);
      distance = duration * 0.0344 / 2;

      if (distance < 50) {
        playFile("/screen.mp3", 4000);
      }
    }
    vTaskDelay(500 / portTICK_PERIOD_MS); // 每500ms循环一次
  }
}

//
void LightSensorTask(void *pvParameters) {
  for (;;) {
    if (isRunning && isPasuing == false) { // 检查 isRunning 状态
      lightLevel = analogRead(lightSensorPin);
      if (lightLevel < 500) {
         playFile("/light.mp3", 9000);
      }
    }
    vTaskDelay(500 / portTICK_PERIOD_MS); // 每500ms循环一次
  }
}

// player task
void PlayerTask(void *pvParameters) {

}
// LCD Task: updates the LCD display
bool hasFinishedDisplayed = false;
void LCDTask(void *pvParameters) {
  lcd.init();        // Initialize the LCD
  lcd.backlight();   // Turn on the backlight
  lcd.clear();       // Clear the display
  lcd.print("Study Time:");
  vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait for 1 second to display "hello world"

   for (;;) {
    lcd.setCursor(0, 1);  
    
    if (isRunning){
    // 如果计时器正在计时
      if (isTiming) {
        unsigned long hours = seconds / 3600;           // 计算小时
        unsigned long minutes = (seconds % 3600) / 60; // 计算分钟
        unsigned long displaySeconds = seconds % 60;   // 计算秒数

        // 显示格式为 "HH:MM:SS"
        if (hours < 10) lcd.print("0");
        lcd.print(hours);
        lcd.print(":");
        if (minutes < 10) lcd.print("0");
        lcd.print(minutes);
        lcd.print(":");
        if (displaySeconds < 10) lcd.print("0");
        lcd.print(displaySeconds);
        lcd.print("        ");
        seconds++;  // 增加秒数
      } else {
        // 停止计时时，显示"STOP"
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.print("PAUSED   ");
      }
    }
    else{
      if(seconds == 0 && isFirstStarting){
      lcd.print("Ready to start");
      }
      else if(seconds == 0 && isFirstStarting == false && !hasFinishedDisplayed && !isFirstStarting){
        lcd.setCursor(0, 1);
        lcd.print("Ready to start");
        hasFinishedDisplayed = true;
      }
      else if(seconds != 0)
      {
        isFirstStarting = false;
        hasFinishedDisplayed = false;
        lcd.setCursor(0, 1);
        lcd.print("                "); 
        lcd.setCursor(0, 1);
        lcd.print("Finished");
        if (seconds <60){
          playFile("/"+String(seconds)+"s.mp3", 4000);
          }
          else if(seconds < 3600){
            playFile("/"+String((seconds % 3600) / 60 + 60)+"s.mp3", 4000);
          }
          else{
            playFile("/"+String(((seconds + 59) / 60) * 60)+"s.mp3", 4000);
          }
      }
      seconds = 0;
      
    }
     vTaskDelay(1000 / portTICK_PERIOD_MS);  // 每秒更新一次LCD显示
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
  // Create the LCD task
  xTaskCreate(
    LCDTask,          // Task function
    "LCDTask",        // Task name
    128,              // Stack size (in words, not bytes)
    NULL,             // Task parameters (none in this case)
    1,                // Task priority
    &LCDTaskHandle    // Task handle to manage the task
  );

  xTaskCreate( 
    IsRunningButtonTask,
    "isRunningButtonTask", 
    128, 
    NULL, 
    1, 
    &IsRunningButtonTaskHandle
  );

  xTaskCreate( 
    IsPausedButtonTask,
    "isPausedButtonTask", 
    128, 
    NULL, 
    1, 
    &IsPausedButtonTaskHandle
  );

   xTaskCreate(
   WorkingTask, 
   "LED Control", 
   128, 
   NULL, 
   1, 
   &WorkTaskHandle);

   xTaskCreate(
    UltrasonicTask, 
    "UltrasonicTask", 
    128, 
    NULL, 
    1, 
    &UltrasonicTaskHandle
    );

    xTaskCreate(LightSensorTask, 
    "LightSensorTask", 
    128, 
    NULL, 
    1, 
    &LightSensorTaskHandle
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
