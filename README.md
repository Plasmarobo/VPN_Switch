## VPN WiFi Switch
A switch to indicate VPN (or any other MQTT device) status.
Works with Adafruit Feather Huzzah.

#### Components
* Adafruit Feather Huzzah
* 3x 270 Ohm resistors (through hole recommended)
* 1 RGB LED (can use 3 LEDS or be adapted for one LED)
* 1 Switch (With missile cover for VPN)
* 1 Switch (For Power/LED toggle)

#### Requirements and Prereqs
* 802.11n/g WiFi network (AC not supported)
* MQTT broker running on local network (Mosquitto recommended)
* VPN Service running on MQTT broker/network gateway
* MQTT-Launcher.py or other MQTT->Script system

#### MQTT Broker Setup
ToDo: OpenWRT tutorial for Mosquitto
ToDo: OpenWRT Firewall/OpenVPN setup tutorial
ToDo: Add sh scripts

#### Hardware assembly
ToDo: This

#### Printing the case
ToDo: This

#### Software
Set your WiFi credentials, and update the IP/hostname of your MQTT Broker.
Compile and upload to the Feather Huzzah

##### Legal
Contribute any bugfixes back please! Otherwise it's yours to do what you like with.

