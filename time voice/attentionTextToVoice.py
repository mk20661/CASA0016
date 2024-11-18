from gtts import gTTS

#text = f"You’ve been studying for 1 hour straight. Take a 5-10 minute break to boost your efficiency!"
text = f"The lighting is insufficient and may affect your study. Please turn on the lights to protect your eyesight"

tts = gTTS(text, lang='en')
tts.save("./attention/light.mp3")