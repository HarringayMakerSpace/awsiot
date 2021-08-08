/* ESP8266 AWS IoT
 *  
 * Simplest possible example (that I could come up with) of using an ESP8266 with AWS IoT.
 * No messing with openssl or spiffs just regular pubsub and certificates in string constants
 * 
 * This is working as at 7th Aug 2021 with the current ESP8266 Arduino core release 3.0.2
 * 
 * Author: Anthony Elder 
 * License: Apache License v2
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "<yourSSID>";
const char* password = "<yourPSWD>";

// Find this awsEndpoint in the AWS Console: Manage - Things, choose your thing
// choose Interact, its the HTTPS Rest endpoint 
const char* awsEndpoint = "xxxxxxxxxx.iot.us-west-2.amazonaws.com";

// For the two certificate strings below paste in the text of your AWS 
// device certificate and private key:

// xxxxxxxxxx-certificate.pem.crt
static const char certificatePemCrt[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDWjCCAkKgAwIBAgIVAPimTITsxQA/FWxfo81vheBh52YVMA0GCSqGSIb3DQEB
CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t
IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yMTA4MDcxMzE4
NDdaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh
dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDGFDfBrbYj9nyKz5dW
sIXsru1nzka6O6iSHzARqmQK57SiNqKsNxd+ReZXfbmAB7BpNd8TxNPNM06LrbbS
Qx4J3JNIsRhBSRUBtWyqtnB6M9CxIT2eJQosKqq4Qis/zSFbIihWuCN2pufpuC9t
J2PKzmkFfibTHFGTl73uTEHKZX0E4AUB9nhzKZee3S+mCgwfytULQw8TRpP2q9/O
1hdJ28od2FIcjXpkPV0zEi6tbW8TIT6JJafoHIpnoh8+X6BEaI/9aeuxAFhDjhCL
ApdRgcATSKj/cqAIZ8Yaw5ti6bADlFitinl4sTPShs1KPv1xnJFc4o0AFW07I+VK
AwzXAgMBAAGjYDBeMB8GA1UdIwQYMBaAFO1GGmLNzzvC2LEr6WAXhsW4OR7QMB0G
A1UdDgQWBBQCb4eeAho1uL7UVNUHWFiN2cWmFjAMBgNVHRMBAf8EAjAAMA4GA1Ud
DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAASC8oiLuPxzENYRZV+Nt1WbZ
XyzajhMJpyuVW1kIjIbvYVUvXWMGXmhFEOe9pPQr89gL/EGxDooM80nTZ2EEO3CL
7wqESX7GNUlbex9aB3KU9k+WFVaJe70g9e0l1zyU7f5wSGx0skQ2au3Zf2QYxZmt
6x3/EjAfu4U8PKz1GMBLKqpNl1J3ndt/kmhxh2TtJAIyQBFf11lIw/zETSPfZhQ3
DhRxmrNJPwUjSyTHwW1VZLpCPuJWxsc7U3mYasQoHoeQfJeZMIYQKfW0h7HajXdW
1yaW6Ws01lLDzz4i6gf/r7J8MJGUTZopmWnJ/65MO9tSUvV7pBQztGsy/6GPyA==
-----END CERTIFICATE-----
)EOF";

// xxxxxxxxxx-private.pem.key
static const char privatePemKey[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEogIBAAKCAQEAxhQ3wa22I/Z8is+XVrCF7K7tZ85Gujuokh8wEapkCue0ojai
rDcXfkXmV325gAewaTXfE8TTzTNOi6220kMeCdyTSLEYQUkVAbVsqrZwejPQsSE9
niUKLCqquEIrP80hWyIoVrgjdqbn6bgvbSdjys5pBX4m0xxRk5e97kxBymV9BOAF
AfZ4cymXnt0vpgoMH8rVC0MPE0aT9qvfztYXSdvKHdhSHI16ZD1dMxIurW1vEyE+
iSWn6ByKZ6IfPl+gRGiP/WnrsQBYQ44QiwKXUYHAE0io/3KgCGfGGsObYumwA5RY
rYp5eLEz0obNSj79cZyRXOKNABVtOyPlSgMM1wIDAQABAoIBAGlhykQ1mijJ83qa
dSa+5ZGFg3Lt0L02Djonc5iCcZVHrtd6f9NlwA8DO0gi8HUIIjfnXFaVKiYK/Ofd
NN3qrWmLu6Q56/CX5CAzmKu+QlsRhB8o0zJ2/FGBJbDAK2hXRz7IDnQMpXlz1QCI
aFsHUNVOOZudgSRh9Ziza24DjiESKtoDCi2cAy2j95yzQwH/XC6e4IHnsaGuP7sv
QOdFy4rOtnqZR0fDQe3ToiFdYs3nHqUG8QRDvatoqBYJn0eab2sjGwn5Fk3x3RwI
b1b8/0kTRvAfA1daJvrlgWp7iXHDerkXqgH1veqwi57FbB73Xp+0yphB+Vu/Csf2
jjnrPCECgYEA99nQ2z0VA2e4hz9akdAdOR60c3wTZpCI2OyqEaid6dBVF5kaIOhq
55o5tp6BvvAOFDmimHUwKRP3mCOvyzs1gHrkraAD/u7dwZrwUjz72aKA5WY1JokO
s4GkrCy+EBbHIRif2K1g26ZuO4Tbod5N3wF7ZsjVIAs26F/Q9cA9DecCgYEAzJd3
p9+p4N/k4pUSiGqlTXyaxkJN/gH5iXvGOumeh3EB4ecX4p6z83DnxdJDnwz4PLk4
tO4Aa8vw0g63x30VaWVOH5r9pim5ix/1nKcGnTdHC5+eS6pAfPRGS+h6iPmd4aNU
RzCHAo9v3XkEPu1Ovfuv0MDwOoKqeUNtJVpYy5ECgYBFx2Igx1KqMcJm+XQzRydj
FHzzYoRHxPtyLzc2X4WN9WRvj2Am0ah/ahFDk96tdgJQuWVIjP2xIvj7MVjl29aV
RIQrSbi2RmPShR+R16+EN1cYvqzKG54y9ZX6D2K3PuJzM+6U30M8s9+nDCGVsfO0
jENEzdgK/H1J8OfwuF24EwKBgAMQyfQz/8Gk2TMj9LEil/N/SHpsB+skcG7E8gVz
9rIk2VhfNBZrNlsReAeYzPaXJSkTxWvAnTUAS016dvqAP0/i+rayhdZ1jZguFGoS
fqDgcSoVbDPKC0lUB7GffCaNRvZYkiy25278Yzq4U3315FYobUv9eBwIVTGX9wy2
O9phAoGAA1UnqYNeeSyD0pxxFm6FiEXPKNxRbusFnyaPK+4kiV0pxcEqgOj0odio
lCmrSyMMZor3Qxs/E6w4HT5HeIb2QZ475p/GcBvGelELkM+yza1luS9d+O9LgxEd
4ZQwpSVGjz6212aDaI8R6zVDRRqaJ+gUenEHi05T+rdTfnVE5hg=
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
    String msg = String("Hello from ESP8266: ") + ++msgCount;
    pubSubClient.publish("outTopic", msg.c_str());
    Serial.print("Published: "); Serial.println(msg);
    lastPublish = millis();
  }
}

void msgReceived(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on "); Serial.print(topic); Serial.print(": ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void pubSubCheckConnect() {
  if ( ! pubSubClient.connected()) {
    Serial.print("PubSubClient connecting to: "); Serial.print(awsEndpoint);
    while ( ! pubSubClient.connected()) {
      Serial.print(".");
      pubSubClient.connect("ESPthing");
    }
    Serial.println(" connected");
    pubSubClient.subscribe("inTopic");
  }
  pubSubClient.loop();
}

void setCurrentTime() {
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

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
