import speech_recognition as sr
import requests
import pyttsx3

BLYNK_TOKEN = "7qTMXbPTv5iq9TEB8xRydNf3hilz5NwN"
base_url = f"https://blynk.cloud/external/api/update?token={BLYNK_TOKEN}"

def speak(text):
    engine = pyttsx3.init()
    engine.say(text)
    engine.runAndWait()

def send_blynk_command(pin, value):
    url = f"{base_url}&{pin}={value}"
    print("Sending URL:", url)
    try:
        response = requests.get(url)
        print("Status code:", response.status_code)
        if response.status_code == 200:
            speak("Command sent successfully")
        else:
            speak("Failed to send command")
    except Exception as e:
        print("Error:", e)
        speak("Error sending request")

def handle_command(command):
    command = command.lower()
    print("You said:", command)

    if "turn on the light" in command or "light on" in command:
        send_blynk_command("V0", 1)
    elif "turn off the light" in command or "light off" in command:
        send_blynk_command("V0", 0)
    elif "enable manual" in command:
        send_blynk_command("V5", 1)
    elif "enable auto" in command:
        send_blynk_command("V5", 0)
    elif "set brightness to" in command:
        try:
            value = int(command.split("to")[-1].strip())
            send_blynk_command("V1", value)
        except:
            speak("Could not understand brightness level")
    else:
        speak("Command not recognized")

def listen():
    print("Script started...")
    r = sr.Recognizer()
    with sr.Microphone() as source:
        print("Microphone opened. Listening...")
        speak("Speak your command")
        audio = r.listen(source)

        try:
            print("Processing audio...")
            command = r.recognize_google(audio)
            handle_command(command)
        except sr.UnknownValueError:
            speak("Could not understand audio")
        except sr.RequestError:
            speak("Speech service error")

if __name__ == "__main__":
    while True:
        listen()
