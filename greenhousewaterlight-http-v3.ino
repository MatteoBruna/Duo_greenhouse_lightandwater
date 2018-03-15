//to be included in project @ online IDE
#include "HttpClient.h"
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
  WiFi.setCredentials("", "", WPA2, WLAN_CIPHER_AES);
  WiFi.setCredentials("", "", WPA2, WLAN_CIPHER_AES);
  WiFi.connect();
  waitUntil(WiFi.ready);
  Particle.connect();
}

STARTUP(WiFi.selectAntenna(ANT_AUTO)); //AUTO will select best antenna or ANT_INTERNAL or ANT_EXTERNAL to select manually
//SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

HttpClient http;

#define TOKEN ""
#define light1 ""
#define light2 ""
#define light3 ""
#define valve1 ""
#define valve2 ""
#define temp ""
#define search_on "\"value\": 1.0"
#define search_off "\"value\": 0.0"

http_header_t headers[] = {
    {"Content-Type", "application/json"},
    {"X-Auth-Token" , TOKEN},
    {NULL, NULL}
};

http_request_t request;
http_response_t response;

void controlstuff(const String& what, int val) {
    request.path = "/api/v1.6/variables/"+ what +"/values";
    request.body = "{\"value\":" + String(val) + "}";
    http.post(request, response, headers);
}


bool automode=0;
ApplicationWatchdog wd(300000, System.reset);  //watchdog che riavvia il sistema se la connessione richiede piu' di 5 minuti

void setup() {
    setupWiFi();
    request.hostname = "things.ubidots.com";
    request.port = 80;
   // Serial.begin(9600);
    
}

void loop() {
    
bool ll1; //per qualche cazzo di motivo se le metto fuori da setup e loop (quindi come global variables) il http non funziona e ritorna l'html invece del json
bool ll2;
bool ll3;
bool v1;
bool v2;


int index_on;
int index_off;

if (!WiFi.ready()){
    setupWiFi(); 
    }
    
wd.checkin(); // resets the AWDT count

    //check come deve essere la luce1 
request.path = "/api/v1.6/variables/"light1"/values";
// http.get(request, response, headers);
// Serial.println(response.status);
// Serial.println(response.body);
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
    // Serial.println("Luce 2");
    // Serial.println(response.status);
    // Serial.println(response.body);
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
    // Serial.println("Temperatura");
    // Serial.println(response.status);
    // Serial.println(response.body);
    String search = String("\"value\": ");
    int controlvalue = response.body.indexOf(search);
    int temperatura = response.body.substring(controlvalue + 9, controlvalue + 9 + 3).toInt();//substring prende l'indice di quando inizia "\"value\" etc...
    // Serial.print("Temperatura: ");
    // Serial.println(temperatura);
    
    
    
    //condizioni delle booleane sul tempo...MA IL COMANDO MAUNALE DEVE FARE OVERRIDE DI QUELLO AUTOMATICO!!!!
    if ((Time.hour()>=23||Time.hour()<=8) && automode==0){
    ll1=1;
    ll2=1;
    // ll3=1;
    automode = 1;
    controlstuff(light1, 1);
    controlstuff(light2, 1);
    // /controlstuff(light3, 1);
    }
    if (Time.hour()<23 && Time.hour()>8 && automode==1){
    ll1=0;
    ll2=0;
    // l3=0;
    automode = 0;
    controlstuff(light1, 0);
    controlstuff(light2, 0);
    //controlstuff(light3, 0);
    }
    
    //action part: align real light status to the one online and start irrigation procedure
    if (temperatura < 30){  
    //controllo luce 1
    if (ll1==1 && digitalRead(A5)==LOW){digitalWrite(A5, HIGH); Particle.publish("Luce 1 Accesa!");}
    else if (ll1==0 && digitalRead(A5)==HIGH){digitalWrite(A5, LOW); Particle.publish("Luce 1 Spenta!");}
    
    //controllo luce 2
    if (ll2==1 && digitalRead(A6)==LOW){digitalWrite(A6, HIGH); Particle.publish("Luce 2 Accesa!");}
    else if (ll2==0 && digitalRead(A6)==HIGH){digitalWrite(A6, LOW); Particle.publish("Luce 2 Spenta!");}
    
    //controllo luce 3
    // if (ll3==1 && digitalRead(A3)==LOW){digitalWrite(A3, HIGH); Particle.publish("Luce 3 Accesa!");}
    // else if (ll3==0 && digitalRead(A3)==HIGH){digitalWrite(A3, LOW); Particle.publish("Luce 3 Spenta!");}
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
    delay(60000); //quanto sta aperta la valvola
    digitalWrite(A2, LOW);
    controlstuff(valve1, 0);
    v1=0;
    }
    
    //controllo valvola 2
    if (v2==1){
    Particle.publish("Irrigazione 2 iniziata");
    digitalWrite(A1, HIGH);
    delay(60000);
    digitalWrite(A1, LOW);
    controlstuff(valve2, 0);
    v2=0;
    }
    

    // Serial.println("Fine ciclo!");
    // Serial.println("ll1=" + String(ll1) + " ll2=" + String(ll2));
    // Serial.println("Time: " + Time.format(TIME_FORMAT_ISO8601_FULL)); //i ltempo e' automaticamente giusto come fuso orario
    delay(25000);
}
