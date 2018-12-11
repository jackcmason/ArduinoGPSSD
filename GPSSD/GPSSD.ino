#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

/*
   9600-baud for GPS
   115200-baud for serial
   D4 is arduino RX 
   D3 is arduino TX

   Wiring Diagram:
   
     +----+   +-------+   +----+
   +-|SD  |   |ARDUINO|   |GPS |
   |C|VCC |---|5V   D3|---|RXD |
   |A|GND |---|GND  D4|---|TXD |
   |R|MOSI|---|D11 GND|---|GND |
   |D|SS  |---|D5 3.3V|---|VCC |
   +-|SCK |---|D13    |   +----+
     |MISO|---|D12    |   |Acc.|  
     +----+   |     A0|---|VCC |
              |     A1|---|X   |
              |     A2|---|Y   |
              |     A3|---|Z   |
              |     A4|---|GND |
              +-------+   +----+
*/

//Pins
static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;
const int gnd = A0;
const int vcc = A4;
const int sd = 5;
const int acc[] = {A3, A2, A1};

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

void setup() {
  Serial.begin(115200);
  ss.begin(GPSBaud);
  initSD(sd);
  initAcc(vcc, gnd);

  File file = SD.open("datalog.txt", FILE_WRITE);
  file.println("------------------------------------------------------------------------------------------------------------------------------");
  file.close();
}

void loop() {
  File file = SD.open("datalog.txt", FILE_WRITE);
  printData(file, gps, acc);
  file.close();
}
  

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

//initialize SD
void initSD(int sdPin) {
  // Open serial communications and wait for port to open:
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(sdPin)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
}

//initialize Accelerometer
void initAcc(int vccPin, int gndPin)
{
  pinMode(gndPin, OUTPUT);
  pinMode(vccPin, OUTPUT);
  digitalWrite(gndPin, LOW);
  digitalWrite(vccPin, HIGH);
}

//Print GPS & Accelorometer data to file
void printData(File file, TinyGPSPlus gps, int acc[]) {
  //get x, y & z from accelerometer
  int x = acc[0];
  int y = acc[1];
  int z = acc[2];
  // if the file is available, write to it:
  if (file) {
    file.print("Sats: ");
    file.print(gps.satellites.value());
    smartDelay(0);
    file.print(" Hdop: ");
    file.print(gps.hdop.hdop());
    smartDelay(0);
    file.print(" Lat: ");
    file.print(gps.location.lat(), 6);
    smartDelay(0);
    file.print(" Lon: ");
    file.print(gps.location.lng(), 6);
    smartDelay(0);
    file.print(" Age: ");
    file.print(gps.location.age());
    smartDelay(0);
    file.print(" Time: ");
    TinyGPSDate d = gps.date;
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    file.print(sz);
    smartDelay(0);
    TinyGPSTime t = gps.time;
    char zs[32];
    sprintf(zs, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    file.print(zs);
    smartDelay(0);
    file.print(" Alt: ");
    file.print(gps.altitude.meters());
    smartDelay(0);
    file.print(" Chars: ");
    file.print(gps.charsProcessed());
    smartDelay(0);
    file.print(" Sents: ");
    file.print(gps.sentencesWithFix());
    smartDelay(0);
    file.print(" CS: ");
    file.print(gps.failedChecksum());
    smartDelay(0);
    file.print(" rX: ");
    file.print(analogRead(x));
    file.print(" rY: ");
    file.print(analogRead(y));
    file.print(" rZ: ");
    file.print(analogRead(z));
    file.println();
    file.close();
  } else {
    Serial.println("error opening datalog.txt");
  }
  
  smartDelay(1000);

  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
}
