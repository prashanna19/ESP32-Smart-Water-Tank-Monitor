# ESP32 Smart Water Tank Monitor

An IoT-based water tank monitoring system using ESP32, FreeRTOS, MQTT and Node-RED. The system measures water levels using an ultrasonic sensor, publishes live readings over MQTT and displays real-time tank information with a five-minute prediction.

# Website Mock Image
<img width="1181" height="627" alt="Pasted 21-11-2026 at 18 09 11" src="https://github.com/user-attachments/assets/3c9fa662-11ef-4306-bb0f-73617e64210f" />


## Features

- Real-time water level monitoring.
- HC-SR04 ultrasonic distance measurement.
- ESP32 firmware using FreeRTOS tasks.
- MQTT communication using Mosquitto.
- Browser-based live monitoring dashboard.
- Node-RED dashboard with animated tank visualization.
- Five-minute water-level prediction using linear regression.
- Shared sensor data protected using a FreeRTOS mutex.

## Hardware Requirements

- ESP32 DevKit V1
- HC-SR04 ultrasonic sensor
- 2.2 kΩ resistor
- 1 kΩ resistor
- Prototype water container
- Jumper wires
- USB cable

## Wiring

| HC-SR04 | ESP32 |
|---|---|
| VCC | 5V / VIN |
| GND | GND |
| TRIG | GPIO 5 |
| ECHO | GPIO 18 through voltage divider |

**Important:** ESP32 GPIO pins operate at 3.3V. Do not connect the HC-SR04's 5V ECHO output directly to GPIO 18.

### Echo Voltage Divider

1. HC-SR04 ECHO → 2.2 kΩ resistor.
2. Other end of 2.2 kΩ resistor → GPIO 18.
3. GPIO 18 junction → 1 kΩ resistor.
4. Other end of 1 kΩ resistor → GND.

## Software Requirements

- Arduino IDE
- ESP32 Arduino board package
- PubSubClient library
- Mosquitto MQTT broker
- Node.js and Node-RED
- Node-RED Dashboard 2
- Modern web browser

## Project Structure

```text
ESP32-Smart-Water-Tank-Monitor/
├── firmware/
│   └── ESP32_Smart_Water_Tank_Monitor/
│       ├── ESP32_Smart_Water_Tank_Monitor.ino
│       └── secrets.example.h
├── dashboard/
│   └── index.html
├── node-red/
│   └── water_tank_flow.json
├── mqtt/
│   └── mosquitto.conf.example
├── docs/
├── screenshots/
├── .gitignore
└── README.md
```

The actual `secrets.h` file is intentionally excluded from Git. Create your own copy from `secrets.example.h`.

## System Architecture

```text
HC-SR04 Ultrasonic Sensor
          |
          v
      ESP32 DevKit
          |
     FreeRTOS Tasks
          |
 Calculate distance,
 water level and fill %
          |
          v
       MQTT Broker
       Mosquitto
          |
     MQTT Topic:
       water/data
          |
     +----+----+
     |         |
     v         v
Web Dashboard  Node-RED
               |
          Linear Regression
          5-Minute Prediction
               |
          Animated Dashboard
```

## Firmware Setup

1. Open Arduino IDE.
2. Install the ESP32 board package.
3. Install the PubSubClient library.
4. Open the firmware project folder.
5. Copy `secrets.example.h` and rename the copy to `secrets.h`.
6. Enter your Wi-Fi name, password and MQTT broker IP in `secrets.h`.
7. Connect the ESP32 and select the appropriate board and port.
8. Upload the firmware.

The prototype tank height is configured as 27 cm. Change `TANK_HEIGHT` in the sketch if your container has different dimensions.

## MQTT Broker Setup

Install Mosquitto and configure:

- TCP listener: port 1883
- WebSocket listener: port 9001

A sample configuration is provided in `mqtt/mosquitto.conf.example`.

Update `MQTT_SERVER` in your local `secrets.h` to the IP address of the computer running Mosquitto.

The ESP32 and MQTT broker must be reachable over the same network.

**Security note:** The example configuration permits anonymous connections for a trusted local demonstration network only. Do not expose an anonymously accessible broker to the public internet.

## Node-RED Setup

1. Install and start Node-RED.
2. Install the Node-RED Dashboard 2 package if necessary.
3. Open the Node-RED editor.
4. Import `node-red/water_tank_flow.json`.
5. Configure the MQTT broker as `127.0.0.1:1883` when Node-RED and Mosquitto run on the same computer.
6. Deploy the flow.
7. Open the dashboard using its configured URL.

The Node-RED flow subscribes to `water/data` and calculates live readings and a five-minute prediction.

### Prediction Method

The Node-RED function retains recent readings over a five-minute window and applies linear regression to estimate the current water-level trend.

It projects that trend five minutes into the future and limits the predicted level between 0 cm and the configured tank height of 27 cm.

This is a trend-based estimate, not a trained machine-learning model. Its accuracy depends on the consistency and quality of sensor readings.

## Browser Dashboard Setup

The browser dashboard uses MQTT over WebSockets on port 9001.

Start a local HTTP server:

```bash
cd dashboard
python3 -m http.server 8080
```

Open:

`http://localhost:8080`

Ensure the MQTT broker's WebSocket listener is accessible from the browser. When accessing the dashboard from another device, use the computer's reachable hostname or IP address.

## MQTT Message Format

**Topic:** `water/data`

Example payload:

```json
{
  "distance": 12.50,
  "waterLevel": 14.50,
  "fillPercent": 53.70
}
```

The ESP32 publishes readings approximately once per second.

## FreeRTOS Tasks

| Task | Responsibility |
|---|---|
| WiFi Task | Connects the ESP32 to Wi-Fi |
| Sensor Task | Reads the ultrasonic sensor and calculates water level |
| MQTT Task | Connects to the MQTT broker and publishes readings |

A FreeRTOS mutex protects shared sensor values when multiple tasks access them.

## Future Improvements

- Automatic Wi-Fi reconnection.
- Improved sensor filtering and noise handling.
- Historical water-level storage.
- Configurable tank dimensions.
- Low-level and high-level alerts.
- Improved forecasting and validation.

## License

No license has been assigned yet. All rights reserved unless a license is added.
