[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/XBO6NBqk)

# Weather staion
Jeg har fået til opgave, at sætte en vejrstation system op. Her skal en Arduino med en vejrsonsor, kunne samle vejrdata og sende det til en MQTT broker, på en Ubuntu server. Dermed skal en MQTTklient kunne lytte med på brokeren, hvor den så gemmer dataen i en database. Til sidst skal jeg sætte Grafana op på serveren, hvor den kan vise grafer af vejrdataen, der blev gemt i DB. Som en ekstropgave, kan jeg opsætte et API der udstiller dataen. Sammen med en frontend der kan fremvise det.
## Værktøjer
- C/C++/C#
- HTML/JS/CSS
- Arduino UNO R4 wifi
- 4 han til hun ledninger
- BME280 Sensor (temperature, humidity. pressure)
- Ubuntu server (192.168.103.11)
- PuTTY (terminalforbindelse med copy paste funktion)
## Arduino opsætning
Opsætningen gør brug af en I2C two-wire protokol. Hvor SDA (Serial Data) og SCL (Serial Clock) skaber en dataforbindelse, mellem arduino og sensor. 
| Arduino ports  | | BME280 sensor |
| ------------- | ------------- | ------------- |
| 5V  | -> | VIN |
| GND | -> | GND |
| SCL | -> | SCL |
| SDA | -> | SDA |

Koden gør brug af Arduinoens wifi funktion. Hvor den henter UTC tid for timestamp, og havniveautryk for at udregne altitude. Min code loop har fokus på at altid tjekke for forbindelse med wifi og serveren. Dermed vil der ikke gå noget data tabt.
