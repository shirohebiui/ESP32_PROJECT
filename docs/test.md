<details>
  
<summary>Recevier.ino</summary>

```bash

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <BLERemoteCharacteristic.h>
#include <BLEAdvertisedDevice.h>

// ===== BLE UUID =====
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-1234567890ab"

static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

bool doConnect = false;

// ===== BLE 장치 탐색 =====
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {

    // 이름으로 Sender 찾기
    if (advertisedDevice.getName() == "ESP32_SENDER") {
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      BLEDevice::getScan()->stop();
    }
  }
};

// ===== 데이터 수신 (notify) =====
void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {

  String received = "";

  // 수신 데이터 문자열 변환
  for (int i = 0; i < length; i++) {
    received += (char)pData[i];
  }

  // 그대로 출력
  Serial.println(received);
}

// ===== 서버 연결 =====
void connectToServer() {

  BLEClient* pClient = BLEDevice::createClient();
  pClient->connect(myDevice);

  BLERemoteService* pService = pClient->getService(SERVICE_UUID);
  pRemoteCharacteristic = pService->getCharacteristic(CHARACTERISTIC_UUID);

  // notify 활성화 (필수)
  if (pRemoteCharacteristic->canNotify()) {
    pRemoteCharacteristic->registerForNotify(notifyCallback);

    uint8_t notificationOn[] = {0x1, 0x0};
    pRemoteCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))
      ->writeValue(notificationOn, 2, true);
  }
}

void setup() {
  Serial.begin(115200);

  BLEDevice::init("");

  // ===== 스캔 시작 =====
  BLEScan* scan = BLEDevice::getScan();
  scan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  scan->setActiveScan(true);
  scan->start(0, nullptr);  // 무한 스캔
}

void loop() {

  // ===== 발견 시 연결 =====
  if (doConnect) {
    connectToServer();
    doConnect = false;
  }
}


```
</details>

<details>
  
<summary>Sender.ino</summary>
  
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

```
</details>
