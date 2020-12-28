#include <WiFiClient.h>
#include "ESP32WebServer.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Arduino.h>

#define DIODA 33

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

 
// Routing
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.on(F("/keypad"), HTTP_GET, setPasswd);
  server.on("/check", checkPasswd);

  server.begin();
  Serial.println("Start serwera HTTP");
  pinMode(DIODA,OUTPUT);
  digitalWrite(DIODA,HIGH);
}




void loop(void){
  server.handleClient();
}