//to be included in project @ online IDE
#include "HttpClient.h"
//#include "math.h"



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


unsigned long previousMillis = 0; //per il timing del ventolino valve2
char config[7] = {0, 0, 0, 0, 0, 0};//per qualche cazzo di motivo se dichiaro queste variabili una per una e  fuori da setup e loop (quindi come global variables) il http non funziona e ritorna l'html invece del json
//sono: ll1, ll2, ll3, v1, v2, automode   ...piu' un carattere per il /null
ApplicationWatchdog wd(300000, System.reset);  //watchdog che riavvia il sistema se la connessione richiede piu' di 5 minuti

void setup() {
    setupWiFi();
    request.hostname = "things.ubidots.com";
    request.port = 80;
    
//Serial.begin(9600);
}

void loop() {

unsigned char commfaultll1 = 0;
unsigned char commfaultll2 = 0;
unsigned char commfaultll3 = 0;
unsigned char commfaultv1 = 0;
unsigned char commfaultv2 = 0;
unsigned char hour = Time.hour();

int index_on = 0;
int index_off = 0;

int controlvalue = 0;
int temperatura = 0;

if (!WiFi.ready()){
    setupWiFi(); 
    }
    
wd.checkin(); // resets the AWDT count

    //check come deve essere la luce1 
    request.path = "/api/v1.6/variables/"light1"/values";
    http.get(request, response, headers);
//Serial.println("Luce 1");
//Serial.println(response.status);
//Serial.println(response.body);
    index_on = response.body.indexOf(search_on);
    index_off = response.body.indexOf(search_off);
    if (index_on == -1 || index_off == -1 || index_on == index_off || response.status == -1) {
    commfaultll1 = 1;
    }
    else if (index_on < index_off) { //light on
    config[0]=1;
    }
    else if (index_on > index_off) { //light off
    config[0]=0;
    }
//Serial.println("ll1 indexon="+ String(index_on)+"  ll1 indexoff=" + String(index_off));
    index_on = 0;
    index_off = 0;

    //check come deve essere la luce2 
    request.path = "/api/v1.6/variables/"light2"/values";
    http.get(request, response, headers);
    // String search_on = String("\"value\": 1.0");
    // String search_off = String("\"value\": 0.0");
    index_on = response.body.indexOf(search_on);
    index_off = response.body.indexOf(search_off);
//Serial.println("Luce 2");
//Serial.println(response.status);
//Serial.println(response.body);
    if (index_on == -1 || index_off == -1 || index_on == index_off || response.status == -1) {
    commfaultll2 = 1;
    }
    else if (index_on < index_off) { //light on
    config[1]=1;
    }
    else if (index_on > index_off) { //light off
    config[1]=0;
    }
//Serial.println("ll2 indexon="+ String(index_on)+"  ll2 indexoff=" + String(index_off));
    index_on = 0;
    index_off = 0;
    
    
    //check come deve essere la luce3
    request.path = "/api/v1.6/variables/"light3"/values";
    http.get(request, response, headers);
    // String search_on = String("\"value\": 1.0");
    // String search_off = String("\"value\": 0.0");
    index_on = response.body.indexOf(search_on);
    index_off = response.body.indexOf(search_off);
//Serial.println("Luce 3");
//Serial.println(response.status);
//Serial.println(response.body);
    if (index_on == -1 || index_off == -1 || index_on == index_off || response.status == -1) {
    commfaultll3 = 1; Serial.println("primo if");
    }
    else if (index_on < index_off) { //light on
    config[2]=1; Serial.println("primo else if");
    }
    else if (index_on > index_off) { //light off
    config[2]=0; Serial.println("secondo else if");
    }
//Serial.println("ll3 indexon="+ String(index_on)+"  ll3 indexoff=" + String(index_off));
    index_on = 0;
    index_off = 0;

    //check se irrigare valvola1 
    request.path = "/api/v1.6/variables/"valve1"/values";
    http.get(request, response, headers);
//Serial.println("valve_1");
//Serial.println(response.status);
//Serial.println(response.body);
    //String search_on = String("\"value\": 1.0");
    //String search_off = String("\"value\": 0.0");
    index_on = response.body.indexOf(search_on);
    index_off = response.body.indexOf(search_off);
    
    if (index_on == -1 || index_off == -1 || index_on == index_off || response.status == -1) {
    commfaultv1 = 1;
    }
    else if (index_on < index_off) { //irrigation to start
    config[3]=1;
    }
    else if (index_on > index_off) { //no irrigation
    config[3]=0;
    }
//Serial.println("v1 indexon="+ String(index_on)+"  v1 indexoff=" + String(index_off));
    index_on = 0;
    index_off = 0;
    
    //check se irrigare valvola2
    request.path = "/api/v1.6/variables/"valve2"/values";
    http.get(request, response, headers);
//Serial.println("valve_2");
//Serial.println(response.status);
//Serial.println(response.body);

    // String search_on = String("\"value\": 1.0");
    // String search_off = String("\"value\": 0.0");
    index_on = response.body.indexOf(search_on);
    index_off = response.body.indexOf(search_off);
    
    if (index_on == -1 || index_off == -1 || index_on == index_off || response.status == -1) {
    commfaultv2 = 1;;
    }
    else if (index_on < index_off) { //irrigation to start
    config[4]=1;
    }
    else if (index_on > index_off) { //no irrigation
    config[4]=0;
    }
//Serial.println("v2 indexon="+ String(index_on)+"  v2 indexoff=" + String(index_off));
    index_on = 0;
    index_off = 0;
    
//Serial.println("appena scaricati: valore v1="+ String(config[3])+"  valore v2=" + String(config[4]));
    
    //scarico la temperatura....se e' troppo alta spengo le luci
    request.path = "/api/v1.6/variables/"temp"/values";
    http.get(request, response, headers);
    // Serial.println("Temperatura");
    // Serial.println(response.status);
    // Serial.println(response.body);

    if (response.status != -1) {
    String search = String("\"value\": ");
    controlvalue = response.body.indexOf(search);
    temperatura = response.body.substring(controlvalue + 9, controlvalue + 9 + 3).toInt();//substring prende l'indice di quando inizia "\"value\" etc...
    }
    // Serial.print("Temperatura: ");
    // Serial.println(temperatura);

//Serial.println("Prima di azionare");
//Serial.println("ll1= " + String(config[0]) + " ll2=" + String(config[1]) + " ll3=" + String(config[2]) + " hour=" + String(hour) + " automode=" + String(automode) + " v1=" + String(config[3])+ " v2=" + String(config[4]));


//**********************************
//**************AZIONE**************
//**********************************
    
    //condizioni delle booleane sul tempo
    if ((hour>=22||hour<=6) && config[5]==0){ //In daylight saving time devo togliere uno
    config[0]=1;
    config[1]=1;
    config[2]=1;
    config[5] = 1; //e' l'automode
    commfaultll1 = 0; //in questo modo il comando automatico (che parte solo una volta al giorno) non e' stoppato da problemi di comunicazione
    commfaultll2 = 0;
    commfaultll3 = 0;
    controlstuff(light1, 1);
    controlstuff(light2, 1);
    controlstuff(light3, 1);
    }
    if (hour<22 && hour>6 && config[5]==1){
    config[0]=0;
    config[1]=0;
    config[2]=0;
    config[5] = 0;
    commfaultll1 = 0; //in questo modo il comando automatico (che parte solo una volta al giorno) non e' stoppato da problemi di comunicazione
    commfaultll2 = 0;
    commfaultll3 = 0;
    controlstuff(light1, 0);
    controlstuff(light2, 0);
    controlstuff(light3, 0);
    }
    
    //action part: align real light status to the one online and start irrigation procedure
    if (temperatura < 30){  
    //controllo luce 1
    if (commfaultll1 == 0){
    if (config[0]==1 && digitalRead(A5)==LOW){digitalWrite(A5, HIGH); Particle.publish("Luce 1 Accesa!");}
    else if (config[0]==0 && digitalRead(A5)==HIGH){digitalWrite(A5, LOW); Particle.publish("Luce 1 Spenta!");}
    }
    //controllo luce 2
    if (commfaultll2 == 0){
    if (config[1]==1 && digitalRead(A6)==LOW){digitalWrite(A6, HIGH); Particle.publish("Luce 2 Accesa!");}
    else if (config[1]==0 && digitalRead(A6)==HIGH){digitalWrite(A6, LOW); Particle.publish("Luce 2 Spenta!");}
    }
    //controllo luce 3
    if (commfaultll1 == 0){
    if (config[2]==1 && digitalRead(A3)==LOW){digitalWrite(A3, HIGH); Particle.publish("Luce 3 Accesa!");}
    else if (config[2]==0 && digitalRead(A3)==HIGH){digitalWrite(A3, LOW); Particle.publish("Luce 3 Spenta!");}
    }
    }
    
    else if (temperatura>30){
    if(digitalRead(A5)==HIGH){digitalWrite(A5, LOW);controlstuff(light1, 0);}
    if(digitalRead(A6)==HIGH){digitalWrite(A6, LOW);controlstuff(light2, 0);}
    if(digitalRead(A3)==HIGH){digitalWrite(A3, LOW);controlstuff(light3, 0);}
    Particle.publish("Temperatura troppo alta, luci tutte spente!");
    }
    
    //controllo valvola 1
    if (config[3]==1 && commfaultv1 == 0){
    Particle.publish("Irrigazione 1 iniziata");
    digitalWrite(A2, HIGH);
    delay(60000); //quanto sta aperta la valvola
    digitalWrite(A2, LOW);
    controlstuff(valve1, 0);
    config[3]=0;
    }
    
    //controllo valvola 2 ventolino ...voglio il comando manuale e quello automatico che azioni per un massimo di 3 minuti non blocking
    
    if(((config[4]==1 && commfaultv2 == 0) || (temperatura > 30)) && digitalRead(A1)==LOW) //lo faccio runnare per n max di 30 minuti in caso di comando o di temperatura troppo alta
    {
    digitalWrite(A1, HIGH);  // Turn it on
    Particle.publish("Ventilazione iniziata");
    config[4]=0;
    previousMillis = millis();  // Remember the time the valve went on
    controlstuff(valve2, 1); //questo e' ridoandante, ma in caso di temperatura alta il ciclo partirebbe senza un input e una volta spento metterei uno 0 come dato senza l'1 iniziale e con tutti 0 ho poi problemi con il fetch dei dati
    }
    else if ((digitalRead(A1)==HIGH && temperatura < 29) || (digitalRead(A1)==HIGH && (millis() - previousMillis > 1800000))) //ventolino fino a che temp <30 o per un max di 30 minuti
    {
    digitalWrite(A1, LOW);  // Turn it off
    previousMillis = 0;
    controlstuff(valve2, 0);
    }
    else if (config[4]==0 && commfaultv2 == 0 && digitalRead(A1)==HIGH)  //in caso l'utente spenga il ventolino
    {
    digitalWrite(A1, LOW);  // Turn it off
    previousMillis = 0;
    controlstuff(valve2, 0);    
    }
    
   // Particle.publish("Ventilazione finita", String(millis() - previousMillis));
//Serial.print("ll1=");
//Serial.println(config[0]);
//Serial.print("ll2=");
//Serial.println(config[1]);
//Serial.print("ll3=");
//Serial.println(config[2]);

//Serial.println("Fine ciclo!");
//Serial.println("ll1= " + String(config[0]) + " ll2=" + String(config[1]) + " ll3=" + String(config[2]) + " hour=" + String(hour) + " automode=" + String(automode) + " v1=" + String(config[3])+ " v2=" + String(config[4]));
//Serial.println("Time: " + Time.format(TIME_FORMAT_ISO8601_FULL)); //i ltempo e' automaticamente giusto come fuso orario
//Serial.println();    
    delay(25000);
}

//http://things.ubidots.com/api/v1.6/variables/5a6870b8c03f971f74abb674/values/?token=RGlKUcqwMXi77GzBEKz2vBhAu30Uvz
