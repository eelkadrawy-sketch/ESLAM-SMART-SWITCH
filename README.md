# ESLAM SMART SWITCH

## Smart Home Automation System

**Personal Electronics Engineering Project — Islam Naser**

An ESP8266-based Smart Home Automation System designed for remote control of electrical loads using a 4-channel relay module, MQTT communication, and HiveMQ Cloud.

The system combines embedded hardware, cloud communication, and a responsive Web/Mobile control interface into one IoT platform.

---

## 📸 Project Gallery

### 📱 Dashboard

![Dashboard](Dashboard.png)

### 🎛️ Smart Control

![Smart Control](smart%20control.png)

---

## 🚀 Project Overview

The system allows users to remotely control four electrical loads through a Web/Mobile interface.

Communication between the control interface and the ESP8266 is handled through MQTT over HiveMQ Cloud.

Each controller has its own device identity and MQTT topic namespace.

---

## ⚡ Main Features

* 4 independent relay channels
* Individual ON/OFF control
* Control all relays simultaneously
* Real-time relay state synchronization
* Remote Web/Mobile control
* MQTT communication
* HiveMQ Cloud integration
* Device-based MQTT identity
* Wi-Fi configuration
* Local Access Point configuration
* Relay name customization
* EEPROM data storage
* MQTT over TLS
* Automatic MQTT reconnection

---

## 🧰 Main Technologies

* ESP8266 D1 Mini
* 4-Channel Relay Module
* MQTT
* HiveMQ Cloud
* WebSocket / WSS
* HTML
* CSS
* JavaScript
* MQTT.js
* EEPROM
* Wi-Fi

---

## 🏗️ System Architecture

The system consists of three main layers.

### Web / Mobile Interface

The user controls the Smart Home system through a responsive Web/Mobile interface.

Communication with the cloud broker uses MQTT over Secure WebSocket (WSS).

### HiveMQ Cloud

HiveMQ Cloud acts as the MQTT broker between the Web/Mobile interface and the ESP8266 controller.

The ESP8266 communicates with the broker using MQTT over TLS.

### ESP8266 Controller

The ESP8266 D1 Mini receives MQTT commands and controls four relay channels.

The controller also publishes the current relay states back to the MQTT broker.

---

## 🔌 Relay Configuration

| Relay   | ESP8266 Pin |
| ------- | ----------- |
| Relay 1 | D1          |
| Relay 2 | D2          |
| Relay 3 | D5          |
| Relay 4 | D6          |

The relay module uses **active-LOW logic**.

---

## ☁️ MQTT Architecture

Each physical controller has its own Device ID.

Example:

`AE0000019`

The MQTT topic structure is:

`eng_ahmed_ebaid/devices/<DEVICE_ID>/`

Example topics:

`eng_ahmed_ebaid/devices/AE0000019/relay1/set`

`eng_ahmed_ebaid/devices/AE0000019/relay1/state`

`eng_ahmed_ebaid/devices/AE0000019/relay2/set`

`eng_ahmed_ebaid/devices/AE0000019/relay2/state`

`eng_ahmed_ebaid/devices/AE0000019/relay3/set`

`eng_ahmed_ebaid/devices/AE0000019/relay3/state`

`eng_ahmed_ebaid/devices/AE0000019/relay4/set`

`eng_ahmed_ebaid/devices/AE0000019/relay4/state`

### Supported Commands

* ON
* OFF
* TOGGLE

### All Relays

`eng_ahmed_ebaid/devices/AE0000019/all/set`

`eng_ahmed_ebaid/devices/AE0000019/all/state`

---

## 📱 Web / Mobile Remote Control

The project includes a responsive Web/Mobile control interface.

Users can:

* Connect to HiveMQ Cloud
* Control each relay independently
* Control all relays simultaneously
* View real-time relay states
* Receive relay names from the ESP8266
* Automatically reconnect after connection loss

Remote interface:

`mobile-web-app/remote_control.html`

---

## 🛠️ Hardware

### Main Components

* ESP8266 D1 Mini
* 4-Channel Relay Module
* 5V Power Supply
* Wi-Fi Network
* Electrical Loads
* Connecting Wires

### 🔌 Wiring Diagram

![Wiring Diagram](wiring.jpeg)

---

## 💻 Software

### Firmware

* Arduino IDE
* ESP8266 Arduino Core
* PubSubClient
* ESP8266WiFi
* ESP8266WebServer
* EEPROM
* WiFiClientSecure

### Remote Interface

* HTML
* CSS
* JavaScript
* MQTT.js
* WebSocket / WSS

### Cloud

* HiveMQ Cloud
* MQTT over TLS

---

## 🧠 ESP8266 Firmware

The firmware provides:

* Relay control
* MQTT communication
* Wi-Fi management
* Access Point configuration
* EEPROM storage
* Relay state publishing
* Relay name publishing
* MQTT command processing
* Local web configuration

Firmware:

`firmware/ESP8266_4Relay/ESP8266_4Relay.ino`

---

## 🔄 System Workflow

User → Web/Mobile Interface → HiveMQ Cloud → ESP8266 D1 Mini → 4-Channel Relay → Electrical Loads

The ESP8266 publishes relay states back to HiveMQ Cloud so the control interface can remain synchronized with the physical device.

---

## 🗂️ Project Structure

* `firmware/` — ESP8266 firmware
* `mobile-web-app/` — Web/Mobile control interface
* `README.md` — Project documentation

---

## 🔐 Security

The system uses device-specific MQTT credentials and MQTT over TLS.

Real credentials should not be included in a public repository.

**Never publish MQTT passwords, Wi-Fi passwords, or other private credentials in a public GitHub repository.**

---

## 📊 Project Highlights

This project demonstrates practical experience in:

* Embedded Systems
* ESP8266 Programming
* IoT Development
* MQTT Communication
* Cloud Connectivity
* Web Development
* Hardware Interfacing
* Relay Control
* Wi-Fi Networking
* Device Authentication
* Real-Time Communication
* System Architecture

---

## 🔮 Future Development

Planned improvements include:

* Native Android application
* Independent user accounts
* OTA firmware updates
* Automated device provisioning
* Sensors and environmental monitoring
* Smart energy monitoring
* Scheduling
* Automation rules
* Push notifications
* Multi-device management
* Multi-user access control

---

## 📌 Project Status

**Prototype / Personal Engineering Project**

The project demonstrates the integration of:

**Embedded Systems + IoT + MQTT + Cloud Connectivity + Web Control**

---

## 👨‍💻 Author

**Islam Naser**

Electronics Engineering Graduate

LinkedIn: [Islam Naser](https://www.linkedin.com/in/eslam-naser-128450375/)

---

## ⭐ Project Summary

An end-to-end Smart Home IoT system combining:

**ESP8266 + 4-Channel Relay + MQTT + HiveMQ Cloud + Web/Mobile Control**

The project demonstrates the integration of embedded hardware, cloud communication, real-time control, and web technologies into a practical Smart Home automation system.
