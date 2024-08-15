#include <esp_now.h>
#include <WiFi.h>
#include <U8g2lib.h>

// Structure to receive the data
typedef struct struct_message {
  float distance;
  bool humanOrAnimal;
  bool motion;
  float weight;
  int num_egg;
} struct_message;

// Create a struct_message instance to hold the incoming data
struct_message incomingData;

// Initialize the display
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
  Serial.print("Received packet from: ");
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  Serial.print("Bytes received: ");
  Serial.println(len);
  memcpy(&incomingData, data, sizeof(incomingData));

  Serial.print("Distance (cm): ");
  String distanceStr;
  if (incomingData.distance != -1) {
    Serial.print(incomingData.distance);
    distanceStr = String(incomingData.distance, 2) + " cm";
  } else {
    Serial.print("out of range");
    distanceStr = "out of range";
  }
  Serial.print("\t");

  Serial.print("Human/Animal: ");
  String humanOrAnimalStr = incomingData.humanOrAnimal ? "Yes" : "No";
  Serial.print(humanOrAnimalStr);
  Serial.print("\t");

  Serial.print("Motion: ");
  String motionStr = incomingData.motion ? "Detected" : "Not Detected";
  Serial.print(motionStr);
  Serial.print("\t");

  Serial.print("Weight (g): ");
  String weightStr = String(incomingData.weight, 2) + " g";
  Serial.print(weightStr);
  Serial.print("\t");

  Serial.print("Number of eggs: ");
  String numEggStr = String(incomingData.num_egg);
  Serial.println(numEggStr);

  // Update the display with the received data
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  u8g2.drawStr(0, 10, ("Distance: " + distanceStr).c_str());
  u8g2.drawStr(0, 20, ("Human/Animal: " + humanOrAnimalStr).c_str());
  u8g2.drawStr(0, 30, ("Motion: " + motionStr).c_str());
  u8g2.drawStr(0, 40, ("Weight: " + weightStr).c_str());
  u8g2.drawStr(0, 50, ("Eggs: " + numEggStr).c_str());

  u8g2.sendBuffer();
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the receive callback function
  esp_now_register_recv_cb(OnDataRecv);

  // Initialize the display
  u8g2.begin();

  Serial.println("ESP-NOW Receiver Initialized");
}

void loop() {
  // No need to do anything here
}
