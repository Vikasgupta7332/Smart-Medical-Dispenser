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

// Device Certificate (from AWS IoT Core)
// IMPORTANT: Ensure this is copied EXACTLY, including BEGIN/END lines and no extra spaces.
const char AWS_CERT_CRT[] = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUZMMNm1Lbjx9qOwI9O6taJdAgetcwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI1MDcyOTE5MDYw
NVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAN2FII6aLQQd8DN++EHH
QjVUFtTdUCoz/3HrfAErVBKh6EkV0vDExIq3x+mJ1Sgqg4dgCez9SkvDEDYNfgBN
qzRRlH/xeu3XLgbbxGaOXxBmtBuqM0IC8mGZIrxUdM98cD+F1FFbBwmlg0MvPO4L
krqRr71mngmXlq9uQ+egT6cx+nOzM5b6EiJoirHQI8NcZcNVv7IsrzA0fAI9E6m6
uOXL2R19tERD4LFtPXtGDW6Ul0DkvT6pWRvRnUjcj+E0xCrsz/GPpB1qAQHg8Wzm
6gvefb+2NGLotED0z7UtPjLlhqBVqRDi6SVoyYBHCQ8/PscpRuqAcMNN7pTZjdEL
LxUCAwEAAaNgMF4wHwYDVR0jBBgwFoAUCe4O1r1P5kZ1daErkNiYlc+zBOYwHQYD
VR0OBBYEFIv5wtTEuKvGK5jcJ9p6j/oCa4odMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQCFhb/u3YEhEGQetwSJ17MXbxmC
b7hr290L1Dofz/GibNoFx8w5WkIOLVwQk6qPhxBLynpAfC+haM/j3ZmHpCm5DCw2
WLqNF+72+fryNPSuv/bIawBs47euOXa4QHbqV9WUPSrjxZqddx6JaPWu78b4HTiH
Tn/DuIdNp1Jvedbh9zvI3BC81lsgNlNeyjv8SHS5qIQpHvRvBDZZpESD+g/HH+Ny
wpTLJfKT9I6UA8S5eAOMlMdjaOGJsJijz7SOVGBQXtyht1Uh/vrOVx5yiTk1US9G
GVWPW95jwCert4k3BXHxX1CKighm5+KSPNIyIQhlK9m9g/5mnqrcmt+mDmyP
-----END CERTIFICATE-----
)EOF";

// Device Private Key (from AWS IoT Core)
// IMPORTANT: Ensure this is copied EXACTLY, including BEGIN/END lines and no extra spaces.
const char AWS_PRIVATE_KEY[] = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEpQIBAAKCAQEA3YUgjpotBB3wM374QcdCNVQW1N1QKjP/cet8AStUEqHoSRXS
8MTEirfH6YnVKCqDh2AJ7P1KS8MQNg1+AE2rNFGUf/F67dcuBtvEZo5fEGa0G6oz
QgLyYZkivFR0z3xwP4XUUVsHCaWDQy887guSupGvvWaeCZeWr25D56BPpzH6c7Mz
lvoSImiKsdAjw1xlw1W/siyvMDR8Aj0Tqbq45cvZHX20REPgsW09e0YNbpSXQOS9
PqlZG9GdSNyP4TTEKuzP8Y+kHWoBAeDxbObqC959v7Y0Yui0QPTPtS0+MuWGoFWp
EOLpJWjJgEcJDz8+xylG6oBww03ulNmN0QsvFQIDAQABAoIBAQCslw2PrXXHUGAW
byWTcmEWVabYQHo1939dzcA21QBS6Ayn7LYF2vjar2e1gCsweTczIVN6broVHyyh
QI/cQqdSQwF1UbqaVl3JAC8cTG6qw6ouJ8R5+PvJqNtUamT8R5LZGzcaD9cC4A7O
KJ7LRP8UF3AaHDqjdJT1p+6GJ/QHach3pebADvijtFZMUUtPZszW5fAUhUARl76+
sVIy71CXKBNxFJJrtf299K6l0aSG2+9FMsxePlW0ddLHowEDq9yUAUo08JtNvMBp
RMnWs5kZzt1umy79OMiBHHooPfQT+g9PCfk6BFNYEGAlc947Nyshj5qtSzfXQGHf
S79F5bXhAoGBAPu0NlTi49ya1q+uyedY1viZR3yVlHmOA4Vv0zye4xZEgKUx8kxw
YZiP5h0mW9sPjg3Vy+MQ5F4GVCCJ3LbemTybfsIOLRSxTeM/J4S7EIOqYlvKJk5/
nh/FvP0JyZWdn+b4dzq5NGcvrYaOIO+2HAMMeGMgUbfVU4VZgctkS5lJAoGBAOFN
B7rcggSu/zUWjKXULCchcs4jB8eKW4RaysdafbGHNGC7IMdSDKO1uRlUhv7nadsU
UjcUrs5MYvbOHx1EYbiqDD4eNew/YhSYfRhjatvkjk+Q0MVppGe9icdyByu8CBCq
w+w4I2Xv1j5jrpYNI6mqAIKATCWG7qsnAm9bJpNtAoGBAOpzqWwWT3LsU28fW3f/
23WCqQDV/4/eBU+kotlDknjqY/378RrcTzwhRZH/ae45pfMjIyAlWuJt+Ky6XXmF
DK1UlZTCSaGNqeQzrmbHLBHIgofHWzkhTDOCJ5B3LmsLgKdO5xgu7S30TikYeHtw
YNVCJ10F/LhEn+CFSA4hJP3pAoGAO1+odNcdsIxJlzz9DcqPZDSNY8OZjj1pBsVC
gSzYyQsfg4ptBfoaeJ4wTHuZI93qaKC/VeKuepFo1y/UxUq7EgZGOayOe1bsyw6w
kNfKVm6yDkfUrOG9QNWTVKs5No5T9ch632qf8YI2DLUKUmLeDh1OCxYyewVHKnWa
T6xOyzUCgYEAnn0I07fB1quOVVpa5PU2uzMzDwQXE3otwQ7lHyRSRmlJlDuWO0Pa
bHg2Gw1VEXE+wI1JlrObhP8m6odQru3+j1vqakm/MJYg5au433cf7eQjyD92Ew0Y
Imb2I/Zuv2a9PelH1vCzOYIdygOXYTwf97dn9PLv0K3WVTTvqekuDY8=
-----END RSA PRIVATE KEY-----
)EOF";
