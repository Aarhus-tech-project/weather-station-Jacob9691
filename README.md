[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/XBO6NBqk)

# Weather station
Jeg har fået til opgave, at sætte en vejrstation system op. Her skal en Arduino med en vejrsonsor, kunne samle vejrdata og sende det til en MQTT broker, på en Ubuntu server. Dermed skal en MQTTklient kunne lytte med på brokeren, hvor den så gemmer dataen i en database. Til sidst skal jeg sætte Grafana op på serveren, hvor den kan vise grafer af vejrdataen, der blev gemt i DB. Som en ekstropgave, kan jeg opsætte et API der udstiller dataen. Sammen med en frontend der kan fremvise det.

## Værktøjer og Teknologier
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

## Netværk opsætning
- Netværk: 192.168.103.0/24
- Default gateway: 192.168.103.1
- Ubuntu server IP: 192.168.103.11
  - Webserver (frontend): http://192.168.103.11
  - API-endpoint: http://192.168.103.11/api/weather
  - Grafana Dashboard: http://192.168.103.11:3000
  
#### Ubuntu login:
- user: gruppe3
- password: Datait2025!

## Server programs
### MQTT broker (mosquitto)
Mosquitto fungerer som MQTT-broker og modtager vejrdata fra Arduinoen. Brokeren er konfigureret til at acceptere lokale forbindelser fra både sensoren og den interne klient.

### MQTT client (C service program)
MQTT-klienten er et program skrevet i C, som abonnerer på mit MQTT-topic (weather/station1) og indsætter de modtagne vejrdata direkte i MySQL-databasen. Programmet er konfigureret til at køre som en baggrundsservice på serveren og starter automatisk op ved boot.

For at beskytte følsomme oplysninger som MySQL-brugernavn og adgangskode, er disse gemt i en .env-fil. Dette sikrer, at loginoplysninger ikke eksponeres i kildekoden eller versionsstyring.

### MySQL Database
Databasen gemmer vejrdata i en tabel med felter såsom temperatur, luftfugtighed, tryk og timestamp. Timestamp gemmes som en BIGINT i dansk tid (CET/CEST) i formatet: YYYYMMDDHHMMSS.

<img width="620" height="181" alt="mysqlDBtable" src="https://github.com/user-attachments/assets/8105e595-b060-4a2b-8fc1-f04395ca3aa3" />

### Grafana
Grafana er sat op til at hente data direkte fra MySQL og visualiserer dem som grafer. Man kan se historik og realtidsdata via dashboardet.

Grafana tilgås på: http://192.168.103.11:3000
<img width="1895" height="753" alt="grafanaImage" src="https://github.com/user-attachments/assets/a742d465-7d88-487a-bf3b-d23ed5f1a655" />

### .NET API webapp (frontend og API)
Et .NET Web API eksponerer de seneste målinger i JSON-format, som kan bruges af andre systemer eller af frontenden. Frontenden er også udviklet i .NET og viser de seneste målinger i et visuelt interface og grafer.
- API endpoint: http://192.168.103.11/api/weather
- Frontend: http://192.168.103.11
<img width="1888" height="888" alt="weatherStationSite" src="https://github.com/user-attachments/assets/d279580c-80ab-4d8b-bf85-b434f847aaa3" />

#### Nginx (proxy server for HTTP)
Nginx er konfigureret som en reverse proxy og håndterer trafik til både API og frontend. Det gør det muligt at samle alle webtjenester under et IP-adresse.




