#include <WiFiClient.h>
#include "ESP32WebServer.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <esp_camera.h>
#include <Arduino.h>
#include <esp_timer.h>
#include <FS.h>
#include <SPIFFS.h>
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <EEPROM.h>            // read and write from flash memory
#define DIODA 33
#define EEPROM_SIZE 1
#define CAMERA_MODEL_AI_THINKER
//#define CAMERA_MODEL_WROVER_KIT

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

const char* ssid = "Lukas";
const char* password = "nalpinalpi";
const String KEYPASSWD = "1234";
ESP32WebServer server(80);

String passwd = "";
const int allowAttempts = 3;
int attemptsNumber = 0;
bool passwdBlocked = LOW;
String diodeStatusStr = "OFF";
bool takeNewPhoto = false;

String SendHTML(){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<meta http-equiv=\"refresh\" content=\"2;URL='http://esp32.local/'\">";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {  background-color: #555555; border: none; color: white; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 30px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>EMBEEDED SYSTEMS. ASSIGMENT NO. 7</h1>\n";
  
  if(passwdBlocked) {
    ptr +="<p>Status diody: " + diodeStatusStr + "</p>\n";
    ptr += "<p>Password: " + passwd + "</p>";
    ptr +="<a class=\"button button-off\" href=\"/\">1</a>\n";
    ptr +="<a class=\"button button-off\" href=\"/\">2</a>\n";
    ptr +="<a class=\"button button-off\" href=\"/\">3</a>\n";
    ptr +="<a class=\"button button-off\" href=\"/\">A</a><br />\n";
    ptr +="<a class=\"button button-off\" href=\"/\">4</a>\n";
    ptr +="<a class=\"button button-off\" href=\"/\">5</a>\n";
    ptr +="<a class=\"button button-off\" href=\"/\">6</a>\n";
    ptr +="<a class=\"button button-off\" href=\"/\">B</a><br />\n";
    ptr +="<a class=\"button button-off\" href=\"/\">7</a>\n";
    ptr +="<a class=\"button button-off\" href=\"/\">8</a>\n";
    ptr +="<a class=\"button button-off\" href=\"/\">9</a>\n";
    ptr +="<a class=\"button button-off\" href=\"/\">C</a><br />\n";
    ptr +="<a class=\"button button-off\" href=\"/\">*</a>\n";
    ptr +="<a class=\"button button-off\" href=\"/\">0</a>\n";
    ptr +="<a class=\"button button-off\" href=\"/\">#</a>\n";
    ptr +="<a class=\"button button-off\" href=\"/\">D</a>\n";
    ptr +="<img src=\"photo.jpg\" alt=\"Suspected's photo\">\n";
  }
  else {
    ptr +="<p>Status diody: " + diodeStatusStr + "</p>\n";
    ptr += "<p>Password: " + passwd + "</p>";
    ptr +="<a class=\"button button-on\" href=\"/keypad?key=1\">1</a>\n";
    ptr +="<a class=\"button button-on\" href=\"/keypad?key=2\">2</a>\n";
    ptr +="<a class=\"button button-on\" href=\"/keypad?key=3\">3</a>\n";
    ptr +="<a class=\"button button-on\" href=\"/keypad?key=A\">A</a><br />";
    ptr +="<a class=\"button button-on\" href=\"/keypad?key=4\">4</a>\n";
    ptr +="<a class=\"button button-on\" href=\"/keypad?key=5\">5</a>\n";
    ptr +="<a class=\"button button-on\" href=\"/keypad?key=6\">6</a>\n";
    ptr +="<a class=\"button button-on\" href=\"/keypad?key=B\">B</a><br />\n";
    ptr +="<a class=\"button button-on\" href=\"/keypad?key=7\">7</a>\n";
    ptr +="<a class=\"button button-on\" href=\"/keypad?key=8\">8</a>\n";
    ptr +="<a class=\"button button-on\" href=\"/keypad?key=9\">9</a>\n";
    ptr +="<a class=\"button button-on\" href=\"/keypad?key=C\">C</a><br />";
    ptr +="<a class=\"button button-on\" href=\"/keypad?key=star\">*</a>\n";
    ptr +="<a class=\"button button-on\" href=\"/keypad?key=0\">0</a>\n";
    ptr +="<a class=\"button button-on\" href=\"/check\">#</a>\n";
    ptr +="<a class=\"button button-on\" href=\"/keypad?key=D\">D</a>\n";
  }
  return ptr;
}



void handleRoot() {
  server.send(200, "text/html", SendHTML());

}

void handleNotFound(){
  String message = "Brak pliku\n\n";
  server.send(404, "text/plain", message);

}

void turn_diodeOn() {
  Serial.println("Dioda zał");
  server.send(200, "text/html", SendHTML()); 
  digitalWrite(DIODA,LOW);

}

void turn_diodeOff() {
  Serial.println("Dioda wył");
  server.send(200, "text/html", SendHTML()); 
    digitalWrite(DIODA,HIGH);
}

void setPasswd() {
  passwd += server.arg(0);
  server.send(200, "text/html", SendHTML()); 
}

void checkPasswd() {
  if(passwd.equals(KEYPASSWD) && attemptsNumber<allowAttempts) {
    passwd = "PASSWORD OK";
    attemptsNumber = 0;
    diodeStatusStr = "switch on for 2 seconds";
    turn_diodeOn();
    delay(2000);
    passwd = "";
  } else if(attemptsNumber<allowAttempts) {
    attemptsNumber++;
    String attemptsLeft = String(allowAttempts - attemptsNumber);    
    if(attemptsNumber<allowAttempts) {
    passwd = "WRONG PASSWORD. " + attemptsLeft + " ATTEMPT(S) LEFT";
    turn_diodeOff();
    delay(2000);
    passwd = "";
    } else {
      passwdBlocked = HIGH;
      takeNewPhoto = true;
      passwd = "SYSTEM BLOCKED. TO MANY WRONG ATTEMPTS";
    };
  } 
  diodeStatusStr = "off";
  turn_diodeOff();
}

void setup(void){
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");


  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Połączono do ");
  Serial.println(ssid);
  Serial.print("Adres IP: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  }
  else {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }


  // OV2640 camera module
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;
 
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }
 
// Routing
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.on(F("/keypad"), HTTP_GET, setPasswd);
  server.on("/check", checkPasswd);
  server.serveStatic("/photo.jpg", SPIFFS, "/photo.jpg");

  server.begin();
  Serial.println("Start serwera HTTP");
  pinMode(DIODA,OUTPUT);
  digitalWrite(DIODA,HIGH);
}

// Check if photo capture was successful
bool checkPhoto( fs::FS &fs ) {
  File f_pic = fs.open("photo.jpg");
  unsigned int pic_sz = f_pic.size();
  return ( pic_sz > 100 );
}

// Capture Photo and Save it to SPIFFS
void capturePhotoSaveSpiffs( void ) {
  camera_fb_t * fb = NULL; // pointer
  bool ok = 0; // Boolean indicating if the picture has been taken correctly
 
  do {
    // Take a photo with the camera
    Serial.println("Taking a photo...");
 
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }
 
    // Photo file name
    Serial.printf("Picture file name: %s\n", "photo.jpg");
    File file = SPIFFS.open("photo.jpg", FILE_WRITE);
 
    // Insert the data in the photo file
    if (!file) {
      Serial.println("Failed to open file in writing mode");
    }
    else {
      file.write(fb->buf, fb->len); // payload (image), payload length
      Serial.print("The picture has been saved in ");
      Serial.print("photo.jpg");
      Serial.print(" - Size: ");
      Serial.print(file.size());
      Serial.println(" bytes");
    }
    // Close the file
    file.close();
    esp_camera_fb_return(fb);
 
    // check if file has been correctly saved in SPIFFS
    ok = checkPhoto(SPIFFS);
  } while (!ok);
}


void loop(void){
  server.handleClient();
  if(takeNewPhoto) {
    capturePhotoSaveSpiffs();
    takeNewPhoto = false;
  }

}