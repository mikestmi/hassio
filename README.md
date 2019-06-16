Energy consumption monitoring and restriction system for buildings.

This project is about my diploma thesis.

The system is based on the Home Assistant platform, using specialized platforms like WeMos and Raspberry Pie.
Home Assistant runs on the Raspberry Pie unit, while two WeMos units are equipped with sensors to measure indoor and outdoor
 temperature, humidity, brightness. Also the indoor WeMos unit has a motion sensor. In addition, there are a Sonoff unit to monitor
 the energy consumption and a Broadlink unit to control the air conditioner via infrared signals.

 The first part of the project is to implement the above devices, to control them all throught the Home Assistant user interface and to 
 monitor their states. After that, the collected data are being saved to a database (InfluxDB) and visualized (Grafana).
 The second part includes the implementation of automation scenarios based on the collected data in order to reduce energy consumption.
