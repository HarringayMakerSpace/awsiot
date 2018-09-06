/* ESP8266 AWS IoT
 *  
 * Simplest possible example (that I could come up with) of using an ESP8266 with AWS IoT.
 * No messing with openssl or spiffs just regular pubsub and certificates in string constants
 * 
 * Note: For this to work you do need to do this:
 *       https://github.com/esp8266/Arduino/issues/3944
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

  uint8_t binaryCert[certificatePemCrt.length() * 3 / 4];
  int len = b64decode(certificatePemCrt, binaryCert);
  wiFiClient.setCertificate(binaryCert, len);
  
  uint8_t binaryPrivate[privatePemKey.length() * 3 / 4];
  len = b64decode(privatePemKey, binaryPrivate);
  wiFiClient.setPrivateKey(binaryPrivate, len);
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
