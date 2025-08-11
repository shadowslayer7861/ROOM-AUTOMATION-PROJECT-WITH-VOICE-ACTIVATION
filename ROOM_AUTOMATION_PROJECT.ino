#define BLYNK_TEMPLATE_ID "TMPL3CM5ZS5Iz"
#define BLYNK_TEMPLATE_NAME "smart home"
#define BLYNK_AUTH_TOKEN "************"

#include <DHT.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <WebServer.h>


#define LED 25
#define PIRPIN 27
#define LDRPIN 33
#define DHTPIN 13
#define DHTTYPE DHT11
#define IRPIN 2
#define IRLED 4
#define ARDUINOIN 32


const char* ssid = "Redmi K20 Pro";
const char* password = "zaid7861";


int x = 0;
int intensity = 0;         
bool manualmode = true;   
bool ledstate = false;   

BlynkTimer timer;
DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width,initial-scale=1.0"/>
  <title>ESP32 Voice Control</title>
  <style>
    body { font-family: Arial, sans-serif; text-align:center; padding:20px; background:#f7f7f7;}
    button { font-size:18px; padding:12px 20px; margin:10px; }
    #status { margin-top:12px; font-weight:600; }
  </style>
</head>
<body>
  <h2>ESP32 Smart Home — Voice Control</h2>
  <p>Say: "turn on light", "turn off light", "bright", "dim", "auto mode on", "manual mode on"</p>
  <button onclick="startListening()">🎤 Speak Command</button>
  <p id="status">Click Speak and say a command</p>

<script>
function startListening() {
  var SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
  if (!SpeechRecognition) {
    document.getElementById('status').innerText = 'SpeechRecognition not supported in this browser';
    return;
  }
  var recognition = new SpeechRecognition();
  recognition.lang = 'en-US';
  recognition.interimResults = false;
  recognition.maxAlternatives = 1;

  recognition.onstart = function() {
    document.getElementById('status').innerText = 'Listening...';
  };

  recognition.onerror = function(event) {
    document.getElementById('status').innerText = 'Error: ' + event.error;
  };

  recognition.onresult = function(event) {
    var cmd = event.results[0][0].transcript.toLowerCase();
    document.getElementById('status').innerText = 'You said: ' + cmd;
    // send to ESP32
    fetch('/voice?cmd=' + encodeURIComponent(cmd))
      .then(resp => resp.text())
      .then(txt => {
         document.getElementById('status').innerText += ' | ESP: ' + txt;
      })
      .catch(err => {
         document.getElementById('status').innerText += ' | Error: ' + err;
      });
  };

  recognition.start();
}
</script>
</body>
</html>
)rawliteral";


void handleRoot() {
  server.send_P(200, "text/html", index_html);
}

void handleOn() {
  ledstate = true;
  Blynk.virtualWrite(V0, 1);
  server.send(200, "text/plain", "OK - LED ON");
}

void handleOff() {
  ledstate = false;
  Blynk.virtualWrite(V0, 0);
  server.send(200, "text/plain", "OK - LED OFF");
}

void handleBrightness() {
  if (server.hasArg("value")) {
    int val = server.arg("value").toInt();
    val = constrain(val, 0, 255);
    intensity = val;
    Blynk.virtualWrite(V1, intensity);
    server.send(200, "text/plain", String("OK - brightness ") + String(intensity));
  } else {
    server.send(400, "text/plain", "Missing 'value' param");
  }
}

void handleVoice() {
  if (server.hasArg("cmd")) {
    String cmd = server.arg("cmd");
    cmd.toLowerCase();

    if (cmd.indexOf("turn on") >= 0 || cmd.indexOf("turn the light on") >= 0 || cmd.indexOf("light on") >= 0) {
      ledstate = true;
      Blynk.virtualWrite(V0, 1);
      server.send(200, "text/plain", "LED turned ON");
      return;
    }
    if (cmd.indexOf("turn off") >= 0 || cmd.indexOf("light off") >= 0) {
      ledstate = false;
      Blynk.virtualWrite(V0, 0);
      server.send(200, "text/plain", "LED turned OFF");
      return;
    }
    if (cmd.indexOf("bright") >= 0 || cmd.indexOf("increase brightness") >= 0 || cmd.indexOf("max") >= 0) {
      intensity = 255;
      Blynk.virtualWrite(V1, intensity);
      server.send(200, "text/plain", "Brightness set to 255");
      return;
    }
    if (cmd.indexOf("dim") >= 0 || cmd.indexOf("decrease brightness") >= 0 || cmd.indexOf("low") >= 0) {
      intensity = 100;
      Blynk.virtualWrite(V1, intensity);
      server.send(200, "text/plain", "Brightness set to 100");
      return;
    }
    if (cmd.indexOf("auto mode") >= 0 || cmd.indexOf("auto") >= 0) {
      manualmode = false;
      Blynk.virtualWrite(V5, 0);
      server.send(200, "text/plain", "Auto mode ON");
      return;
    }
    if (cmd.indexOf("manual mode") >= 0 || cmd.indexOf("manual") >= 0) {
      manualmode = true;
      Blynk.virtualWrite(V5, 1);
      server.send(200, "text/plain", "Manual mode ON");
      return;
    }

 
    server.send(200, "text/plain", "Unknown command: " + cmd);
  } else {
    server.send(400, "text/plain", "Missing 'cmd' param");
  }
}


void sendSensorDH() {
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  int light = analogRead(LDRPIN);

  if (!isnan(temp)) Blynk.virtualWrite(V3, temp);
  if (!isnan(humidity)) Blynk.virtualWrite(V2, humidity);
  Blynk.virtualWrite(V4, light);
}


  intensity = param.asInt();

}

BLYNK_WRITE(V0) {
  ledstate = param.asInt();
}

BLYNK_WRITE(V5) {
  manualmode = param.asInt();
}


void setup() {
  Serial.begin(115200);

  pinMode(PIRPIN, INPUT);
  pinMode(LDRPIN, INPUT);
  pinMode(IRPIN, INPUT);
  pinMode(IRLED, OUTPUT);
  pinMode(LED, OUTPUT);

  dht.begin();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());


  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);


  timer.setInterval(2000L, sendSensorDH);


  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/brightness", handleBrightness);
  server.on("/voice", handleVoice);
  server.begin();
  Serial.println("Web server started");


  analogWrite(LED, 255);
  Blynk.virtualWrite(V0, HIGH);
  analogWrite(LED, 255);
}

void loop() {
  Blynk.run();
  timer.run();
  server.handleClient();

  int motion = digitalRead(PIRPIN);
  int light = analogRead(LDRPIN);
  int IR = digitalRead(IRPIN);
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  Serial.print("Motion: "); Serial.print(motion);
  Serial.print(" | Light: "); Serial.print(light);
  Serial.print(" | Temp: "); Serial.print(temp);
  Serial.print(" °C | Humidity: "); Serial.print(humidity);
  Serial.print(" % | IR: "); Serial.println(IR);

  if (motion == HIGH) {
    x++;
  }
  Serial.println("Motion count: " + String(x));

  
  if (ledstate) {
    if (manualmode) {
      analogWrite(LED, intensity);
    } else {
      int autoIntensity = map(light, 0, 4095, 0, 255);
      analogWrite(LED, autoIntensity);
    }
  } else {
    analogWrite(LED, 0);
  }

  if (IR == 0) {
    digitalWrite(IRLED, HIGH);
    Blynk.virtualWrite(V6, HIGH);
  } else {
    digitalWrite(IRLED, LOW);
    Blynk.virtualWrite(V6, LOW);
  }

  delay(1000);
}
