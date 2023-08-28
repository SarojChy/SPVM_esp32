#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <MFRC522.h>
#include <RTClib.h> // for the RTC
#include <HTTPClient.h>
#include <Preferences.h>
#include <AsyncTCP.h>
#include <LiquidCrystal_I2C.h>
#include <ESPAsyncWebServer.h>

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
IPAddress localIP;
//IPAddress localIP(192, 168, 1, 200); // hardcoded
// Set your Gateway IP address
//IPAddress localGateway;
IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 0, 0);

WiFiClient client;
HTTPClient http;

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "uid";
//Variables to save values from HTML form
String ssid;
String pass;
String uid;

//const char* ssid = "nepatronix_2.4";
//const char* password = "CLB269DA03";

const char* ssid_Path = "/ssid.txt";
const char* pass_Path = "/pass.txt";
const char* uid_Path = "/uid.txt";

const char* UIDPath = "/TEST.txt";
const char* logPath = "/DATA.txt";

Preferences preferences;
#define SS_PIN 2
#define RST_PIN 4
#define SD_pin 15

#define ResetWiFi 34 // add 10k resistor to 3.3v and after resistor of pin iside to button then to ground
#define Resetpin 35 // previously used 34 in DOIT version

//#define ResetWiFi 12 // add 10k resistor to 3.3v and after resistor of pin iside to button then to ground
//#define Resetpin 13 // previously used 34 in DOIT version

#define BUZZ 13 //define pin buzzer pin
#define LED 12 // LED for blinking fror WiFi Status
#define motorEN1 14  //enbalbe pin for driver 1
#define motorEN2 27 //enbalbe pin for driver 2


#define stepPin  26
#define dirPin  25
#define stepPin2 33
#define dirPin2 32

const char* host = "maker.ifttt.com";
bool SendMsg = false;


unsigned long currentMillis;
const unsigned long readTagInterval = 200;
unsigned long prevTagIntereval = 0;
const unsigned long WiFiInterval = 5000;
unsigned long prevWiFiIntereval = 0;
const unsigned long eventAPmode = 15000;  //Turn into Access point mode for WiFi configuration for 15 Secconds
unsigned long prevAPevent = 0;
const unsigned long LED_Blink = 500; // If WiFi is not connected then LED blink for each 500 miliseconds
unsigned long LED_Prev = 0;



File myFile; //
RTC_DS1307 rtc;

const int Nema17Motor_Steps = 200;
const int PadNo = 60;   //Total number of PAD

String each_uid; // individual uid of tag

int NpadLim = PadNo / 2;

LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 mfrc522(SS_PIN, RST_PIN);
int NPadNo;
String  DataTemp;
String  UID;
//bool Valid_UID = false;
String Valid_UID;

void setup()
{
  Serial.begin(115200);

  digitalWrite(motorEN1, HIGH);
  digitalWrite(motorEN2, HIGH);
  WiFi.mode(WIFI_STA);
  Buzzer();
  SPI.begin();
  lcd.init();
  lcd.backlight();
  mfrc522.PCD_Init();
  initSPIFFS();
  WiFi.mode(WIFI_STA);


  // Reading values from Access Point mode
  ssid = readFile(SPIFFS, ssid_Path);
  pass = readFile(SPIFFS, pass_Path);
  uid = readFile (SPIFFS, uid_Path);

  delay(500);
  uid = uid.c_str();
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(uid);
  WiFi.begin(ssid.c_str(), pass.c_str());
  displayMsg("Connecting WIFi");
  delay(4000);
  lcd.clear();
  if (!SD.begin(SD_pin)) {
    Serial.println("Error initializing SD card");
    displayMsg("SD InIt failed!");
    while (1);
  }
  Serial.println("SD card initialized");

  //  if (!SD.exists("TEST.txt")) {
  //
  //    Serial.println("NO Exists Creating");
  //    File Data = SD.open("TEST.txt", FILE_WRITE);
  //    Data.close();
  //  }
  //  else {
  //    Serial.println("File exits");
  //  }


  if (!rtc.begin()) {
    //Serial.println("Couldn't find RTC");
    displayMsg("RTC init failed!");
    while (1);
  }
  Serial.println("RTC Initialized initialized");

  preferences.begin("my-app", false);

  pinMode(BUZZ, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(motorEN1, OUTPUT);

  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  pinMode(motorEN2, OUTPUT);

  digitalWrite(dirPin, HIGH);
  digitalWrite(dirPin2, HIGH);

  pinMode(Resetpin, INPUT_PULLUP);
  pinMode(ResetWiFi, INPUT_PULLUP);
  NPadNo = preferences.getInt("NPadNo", 0);

  delay(1000);



  lcd.clear();
  displayMsg("    Namaste");
  delay(2000);
  displayMsg("** Nepatronix **");
  for (int a = 0; a <= 15; a++) {
    lcd.setCursor(a, 1);
    lcd.print(".");
    delay(150);
  }
  delay(1000);
  displayMsg(" Tap Your Card");
  lcd.setCursor(1, 1);
  lcd.print("Available: ");
  lcd.setCursor(12, 1);
  lcd.print(NPadNo);

}

void loop() {


  currentMillis = millis();

  NPadNo = preferences.getInt("NPadNo", 0);
  Serial.println(NPadNo);
  delay(200);

  if (digitalRead(Resetpin) == 0) {
    Serial.println(digitalRead(Resetpin));
    delay(20);
    Reset();
  }

  if (digitalRead(ResetWiFi) == LOW) {
    if (currentMillis - prevAPevent >= eventAPmode) {
      AP_Mode();
      prevAPevent = currentMillis;
    }
  }

  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - prevWiFiIntereval >=  WiFiInterval)) {
    Serial.println("WiFi Disconnected");
    WiFi.disconnect();
    delay(50);
    WiFi.reconnect();
    prevWiFiIntereval = currentMillis;
  }

  //  If WiFi not Connected then Blinks the LED at 500 miliseconds intervals
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - LED_Prev >= LED_Blink)) {
    digitalWrite(LED, !digitalRead(LED));
    LED_Prev = currentMillis;
  }
  else {
    //  If connected the just LIGHT the L ED ON
    digitalWrite(LED, HIGH);
  }


  if ((NPadNo > 0) && (currentMillis - prevTagIntereval >=  readTagInterval) && mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.println("Reading");

    //Serial.println("Card found");
    each_uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      each_uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    each_uid.toUpperCase();
    each_uid.replace(" ", ""); // Remove spaces from tag data
    Serial.print("Data : ");
    Serial.println(each_uid);

    File myFile = SD.open(UIDPath, FILE_READ);
     if (!myFile) {
      Serial.println("Error opening FILE");
    }
    
    if (myFile) {
      Serial.println("File is Opned");
      while (myFile.available()) {
        String line = myFile.readStringUntil('\n');
        if (line.startsWith(each_uid)) {

          preferences.putInt("ValidUID", 1);
          myFile.close();
          break;
        }
        line = "";
      }
    }
   
    verify(); //checking either UIDs presein SD or not
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    prevTagIntereval = currentMillis;
  }
  if (NPadNo == 0) {
    nopad();
    int msg = preferences.getInt("sendMsg", 0);
    if (msg == 0) {
      sendSMS();
    }
  }

}


void verify() {

  if (preferences.getInt("ValidUID", 0) == 1) {

    //    Serial.println("Valid: " + each_uid);
    Buzzer();
    checkForStack();
    logDataOnSD();
    count();

  }

  if (preferences.getInt("ValidUID", 0) == 0) {
    Serial.println("NOT Valid");
    tone(BUZZ, 200);
    delay(1500);
    noTone(BUZZ);
    invalidCard();
  }
  preferences.putInt("ValidUID", 0);
}
