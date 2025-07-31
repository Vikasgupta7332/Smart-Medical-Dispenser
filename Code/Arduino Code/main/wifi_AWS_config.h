// AWS IoT and Wi-Fi Libraries
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> // For creating JSON payloads for MQTT

// ------------------ Wi-Fi Configuration ------------------
#define WIFI_SSID "POCO X4 Pro 5G" // Replace with your Wi-Fi SSID
#define WIFI_PASSWORD "Vikas1234" // Replace with your Wi-Fi password

// ------------------ AWS IoT Configuration ------------------
// IMPORTANT: Replace with your actual AWS IoT Core credentials and endpoint
#define AWS_IOT_ENDPOINT "az5rwhhidqrry-ats.iot.ap-south-1.amazonaws.com" // e.g., a1b2c3d4e5f6g.iot.us-east-1.amazonaws.com
#define CLIENT_ID "ESP32PillDispenser" // Unique client ID for your Thing
#define AWS_IOT_PUBLISH_TOPIC "pillbox/status" // Topic to publish pill intake status
#define AWS_IOT_SUBSCRIBE_TOPIC "pill/commands" // Optional: Topic to subscribe for cloud commands (e.g., schedule sync)

// Amazon Root CA 1 (from AWS IoT Core documentation)
// IMPORTANT: Ensure this is copied EXACTLY, including BEGIN/END lines and no extra spaces.
const char AWS_CERT_CA[] = R"EOF(
-----BEGIN CERTIFICATE-----
//"Enter your Amazon Root CA 1 Certificate"
-----END CERTIFICATE-----
)EOF";

// Device Certificate (from AWS IoT Core)
// IMPORTANT: Ensure this is copied EXACTLY, including BEGIN/END lines and no extra spaces.
const char AWS_CERT_CRT[] = R"EOF(
-----BEGIN CERTIFICATE-----
//"Device Certificate (from AWS IoT Core)"
-----END CERTIFICATE-----
)EOF";

// Device Private Key (from AWS IoT Core)
// IMPORTANT: Ensure this is copied EXACTLY, including BEGIN/END lines and no extra spaces.
const char AWS_PRIVATE_KEY[] = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
 //"Device Private Key (from AWS IoT Core)"
-----END RSA PRIVATE KEY-----
)EOF";
