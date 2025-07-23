//ESPNOW defs
#include "ESP32_NOW.h"
#include "WiFi.h"
#include <esp_mac.h>  // For the MAC2STR and MACSTR macros
#include <vector>
#define ESPNOW_WIFI_CHANNEL 6

//Clock defs
#define CLOCKS_PER_QUARTER 24
#define MIN_BPM 60
#define MAX_BPM 180

// MIDI Clock packet struct (same as sender)
typedef struct __attribute__((packed)) {
  uint32_t clockCount;
  uint32_t timestampMillis;
  uint8_t flags;
  uint16_t bpm;
} MidiClockPacket;

// Timing state
volatile uint32_t localClockCount = 0;
hw_timer_t* timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// Estimated clock interval (microseconds)
volatile uint32_t clockIntervalMicros = 20833;  // default = 120 BPM

// PLL smoothing factor
const float alpha = 0.05;  // Lower = slower correction, Higher = snappier

void IRAM_ATTR onLocalClockTick() {
  portENTER_CRITICAL_ISR(&timerMux);
  localClockCount++;
  portEXIT_CRITICAL_ISR(&timerMux);
  sendMidiClockMessage(MIDI_CLOCK_TICK);
}

void adjustTimerInterval(uint32_t newIntervalMicros) {
  static uint32_t smoothedInterval = newIntervalMicros;
  smoothedInterval = smoothedInterval * (1.0 - alpha) + newIntervalMicros * alpha;
  clockIntervalMicros = smoothedInterval;
  timerAlarm(timer,smoothedInterval, true, 0);
}

// Creating a new class that inherits from the ESP_NOW_Peer class is required.

class ESP_NOW_Peer_Class : public ESP_NOW_Peer {
public:
  // Constructor of the class
  ESP_NOW_Peer_Class(const uint8_t *mac_addr, uint8_t channel, wifi_interface_t iface, const uint8_t *lmk) : ESP_NOW_Peer(mac_addr, channel, iface, lmk) {}

  // Destructor of the class
  ~ESP_NOW_Peer_Class() {}

  // Function to register the master peer
  bool add_peer() {
    if (!add()) {
      log_e("Failed to register the broadcast peer");
      return false;
    }
    return true;
  }

  // Function to print the received messages from the master
  void onReceive(const uint8_t *data, size_t len, bool broadcast) {
    if (len != sizeof(MidiClockPacket)) {
      return;
    }

    MidiClockPacket packet;
    memcpy(&packet, data, sizeof(MidiClockPacket));

    // Estimate interval from last beat to this one
    static uint32_t lastMillis = 0;
    uint32_t nowMillis = packet.timestampMillis;
    uint32_t elapsedMillis = nowMillis - lastMillis;
    lastMillis = nowMillis;

    // Compute new interval per clock tick (24 PPQN)
    float bpm = packet.bpm;
    float intervalMs = (60000.0 / bpm) / CLOCKS_PER_QUARTER;
    uint32_t newIntervalMicros = intervalMs * 1000.0;

    adjustTimerInterval(newIntervalMicros);  // assumed external or friend function

    if (packet.flags & 0x01) {
      // START received
      portENTER_CRITICAL(&timerMux);
      localClockCount = packet.clockCount;
      portEXIT_CRITICAL(&timerMux);
    }

    // Debug output
    // Serial.printf("SYNC — Count: %lu, BPM: %.2f, Interval: %.2f ms (%lu us)\n",
    //               packet.clockCount, bpm, intervalMs, newIntervalMicros);
  }
};

/* Global Variables */

// List of all the masters. It will be populated when a new master is registered
std::vector<ESP_NOW_Peer_Class> masters;

/* Callbacks */

// Callback called when an unknown peer sends a message
void register_new_master(const esp_now_recv_info_t *info, const uint8_t *data, int len, void *arg) {
  if (memcmp(info->des_addr, ESP_NOW.BROADCAST_ADDR, 6) == 0) {
    Serial.printf("Unknown peer " MACSTR " sent a broadcast message\n", MAC2STR(info->src_addr));
    Serial.println("Registering the peer as a master");

    ESP_NOW_Peer_Class new_master(info->src_addr, ESPNOW_WIFI_CHANNEL, WIFI_IF_STA, NULL);

    masters.push_back(new_master);
    if (!masters.back().add_peer()) {
      Serial.println("Failed to register the new master");
      return;
    }
  } else {
    // The slave will only receive broadcast messages
    log_v("Received a unicast message from " MACSTR, MAC2STR(info->src_addr));
    log_v("Igorning the message");
  }
}


void setupESPNow() {
  // Initialize the Wi-Fi module
  WiFi.mode(WIFI_STA);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL);
  while (!WiFi.STA.started()) {
    delay(100);
  }

  // Initialize the ESP-NOW protocol
  if (!ESP_NOW.begin()) {
    Serial.println("Failed to initialize ESP-NOW");
    Serial.println("Reeboting in 5 seconds...");
    delay(5000);
    ESP.restart();
  }

  // Register the new peer callback
  ESP_NOW.onNewPeer(register_new_master, NULL);

  Serial.println("Setup complete. Waiting for a master to broadcast a message...");
}

void setupClockTimer() {
  timer = timerBegin(1000000);  // Timer 0, divider 8 (10 microsecond tick)
  timerAttachInterrupt(timer, &onLocalClockTick);  // Attach callback function
}
