#include <WiFi.h>
#include <HTTPClient.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <BLERemoteCharacteristic.h>
#include <BLEAdvertisedDevice.h>

// ===== WiFi 설정 =====
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";

// ===== AWS API Gateway URL =====
const char* serverUrl = "https://your-api-id.execute-api.region.amazonaws.com/prod";

// ===== BLE UUID =====
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-1234567890ab"

static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

bool doConnect = false;

// ===== BLE 장치 탐색 =====
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {

    if (advertisedDevice.getName() == "ESP32_SENDER") {
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      BLEDevice::getScan()->stop();
    }
  }
};

// ===== AWS 전송 =====
void sendToAWS(String data) {

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    // 그대로 JSON 형태로 전송
    String body = "{\"data\":\"" + data + "\"}";

    int httpResponseCode = http.POST(body);

    Serial.print("HTTP Response: ");
    Serial.println(httpResponseCode);

    http.end();
  }
}

// ===== BLE 수신 =====
void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {

  String received = "";

  for (int i = 0; i < length; i++) {
    received += (char)pData[i];
  }

  Serial.print("Received: ");
  Serial.println(received);

  // ===== AWS로 전송 =====
  sendToAWS(received);
}

// ===== BLE 연결 =====
void connectToServer() {

  BLEClient* pClient = BLEDevice::createClient();
  pClient->connect(myDevice);

  BLERemoteService* pService = pClient->getService(SERVICE_UUID);
  pRemoteCharacteristic = pService->getCharacteristic(CHARACTERISTIC_UUID);

  if (pRemoteCharacteristic->canNotify()) {
    pRemoteCharacteristic->registerForNotify(notifyCallback);

    uint8_t notificationOn[] = {0x1, 0x0};
    pRemoteCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))
      ->writeValue(notificationOn, 2, true);
  }
}

void setup() {
  Serial.begin(115200);

  // ===== WiFi 연결 =====
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");

  // ===== BLE 시작 =====
  BLEDevice::init("");

  BLEScan* scan = BLEDevice::getScan();
  scan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  scan->setActiveScan(true);
  scan->start(0, nullptr);
}

void loop() {

  if (doConnect) {
    connectToServer();
    doConnect = false;
  }
}