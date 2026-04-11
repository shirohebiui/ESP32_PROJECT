<details>
<summary>Recevier.ino</summary>

```bash

  // ===== 발견 시 연결 =====
  if (doConnect) {
    connectToServer();
    doConnect = false;
  }
}


```
<summary>Recevier.ino</summary>
  
```bash
  
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// ===== BLE UUID =====
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-1234567890ab"

// ===== 센서 핀 =====
#define SENSOR_PIN 25   // DO 연결

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// ===== BLE 연결 상태 =====
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    BLEDevice::startAdvertising();
  }
};

void setup() {
  Serial.begin(115200); //Serial Speed 115200 baud

  // ===== 센서 입력 설정 =====
  pinMode(SENSOR_PIN, INPUT);

  // ===== BLE 초기화 =====
  BLEDevice::init("ESP32_SENDER");

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);

  BLEDevice::startAdvertising();
}

void loop() {

  // ===== 센서값 읽기 (0 또는 1) =====
  int value = digitalRead(SENSOR_PIN);

  // ===== 전송 포맷 =====
  int Ch = 0;
  String msg = "{Ch : " + Int(Ch) + ",value:" + String(value) + "}";

  Serial.println(msg);

  // ===== BLE 전송 =====
  if (deviceConnected) {
    pCharacteristic->setValue(msg.c_str());
    pCharacteristic->notify();
  }

  delay(2000);  // 2초 주기
}

</details>

```
