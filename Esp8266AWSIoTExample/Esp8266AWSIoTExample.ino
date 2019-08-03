/* ESP8266 AWS IoT
 *  
 * Simplest possible example (that I could come up with) of using an ESP8266 with AWS IoT.
 * No messing with openssl or spiffs just regular pubsub and certificates in string constants
 * 
 * This is working as at 3rd Aug 2019 with the current ESP8266 Arduino core release:
 * SDK:2.2.1(cfd48f3)/Core:2.5.2-56-g403001e3=20502056/lwIP:STABLE-2_1_2_RELEASE/glue:1.1-7-g82abda3/BearSSL:6b9587f
 * 
 * Author: Anthony Elder 
 * License: Apache License v2
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
extern "C" {
#include "libb64/cdecode.h"
}

const char* ssid = "<yourSSID>";
const char* password = "<yourPSWD>";

// Find this awsEndpoint in the AWS Console: Manage - Things, choose your thing
// choose Interact, its the HTTPS Rest endpoint 
const char* awsEndpoint = "xxxxxxxxxx.iot.us-west-2.amazonaws.com";

// For the two certificate strings below paste in the text of your AWS 
// device certificate and private key, comment out the BEGIN and END 
// lines, add a quote character at the start of each line and a quote 
// and backslash at the end of each line:

// xxxxxxxxxx-certificate.pem.crt
const String certificatePemCrt = \
//-----BEGIN CERTIFICATE-----
"MIIDWTCCAkGgAwIBAgIUQNkz+5jPnrBkY+0xAK7oIPZSvWwwDQYJKoZIhvcNAQEL" \
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g" \
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTE3MTIxNzEwNDMy" \
"OFoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGAaseAwwTQVdTIElvVCBDZXJ0aWZpY2F0" \
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAPAG3DPHYLqwhRrp5erJ" \
"6Nn24IGCGwMKGTU5mF2cBDPnwcTdAA8kX71EuhRBqhNM5zhxmsWj+zAmXfMKKT7L" \
"pvRfFyca1BwZH2bYBDmJbyITJAnljR6Jh6lPdGPm0vx2uaVYmI2s2pjQeVyKHl5I" \
"AFJpTGzoVZtxy4EeDmDFhiDrEGNqpzDEBRcNnyEN1fB7uvfSt9CgXMrjH2kFExHU" \
"CGCQwabWhwrpFmKmXo2gDKf5h221XdC9162kOzCgDcU+kL00cAHD4avYOro0p9W7" \
"GPwXeMhWieCDTjtIsps/ECd26lLlCzRfdTWqjrmZtKVpFyNXCZ9PLq3TDM/l9y0j" \
"mI0CAwEAAaNgMF4wHwYDVR0jBBgwFoAU/gnLwuy6sBTHz/XqmgIMARMfU7EwHQYD" \
"VR0OBBYEFHIbCyR1g2WNGzwNYKUQ1y/63ojCMAwGA1UdEwEB/wQCMAAwDgYDVR0P" \
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBFmQZU+iVcFVeQP1HhbzVOOEAC" \
"qbg4J4N0F1nL3woQBaP6dc1kzB6V4+rhkHec86A+jzqVMvx14Db9ROVNPRJU2Faa" \
"DIwdLDvJVqrcBUvNsnsZZw+vq+i0foP5AI0vU9TqMgkIPJRSXjrRJrykQyMfyrOt" \
"zHG7+NMe3qKnOYclMso0hb1vUpyM8T1An5p+dFP8BrBBHLakRLf2wT9b0TMDHoLi" \
"vyYczV3OLn62Y8ukZPxfBHOKwrrm10bDVAPd2twM9DbIlZnWkZ/L80pztUlMDi96" \
"9MP9jfHlJLDFu5pLv42p3Lpd+6YWAvs+8R740WVBEHumSEv3JZrPE3W8tJlH"; 
//-----END CERTIFICATE-----

// xxxxxxxxxx-private.pem.key
const String privatePemKey = \
//-----BEGIN RSA PRIVATE KEY-----
"MIIEpAIBAAKCAQEA8AbcM8dgurCFGunl6sno2fbggYIbAwoZNTmYXZwEM+fBxN0A" \
"DyRfvUS6FEGqE0znOHGaxaP7MCZd8wopPsum9F8XJxrUHBkfZtgEOYlvIhMkCeWN" \
"HomHqU90Y+bS/Ha5pViYjazamNB5XIoeXkgAUmlMbOhVm3HLgR4OYMWGIOsQY2qn" \
"MMQFFw2fIQ3V8Hu699K30KBcyuaMeQUTEdQIYJDBptaHCukWYqZejaAMp/mHbbVd" \
"0L3XraQ7MKANxT6QvTRwAcPhq9g6ujSn1bsY/Bd4yFaJ4INOO0iymz8QJ3bqUuUL" \
"NF91NaqOuZm0pWkXI1cJn08urdMMz+X3LSOYjQIDAQABAoIBAEQi7meUdZUJfv+A" \
"3k+kOYGGX3zkHh7iwv0k+Ge6XkcNxyfHRLkHrlXgKKKPcWRBNHIFbdYlEq08dvcD" \
"L2owDZjZVY04RHYle5CKSci7hty5akWU8FZ23BfoAjZJ1CLu4Qxa9hNhujwfEhR+" \
"8Nf6wHykf5l66rJRv19L1PELoWO+SgRiAH6KGAZPLvGD5UX0uqIf+UOJNF0xAZll" \
"9y3bbOL9Ib1GqSIfgUg36CuWQLWVTQQC1JT1e8genu0IniR5DuIEXievktzrb29c" \
"dY+gemApPfH1+u0iX7tNe5o8b9Ljd82eXmIvn+qKWjv9for+pQrNCKOIGUllvFpG" \
"HS0PAVECgYEA/k9iUXyknkaJ0SaHwQ9vb7tobbYRTJimqOSVNmyzKIsjexdj4xDz" \
"aNssQkoAIorfVUFiGPJNJ2fEiJ44BJoXTGteaQIFONiVuykWEENj78ytlBMnqMTt" \
"qdHRWhmd15/gjDDcSrYtnBkO3l9V0ARydhyf5hamVE6OQCXgcXHI1rcCgYEA8Z8t" \
"pCGCqXb4+LyQAXqWYs9JRdE1guz5flrw+3iQS+cfYZubSIp9sdOHUKvJIwd/jCYN" \
"d7sFEkfSgCiDEp7q5Q4I4jryMt0UpLyapNXnZfFU5Lf9xQxuHDcWC/Jo/qKDOyqX" \
"QWS82uX5sJh3YKGpn/XmJv5iVUKa+JZkiEZxZtsCgYEA6TZbb2bif9MoJ4kyDTXe" \
"d5RhW9sPWjLTAyc8PsDIO3H9g7AbhevRjGMR32iFXygDTue60i3B21Upwi92J6I3" \
"o3kDB0E3EL5t8UjoVtm2F/6XX6eBQx/04zrqxbmgKlEPVwCnxEv036WLIRpEU5MA" \
"dJgjeplixwWTJfiWLvKtv1kCgYEArNdCTfPOxS0fIse4d+ZaZ6qCbtgeYF7mhpXF" \
"Zw4yoqldkHRAm56+FZQMaw/H3TAC9J1P6wM0uIP6N8VDJfHadCoa/sJN+Uae+s0U" \
"DrAZTJpbMRXRCcE/jx10vu0UYHqeR5fLBXNriWcTu1kFpHFgptW/Yst8SiNQOl1S" \
"4w0JGBsCgYBK2fz0rT8UaN4VSQAMa/j/Abx9bsCVSM6RTc095fKzXcLBS/1Q7xb8" \
"W+6jIbUCFMoHEBbUvWCE/c8W+gxdWcr/ihPrTykM6PFAva6hfS5NRcQop2yv1LOj" \
"zda4WVWGq8TDnYN/rjvs3KNhkoq0guqcslM29M54TAtO8vBFsHQoQA==";
//-----END RSA PRIVATE KEY-----

// This is the AWS IoT CA Certificate from: 
// https://docs.aws.amazon.com/iot/latest/developerguide/managing-device-certs.html#server-authentication
// This one in here is the 'RSA 2048 bit key: Amazon Root CA 1' which is valid 
// until January 16, 2038 so unless it gets revoked you can leave this as is:
const String caPemCrt = \
//-----BEGIN CERTIFICATE-----
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF" \
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6" \
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL" \
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv" \
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj" \
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM" \
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw" \
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6" \
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L" \
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm" \
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC" \
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA" \
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI" \
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs" \
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv" \
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU" \
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy" \
"rqXRfboQnoZsG4q5WTP468SQvvG5";
//-----END CERTIFICATE-----

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

  uint8_t binaryCert[certificatePemCrt.length() * 3 / 4];
  int len = b64decode(certificatePemCrt, binaryCert);
  wiFiClient.setCertificate(binaryCert, len);
  
  uint8_t binaryPrivate[privatePemKey.length() * 3 / 4];
  len = b64decode(privatePemKey, binaryPrivate);
  wiFiClient.setPrivateKey(binaryPrivate, len);

  uint8_t binaryCA[caPemCrt.length() * 3 / 4];
  len = b64decode(caPemCrt, binaryCA);
  wiFiClient.setCACert(binaryCA, len);
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

int b64decode(String b64Text, uint8_t* output) {
  base64_decodestate s;
  base64_init_decodestate(&s);
  int cnt = base64_decode_block(b64Text.c_str(), b64Text.length(), (char*)output, &s);
  return cnt;
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
