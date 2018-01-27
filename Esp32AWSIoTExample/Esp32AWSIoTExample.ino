/* ESP32 AWS IoT
 *  
 * Simplest possible example (that I could come up with) of using an ESP32 with AWS IoT.
 *  
 * Author: Anthony Elder 
 * License: Apache License v2
 */
#include <WiFiClientSecure.h>
#include <PubSubClient.h> // install with Library Manager, I used v2.6.0

const char* ssid = "<yourSSID>";
const char* password = "<yourPSWD>";

const char* awsEndpoint = "<yourAwsEndpoint>.iot.us-west-2.amazonaws.com";

// Update the two certificate strings below. Paste in the text of your AWS 
// device certificate and private key. Add a quote character at the start
// of each line and a backslash, n, quote, space, backslash at the end 
// of each line:

// xxxxxxxxxx-certificate.pem.crt
const char* certificate_pem_crt = \

"-----BEGIN CERTIFICATE-----\n" \
"MIIDWTCCAkGgAwIBAgIUJ1gT+uvYaUrsqYAuKDGCCY+exfEwDQYJKoZIhvcNAQEL\n" \
"BQAwTTFLMEkGA1UECwxCQW1hem9uuFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n" \
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTE3MTIxNjE4MDkx\n" \
"OFoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n" \
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAN5n9HVMKEGOO7C9Qy/c\n" \
"ipcKGnwaUcIFy6qOm2/mRSsqpkplNBS+fDfWL8LcIJCPO0UWBMxrLNZW9Q8YDu/o\n" \
"JrOYNAeG6T7Ne96z60iyNiYEVH2E1TJ0LdS355LAfpLJj32DHXIyYfK8AR05Qv8h\n" \
"776RZSavKrvZeUwPa463IbKSpNAMEIVBtU8bXoWlZ3Hg2V161eueYtrz55fwuPtj\n" \
"yEX1uPH7wvV0lRorSRgY8JpeiSyVSwNooNLGJpJ7CcGJzQu2wOKgINehPaAXJh+y\n" \
"4DMGeo6+Xt5UUekGYLLq7NPEkHDn38g+3IDlDIQy+mFeElnKKBEePseJTL2WeHx/\n" \
"xb0CAwEAAaNgMF4wHwYDVR0jBBgwFoAUfvUM4Gz7gV8IORfeFko4YpLjLfswHQYD\n" \
"VR0OBBYEFOeKWdfggI/JTYPTiHHBf6u2qk61MAwGA1UdEwEB/wQCMAAwDgYDVR0P\n" \
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQAHnWMj9PsSlFZy+lovfVJeRT8H\n" \
"5Bynpkm+yDDdx2AZLqoR7WDHGpr6osmwj3hXOffzAYk8DhtPSg2Wmsu0mpTs1rXa\n" \
"CMzqziao4QZLcxhKNGodQ9Q05iL8Hm+sNvo/eeYFXsrgc9LIHqUwghj+JikS1hGR\n" \
"kOgRVO/dOX4qOGoIF4ZLKzHNwRtnOuVvGTjZPP7o383OfTaKxcC41S+q7But9YSo\n" \
"eKjEflxSW01Vi6P3fCDaqJGz+6L8U8WgmdeWEmwrg1bO7r9FfZynkFBqsj9Xbjp1\n" \
"EE+FyLJKdNHS2TwqjxAgKVvXobGnaVdsq5JEkiTSgQtiaBMKF0lwHYW7B+fr\n" \
"-----END CERTIFICATE-----\n";

// xxxxxxxxxx-private.pem.key
const char* private_pem_key = \

"-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEpAIBAAKCAQEA3mf0dUwoQY47sL1DL9yKlwoafBpRwgXLqo6bb+ZFKyqmSmU0\n" \
"FL58N9YvwtwgkI87RRYEzGss1lb1DxgO7+gms5g0B4bpPs173rPrSLI2JgRUfYTV\n" \
"MnQt1LfnksB+ksmPfYMdcjJh8rwBHTlC/yHvvpFlJq8qu9l5TA9rjrchspKk0AwQ\n" \
"hUG1TxtehaVnceDZXXrV655i2vPnl/C4+2PIRfW48fvC9XSVGitJGBjwml6JLJVL\n" \
"A2ig0sYmknsJwYnNC7bA4qAg16E9oBcmH7LgMwZ6jr5e3lRR6QZgsurs08SQcOff\n" \
"yD7cgOUMhDL6YV4SWcooER4+x4lMvZZ4fH/FvQIDAQABAoIBAHt4+x6jyTWb4Ued\n" \
"y08nAUoZMdr0Qa+VyBxIT8Mv8/ajFx72gJODrTLSlnF4BBw9aIpSTxet6JaWcDaR\n" \
"BLnsCJBsT3ZeT9+t9LcO6/DP/3/bp4yH0lTQayFNeCLg+2RCwobL4+0iS36XIYc3\n" \
"4xoBnTgPQDjZxr/DXLP9W5k/0lVwSr9h4JI3jgU01M948C/I0UKRFnDnQVVW3eLw\n" \
"ud7Be+cstcjInQoL4ijEOgGhmHDxZcBwO9gT5lPlp4axYlLh0uItz0fj+XO8UNk5\n" \
"cfLYurnPQIzIvcPQ+ipR4rtMfu/0BCWpMLU4R+8V97npyvrX1TuZjaemJtp+RmUb\n" \
"OnfN0AECgYEA/4D7FxfWUPBAkfZp7e8FPN8rzBhlT7qlMSvDj9dKUca13tCfQwrz\n" \
"lzmCM2lNPBBo4wEN1kqv1bVjSscQFvC/OXstHpbyOwgbP3xPV4psuzzDPCLja6HU\n" \
"PABlhh8pX76Jlfq5pSJkSixucZfRGbQapnGXMsYIr7366xE4ek8Z6d0CgYEA3taF\n" \
"J3FNcecNgDO5HOlaI/S5IpP4jWijApFy+xnGay2u39VFRJG6/tupg/eNJ3ZNTKiP\n" \
"XMv5bH7LIxTeKfLCp8zfvrzL1W7HevLZg/h94xYqEPfV7CsPTDQx/HuOtPXM9jpr\n" \
"w415UzwpVFak8Gerx6ISeIywlnZ+nVC31RC+vWECgYEAr+va9reO9QO3Mcj8frsI\n" \
"XDU9LrXNa+wTOZvsdunBXjT3nIcAYWT0+7a5EzT53NVfoo1jPZ0HHJpb12zggysY\n" \
"Pze0hU9hUYvvVvL8nkXNP85CjvFmrQEr5HRRctuZ3TaP/Rg5Ip5fd0LIeKYTbyM3\n" \
"BktH08nOyVKBpcChpnU4XnECgYEAtUYrYa+xAYSlHfJ1lwhZdMCCzpcy7c3+NXue\n" \
"3pfUKqIqDRoQVDx8sxB8cMczw7NgJIlGvoWL7O+kTnCOXe+G3Dw0k5Q/+Os6SVE/\n" \
"EzVVKyN/qvgBxTIuKp6XPz5AFAOU7YUO3zC6TrS1Jk+ps4uSWT8F6Z0BlhTk4RKl\n" \
"ySTrFWECgYBNt4adRmd6SkfpWdo4cjaHbjj1tdqPAUhJSG6aoUWAnEhd8Es3w3kV\n" \
"x/rBqq5zpObJKzeCilkxIvI6+PLAyjyDHhKo8PqEvtV4Pbu9Tx4dOHjsAc8lWwyI\n" \
"h/uQxKqs3u/3+B5X5WP4lx+IuW3iw/tsX7bpAtLpKYzujWBMVQ7FIw==\n" \
"-----END RSA PRIVATE KEY-----\n";

/* root CA can be downloaded in:
  https://www.symantec.com/content/en/us/enterprise/verisign/roots/VeriSign-Class%203-Public-Primary-Certification-Authority-G5.pem
*/
const char* rootCA = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIE0zCCA7ugAwIBAgIQGNrRniZ96LtKIVjNzGs7SjANBgkqhkiG9w0BAQUFADCB\n" \
"yjELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQL\n" \
"ExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJp\n" \
"U2lnbiwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxW\n" \
"ZXJpU2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0\n" \
"aG9yaXR5IC0gRzUwHhcNMDYxMTA4MDAwMDAwWhcNMzYwNzE2MjM1OTU5WjCByjEL\n" \
"MAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQLExZW\n" \
"ZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJpU2ln\n" \
"biwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxWZXJp\n" \
"U2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0aG9y\n" \
"aXR5IC0gRzUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCvJAgIKXo1\n" \
"nmAMqudLO07cfLw8RRy7K+D+KQL5VwijZIUVJ/XxrcgxiV0i6CqqpkKzj/i5Vbex\n" \
"t0uz/o9+B1fs70PbZmIVYc9gDaTY3vjgw2IIPVQT60nKWVSFJuUrjxuf6/WhkcIz\n" \
"SdhDY2pSS9KP6HBRTdGJaXvHcPaz3BJ023tdS1bTlr8Vd6Gw9KIl8q8ckmcY5fQG\n" \
"BO+QueQA5N06tRn/Arr0PO7gi+s3i+z016zy9vA9r911kTMZHRxAy3QkGSGT2RT+\n" \
"rCpSx4/VBEnkjWNHiDxpg8v+R70rfk/Fla4OndTRQ8Bnc+MUCH7lP59zuDMKz10/\n" \
"NIeWiu5T6CUVAgMBAAGjgbIwga8wDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8E\n" \
"BAMCAQYwbQYIKwYBBQUHAQwEYTBfoV2gWzBZMFcwVRYJaW1hZ2UvZ2lmMCEwHzAH\n" \
"BgUrDgMCGgQUj+XTGoasjY5rw8+AatRIGCx7GS4wJRYjaHR0cDovL2xvZ28udmVy\n" \
"aXNpZ24uY29tL3ZzbG9nby5naWYwHQYDVR0OBBYEFH/TZafC3ey78DAJ80M5+gKv\n" \
"MzEzMA0GCSqGSIb3DQEBBQUAA4IBAQCTJEowX2LP2BqYLz3q3JktvXf2pXkiOOzE\n" \
"p6B4Eq1iDkVwZMXnl2YtmAl+X6/WzChl8gGqCBpH3vn5fJJaCGkgDdk+bW48DW7Y\n" \
"5gaRQBi5+MHt39tBquCWIMnNZBU4gcmU7qKEKQsTb47bDN0lAtukixlE0kF6BWlK\n" \
"WE9gyn6CagsCqiUXObXbf+eEZSqVir2G3l6BFoMtEMze/aiCKm0oHw0LxOXnGiYZ\n" \
"4fQRbxC1lfznQgUy286dUV4otp6F01vvpX1FQHKOtw5rDgb7MzVIcbidJ4vEZV8N\n" \
"hnacRHr2lVz2XTIIM6RUthg/aFzyQkqFOFSDX9HoLPKsEdao7WNq\n" \
"-----END CERTIFICATE-----\n";

WiFiClientSecure wiFiClient;
void msgReceived(char* topic, byte* payload, unsigned int len);
PubSubClient pubSubClient(awsEndpoint, 8883, msgReceived, wiFiClient); 

void setup() {
  Serial.begin(115200); delay(50); Serial.println();
  Serial.println("ESP32 AWS IoT Example");
  Serial.printf("SDK version: %s\n", ESP.getSdkVersion());

  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  WiFi.waitForConnectResult();
  Serial.print(", WiFi connected, IP address: "); Serial.println(WiFi.localIP());

  wiFiClient.setCACert(rootCA);
  wiFiClient.setCertificate(certificate_pem_crt);
  wiFiClient.setPrivateKey(private_pem_key);
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
    Serial.print("PubSubClient connecting to: "); Serial.print(awsEndpoint);
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

