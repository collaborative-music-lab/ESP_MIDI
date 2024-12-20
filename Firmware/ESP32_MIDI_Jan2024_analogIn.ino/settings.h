#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define GATEWAY_RXD2 14
#define GATEWAY_TXD2 15

#define ESP1_LED_PIN 32
#define ESP2_LED_PIN 32
#define LED_COUNT   2
#define CHANNEL     0

const char* SSID  = "Chuney_Home2G";
const char* WIFI_PW  = "littlelion";

//#define GATEWAY_MAC "30:AE:A4:00:57:51
// slave address 30:ae:a4:00:57:50  30:ae:a4:00:57:50
// master address 30:ae:a4:22:ca:8c
// F0:08:D1:80:E2:B1
//const uint8_t gatewayAddress[] = {0x30, 0xAE, 0xA4, 0x00, 0x57, 0x50};
uint8_t gatewayAddress[] = {0xF0, 0x08, 0xD1, 0x80, 0xE2, 0xB1};
//const uint8_t gatewayAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#define CHANNEL 0
#define PRINTSCANRESULTS 0
#define DELETEBEFOREPAIR 0

//Structure example to send data
//Must match the receiver structure
typedef struct EspNowMessage {
    uint16_t counter;
    uint16_t lastCycleDuration;
    uint16_t errorCount;
    char payload[200];
} EspNowMessage;