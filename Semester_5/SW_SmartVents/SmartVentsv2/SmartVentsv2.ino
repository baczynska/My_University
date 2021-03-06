// Program odczytuje temperaturę z czujnika

#include <OneWire.h>
#include <DS18B20.h>
#include <Servo.h>
#include <WiFiNINA.h>
#include <SPI.h>

// Numer pinu z czujnikiem
#define ONEWIRE_PIN 2
// Numer pinu z serwem
#define SERVO_PIN 3

// Adres czujnika
byte address[8] = {0x28, 0x37, 0x62, 0x1B, 0x5F, 0x14, 0x1, 0xE5};

// Konfiguracja sieci
char ssid[] = "HUAWEI_P9_6684";
char pass[] = "41a62c89";
int keyIndex = 0;

int status = WL_IDLE_STATUS;

// Adres serwera
IPAddress server(192,168,43,2);

WiFiClient client;

OneWire onewire(ONEWIRE_PIN);
DS18B20 sensors(&onewire);
Servo servo;

float temperature = 20.00;
float regulation_temperature = 21.43;
int pos = 0;
String temp = "";

void setup() {
  while(!Serial);
  Serial.begin(9600);

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Prosze zaktualiowac oprogramowanie");
  }

  // Laczymy sie z siecia
  while (status != WL_CONNECTED) {
    Serial.print("Proba polaczenia do sieci o SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);

    delay(10000);
  }
  Serial.println("Polaczono do WiFi");
  printWifiStatus();

  Serial.println("\nLaczenie z serwerem...");
  // if you get a connection, report back via serial:
  if (client.connect(server, 10000)) {
    Serial.println("Polaczono z serwerem");
  }

  
  sensors.begin();
  sensors.request(address);

  servo.attach(SERVO_PIN);
  servo.write(pos);
  delay(500);
}

void loop() {
  // Obsluga czujnika
  if (sensors.available())
  {
    temperature = sensors.readTemperature(address);

    Serial.print("temperatura ");
    Serial.print(temperature);
    int num = client.print(String(temperature));
    Serial.println(F(" 'C"));
    Serial.print("ilosc znakow ");
    Serial.println(String(num));

    sensors.request(address);
  }
  /*temperature = 25.58;*/
  
  //Serial.print(temperature);
  //Serial.println(F(" 'C"));
  /*Wysylamy ilosc bajtow do odebrania przed temperatura*/
  //client.print(String(temperature));

  // Obsluga serwa
  if (temperature > regulation_temperature && pos == 0)
  {
    Serial.println("Temperature over threshold. Rotating...");
    for (; pos <= 90; pos+=5) {
      servo.write(pos);
      Serial.print(pos);
      Serial.print(" ");
      delay(30);
    }
    pos -= 5;
  }

  if (temperature <= regulation_temperature && pos == 90)
  {
    Serial.println("Temperature below threshold. Rotating...");
    for (; pos >= 0; pos -= 5) {
      servo.write(pos);
      Serial.print(pos);
      Serial.print(" ");
      delay(30);
    }
    pos += 5;
  }

  // Obsluga sieci
  int start = 0;
  int iter = 0;
  while (client.available()) {
    if (iter == 0) temp = "";
    iter = 1;
    char c = client.read();
    if (c == 'k') {
      start = 0;
      break;
    }
    if (start == 1) {
      temp = temp + c;
    }
    if (c == 's') { Serial.write("s"); start = 1; }
  }
  iter = 0;
  regulation_temperature = temp.toFloat();
  //rSerial.println(regulation_temperature);
  
  /*delay(20);*/

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("Rozlaczanie z serwerem.");
    client.stop();

    // do nothing forevermore:
    while (true);
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
