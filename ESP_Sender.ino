#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_NAU7802.h>
#include <esp_now.h>
#include <WiFi.h>

// Create instances for the sensors
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_NAU7802 nau = Adafruit_NAU7802();

int pirPin = A0; // PIR sensor connected to GPIO A0
int pirState = LOW; // Start with the PIR sensor assuming no motion

// MAC address of the receiver ESP (replace with actual MAC address)
uint8_t broadcastAddress[] = {0x44, 0x17, 0x93, 0xE0, 0xE7, 0x54};

// Structure to hold the data
typedef struct struct_message {
  float distance;
  bool humanOrAnimal;
  bool motion;
  float weight;
  int num_egg;
} struct_message;

struct_message myData;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initialize VL53L0X sensor
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while (1);
  }
  Serial.println(F("VL53L0X API Simple Ranging example\n\n"));

  // Initialize MLX90614 sensor
  Wire.begin(); // Initialize I2C communication
  if (!mlx.begin()) { // Attempt to initialize the sensor
    Serial.println("Error connecting to MLX90614 sensor. Check your connections.");
    while (1); // If sensor initialization fails, print error and halt execution
  }

  // Initialize PIR sensor
  pinMode(pirPin, INPUT); // Set the PIR sensor pin as input

  // Initialize NAU7802 sensor
  if (!nau.begin()) {
    Serial.println("Failed to find NAU7802");
    while (1);
  }
  Serial.println("Found NAU7802");

  nau.setLDO(NAU7802_3V0);
  Serial.print("LDO voltage set to ");
  switch (nau.getLDO()) {
    case NAU7802_4V5:  Serial.println("4.5V"); break;
    case NAU7802_4V2:  Serial.println("4.2V"); break;
    case NAU7802_3V9:  Serial.println("3.9V"); break;
    case NAU7802_3V6:  Serial.println("3.6V"); break;
    case NAU7802_3V3:  Serial.println("3.3V"); break;
    case NAU7802_3V0:  Serial.println("3.0V"); break;
    case NAU7802_2V7:  Serial.println("2.7V"); break;
    case NAU7802_2V4:  Serial.println("2.4V"); break;
    case NAU7802_EXTERNAL:  Serial.println("External"); break;
  }

  nau.setGain(NAU7802_GAIN_128);
  Serial.print("Gain set to ");
  switch (nau.getGain()) {
    case NAU7802_GAIN_1:  Serial.println("1x"); break;
    case NAU7802_GAIN_2:  Serial.println("2x"); break;
    case NAU7802_GAIN_4:  Serial.println("4x"); break;
    case NAU7802_GAIN_8:  Serial.println("8x"); break;
    case NAU7802_GAIN_16:  Serial.println("16x"); break;
    case NAU7802_GAIN_32:  Serial.println("32x"); break;
    case NAU7802_GAIN_64:  Serial.println("64x"); break;
    case NAU7802_GAIN_128:  Serial.println("128x"); break;
  }

  nau.setRate(NAU7802_RATE_10SPS);
  Serial.print("Conversion rate set to ");
  switch (nau.getRate()) {
    case NAU7802_RATE_10SPS:  Serial.println("10 SPS"); break;
    case NAU7802_RATE_20SPS:  Serial.println("20 SPS"); break;
    case NAU7802_RATE_40SPS:  Serial.println("40 SPS"); break;
    case NAU7802_RATE_80SPS:  Serial.println("80 SPS"); break;
    case NAU7802_RATE_320SPS:  Serial.println("320 SPS"); break;
  }

  // Take 10 readings to flush out readings
  for (uint8_t i=0; i<10; i++) {
    while (!nau.available()) delay(1);
    nau.read();
  }

  while (!nau.calibrate(NAU7802_CALMOD_INTERNAL)) {
    Serial.println("Failed to calibrate internal offset, retrying!");
    delay(1000);
  }
  Serial.println("Calibrated internal offset");

  while (!nau.calibrate(NAU7802_CALMOD_OFFSET)) {
    Serial.println("Failed to calibrate system offset, retrying!");
    delay(1000);
  }
  Serial.println("Calibrated system offset");

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register for a callback function on data sent
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  // VL53L0X sensor measurement
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  float distance_cm = -1;
  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    distance_cm = measure.RangeMilliMeter / 10.0; // Convert from mm to cm
  }
  myData.distance = distance_cm;

  // MLX90614 sensor measurement
  double ambientTemp = mlx.readAmbientTempC();  // Read ambient temperature
  double objectTemp = mlx.readObjectTempC();    // Read object temperature
  double tempDifference = objectTemp - ambientTemp;  // Calculate temperature difference

  myData.humanOrAnimal = false;
  // Check if the object temperature is within the human/animal body temperature range
  if (tempDifference > 2.0 && objectTemp > 27.0 && objectTemp < 42.0) {
    myData.humanOrAnimal = true;
  }

  // PIR sensor measurement
  myData.motion = digitalRead(pirPin) == HIGH;

  // NAU7802 sensor measurement
  while (!nau.available()) {
    delay(1000);
  }
  int32_t val = nau.read();
  float weight = (val + 48.299) / 210.35;

  if (weight < 10) { // If weight is less than a small threshold, set to zero
    weight = 0;
  }
  myData.weight = weight;

  if (weight > 50 && weight < 75) {
    myData.num_egg = 1;
  } else if (weight >= 75 && weight < 140) {
    myData.num_egg = 2;
  } else if (weight >= 140 && weight < 210) {
    myData.num_egg = 3;
  } else {
    myData.num_egg = 0; // No eggs
  }

  // Send data
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.print("Error sending the data: ");
    Serial.println(result);
  }

  // Print results in columns
  Serial.print("Distance (cm): ");
  if (myData.distance != -1) {
    Serial.print(myData.distance);
  } else {
    Serial.print("out of range");
  }
  Serial.print("\t");

  Serial.print("Human/Animal: ");
  Serial.print(myData.humanOrAnimal ? "Yes" : "No");
  Serial.print("\t");

  Serial.print("Motion: ");
  Serial.print(myData.motion ? "Detected" : "Not Detected");
  Serial.print("\t");

  Serial.print("Weight(g): ");
  Serial.print(myData.weight);
  Serial.print("\t");

  Serial.print("Number of eggs: ");
  Serial.println(myData.num_egg);

  delay(1000);  // Change delay to 1 second
}
