/* ESP32 AWS IoT CQ
 *  
 * Simplest possible example (that I could come up with) of using a CQ Device with AWS IoT
 *  
 * Author: Anthony Elder 
 * License: Apache License v2
 */
#include <WiFiClientSecure.h>
#include <PubSubClient.h> // install with Library Manager, I used v2.6.0
#include <CQSecurityGatwayClient.h>

const char* ssid = "BTHub6-RX2F";
const char* password = "u3CA3bteMr4t";

const char* cqSecurityGateway = "kmsstaging.cryptoquantique.com";

// this is the root certfificate for the CQ Security Gateway
const char* CQ_OEM_ROOT = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIBXzCCAQSgAwIBAgIIFg3xm+VB7dAwCgYIKoZIzj0EAwIwEjEQMA4GA1UEAxMH\n" \
"T0VNUm9vdDAgGA8wMDAxMDEwMTAwMDAwMFoXDTMwMDUwOTEwMTEzNVowEjEQMA4G\n" \
"A1UEAxMHT0VNUm9vdDBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABKvqx1Ptldxm\n" \
"9sbpWD+KsGWZ5YA6qUX3pxEVZwWwxW3/j6/pNRdM1a7KuCDZ7gzcu6mdZb0eAyw/\n" \
"1u6K/J5XA8WjQjBAMA4GA1UdDwEB/wQEAwIChDAdBgNVHSUEFjAUBggrBgEFBQcD\n" \
"AgYIKwYBBQUHAwEwDwYDVR0TAQH/BAUwAwEB/zAKBggqhkjOPQQDAgNJADBGAiEA\n" \
"9uKaotfTQeun0s5v3PUVgIntZmx4XLmPm4ZwCfYP7UUCIQD+/bK521p96DgdY5YO\n" \
"flDgVMTRnKwo17mwvQAy1v9eyQ==\n" \
"-----END CERTIFICATE-----\n";

WiFiClientSecure wiFiClient;

//CQSecurityGatwayClient cqClient(cqSecurityGateway, wiFiClient);
CQSecurityGatwayClient cqClient;

void msgReceived(char* topic, byte* payload, unsigned int len);
PubSubClient pubSubClient(wiFiClient); 

CQSecurityGatwayClient::Enroll_Resp enrollResp;

void setup() {
  Serial.begin(115200); delay(50); Serial.println();
  Serial.println("ESP32 AWS IoT Example");
  
  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  WiFi.waitForConnectResult();
  Serial.print(", WiFi connected, IP address: "); Serial.println(WiFi.localIP());

  Serial.print("Enrolling device");
  wiFiClient.setCACert(CQ_OEM_ROOT);
  int rc = cqClient.enroll(&enrollResp);
  if (rc < 0) {
     Serial.printf("CQ Enroll error: %i\n", rc);
     delay(5000); ESP.restart();
  }
  Serial.println(". Enrolled.");

  wiFiClient.setCACert(enrollResp.rootCA);
  wiFiClient.setCertificate(enrollResp.dcrtPEM);
  wiFiClient.setPrivateKey(enrollResp.dcrtKeyPEM); // TODO: shouldn't need to reveal the private key

  pubSubClient.setServer(enrollResp.endpoint, enrollResp.endpointPort);
  pubSubClient.setCallback(msgReceived);
}

unsigned long lastPublish;
int msgCount;

void loop() {

  pubSubCheckConnect();

  if (millis() - lastPublish > 10000) {
    String msg = String("Hello from ESP8266: ") + ++msgCount;
    boolean rc = pubSubClient.publish("outTopic", msg.c_str());
    Serial.print("Published, rc="); Serial.print( (rc ? "OK: " : "FAILED: ") );
    Serial.println(msg);
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
    Serial.print("PubSubClient connecting to: "); Serial.print(enrollResp.endpoint);
    while ( ! pubSubClient.connected()) {
      Serial.print(".");
      pubSubClient.connect("ESPthingXXXX");
      delay(1000);
    }
    Serial.println(" connected");
    pubSubClient.subscribe("inTopic");
  }
  pubSubClient.loop();
}

