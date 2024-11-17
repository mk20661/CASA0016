from gtts import gTTS

# 要转换的文本

from gtts import gTTS

# 循环从 1 到 59 生成语音
for i in range(1, 60):
    text = f"You have studied {i} seconds{'s' if i > 1 else ''}."
    tts = gTTS(text, lang='en')
    # 格式化文件名为 4 位数
    filename = "./timer/"+f"{i:04}.mp3"
    # 保存语音到文件
    tts.save(filename)
    print(f"Generated {filename}")

