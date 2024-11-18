from gtts import gTTS
import time


# range 1 to 59 s
for i in range(1, 60):
    text = f"You have studied {i} second{'s' if i > 1 else ''}."
    tts = gTTS(text, lang='en')
    # 格式化文件名为 4 位数
    filename = "./timer/"+f"{i}s.mp3"
    # save to folder
    tts.save(filename)
    print(f"Generated {filename}")
    time.sleep(0.5)
#range 1 to 59 mins
for i in range(1, 60):
    text = f"You have studied {i} minute{'s' if i > 1 else ''}."
    tts = gTTS(text, lang='en')
    # 格式化文件名为 4 位数
    filename = "./timer/"+f"{i*60}s.mp3"
    # save to folder
    tts.save(filename)
    print(f"Generated {filename}")
    time.sleep(0.5)

for i in range(1, 7):
    for j in range(0,60):
        if j == 0:
            text = f"You have studied {i} hour{'s' if i > 1 else ''}."
        else:
            text = f"You have studied {i} hour{'s' if i > 1 else ''} and {j} minute{'s' if j > 1 else ''}."
        tts = gTTS(text, lang='en')
        # 格式化文件名为 4 位数
        filename = "./timer/"+f"{i*3600+j*60}s.mp3"
        # save to folder
        tts.save(filename)
        print(f"Generated {filename}")
        time.sleep(0.5)

text = f"You have studied 7 hours"
filename = "./timer/"+f"{7*3600}s.mp3"
tts = gTTS(text, lang='en')
tts.save(filename)
print(f"Generated {filename}")

void PlayerTask(void *pvParameters) {
  playerSerial.begin(9600); 
  for (;;) {
    if(!isRunning){
      if(seconds != 0)
      {
        isFirstStarting = false;
        hasFinishedDisplayed = false;
        if (seconds <60){
          playFile("/"+String(seconds)+"s.mp3", 4000);
          }
          else if(seconds < 3600){
            playFile("/"+String((seconds % 3600) / 60)+"s.mp3", 4000);
          }
          else{
            playFile("/"+String(seconds / 3600)+"s.mp3", 4000);
          }
      }
      seconds = 0;
    }
  }
}