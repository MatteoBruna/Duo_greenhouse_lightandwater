// // This #include statement was automatically added by the Particle IDE.
// #include <HttpClient.h>

//to be included in project @ online IDE
#include "HttpClient/HttpClient.h"
#include "math.h"


class RunBeforeSetup {  //altrimenti al reset alcune pin vanno alti e le valvole si aprono
public:
	RunBeforeSetup() {
    pinMode(A5, OUTPUT); // light1 (ma mando il segnale da D7)
    digitalWrite(A5, LOW);
    pinMode(A6, OUTPUT); // light2
    digitalWrite(A6, LOW);
    pinMode(A3, OUTPUT); // light3
    digitalWrite(A3, LOW);
    pinMode(A2, OUTPUT); // valve1
    digitalWrite(A2, LOW);
    pinMode(A1, OUTPUT); // valve2
    digitalWrite(A1, LOW);
	}
};
RunBeforeSetup RunBeforeSetup;

void setupWiFi() {
  WiFi.on();
  WiFi.disconnect();
  WiFi.clearCredentials();
  WiFi.setCredentials("ssid1", "pass1", WPA2, WLAN_CIPHER_AES);
  WiFi.setCredentials("ssid20", "pass2", WPA2, WLAN_CIPHER_AES);
  WiFi.connect();
  waitUntil(WiFi.ready);
  Particle.connect();
}

STARTUP(WiFi.selectAntenna(ANT_AUTO)); //AUTO will select best antenna or ANT_INTERNAL or ANT_EXTERNAL to select manually
SYSTEM_MODE(SEMI_AUTOMATIC);

HttpClient http;

#ifndef TOKEN
#define TOKEN ""  // Add here your Ubidots TOKEN
#endif
#define light1 ""   //queste le creo sul sito ubidots aggiungendo una variable, poi copio l'ID; il token e' del dispositivo
#define light2 ""
#define light3 ""
#define valve1 ""
#define valve2 ""
#define temp ""

// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
      { "Content-Type", "application/json" },
      { "X-Auth-Token" , TOKEN },
        { NULL, NULL } // NOTE: Always terminate headers will NULL
};

http_request_t request;
http_response_t response;

//funzione per accendere o spegnere qualcosa
void controlstuff(const String& what, int val) {
    request.path = "/api/v1.6/variables/"+ what +"/values";
    request.body = "{\"value\":" + String(val) + "}";
    http.post(request, response, headers);
}

bool ll1=0;//queste sono quelle diretamente collegate al controllo online
bool ll2=0;
bool ll3=0;
bool l1=0;
bool l2=0;
bool l3=0;
bool v1=0;
bool v2=0;
    
String search_on = String("\"value\": 1.0");
String search_off = String("\"value\": 0.0");

int index_on;
int index_off;


void setup() {
    setupWiFi();
    request.hostname = "things.ubidots.com";
    request.port = 80;
    //Serial.begin(9600);
    //variabili booleane che mi dicono se luci e valvole deveono essere 0 o 1
}


void loop() {
 
 //prima dovrei capire se sono connesso...altrimeti tutto qusto lo salto....dopo un certo numero di cicli in cui non sono connesso faccio reset
if (!WiFi.ready()){
 setupWiFi();  
}

//check come deve essere la luce1 
request.path = "/api/v1.6/variables/"light1"/values";
http.get(request, response, headers);
// String search_on = String("\"value\": 1.0");
// String search_off = String("\"value\": 0.0");
index_on = response.body.indexOf(search_on);
index_off = response.body.indexOf(search_off);

if (index_on == -1 && index_off == -1) {
    delay(0);
}
else if (index_on < index_off) { //light on
 ll1=1;
}
else if (index_on > index_off) { //light off
 ll1=0;
}

//check come deve essere la luce2 
request.path = "/api/v1.6/variables/"light2"/values";
http.get(request, response, headers);
// String search_on = String("\"value\": 1.0");
// String search_off = String("\"value\": 0.0");
index_on = response.body.indexOf(search_on);
index_off = response.body.indexOf(search_off);

if (index_on == -1 && index_off == -1) {
    delay(0);
}
else if (index_on < index_off) { //light on
 ll2=1;
}
else if (index_on > index_off) { //light off
 ll2=0;
}


//check come deve essere la luce3
request.path = "/api/v1.6/variables/"light3"/values";
http.get(request, response, headers);
// String search_on = String("\"value\": 1.0");
// String search_off = String("\"value\": 0.0");
index_on = response.body.indexOf(search_on);
index_off = response.body.indexOf(search_off);

if (index_on == -1 && index_off == -1) {
    delay(0);
}
else if (index_on < index_off) { //light on
 ll3=1;
}
else if (index_on > index_off) { //light off
 ll3=0;
}

//check se irrigare valvola1 
request.path = "/api/v1.6/variables/"valve1"/values";
http.get(request, response, headers);
// String search_on = String("\"value\": 1.0");
// String search_off = String("\"value\": 0.0");
index_on = response.body.indexOf(search_on);
index_off = response.body.indexOf(search_off);

if (index_on == -1 && index_off == -1) {
    delay(0);
}
else if (index_on < index_off) { //irrigation to start
 v1=1;
}
else if (index_on > index_off) { //no irrigation
 v1=0;
}

//check se irrigare valvola2
request.path = "/api/v1.6/variables/"valve2"/values";
http.get(request, response, headers);
// String search_on = String("\"value\": 1.0");
// String search_off = String("\"value\": 0.0");
index_on = response.body.indexOf(search_on);
index_off = response.body.indexOf(search_off);

if (index_on == -1 && index_off == -1) {
    delay(0);
}
else if (index_on < index_off) { //irrigation to start
 v2=1;
}
else if (index_on > index_off) { //no irrigation
 v2=0;
}
//Particle.publish("appena scaricati: valore v1: "+ String(v1)+"valore v2: " + String(v2));

//scarico la temperatura....se e' troppo alta spengo le luci
request.path = "/api/v1.6/variables/"temp"/values";
http.get(request, response, headers);
// Serial.println(response.status);
// Serial.println(response.body);
String search = String("\"value\": ");
int controlvalue = response.body.indexOf(search);
int temperatura = response.body.substring(controlvalue + 9, controlvalue + 9 + 3).toInt();//substring prende l'indice di quando inizia "\"value\" etc...
// Serial.print("Temperatura: ");
// Serial.println(valore);



//condizioni delle booleane sul tempo...MA IL COMANDO MAUNALE DEVE FARE OVERRIDE DI QUELLO AUTOMATICO!!!!
if (Time.hour()>=23||Time.hour()<=8){
    l1=1;
    l2=1;
   // l3=1;
}
if (Time.hour()<23 && Time.hour()>8){
    l1=0;
    l2=0;
   // l3=0;
}

//action part: align real light status to the one online and start irrigation procedure
if (temperatura < 30){
    //controllo luce 1
    if (ll1==1){
        if(digitalRead(A5)==LOW){digitalWrite(A5, HIGH); Particle.publish("Luce 1 accesa!");}
        else if (l1==0 && digitalRead(A5)==HIGH){digitalWrite(A5, LOW);controlstuff(light1, 0); Particle.publish("Luce 1 spenta!");
    }
    else if(ll1==0){
        if (l1==1 && digitalRead(A5)==LOW){digitalWrite(A5, HIGH); controlstuff(light1, 1); Particle.publish("Luce 1 accesa!");}
        else if (l1==0 && digitalRead(A5)==HIGH){digitalWrite(A5, LOW);controlstuff(light1, 0); Particle.publish("Luce 1 spenta!");}
        else if (l1==1 && digitalRead(A5)==HIGH){digitalWrite(A5, LOW);controlstuff(light1, 0); Particle.publish("Luce 1 spenta!");}
    }
    
    
    //controllo luce 2
    if (ll2==1){
        if(digitalRead(A6)==LOW){digitalWrite(A6, HIGH); Particle.publish("Luce 2 accesa!");}
    }
    else if(ll2==0){
        if (l2==1 && digitalRead(A6)==LOW){digitalWrite(A6, HIGH); controlstuff(light2, 1); Particle.publish("Luce 2 accesa!");}
        else if (l2==0 && digitalRead(A6)==HIGH){digitalWrite(A6, LOW);controlstuff(light2, 0); Particle.publish("Luce 2 spenta!");}
        else if (l2==1 && digitalRead(A6)==HIGH){digitalWrite(A6, LOW);controlstuff(light2, 0); Particle.publish("Luce 1 spenta!");}
    }
    
    
    //controllo luce 3
    if (ll3==1){
        if(digitalRead(A3)==LOW){digitalWrite(A3, HIGH); Particle.publish("Luce 3 accesa!");}
    }
    else if(ll3==0){
        if (l3==1 && digitalRead(A3)==LOW){digitalWrite(A3, HIGH); controlstuff(light3, 1); Particle.publish("Luce 3 accesa!");}
        else if (l3==0 && digitalRead(A3)==HIGH){digitalWrite(A3, LOW);controlstuff(light3, 0); Particle.publish("Luce 3 spenta!");}
        else if (l3==1 && digitalRead(A3)==HIGH){digitalWrite(A3, LOW);controlstuff(light3, 0); Particle.publish("Luce 3 spenta!");}
    }
}
else if (temperatura>30){
    digitalWrite(A5, LOW);controlstuff(light1, 0);
    digitalWrite(A6, LOW);controlstuff(light2, 0);
    digitalWrite(A3, LOW);controlstuff(light3, 0);
    Particle.publish("Temperatura troppo alta, luci tutte spente!");
}

//controllo valvola 1
if (v1==1){
    Particle.publish("Irrigazione 1 iniziata");
    digitalWrite(A2, HIGH);
    delay(90000); //quanto sta aperta la valvola
    digitalWrite(A2, LOW);
    controlstuff(valve1, 0);
    v1=0;
}

//controllo valvola 2
if (v2==1){
    Particle.publish("Irrigazione 2 iniziata");
    digitalWrite(A1, HIGH);
    delay(90000);
    digitalWrite(A1, LOW);
    controlstuff(valve2, 0);
    v2=0;
}

delay(15000);
    
}
