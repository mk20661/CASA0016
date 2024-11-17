#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino_FreeRTOS.h>

// Initialize the LCD with I2C address 0x27 and 16x2 size
LiquidCrystal_I2C lcd(0x27, 16, 2);
//Defein pin port
int greenLED = 8;
int redLED = 13; 
int isRunningButton = 11;
int isPausedButton = 12;  
//State of different sensor
bool isRuning = false;
bool isTiming = false;
int seconds = 0;
// Task handle for the LCD task
TaskHandle_t WorkTaskHandle;
TaskHandle_t LCDTaskHandle;
TaskHandle_t IsRunningButtonTaskHandle;
TaskHandle_t IsPausedButtonTaskHandle;

//Working state task
void WorkingTask(void *pvParameters){
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  for (;;) {
    // 根据 isRunning 状态控制红灯和绿灯
    if (isRuning) {
      digitalWrite(greenLED, HIGH); // 开启绿灯
      digitalWrite(redLED, LOW);   // 关闭红灯
    } else {
      digitalWrite(greenLED, LOW); // 关闭绿灯
      digitalWrite(redLED, HIGH); // 开启红灯
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
      isRuning = !isRuning;
      delay(100);  // 防止按钮反弹
    }
    lastButtonState = buttonState;
    vTaskDelay(100 / portTICK_PERIOD_MS);  // 每100ms检测一次按钮
  }
}

//
// void IsPausedButtonTask(void *pvParameters) {
//   int lastButtonState = LOW;
//   pinMode(isPausedButton, INPUT);
  
//   for (;;) {
//     int buttonState = digitalRead(isPausedButton);
//     if (buttonState == HIGH && lastButtonState == LOW) {
//       isTiming = !isTiming;
//       delay(100);  // 防止按钮反弹
//     }
//     lastButtonState = buttonState;
//     vTaskDelay(100 / portTICK_PERIOD_MS);  // 每100ms检测一次按钮
//   }
// }
void IsPausedButtonTask(void *pvParameters) {
  int lastButtonState = LOW;
  pinMode(isPausedButton, INPUT);

  for (;;) {
    int buttonState = digitalRead(isPausedButton);

    // 检查按钮按下，同时确保 isRuning 为 true（绿灯亮）
    if (buttonState == HIGH && lastButtonState == LOW && isRuning) {
      isTiming = !isTiming; // 切换计时器状态
      delay(100);           // 防止按钮反弹
    }
    lastButtonState = buttonState;

    vTaskDelay(100 / portTICK_PERIOD_MS); // 每100ms检测一次按钮
  }
}


// LCD Task: updates the LCD display
void LCDTask(void *pvParameters) {
  lcd.init();        // Initialize the LCD
  lcd.backlight();   // Turn on the backlight
  lcd.clear();       // Clear the display
  lcd.print("Study Time:");
  vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait for 1 second to display "hello world"

   for (;;) {
    lcd.setCursor(0, 1);  // 设置光标位置在第二行

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

      seconds++;  // 增加秒数
    } else {
      // 停止计时时，显示"STOP"
      lcd.print("PAUSED   ");
    }
     vTaskDelay(1000 / portTICK_PERIOD_MS);  // 每秒更新一次LCD显示
   }
}

void setup() {
  // Create the LCD task
  xTaskCreate(
    LCDTask,          // Task function
    "LCDTask",        // Task name
    128,              // Stack size (in words, not bytes)
    NULL,             // Task parameters (none in this case)
    1,                // Task priority
    &LCDTaskHandle    // Task handle to manage the task
  );

  xTaskCreate( IsRunningButtonTask,
    "isRunningButtonTask", 
    128, 
    NULL, 
    1, 
    & IsRunningButtonTaskHandle
  );

  xTaskCreate( IsPausedButtonTask,
    "isPausedButtonTask", 
    128, 
    NULL, 
    1, 
    & IsPausedButtonTaskHandle
  );

   xTaskCreate(WorkingTask, 
   "LED Control", 
   128, 
   NULL, 
   1, 
   &WorkTaskHandle);

  // Start the FreeRTOS scheduler
  vTaskStartScheduler();
}

void loop() {
  // Empty loop since all functionality is managed by FreeRTOS tasks
}
