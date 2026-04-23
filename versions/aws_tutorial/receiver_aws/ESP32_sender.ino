#include <NimBLEDevice.h>

// ===== UUID =====
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-1234567890ab"

// ===== 센서 핀 =====
#define SENSOR_PIN       25
#define SEND_INTERVAL_MS 2000

NimBLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;
bool prevConnected   = false;
unsigned long lastSendMs = 0;

// ===== 연결 상태 콜백 (2.x 시그니처) =====
class MyServerCallbacks : public NimBLEServerCallbacks {
  // ✅ 2.x: NimBLEConnInfo& 파라미터 추가
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
    deviceConnected = true;
    Serial.print("[BLE] Client connected: ");
    Serial.println(connInfo.getAddress().toString().c_str());
  }

  // ✅ 2.x: NimBLEConnInfo& + int reason 파라미터 추가
  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
    deviceConnected = false;
    Serial.print("[BLE] Disconnected. Reason: ");
    Serial.println(reason);
    NimBLEDevice::getAdvertising()->start();
  }
};

// ===== 광고 설정 =====
void startAdvertising() {
  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();

  // 광고 패킷: 서비스 UUID
  NimBLEAdvertisementData advData;
  advData.setCompleteServices(NimBLEUUID(SERVICE_UUID));
  pAdvertising->setAdvertisementData(advData);

  // Scan response: 디바이스 이름
  NimBLEAdvertisementData scanData;
  scanData.setName("ESP32_SENDER");
  pAdvertising->setScanResponseData(scanData);

  // ✅ 2.x: setMinPreferred/setMaxPreferred → setPreferredParams(min, max)
  pAdvertising->setPreferredParams(0x06, 0x12);

  pAdvertising->start();
  Serial.println("[BLE] Advertising started.");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(SENSOR_PIN, INPUT);

  NimBLEDevice::init("ESP32_SENDER");

  // ✅ 2.x: ESP_PWR_LVL_P9 → 정수 dBm 값 (9)
  NimBLEDevice::setPower(9);

  NimBLEServer* pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  NimBLEService* pService = pServer->createService(SERVICE_UUID);

  // CCCD는 NOTIFY 설정 시 자동 추가 — 수동 추가 불필요
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    NIMBLE_PROPERTY::NOTIFY
  );

  pService->start();
  startAdvertising();
}

void loop() {
  unsigned long now = millis();

  int sensorValue = digitalRead(SENSOR_PIN);

  if (!deviceConnected && prevConnected) {
    NimBLEDevice::getAdvertising()->start();
    prevConnected = false;
  }
  if (deviceConnected && !prevConnected) {
    prevConnected = true;
  }


  // ✅ 연결 대기 중 상태 출력 (5초마다)
  static unsigned long lastStatusMs = 0;
  if (!deviceConnected && (now - lastStatusMs >= 5000)) {
    lastStatusMs = now;
    Serial.print("[BLE] Waiting for connection... Sensor : ");
    Serial.println(sensorValue);
  }

  if (deviceConnected && (now - lastSendMs >= SEND_INTERVAL_MS)) {
    lastSendMs = now;

    int ch = 0;
    String msg = "{Ch:" + String(ch) + ",value:" + String(sensorValue) + "}";

    pCharacteristic->setValue(msg.c_str());
    pCharacteristic->notify();

    Serial.print("[SEND] ");
    Serial.println(msg);
  }
}