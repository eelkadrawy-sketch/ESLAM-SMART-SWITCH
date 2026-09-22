# ESLAM SMART SWITCH

## Smart Home Automation System

**Personal Electronics Engineering Project — Islam Naser**

An ESP8266-based Smart Home Automation System designed for remote control of electrical loads using a 4-channel relay module, MQTT communication, and HiveMQ Cloud.

The system combines embedded hardware, cloud-based communication, and a responsive Web/Mobile control interface into one IoT platform.

---

## 📸 Project Gallery

### 📱 Dashboard

![Dashboard](Dashboard.png)

### 🎛️ Smart Control

![Smart Control](smart%20control.png)

---

## 🚀 Project Overview

The system allows users to remotely control four electrical loads through a Web/Mobile interface.

Communication between the control interface and the ESP8266 is handled through **MQTT over HiveMQ Cloud**, with a dedicated device identity for each controller.

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
* Secure MQTT communication using TLS
* Automatic MQTT reconnection

---

## 🧰 Main Technologies

* ESP8266 D1 Mini
* 4-Channel Relay Module
* MQTT
* HiveMQ Cloud
* WebSocket / Secure WebSocket (WSS)
* HTML
* CSS
* JavaScript
* MQTT.js
* EEPROM
* Wi-Fi

---

## 🏗️ System Architecture

The system consists of three main layers:

### 1. Web / Mobile Interface

The user interacts with the Smart Home system through a responsive Web/Mobile interface.

```text
Web
```
