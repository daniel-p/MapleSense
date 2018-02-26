# MAPLE Sense


# The Idea

A hardware/software solution consisting of wireless sensors that can work for years on two AAA batteries, a compact Raspberry Pi based gateway server and a web based UI.

## Tech
### Gateway
    
##### Hardware
* [Raspberry Pi](https://www.raspberrypi.org/) 
* nRF24L01+ Transceiver

##### Software

* [MySensors](https://www.mysensors.org/build/raspberry) - Low Power Arduino sensor library
* [Mosquitto](https://mosquitto.org/) MQTT Broker 
* [Node-RED](https://nodered.org/) - Node-RED with some plugins
[Node-RED MongoDB Plugin](https://flows.nodered.org/node/node-red-node-mongodb)
[Node-RED Dashboard plugin](https://flows.nodered.org/node/node-red-dashboard)

### Sensor

##### Hardware Components
* [Arduino Pro Mini](https://store.arduino.cc/usa/arduino-pro-mini) - Modded for low power
* nRF24L01+ Transceiver
* Reed Switch 
* 2 AAA Batteries
* 1 MOhm PullUp Resistor
* Perfboard
* 3D PrintedCase
* Tools

