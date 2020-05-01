#include <max6675.h>
#include <Ethernet.h>

/* Begin Config Section *************************************************/

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 240);

#define CONFIG_GND_PIN      3 //this line can be deleted if use GNC original pin
#define CONFIG_VCC_PIN      4 //this line can be deleted if use VCC original pin
#define CONFIG_SCLK_PIN     5
#define CONFIG_CS_PIN       6
#define CONFIG_SO_PIN       7

const unsigned long DhcpLeaseMaintainInterval = 30000; //Dhcp Lease Maintain

/* End Config Section ***************************************************/


/* Notes: 
  Web Server adaptated from https://www.arduino.cc/en/Tutorial/WebServer
  
  MAX6675 config:
  With ethernet shield, can't be used these pins:
  https://www.arduino.cc/en/Main/ArduinoEthernetShieldV1
  Arduino communicates with both the W5100 and SD card using the SPI bus (through the ICSP header).
  This is on digital pins 10, 11, 12, and 13 on the Uno and pins 50, 51, and 52 on the Mega.
  On both boards, pin 10 is used to select the W5100 and pin 4 for the SD card (Only if used). These pins cannot be used for general I/O.
*/

EthernetServer server(80);
MAX6675 thermocouple(CONFIG_SCLK_PIN, CONFIG_CS_PIN, CONFIG_SO_PIN);
EthernetClient client;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  delay(3000); // To get time for reset or open serial montitor.
  
  Serial.println(F("Initializing. Wait 20 seconds..."));
  delay(20000);
  
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  
  Serial.println(F("Getting IP. Wait 20 seconds..."));
  delay(20000);
  
  Serial.print(F("server is: "));
  Serial.println(Ethernet.localIP());

  
  //DO NOT FORGET REVIEW THIS LINE:
  pinMode(CONFIG_VCC_PIN, OUTPUT); digitalWrite(CONFIG_VCC_PIN, HIGH); //this line can be deleted if use VCC original pin  
  pinMode(CONFIG_GND_PIN, OUTPUT); digitalWrite(CONFIG_GND_PIN, LOW);  //this line can be deleted if use GND original pin
}

void loop() {

  MaintainDhcpLeasing();
  
  // listen for incoming clients
  EthernetClient client = server.available();
  
  if (client) {
    Serial.println(F("new client"));
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println(F("HTTP/1.1 200 OK"));
          client.println(F("Content-Type: application/json"));
          client.println(F("Connection: close"));
          
          client.println();
          client.println(GetJsonMessage());
          //client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(100);
    // close the connection:
    client.stop();
    Serial.println(F("client disonnected"));
  }
}

void MaintainDhcpLeasing()
{
  currentMillis = millis();

  //millis was reset. Reset previosMillis too
  if ((currentMillis - previousMillis) < 0 )
  {
    Serial.println(F("Arduino internal clock reset"));
    Ethernet.maintain();  //execute periodically to maintain DHCP lease
    
    previousMillis = 0;
    return;
  }

  if ( (currentMillis - previousMillis) > DhcpLeaseMaintainInterval) 
  {
    Serial.println(F("Executing maintain DHCP lease routine"));
    Ethernet.maintain();  //execute periodically to maintain DHCP lease
    previousMillis = currentMillis;
  }
}

double GetSensorValue()
{
  /*
  double sumTemperatures = 0;
  double temperatureValue;
  for (int i = 0; i < NumberOfTemperatureReads; i++)
  {
    delay(100);
    temperatureValue =thermocouple.readCelsius();
    
    Serial.print(F("Temperature value (°C) - Step "));
    Serial.println(String(i) + ": " + temperatureValue);
  
    sumTemperatures += temperatureValue;
  }
  sumTemperatures = sumTemperatures / NumberOfTemperatureReads;

  sumTemperatures = round(sumTemperatures * 100.0) / 100.0;

  Serial.print(F("Temperature value (°C): "));
  Serial.println(sumTemperatures);
  */

  double temperatureValue;
  temperatureValue =thermocouple.readCelsius();

  Serial.print(F("Temperature value (°C): "));
  Serial.println(temperatureValue);
  return temperatureValue;
    
}

String GetJsonMessage()
{
  String value = String(GetSensorValue());
  String json = "{ \"ThermocoupleName\": \"main\", \"Value\": " + value + ", \"Scale\": \"Celsius\" }";

  return json;
}
