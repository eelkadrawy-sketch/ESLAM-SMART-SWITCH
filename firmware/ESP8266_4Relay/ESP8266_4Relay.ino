#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// =====================================================
// ESP8266 4 RELAY SMART CONTROL
// MQTT + HiveMQ CLOUD
// FINAL VERSION
// =====================================================


// =====================================================
// DEVICE ID
// =====================================================
// لكل بوردة غيّر السطر ده فقط

const char* DEVICE_ID = "ESLAM";


// =====================================================
// MQTT CREDENTIAL
// =====================================================
// لازم الـ Username يطابق الـ DEVICE_ID

const char* MQTT_USERNAME = "USER";

// ضع هنا Password الخاصة بـ Credential AE000001
const char* MQTT_PASSWORD = "PASS";


// =====================================================
// ACCESS POINT
// =====================================================

const char* AP_SSID = "Eng ESLAM";
const char* AP_PASSWORD = "12345678";

IPAddress AP_IP(192, 168, 4, 1);
IPAddress AP_GATEWAY(192, 168, 4, 1);
IPAddress AP_SUBNET(255, 255, 255, 0);


// =====================================================
// HIVEMQ CLOUD
// =====================================================

const char* MQTT_HOST =
  "fa214242813b40d895f06590b2db5c5d.s1.eu.hivemq.cloud";

const uint16_t MQTT_PORT = 8883;


// =====================================================
// MQTT TOPICS
// =====================================================
// لا يوجد AE000001 مكتوب هنا يدويًا.
// كل الـ Topics تعتمد على DEVICE_ID.

String MQTT_BASE =
  String("eng_ahmed_ebaid/devices/") +
  DEVICE_ID +
  "/";

String MQTT_TOPIC_RELAY1_SET =
  MQTT_BASE + "relay1/set";

String MQTT_TOPIC_RELAY2_SET =
  MQTT_BASE + "relay2/set";

String MQTT_TOPIC_RELAY3_SET =
  MQTT_BASE + "relay3/set";

String MQTT_TOPIC_RELAY4_SET =
  MQTT_BASE + "relay4/set";

String MQTT_TOPIC_ALL_SET =
  MQTT_BASE + "all/set";


String MQTT_TOPIC_RELAY1_STATE =
  MQTT_BASE + "relay1/state";

String MQTT_TOPIC_RELAY2_STATE =
  MQTT_BASE + "relay2/state";

String MQTT_TOPIC_RELAY3_STATE =
  MQTT_BASE + "relay3/state";

String MQTT_TOPIC_RELAY4_STATE =
  MQTT_BASE + "relay4/state";

String MQTT_TOPIC_ALL_STATE =
  MQTT_BASE + "all/state";


String MQTT_TOPIC_RELAY1_NAME =
  MQTT_BASE + "relay1/name";

String MQTT_TOPIC_RELAY2_NAME =
  MQTT_BASE + "relay2/name";

String MQTT_TOPIC_RELAY3_NAME =
  MQTT_BASE + "relay3/name";

String MQTT_TOPIC_RELAY4_NAME =
  MQTT_BASE + "relay4/name";


// =====================================================
// MQTT CLIENT
// =====================================================

BearSSL::WiFiClientSecure mqttSecureClient;
PubSubClient mqttClient(mqttSecureClient);

unsigned long lastMQTTAttempt = 0;
const unsigned long MQTT_RECONNECT_INTERVAL = 5000;


// =====================================================
// WIFI RECONNECT
// =====================================================

unsigned long lastWiFiAttempt = 0;
const unsigned long WIFI_RECONNECT_INTERVAL = 15000;


// =====================================================
// RELAY PINS
// =====================================================

#define RELAY1 D1
#define RELAY2 D2
#define RELAY3 D5
#define RELAY4 D6

// Active LOW Relay Module
#define RELAY_ON LOW
#define RELAY_OFF HIGH


// =====================================================
// EEPROM
// =====================================================

#define EEPROM_SIZE 512

#define WIFI_SSID_ADDR 0
#define WIFI_PASS_ADDR 64

#define RELAY1_NAME_ADDR 224
#define RELAY2_NAME_ADDR 256
#define RELAY3_NAME_ADDR 288
#define RELAY4_NAME_ADDR 320

#define EEPROM_MAGIC_ADDR 500
#define EEPROM_MAGIC_VALUE 0xAA


// =====================================================
// WEB SERVER
// =====================================================

ESP8266WebServer server(80);


// =====================================================
// RELAY STATES
// =====================================================

bool relay1State = false;
bool relay2State = false;
bool relay3State = false;
bool relay4State = false;


// =====================================================
// RELAY NAMES
// =====================================================

String relay1Name = "Relay 1";
String relay2Name = "Relay 2";
String relay3Name = "Relay 3";
String relay4Name = "Relay 4";


// =====================================================
// WIFI SETTINGS
// =====================================================

String savedSSID = "";
String savedPassword = "";


// =====================================================
// FORWARD DECLARATIONS
// =====================================================

void mqttCallback(
  char* topic,
  byte* payload,
  unsigned int length
);

void connectToMQTT();

void publishRelayState(int relay);

void publishAllStates();

void publishAllNames();

void setRelay(
  int relay,
  bool state
);

void connectToHomeWiFi();


// =====================================================
// EEPROM SAVE STRING
// =====================================================

void saveString(
  int address,
  String value,
  int maxLength
) {

  for (int i = 0; i < maxLength; i++) {

    if (i < value.length()) {

      EEPROM.write(
        address + i,
        value[i]
      );

    } else {

      EEPROM.write(
        address + i,
        0
      );
    }
  }

  EEPROM.commit();
}


// =====================================================
// EEPROM READ STRING
// =====================================================

String readString(
  int address,
  int maxLength
) {

  String value = "";

  for (int i = 0; i < maxLength; i++) {

    char c = EEPROM.read(address + i);

    if (c == 0) {
      break;
    }

    value += c;
  }

  return value;
}


// =====================================================
// LOAD SETTINGS
// =====================================================

void loadSettings() {

  EEPROM.begin(EEPROM_SIZE);

  if (
    EEPROM.read(EEPROM_MAGIC_ADDR)
    != EEPROM_MAGIC_VALUE
  ) {

    savedSSID = "";
    savedPassword = "";

    relay1Name = "Relay 1";
    relay2Name = "Relay 2";
    relay3Name = "Relay 3";
    relay4Name = "Relay 4";

    saveString(
      WIFI_SSID_ADDR,
      savedSSID,
      64
    );

    saveString(
      WIFI_PASS_ADDR,
      savedPassword,
      64
    );

    saveString(
      RELAY1_NAME_ADDR,
      relay1Name,
      32
    );

    saveString(
      RELAY2_NAME_ADDR,
      relay2Name,
      32
    );

    saveString(
      RELAY3_NAME_ADDR,
      relay3Name,
      32
    );

    saveString(
      RELAY4_NAME_ADDR,
      relay4Name,
      32
    );

    EEPROM.write(
      EEPROM_MAGIC_ADDR,
      EEPROM_MAGIC_VALUE
    );

    EEPROM.commit();

  } else {

    savedSSID =
      readString(
        WIFI_SSID_ADDR,
        64
      );

    savedPassword =
      readString(
        WIFI_PASS_ADDR,
        64
      );

    relay1Name =
      readString(
        RELAY1_NAME_ADDR,
        32
      );

    relay2Name =
      readString(
        RELAY2_NAME_ADDR,
        32
      );

    relay3Name =
      readString(
        RELAY3_NAME_ADDR,
        32
      );

    relay4Name =
      readString(
        RELAY4_NAME_ADDR,
        32
      );

    if (relay1Name.length() == 0)
      relay1Name = "Relay 1";

    if (relay2Name.length() == 0)
      relay2Name = "Relay 2";

    if (relay3Name.length() == 0)
      relay3Name = "Relay 3";

    if (relay4Name.length() == 0)
      relay4Name = "Relay 4";
  }


  Serial.println();
  Serial.println("================================");
  Serial.println("SAVED SETTINGS");
  Serial.println("================================");

  Serial.print("WiFi SSID: ");
  Serial.println(savedSSID);

  Serial.print("Relay 1: ");
  Serial.println(relay1Name);

  Serial.print("Relay 2: ");
  Serial.println(relay2Name);

  Serial.print("Relay 3: ");
  Serial.println(relay3Name);

  Serial.print("Relay 4: ");
  Serial.println(relay4Name);

  Serial.println("================================");
}


// =====================================================
// PUBLISH ONE RELAY STATE
// =====================================================

void publishRelayState(int relay) {

  if (!mqttClient.connected()) {
    return;
  }

  String topic = "";
  bool state = false;


  switch (relay) {

    case 1:
      topic = MQTT_TOPIC_RELAY1_STATE;
      state = relay1State;
      break;

    case 2:
      topic = MQTT_TOPIC_RELAY2_STATE;
      state = relay2State;
      break;

    case 3:
      topic = MQTT_TOPIC_RELAY3_STATE;
      state = relay3State;
      break;

    case 4:
      topic = MQTT_TOPIC_RELAY4_STATE;
      state = relay4State;
      break;

    default:
      return;
  }


  bool result =
    mqttClient.publish(
      topic.c_str(),
      state ? "ON" : "OFF",
      true
    );


  Serial.print("Publish state Relay ");
  Serial.print(relay);
  Serial.print(": ");

  Serial.println(
    result ? "OK" : "FAILED"
  );
}


// =====================================================
// PUBLISH ALL STATES
// =====================================================

void publishAllStates() {

  if (!mqttClient.connected()) {
    return;
  }

  publishRelayState(1);
  publishRelayState(2);
  publishRelayState(3);
  publishRelayState(4);


  String allState = "{";

  allState += "\"relay1\":";
  allState +=
    relay1State ? "true" : "false";

  allState += ",\"relay2\":";
  allState +=
    relay2State ? "true" : "false";

  allState += ",\"relay3\":";
  allState +=
    relay3State ? "true" : "false";

  allState += ",\"relay4\":";
  allState +=
    relay4State ? "true" : "false";

  allState += "}";


  mqttClient.publish(
    MQTT_TOPIC_ALL_STATE.c_str(),
    allState.c_str(),
    true
  );
}


// =====================================================
// PUBLISH NAMES
// =====================================================

void publishAllNames() {

  if (!mqttClient.connected()) {
    return;
  }

  mqttClient.publish(
    MQTT_TOPIC_RELAY1_NAME.c_str(),
    relay1Name.c_str(),
    true
  );

  mqttClient.publish(
    MQTT_TOPIC_RELAY2_NAME.c_str(),
    relay2Name.c_str(),
    true
  );

  mqttClient.publish(
    MQTT_TOPIC_RELAY3_NAME.c_str(),
    relay3Name.c_str(),
    true
  );

  mqttClient.publish(
    MQTT_TOPIC_RELAY4_NAME.c_str(),
    relay4Name.c_str(),
    true
  );
}


// =====================================================
// SET RELAY
// =====================================================

void setRelay(
  int relay,
  bool state
) {

  int pin;


  switch (relay) {

    case 1:

      pin = RELAY1;
      relay1State = state;

      break;


    case 2:

      pin = RELAY2;
      relay2State = state;

      break;


    case 3:

      pin = RELAY3;
      relay3State = state;

      break;


    case 4:

      pin = RELAY4;
      relay4State = state;

      break;


    default:

      Serial.println(
        "ERROR: Invalid relay number"
      );

      return;
  }


  Serial.println();
  Serial.println("-----------------------------");

  Serial.print("Relay: ");
  Serial.println(relay);

  Serial.print("State: ");
  Serial.println(
    state ? "ON" : "OFF"
  );

  Serial.print("GPIO: ");
  Serial.println(pin);

  Serial.print("GPIO Level: ");

  Serial.println(
    state ? "LOW" : "HIGH"
  );


  digitalWrite(
    pin,
    state ? RELAY_ON : RELAY_OFF
  );


  Serial.println(
    "GPIO updated successfully"
  );

  Serial.println("-----------------------------");


  publishRelayState(relay);
}


// =====================================================
// MQTT CALLBACK
// =====================================================

void mqttCallback(
  char* topic,
  byte* payload,
  unsigned int length
) {

  String message = "";

  for (
    unsigned int i = 0;
    i < length;
    i++
  ) {

    message +=
      (char)payload[i];
  }


  message.trim();
  message.toUpperCase();


  String receivedTopic = String(topic);


  Serial.println();
  Serial.println("================================");
  Serial.println("MQTT MESSAGE RECEIVED");
  Serial.println("================================");

  Serial.print("Topic: ");
  Serial.println(receivedTopic);

  Serial.print("Payload: ");
  Serial.println(message);

  Serial.println("================================");


  // ===================================================
  // RELAY 1
  // ===================================================

  if (
    receivedTopic ==
    MQTT_TOPIC_RELAY1_SET
  ) {

    Serial.println(
      "COMMAND FOR RELAY 1"
    );


    if (
      message == "ON" ||
      message == "1"
    ) {

      setRelay(1, true);

    }

    else if (
      message == "OFF" ||
      message == "0"
    ) {

      setRelay(1, false);

    }

    else if (
      message == "TOGGLE"
    ) {

      setRelay(
        1,
        !relay1State
      );
    }

    return;
  }


  // ===================================================
  // RELAY 2
  // ===================================================

  if (
    receivedTopic ==
    MQTT_TOPIC_RELAY2_SET
  ) {

    Serial.println(
      "COMMAND FOR RELAY 2"
    );


    if (
      message == "ON" ||
      message == "1"
    ) {

      setRelay(2, true);

    }

    else if (
      message == "OFF" ||
      message == "0"
    ) {

      setRelay(2, false);

    }

    else if (
      message == "TOGGLE"
    ) {

      setRelay(
        2,
        !relay2State
      );
    }

    return;
  }


  // ===================================================
  // RELAY 3
  // ===================================================

  if (
    receivedTopic ==
    MQTT_TOPIC_RELAY3_SET
  ) {

    Serial.println(
      "COMMAND FOR RELAY 3"
    );


    if (
      message == "ON" ||
      message == "1"
    ) {

      setRelay(3, true);

    }

    else if (
      message == "OFF" ||
      message == "0"
    ) {

      setRelay(3, false);

    }

    else if (
      message == "TOGGLE"
    ) {

      setRelay(
        3,
        !relay3State
      );
    }

    return;
  }


  // ===================================================
  // RELAY 4
  // ===================================================

  if (
    receivedTopic ==
    MQTT_TOPIC_RELAY4_SET
  ) {

    Serial.println(
      "COMMAND FOR RELAY 4"
    );


    if (
      message == "ON" ||
      message == "1"
    ) {

      setRelay(4, true);

    }

    else if (
      message == "OFF" ||
      message == "0"
    ) {

      setRelay(4, false);

    }

    else if (
      message == "TOGGLE"
    ) {

      setRelay(
        4,
        !relay4State
      );
    }

    return;
  }


  // ===================================================
  // ALL RELAYS
  // ===================================================

  if (
    receivedTopic ==
    MQTT_TOPIC_ALL_SET
  ) {

    Serial.println(
      "COMMAND FOR ALL RELAYS"
    );


    if (
      message == "ON" ||
      message == "1"
    ) {

      setRelay(1, true);
      setRelay(2, true);
      setRelay(3, true);
      setRelay(4, true);

    }

    else if (
      message == "OFF" ||
      message == "0"
    ) {

      setRelay(1, false);
      setRelay(2, false);
      setRelay(3, false);
      setRelay(4, false);
    }

    return;
  }


  Serial.println(
    "UNKNOWN MQTT TOPIC"
  );
}


// =====================================================
// CONNECT HOME WIFI
// =====================================================

void connectToHomeWiFi() {

  if (
    savedSSID.length() == 0
  ) {

    Serial.println();
    Serial.println(
      "No saved Home WiFi"
    );

    return;
  }


  Serial.println();
  Serial.println(
    "================================"
  );

  Serial.println(
    "CONNECTING TO HOME WIFI"
  );

  Serial.println(
    "================================"
  );


  Serial.print(
    "SSID: "
  );

  Serial.println(
    savedSSID
  );


  WiFi.begin(
    savedSSID.c_str(),
    savedPassword.c_str()
  );


  int counter = 0;


  while (
    WiFi.status() != WL_CONNECTED &&
    counter < 30
  ) {

    delay(500);

    Serial.print(".");

    counter++;
  }


  Serial.println();


  if (
    WiFi.status() == WL_CONNECTED
  ) {

    Serial.println();
    Serial.println(
      "================================"
    );

    Serial.println(
      "HOME WIFI CONNECTED"
    );

    Serial.println(
      "================================"
    );


    Serial.print(
      "ESP Home IP: "
    );

    Serial.println(
      WiFi.localIP()
    );


    Serial.print(
      "Gateway: "
    );

    Serial.println(
      WiFi.gatewayIP()
    );


    Serial.print(
      "RSSI: "
    );

    Serial.println(
      WiFi.RSSI()
    );


    Serial.println(
      "================================"
    );

  } else {

    Serial.println();

    Serial.println(
      "HOME WIFI CONNECTION FAILED"
    );
  }
}


// =====================================================
// CONNECT MQTT
// =====================================================

void connectToMQTT() {

  if (
    WiFi.status() != WL_CONNECTED
  ) {

    Serial.println(
      "WiFi not connected - MQTT skipped"
    );

    return;
  }


  if (
    mqttClient.connected()
  ) {

    return;
  }


  Serial.println();
  Serial.println(
    "================================"
  );

  Serial.println(
    "CONNECTING TO HIVEMQ MQTT"
  );

  Serial.println(
    "================================"
  );


  Serial.print(
    "Host: "
  );

  Serial.println(
    MQTT_HOST
  );


  Serial.print(
    "Port: "
  );

  Serial.println(
    MQTT_PORT
  );


  Serial.print(
    "Device ID: "
  );

  Serial.println(
    DEVICE_ID
  );


  Serial.print(
    "Username: "
  );

  Serial.println(
    MQTT_USERNAME
  );


  Serial.print(
    "Base Topic: "
  );

  Serial.println(
    MQTT_BASE
  );


  String clientId =
    "ESP8266-";

  clientId +=
    String(
      ESP.getChipId(),
      HEX
    );


  Serial.print(
    "Client ID: "
  );

  Serial.println(
    clientId
  );


  bool connected =
    mqttClient.connect(
      clientId.c_str(),
      MQTT_USERNAME,
      MQTT_PASSWORD
    );


  if (connected) {

    Serial.println();
    Serial.println(
      "MQTT CONNECTED!"
    );


    Serial.println();
    Serial.println(
      "SUBSCRIBING..."
    );


    bool s1 =
      mqttClient.subscribe(
        MQTT_TOPIC_RELAY1_SET.c_str()
      );

    bool s2 =
      mqttClient.subscribe(
        MQTT_TOPIC_RELAY2_SET.c_str()
      );

    bool s3 =
      mqttClient.subscribe(
        MQTT_TOPIC_RELAY3_SET.c_str()
      );

    bool s4 =
      mqttClient.subscribe(
        MQTT_TOPIC_RELAY4_SET.c_str()
      );

    bool sa =
      mqttClient.subscribe(
        MQTT_TOPIC_ALL_SET.c_str()
      );


    Serial.print(
      "Relay 1 subscribe: "
    );

    Serial.println(
      s1 ? "OK" : "FAILED"
    );


    Serial.print(
      "Relay 2 subscribe: "
    );

    Serial.println(
      s2 ? "OK" : "FAILED"
    );


    Serial.print(
      "Relay 3 subscribe: "
    );

    Serial.println(
      s3 ? "OK" : "FAILED"
    );


    Serial.print(
      "Relay 4 subscribe: "
    );

    Serial.println(
      s4 ? "OK" : "FAILED"
    );


    Serial.print(
      "ALL subscribe: "
    );

    Serial.println(
      sa ? "OK" : "FAILED"
    );


    publishAllStates();

    publishAllNames();


    Serial.println();
    Serial.println(
      "================================"
    );

    Serial.println(
      "MQTT READY FOR COMMANDS"
    );

    Serial.println(
      "================================"
    );


  } else {

    Serial.println();

    Serial.println(
      "MQTT CONNECTION FAILED"
    );


    Serial.print(
      "MQTT State: "
    );

    Serial.println(
      mqttClient.state()
    );


    Serial.println(
      "Retrying after 5 seconds..."
    );


    Serial.println(
      "================================"
    );
  }
}


// =====================================================
// MAIN PAGE
// =====================================================

const char MAIN_PAGE[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html lang="ar" dir="rtl">

<head>

<meta charset="UTF-8">

<meta name="viewport"
content="width=device-width,initial-scale=1">

<title>ENG. AHMED EBAID</title>

<style>

*{
  box-sizing:border-box;
}

body{
  margin:0;
  font-family:Arial,sans-serif;
  background:
    radial-gradient(
      circle at top,
      #3b0764,
      #170b2e 40%,
      #050505
    );
  color:white;
  min-height:100vh;
}

.header{
  background:rgba(20,10,35,.95);
  padding:22px;
  text-align:center;
  border-bottom:1px solid #6d28d9;
}

.title{
  font-size:27px;
  font-weight:bold;
  color:#d8b4fe;
}

.subtitle{
  margin-top:8px;
  color:#a78bfa;
}

.container{
  width:100%;
  max-width:550px;
  margin:auto;
  padding:18px;
}

.card{
  background:
    linear-gradient(
      145deg,
      #1e1235,
      #0d0d12
    );
  border:1px solid #4c1d95;
  border-radius:20px;
  padding:18px;
  margin-bottom:15px;
}

.name{
  font-size:20px;
  font-weight:bold;
}

.state{
  margin-top:6px;
  color:#a78bfa;
}

button{
  width:100%;
  border:0;
  border-radius:14px;
  padding:15px;
  margin-top:15px;
  font-size:18px;
  font-weight:bold;
  color:white;
  cursor:pointer;
}

.btn-on{
  background:
    linear-gradient(
      135deg,
      #16a34a,
      #15803d
    );
}

.btn-off{
  background:
    linear-gradient(
      135deg,
      #4b5563,
      #1f2937
    );
}

.all-buttons{
  display:flex;
  gap:10px;
}

.all-buttons button{
  flex:1;
}

.btn-all-on{
  background:
    linear-gradient(
      135deg,
      #7e22ce,
      #581c87
    );
}

.btn-all-off{
  background:
    linear-gradient(
      135deg,
      #b91c1c,
      #7f1d1d
    );
}

.edit-button{
  background:
    linear-gradient(
      135deg,
      #9333ea,
      #6d28d9
    );
}

.wifi-button{
  background:
    linear-gradient(
      135deg,
      #2563eb,
      #1d4ed8
    );
}

.connection{
  text-align:center;
  margin-top:20px;
  color:#a78bfa;
}

</style>

</head>

<body>

<div class="header">

<div class="title">
ENG. AHMED EBAID
</div>

<div class="subtitle">
ESP8266 • 4 Relay • DEVICE
</div>

</div>

<div class="container">

<div class="card">

<div class="name" id="name1">
Relay 1
</div>

<div class="state" id="state1">
OFF
</div>

<button
id="button1"
class="btn-off"
onclick="toggleRelay(1)">

تشغيل

</button>

</div>


<div class="card">

<div class="name" id="name2">
Relay 2
</div>

<div class="state" id="state2">
OFF
</div>

<button
id="button2"
class="btn-off"
onclick="toggleRelay(2)">

تشغيل

</button>

</div>


<div class="card">

<div class="name" id="name3">
Relay 3
</div>

<div class="state" id="state3">
OFF
</div>

<button
id="button3"
class="btn-off"
onclick="toggleRelay(3)">

تشغيل

</button>

</div>


<div class="card">

<div class="name" id="name4">
Relay 4
</div>

<div class="state" id="state4">
OFF
</div>

<button
id="button4"
class="btn-off"
onclick="toggleRelay(4)">

تشغيل

</button>

</div>


<div class="card">

<div class="name">
التحكم الكامل
</div>

<div class="all-buttons">

<button
class="btn-all-on"
onclick="allRelays(1)">

تشغيل الكل

</button>

<button
class="btn-all-off"
onclick="allRelays(0)">

إيقاف الكل

</button>

</div>

</div>


<div class="card">

<button
class="edit-button"
onclick="location.href='/names'">

✏️ تعديل أسماء الريلايات

</button>


<button
class="wifi-button"
onclick="location.href='/wifi'">

📶 إعدادات WiFi

</button>


<button
class="edit-button"
onclick="location.href='/remote'">

🌐 Remote Control

</button>

</div>


<div class="connection"
id="connection">

متصل بالـ ESP8266

</div>

</div>


<script>

function updateRelay(number,state){

  let button =
    document.getElementById(
      "button"+number
    );

  let status =
    document.getElementById(
      "state"+number
    );


  if(state){

    button.innerHTML =
      "إيقاف";

    button.className =
      "btn-on";

    status.innerHTML =
      "ON";

  }else{

    button.innerHTML =
      "تشغيل";

    button.className =
      "btn-off";

    status.innerHTML =
      "OFF";
  }
}


function toggleRelay(number){

  fetch(
    "/toggle?relay="+number
  )

  .then(
    response =>
      response.json()
  )

  .then(
    data => {

      updateRelay(
        number,
        data.state
      );

      document
        .getElementById(
          "connection"
        )
        .innerHTML =
        "تم تحديث الريلاي "
        + number;
    }
  )

  .catch(
    error => {

      document
        .getElementById(
          "connection"
        )
        .innerHTML =
        "خطأ في الاتصال";
    }
  );
}


function allRelays(state){

  fetch(
    "/all?state="+state
  )

  .then(
    response =>
      response.json()
  )

  .then(
    data => {

      updateRelay(
        1,
        data.relay1
      );

      updateRelay(
        2,
        data.relay2
      );

      updateRelay(
        3,
        data.relay3
      );

      updateRelay(
        4,
        data.relay4
      );


      document
        .getElementById(
          "connection"
        )
        .innerHTML =
        state
        ?
        "تم تشغيل جميع الريلايات"
        :
        "تم إيقاف جميع الريلايات";
    }
  );
}


function getStatus(){

  fetch("/status")

  .then(
    response =>
      response.json()
  )

  .then(
    data => {

      updateRelay(
        1,
        data.relay1
      );

      updateRelay(
        2,
        data.relay2
      );

      updateRelay(
        3,
        data.relay3
      );

      updateRelay(
        4,
        data.relay4
      );


      document
        .getElementById(
          "name1"
        )
        .innerHTML =
        data.name1;


      document
        .getElementById(
          "name2"
        )
        .innerHTML =
        data.name2;


      document
        .getElementById(
          "name3"
        )
        .innerHTML =
        data.name3;


      document
        .getElementById(
          "name4"
        )
        .innerHTML =
        data.name4;
    }
  );
}


getStatus();

</script>

</body>

</html>

)rawliteral";


// =====================================================
// REMOTE INFO PAGE
// =====================================================

const char REMOTE_INFO_PAGE[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html lang="ar" dir="rtl">

<head>

<meta charset="UTF-8">

<meta name="viewport"
content="width=device-width,initial-scale=1">

<title>Remote Control</title>

<style>

body{
  margin:0;
  font-family:Arial;
  background:
    radial-gradient(
      circle at top,
      #3b0764,
      #170b2e 40%,
      #050505
    );
  color:white;
  min-height:100vh;
  padding:20px;
}

.box{
  max-width:550px;
  margin:auto;
  background:#1e1235;
  border:1px solid #6d28d9;
  border-radius:20px;
  padding:25px;
}

h1{
  color:#d8b4fe;
  text-align:center;
}

p{
  line-height:1.8;
  color:#ddd;
}

.code{
  background:#08050c;
  border:1px solid #4c1d95;
  border-radius:12px;
  padding:12px;
  direction:ltr;
  text-align:left;
  overflow:auto;
}

button{
  width:100%;
  border:0;
  border-radius:14px;
  padding:15px;
  margin-top:12px;
  font-size:18px;
  font-weight:bold;
  color:white;
  background:
    linear-gradient(
      135deg,
      #9333ea,
      #6d28d9
    );
}

</style>

</head>

<body>

<div class="box">

<h1>
🌐 Remote Control
</h1>

<p>
التحكم من خارج البيت يتم من خلال HiveMQ Cloud.
</p>

<p>
Device ID:
<b id="device">DEVICE</b>
</p>

<div class="code">

Host:
fa214242813b40d895f06590b2db5c5d.s1.eu.hivemq.cloud

<br><br>

MQTT Port:
8883

<br><br>

WebSocket Port:
8884

<br><br>

Path:
/mqtt

<br><br>

Base Topic:
eng_ahmed_ebaid/devices/DEVICE/

</div>

<button
onclick="location.href='/'">

← Back

</button>

</div>

</body>

</html>

)rawliteral";


// =====================================================
// ROOT
// =====================================================

void handleRoot(){

  String page =
    FPSTR(MAIN_PAGE);

  page.replace(
    "DEVICE",
    DEVICE_ID
  );

  server.send(
    200,
    "text/html; charset=UTF-8",
    page
  );
}


// =====================================================
// REMOTE INFO
// =====================================================

void handleRemoteInfo(){

  String page =
    FPSTR(REMOTE_INFO_PAGE);

  page.replace(
    "DEVICE",
    DEVICE_ID
  );

  server.send(
    200,
    "text/html; charset=UTF-8",
    page
  );
}


// =====================================================
// TOGGLE
// =====================================================

void handleToggle(){

  if(
    !server.hasArg("relay")
  ){

    server.send(
      400,
      "text/plain",
      "Relay missing"
    );

    return;
  }


  int relay =
    server.arg("relay").toInt();


  if(
    relay < 1 ||
    relay > 4
  ){

    server.send(
      400,
      "text/plain",
      "Invalid relay"
    );

    return;
  }


  bool currentState = false;


  if(relay == 1)
    currentState = relay1State;

  if(relay == 2)
    currentState = relay2State;

  if(relay == 3)
    currentState = relay3State;

  if(relay == 4)
    currentState = relay4State;


  bool newState =
    !currentState;


  setRelay(
    relay,
    newState
  );


  String json =
    "{\"state\":";

  json +=
    newState
    ? "true"
    : "false";

  json += "}";


  server.send(
    200,
    "application/json",
    json
  );
}


// =====================================================
// ALL
// =====================================================

void handleAll(){

  if(
    !server.hasArg("state")
  ){

    server.send(
      400,
      "text/plain",
      "State missing"
    );

    return;
  }


  bool state =
    server.arg("state").toInt();


  setRelay(1,state);
  setRelay(2,state);
  setRelay(3,state);
  setRelay(4,state);


  String json = "{";


  json += "\"relay1\":";
  json +=
    relay1State
    ? "true"
    : "false";


  json += ",\"relay2\":";
  json +=
    relay2State
    ? "true"
    : "false";


  json += ",\"relay3\":";
  json +=
    relay3State
    ? "true"
    : "false";


  json += ",\"relay4\":";
  json +=
    relay4State
    ? "true"
    : "false";


  json += "}";


  server.send(
    200,
    "application/json",
    json
  );
}


// =====================================================
// STATUS
// =====================================================

void handleStatus(){

  String json = "{";


  json += "\"relay1\":";
  json +=
    relay1State
    ? "true"
    : "false";


  json += ",\"relay2\":";
  json +=
    relay2State
    ? "true"
    : "false";


  json += ",\"relay3\":";
  json +=
    relay3State
    ? "true"
    : "false";


  json += ",\"relay4\":";
  json +=
    relay4State
    ? "true"
    : "false";


  json += ",\"name1\":\"";
  json += relay1Name;
  json += "\"";


  json += ",\"name2\":\"";
  json += relay2Name;
  json += "\"";


  json += ",\"name3\":\"";
  json += relay3Name;
  json += "\"";


  json += ",\"name4\":\"";
  json += relay4Name;
  json += "\"";


  json += "}";


  server.send(
    200,
    "application/json",
    json
  );
}


// =====================================================
// NAMES PAGE
// =====================================================

const char NAMES_PAGE[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html lang="ar" dir="rtl">

<head>

<meta charset="UTF-8">

<meta name="viewport"
content="width=device-width,initial-scale=1">

<title>Edit Names</title>

<style>

body{
  font-family:Arial;
  background:#10051c;
  color:white;
  padding:20px;
}

.box{
  max-width:550px;
  margin:auto;
}

input{
  width:100%;
  padding:14px;
  margin:8px 0 15px;
  box-sizing:border-box;
  border-radius:10px;
  border:1px solid #6d28d9;
  background:#08050c;
  color:white;
  font-size:17px;
}

button{
  width:100%;
  padding:15px;
  margin-top:10px;
  border:0;
  border-radius:12px;
  font-size:18px;
  font-weight:bold;
}

.save{
  background:#7e22ce;
  color:white;
}

.back{
  background:#374151;
  color:white;
}

</style>

</head>

<body>

<div class="box">

<h2>
✏️ Edit Names
</h2>


<label>
Relay 1
</label>

<input id="name1">


<label>
Relay 2
</label>

<input id="name2">


<label>
Relay 3
</label>

<input id="name3">


<label>
Relay 4
</label>

<input id="name4">


<button
class="save"
onclick="saveNames()">

💾 Save Names

</button>


<button
class="back"
onclick="location.href='/'">

← Back

</button>


<p id="message"></p>

</div>


<script>

function loadNames(){

  fetch("/names/status")

  .then(
    r => r.json()
  )

  .then(
    data => {

      name1.value =
        data.name1;

      name2.value =
        data.name2;

      name3.value =
        data.name3;

      name4.value =
        data.name4;
    }
  );
}


function saveNames(){

  let url =
    "/names/save" +

    "?name1=" +
    encodeURIComponent(
      name1.value
    ) +

    "&name2=" +
    encodeURIComponent(
      name2.value
    ) +

    "&name3=" +
    encodeURIComponent(
      name3.value
    ) +

    "&name4=" +
    encodeURIComponent(
      name4.value
    );


  fetch(url)

  .then(
    r => r.json()
  )

  .then(
    data => {

      if(data.success){

        message.innerHTML =
          "✅ تم الحفظ";

        setTimeout(
          () =>
            location.href="/",
          1000
        );
      }
    }
  );
}


loadNames();

</script>

</body>

</html>

)rawliteral";


// =====================================================
// NAMES HANDLERS
// =====================================================

void handleNamesPage(){

  server.send_P(
    200,
    "text/html; charset=UTF-8",
    NAMES_PAGE
  );
}


void handleNamesStatus(){

  String json = "{";


  json += "\"name1\":\"";
  json += relay1Name;
  json += "\"";


  json += ",\"name2\":\"";
  json += relay2Name;
  json += "\"";


  json += ",\"name3\":\"";
  json += relay3Name;
  json += "\"";


  json += ",\"name4\":\"";
  json += relay4Name;
  json += "\"";


  json += "}";


  server.send(
    200,
    "application/json",
    json
  );
}


void handleNamesSave(){

  if(
    !server.hasArg("name1") ||
    !server.hasArg("name2") ||
    !server.hasArg("name3") ||
    !server.hasArg("name4")
  ){

    server.send(
      400,
      "text/plain",
      "Names missing"
    );

    return;
  }


  relay1Name =
    server.arg("name1");

  relay2Name =
    server.arg("name2");

  relay3Name =
    server.arg("name3");

  relay4Name =
    server.arg("name4");


  if(
    relay1Name.length() == 0
  )
    relay1Name = "Relay 1";


  if(
    relay2Name.length() == 0
  )
    relay2Name = "Relay 2";


  if(
    relay3Name.length() == 0
  )
    relay3Name = "Relay 3";


  if(
    relay4Name.length() == 0
  )
    relay4Name = "Relay 4";


  saveString(
    RELAY1_NAME_ADDR,
    relay1Name,
    32
  );

  saveString(
    RELAY2_NAME_ADDR,
    relay2Name,
    32
  );

  saveString(
    RELAY3_NAME_ADDR,
    relay3Name,
    32
  );

  saveString(
    RELAY4_NAME_ADDR,
    relay4Name,
    32
  );


  publishAllNames();


  server.send(
    200,
    "application/json",
    "{\"success\":true}"
  );
}


// =====================================================
// WIFI PAGE
// =====================================================

const char WIFI_PAGE[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html lang="ar" dir="rtl">

<head>

<meta charset="UTF-8">

<meta name="viewport"
content="width=device-width,initial-scale=1">

<title>WiFi Settings</title>

<style>

body{
  font-family:Arial;
  background:#080b12;
  color:white;
  padding:20px;
}

.box{
  max-width:550px;
  margin:auto;
}

input{
  width:100%;
  padding:14px;
  box-sizing:border-box;
  margin:8px 0 15px;
  border-radius:10px;
  background:#050810;
  border:1px solid #2563eb;
  color:white;
  font-size:17px;
}

button{
  width:100%;
  padding:15px;
  margin-top:10px;
  border:0;
  border-radius:12px;
  font-size:18px;
  font-weight:bold;
  color:white;
}

.save{
  background:#2563eb;
}

.delete{
  background:#dc2626;
}

.back{
  background:#374151;
}

.info{
  margin-top:20px;
  line-height:1.8;
  color:#bfdbfe;
}

</style>

</head>

<body>

<div class="box">

<h2>
📶 WiFi Settings
</h2>


<form action="/wifi/save">

<label>
WiFi Name
</label>

<input
name="ssid"
value="%SSID%"
required>


<label>
WiFi Password
</label>

<input
type="password"
name="password"
value="%PASSWORD%">


<button
class="save"
type="submit">

💾 Save & Connect

</button>

</form>


<button
class="delete"
onclick="forgetWiFi()">

🗑️ Forget WiFi

</button>


<button
class="back"
onclick="location.href='/'">

← Back

</button>


<div class="info">

الشبكة الحالية:

<br>

<b>%SSID%</b>

<br><br>

ESP Home IP:

<br>

<b>%HOME_IP%</b>

<br><br>

ESP Access Point:

<br>

<b>192.168.4.1</b>

</div>

</div>


<script>

function forgetWiFi(){

  if(
    confirm("حذف شبكة WiFi؟")
  ){

    location.href =
      "/wifi/delete";
  }
}

</script>

</body>

</html>

)rawliteral";


// =====================================================
// WIFI PAGE HANDLER
// =====================================================

void handleWiFiPage(){

  String page =
    FPSTR(WIFI_PAGE);


  page.replace(
    "%SSID%",
    savedSSID
  );


  page.replace(
    "%PASSWORD%",
    savedPassword
  );


  String homeIP =
    "غير متصل";


  if(
    WiFi.status() ==
    WL_CONNECTED
  ){

    homeIP =
      WiFi.localIP().toString();
  }


  page.replace(
    "%HOME_IP%",
    homeIP
  );


  server.send(
    200,
    "text/html; charset=UTF-8",
    page
  );
}


// =====================================================
// SAVE WIFI
// =====================================================

void handleWiFiSave(){

  if(
    !server.hasArg("ssid")
  ){

    server.send(
      400,
      "text/plain",
      "SSID missing"
    );

    return;
  }


  savedSSID =
    server.arg("ssid");


  if(
    server.hasArg("password")
  ){

    savedPassword =
      server.arg("password");

  } else {

    savedPassword = "";
  }


  savedSSID.trim();


  saveString(
    WIFI_SSID_ADDR,
    savedSSID,
    64
  );


  saveString(
    WIFI_PASS_ADDR,
    savedPassword,
    64
  );


  server.send(
    200,
    "text/html; charset=UTF-8",

    "<html><body "
    "style='background:#08050c;"
    "color:white;text-align:center;"
    "font-family:Arial;padding:50px'>"
    "<h2>تم حفظ WiFi</h2>"
    "<p>ESP8266 سيتم إعادة تشغيله الآن...</p>"
    "</body></html>"
  );


  delay(1500);

  ESP.restart();
}


// =====================================================
// DELETE WIFI
// =====================================================

void handleWiFiDelete(){

  savedSSID = "";
  savedPassword = "";


  saveString(
    WIFI_SSID_ADDR,
    "",
    64
  );


  saveString(
    WIFI_PASS_ADDR,
    "",
    64
  );


  server.send(
    200,
    "text/html; charset=UTF-8",

    "<html><body "
    "style='background:#08050c;"
    "color:white;text-align:center;"
    "font-family:Arial;padding:50px'>"
    "<h2>تم حذف WiFi</h2>"
    "<p>ESP8266 سيتم إعادة تشغيله...</p>"
    "</body></html>"
  );


  delay(1500);

  ESP.restart();
}


// =====================================================
// SETUP
// =====================================================

void setup(){

  Serial.begin(115200);

  delay(500);


  Serial.println();
  Serial.println();


  Serial.println(
    "################################"
  );

  Serial.println(
    "ESP8266 SMART RELAY FINAL"
  );

  Serial.print(
    "DEVICE ID: "
  );

  Serial.println(
    DEVICE_ID
  );

  Serial.print(
    "MQTT USERNAME: "
  );

  Serial.println(
    MQTT_USERNAME
  );

  Serial.print(
    "BASE TOPIC: "
  );

  Serial.println(
    MQTT_BASE
  );

  Serial.println(
    "################################"
  );


  // ===================================================
  // RELAY PINS
  // ===================================================

  pinMode(
    RELAY1,
    OUTPUT
  );

  pinMode(
    RELAY2,
    OUTPUT
  );

  pinMode(
    RELAY3,
    OUTPUT
  );

  pinMode(
    RELAY4,
    OUTPUT
  );


  // ===================================================
  // ALL RELAYS OFF
  // ===================================================

  digitalWrite(
    RELAY1,
    RELAY_OFF
  );

  digitalWrite(
    RELAY2,
    RELAY_OFF
  );

  digitalWrite(
    RELAY3,
    RELAY_OFF
  );

  digitalWrite(
    RELAY4,
    RELAY_OFF
  );


  Serial.println(
    "Relays initialized OFF"
  );


  // ===================================================
  // EEPROM
  // ===================================================

  loadSettings();


  // ===================================================
  // WIFI
  // ===================================================

  WiFi.mode(
    WIFI_AP_STA
  );

  WiFi.setAutoReconnect(
    true
  );

  WiFi.persistent(
    false
  );


  // ===================================================
  // AP
  // ===================================================

  WiFi.softAPConfig(
    AP_IP,
    AP_GATEWAY,
    AP_SUBNET
  );


  bool apResult =
    WiFi.softAP(
      AP_SSID,
      AP_PASSWORD
    );


  if(apResult){

    Serial.println();
    Serial.println(
      "================================"
    );

    Serial.println(
      "ACCESS POINT STARTED"
    );

    Serial.println(
      "================================"
    );


    Serial.print(
      "SSID: "
    );

    Serial.println(
      AP_SSID
    );


    Serial.print(
      "IP: "
    );

    Serial.println(
      WiFi.softAPIP()
    );


    Serial.println(
      "================================"
    );

  } else {

    Serial.println(
      "ACCESS POINT FAILED"
    );
  }


  // ===================================================
  // HOME WIFI
  // ===================================================

  connectToHomeWiFi();


  // ===================================================
  // MQTT TLS
  // ===================================================

  // للتجربة الأولية مع HiveMQ Cloud
  mqttSecureClient.setInsecure();


  mqttClient.setServer(
    MQTT_HOST,
    MQTT_PORT
  );


  mqttClient.setCallback(
    mqttCallback
  );


  mqttClient.setBufferSize(
    512
  );


  // ===================================================
  // MQTT FIRST CONNECTION
  // ===================================================

  connectToMQTT();


  // ===================================================
  // WEB ROUTES
  // ===================================================

  server.on(
    "/",
    handleRoot
  );


  server.on(
    "/remote",
    handleRemoteInfo
  );


  server.on(
    "/toggle",
    handleToggle
  );


  server.on(
    "/all",
    handleAll
  );


  server.on(
    "/status",
    handleStatus
  );


  server.on(
    "/names",
    handleNamesPage
  );


  server.on(
    "/names/status",
    handleNamesStatus
  );


  server.on(
    "/names/save",
    handleNamesSave
  );


  server.on(
    "/wifi",
    handleWiFiPage
  );


  server.on(
    "/wifi/save",
    handleWiFiSave
  );


  server.on(
    "/wifi/delete",
    handleWiFiDelete
  );


  // ===================================================
  // START WEB SERVER
  // ===================================================

  server.begin();


  Serial.println();

  Serial.println(
    "================================"
  );

  Serial.println(
    "WEB SERVER STARTED"
  );

  Serial.println(
    "================================"
  );


  Serial.print(
    "AP URL: http://"
  );

  Serial.println(
    WiFi.softAPIP()
  );


  if(
    WiFi.status() ==
    WL_CONNECTED
  ){

    Serial.print(
      "Home URL: http://"
    );

    Serial.println(
      WiFi.localIP()
    );
  }


  Serial.println();

  Serial.println(
    "================================"
  );

  Serial.println(
    "READY"
  );

  Serial.println(
    "================================"
  );
}


// =====================================================
// LOOP
// =====================================================

void loop(){

  // ===================================================
  // WEB SERVER
  // ===================================================

  server.handleClient();


  // ===================================================
  // WIFI + MQTT
  // ===================================================

  if(
    WiFi.status() ==
    WL_CONNECTED
  ){

    // -------------------------------
    // MQTT
    // -------------------------------

    if(
      !mqttClient.connected()
    ){

      unsigned long now =
        millis();


      if(
        now - lastMQTTAttempt
        >=
        MQTT_RECONNECT_INTERVAL
      ){

        lastMQTTAttempt =
          now;

        connectToMQTT();
      }

    } else {

      mqttClient.loop();
    }

  } else {

    // -------------------------------
    // MQTT DISCONNECTED
    // -------------------------------

    if(
      mqttClient.connected()
    ){

      mqttClient.disconnect();
    }


    // -------------------------------
    // WIFI RECONNECT
    // -------------------------------

    unsigned long now =
      millis();


    if(
      savedSSID.length() > 0 &&
      now - lastWiFiAttempt
      >=
      WIFI_RECONNECT_INTERVAL
    ){

      lastWiFiAttempt =
        now;


      Serial.println();
      Serial.println(
        "WiFi disconnected - reconnecting..."
      );


      WiFi.begin(
        savedSSID.c_str(),
        savedPassword.c_str()
      );
    }
  }
}
