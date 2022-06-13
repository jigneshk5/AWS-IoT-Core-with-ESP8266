#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Arduino_JSON.h>
#include "DHT.h"

#define DHTTYPE DHT11   // DHT 11

const int DHTPin = 5;   //D1
const int red = D2;
const int green = D3;
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

const char* ssid = "Enter Wifi Name";
const char* password = "Enter Wifi Password";

// Find this awsEndpoint in the AWS Console: Manage - Things, choose your thing
// choose Interact, its the HTTPS Rest endpoint 
const char* awsEndpoint = "a3r2gm16zr92x7-ats.iot.us-east-2.amazonaws.com";  //update with your Broker Url

// For the two certificate strings below paste in the text of your AWS 
// device certificate and private key:

// xxxxxxxxxx-certificate.pem.crt
static const char certificatePemCrt[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDWjCCAkKgAwIBAgIVAPBUUAiLaPv4qdRd0nr4se6YyTuZMA0GCSqGSIb3DQEB
CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t
IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yMTA5MDIxODM0
NDFaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh
dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQD5NZpZNrM2NywutOHF
Jmy0N22NFDRg8sjiw1X63pF0gzfPwy9/PZzYOXosQNm8JCQWvskgouFhtD88wQcE
w3IM5LFERpvn6+PWwT2oAyZhgli2BgWJw36InI/wTpVGkdwt4GAWFpRDJyOySbyx
ccY3A63EqGH84gzC3yD3laPHvle8Fd4eqS5OEal/K3sSiZFx0P3Zwh3abwUiBade
C3zFfkRMXn4XVSMrFSRIPuif80n4waaX/5ekkeqoJRgM17gkGongEws2zs+k8Q5C
J1aPJaymJo2SSMJD832QwrGaMgo8OOd7catt0SvWgVlSBwwkkEL67k8uIDV+N0Dq
TlVpAgMBAAGjYDBeMB8GA1UdIwQYMBaAFMxJAL2PlZYPz9TmYj7HIkUUJdlcMB0G
A1UdDgQWBBS8DCgQgO6Er0dTw6gk66xoOx8M2jAMBgNVHRMBAf8EAjAAMA4GA1Ud
DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAZE6nSnLOJVAnT/f/Cc6/kSyh
vCA6gGOC+FvSmRX3Q3Injazg2JBdfBWoc5mFmm3qe6aa4eVk8i+QsOCJfCxg6tgk
Ol8x4icEiZ9slQ8l+bIU1bHPEfMeEic8+1b+p/38PUc3YTmIctzaBWnCNH+mhaYc
eVI42cIU/OB7KfwuJ7J25jPT+Wjy2LJjMvARVbopxa1uel81ygon3BXx3z2mTXJi
X9srbC3CyRmV6LrhpbnY5Yht1uuguoO0SaTmS4CJVS4BVNNdgOa71zr7J2D0Eegv
n8VDvbT71mDhRKzxXEDbNysaIjMkXbis3pU6K7XQw8KLaWQsGMXwbrjHNttRsQ==
-----END CERTIFICATE-----
)EOF";

// xxxxxxxxxx-private.pem.key
static const char privatePemKey[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEA+TWaWTazNjcsLrThxSZstDdtjRQ0YPLI4sNV+t6RdIM3z8Mv
fz2c2Dl6LEDZvCQkFr7JIKLhYbQ/PMEHBMNyDOSxREab5+vj1sE9qAMmYYJYtgYF
icN+iJyP8E6VRpHcLeBgFhaUQycjskm8sXHGNwOtxKhh/OIMwt8g95Wjx75XvBXe
HqkuThGpfyt7EomRcdD92cId2m8FIgWnXgt8xX5ETF5+F1UjKxUkSD7on/NJ+MGm
l/+XpJHqqCUYDNe4JBqJ4BMLNs7PpPEOQidWjyWspiaNkkjCQ/N9kMKxmjIKPDjn
e3GrbdEr1oFZUgcMJJBC+u5PLiA1fjdA6k5VaQIDAQABAoIBAQDXBjQmU/cviU1b
A4FmHXBf0w94Uf2bxQAmAMsYCaBResLWRiCeVigVvQo4UsBMAJcO9REPUtrK9b42
ioqmxoiKrWxyb8YnxzYwX032wLkGG3S1od53brGn6AaeHv/YAxSKlPXzOQo2Ivg+
5jakmHmhkFAHxg2+J8FBB0sntPq65UJnwu7AjZWFgPloa2xgz147hbOpT+rsNJpJ
Lm7oRTF9vnRQmY59wHUQBG4jdjhWpet3Sy9e/6hLTSR2czYKFez79mHnujQ27Ohq
EP8rLeBqcF1rpb7ttn4Run5QXcsTC3To3ramKVe0Fcswu2Dqxl3Jhtr0G7y/wx82
ChMnG6jxAoGBAP1Mi63yqQZCx1S+ZBgwOIOjHfwzqxuknZQrkbk5xz2Gog+Fmvb3
jZWuEs8GUx1bTDbYMZUVeYt0979w+IIAe4NAWvzSex53a2EmwK0MiXTzk4qHm225
QoiRCrL3KOmFvUzeEZ9e8g5JWF8PUXfw0yz12EuwbayaHY+vJCL0enk1AoGBAPvd
5Lrg+Dztfpts7TroYlx7UmwTBoPUAWQm1PT6pcOhY/xjymGxGTvAgMXQPJsG34mb
l1mDppjOQ+m6DqU+qYIjTK2gOFw4b7LlBcu2ESVUKgfhKtd4HhbbQf7MoYMU20Wp
lBPCXvI07jiJbJ52XIvVIsdbbAvFJQuoIDV29mXlAoGACQ4+hCaDwnSKuc3guDip
HJOtU8HpISDefVidahjVta5lhtaI9sFc73f8tigMuqTz8ZYDF8hfwwuYe+CnEcbA
o62eUyGXQzyTuzJuewFSvLqkkLROazrPVSCSWFmbvdWJMgFScfcsAjS/Ew7yI66Z
bicP54zULvKHkgZ57UH+Su0CgYAk/nCMKmMLv+5qsdMeZeecodh3W0wHrOoPZcy9
ttsGIvxmo9mkA7krF5lSdMWSF65ZrerLERU1OIatG9Du4IdRQBIRJolgskFly1HQ
nAtkqjoKMj5yq4fv34CQBkpq0HNshf3j8Ra1CKU81KKJm+T3/PRAx+LUz74jOiyQ
7co/KQKBgH7XSFKhJ9L01XV8cz9hpLflI1iVZklrC7ipizKJ+X/wbJ3RMYjk0/Pp
YUTFRJwEFuOR6TFm4fjWYGbsHiieqoz+6c0M6UM8O6CxdPPfrNbnnhMHmG+XqRnl
csgSETEVmuOpHFcfZjEZysdONxfKDgp+HrzN9UbXBNik9DmPrm9r
-----END RSA PRIVATE KEY-----
)EOF";

// This is the AWS IoT CA Certificate from: 
// https://docs.aws.amazon.com/iot/latest/developerguide/managing-device-certs.html#server-authentication
// This one in here is the 'RSA 2048 bit key: Amazon Root CA 1' which is valid 
// until January 16, 2038 so unless it gets revoked you can leave this as is:
static const char caPemCrt[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

BearSSL::X509List client_crt(certificatePemCrt);
BearSSL::PrivateKey client_key(privatePemKey);
BearSSL::X509List rootCert(caPemCrt);

WiFiClientSecure wiFiClient;
void msgReceived(char* topic, byte* payload, unsigned int len);
PubSubClient pubSubClient(awsEndpoint, 8883, msgReceived, wiFiClient); 


void setup() {
  Serial.begin(115200); Serial.println();
  Serial.println("ESP8266 AWS IoT Example");

  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  WiFi.waitForConnectResult();
  Serial.print(", WiFi connected, IP address: "); Serial.println(WiFi.localIP());
  
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  dht.begin();
  // get current time, otherwise certificates are flagged as expired
  setCurrentTime();

  wiFiClient.setClientRSACert(&client_crt, &client_key);
  wiFiClient.setTrustAnchors(&rootCert);
}

unsigned long lastPublish;
int msgCount;

void loop() {

  pubSubCheckConnect();

  if (millis() - lastPublish > 10000) {
    
    float h = dht.readHumidity();
    float temp = dht.readTemperature();
    if (isnan(h) || isnan(temp)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    Serial.println("Temp: " + String(temp) + " , Humidity: " + String(h));
    String str= "{\"temp\" : "+String(temp)+" , \"humidity\": "+String(h)+"}";
    pubSubClient.publish("ted/telemetry", str.c_str ());
    
    lastPublish = millis();
  }
}

void msgReceived(char* topic, byte* payload, unsigned int length) {

    Serial.print("Message received on "); Serial.print(topic); Serial.print(": ");
    String msgIN = "";
    for (int i = 0; i < length; i++) {
       msgIN += (char)payload[i];
    }
    String msgString = msgIN;
    Serial.println(msgString);
    JSONVar myObject = JSON.parse(msgString);

  if(String(topic) == "$aws/things/zen/shadow/get/accepted"){
  
      String redstr= (const char*) myObject["state"]["desired"]["red"];
      String greenstr= (const char*) myObject["state"]["desired"]["green"];

       int redstr1 = (int) myObject["state"]["reported"]["red"];
      int greenstr1 = (int) myObject["state"]["reported"]["green"];
      
      Serial.println("Red: "+redstr+" ,Red1: "+redstr1);
      Serial.println("Green: "+greenstr+" ,Green1: "+greenstr1);
      
      if(redstr.toInt()!=redstr1 || greenstr.toInt()!=greenstr1){
        if (redstr == "1") {
          digitalWrite(red, HIGH);              
        } else {
          digitalWrite(red, LOW);
        }
        if (greenstr == "1") {
          digitalWrite(green, HIGH);              
        } else {
          digitalWrite(green, LOW);
        }
        String str = "{ \"state\": { \"reported\": { \"red\":"+redstr+", \"green\":"+greenstr+"} } }";
        pubSubClient.publish("$aws/things/zen/shadow/update",str.c_str ());
      }
  }
  if(String(topic) == "$aws/things/zen/shadow/update/delta"){
    pubSubClient.publish("$aws/things/zen/shadow/get","");
  }
  Serial.println();
}

void pubSubCheckConnect() {
  if ( ! pubSubClient.connected()) {
    Serial.print("PubSubClient connecting to: "); Serial.print(awsEndpoint);
    while ( ! pubSubClient.connected()) {
      Serial.print(".");
      pubSubClient.connect("zen");
    }
    Serial.println(" connected");
    pubSubClient.subscribe("$aws/things/zen/shadow/update/delta");
    pubSubClient.subscribe("$aws/things/zen/shadow/get/accepted");
    pubSubClient.publish("$aws/things/zen/shadow/get","");
  }
  pubSubClient.loop();
}

void setCurrentTime() {
  configTime(5.5 * 3600, 0, "pool.ntp.org", "time.nist.gov");  //UTC

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: "); Serial.print(asctime(&timeinfo));
}
