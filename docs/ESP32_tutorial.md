환경
IDE : ArduinoIDE

Module : ESP32 CP2102


<img width="1009" height="646" alt="ESP32_CP20102_PIN_MAP" src="https://github.com/user-attachments/assets/ccaba6bb-5eda-4abc-b81e-f83096b9242d" />

USB 드라이버 : CP210x Universal Windows Driver
```bash
https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads
```


도구 → 보드 → 보드 매니저 → 검색 ( NimBLE ) → “NimBLE-Arduino” 설치

<img width="220" height="483" alt="ESP_setting_1" src="https://github.com/user-attachments/assets/e97a4c10-667c-4d61-8d2f-01ab70ca506a" />


도구 → 보드 → esp32 → ESP32 Dev Module 선택

<img width="900" height="147" alt="ESP32_setting_3" src="https://github.com/user-attachments/assets/773349bf-c888-4a0b-b0e6-08e169da2a88" />

시리얼 모니터 속도 115200baud설정 안할시 속도 불일치로 값을 제대로 못읽는다.
시리얼 모니터의 스피드를 1150200baud로 변경

<img width="1306" height="858" alt="Serial_monitor_1" src="https://github.com/user-attachments/assets/b8b5f58d-d47e-4f6c-ade0-72b13f70bb52" />

시리얼모니터에서 조건에 따라 변화하는 값을 통해 연결 확인

<img width="559" height="303" alt="Serial_moniotr_2" src="https://github.com/user-attachments/assets/8c19fbd2-f9a3-4cdc-9a13-ae6806262fd4" />


<details>
  
<summary>Recevier.ino</summary>

```bash

#include <NimBLEDevice.h>

// ===== UUID =====
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-1234567890ab"

static const NimBLEAdvertisedDevice* myDevice = nullptr;
static NimBLEClient* pClient                  = nullptr;
bool doConnect = false;

// ===== notify 수신 콜백 =====
void notifyCallback(
  NimBLERemoteCharacteristic* pRC,
  uint8_t* pData,
  size_t length,
  bool isNotify)
{
  String received = "";
  for (size_t i = 0; i < length; i++) {
    received += (char)pData[i];
  }
  Serial.print("[RECV] ");
  Serial.println(received);
}

// ===== 클라이언트 연결 콜백 =====
class MyClientCallbacks : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient* pClient) override {
    Serial.println("[BLE] Connected to server.");
  }

  void onDisconnect(NimBLEClient* pClient, int reason) override {
    Serial.print("[BLE] Disconnected. Reason: ");
    Serial.println(reason);
    NimBLEDevice::getScan()->start(0, false);
    Serial.println("[BLE] Restarting scan...");
  }
};

// ===== 스캔 콜백 =====
class MyScanCallbacks : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* dev) override {
    if (dev->isAdvertisingService(NimBLEUUID(SERVICE_UUID))) {
      Serial.print("[SCAN] Target found: ");
      Serial.println(dev->getAddress().toString().c_str());

      NimBLEDevice::getScan()->stop();
      myDevice  = dev;
      doConnect = true;
    }
  }

  void onScanEnd(const NimBLEScanResults& results, int reason) override {
    Serial.print("[SCAN] Scan ended. reason: ");
    Serial.println(reason);
    if (!doConnect && (!pClient || !pClient->isConnected())) {
      NimBLEDevice::getScan()->start(0, false);
    }
  }
};

// ===== 서버 연결 함수 =====
bool connectToServer() {
  Serial.println("[BLE] Connecting...");

  pClient = NimBLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallbacks());

  if (!pClient->connect(myDevice)) {
    Serial.println("[BLE] Connect FAIL");
    NimBLEDevice::deleteClient(pClient);
    pClient = nullptr;
    return false;
  }

  Serial.println("[BLE] Connected!");

  NimBLERemoteService* pService = pClient->getService(SERVICE_UUID);
  if (!pService) {
    Serial.println("[BLE] Service NOT found");
    pClient->disconnect();
    return false;
  }

  NimBLERemoteCharacteristic* pChar = pService->getCharacteristic(CHARACTERISTIC_UUID);
  if (!pChar) {
    Serial.println("[BLE] Characteristic NOT found");
    pClient->disconnect();
    return false;
  }

  if (pChar->canNotify()) {
    if (pChar->subscribe(true, notifyCallback)) {
      Serial.println("[BLE] Notify subscribed OK");
    } else {
      Serial.println("[BLE] Notify subscribe FAIL");
      pClient->disconnect();
      return false;
    }
  }

  return true;
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("===== RECEIVER START =====");

  NimBLEDevice::init("");

  NimBLEScan* scan = NimBLEDevice::getScan();
  scan->setScanCallbacks(new MyScanCallbacks());
  scan->setActiveScan(true);
  scan->setInterval(100);
  scan->setWindow(99);
  scan->setMaxResults(0);

  scan->start(0, false);
  Serial.println("[SCAN] Started.");
}

void loop() {
  if (doConnect) {
    doConnect = false;
    if (connectToServer()) {
      Serial.println(">>> CONNECT SUCCESS <<<");
    } else {
      Serial.println(">>> CONNECT FAIL — restarting scan <<<");
      NimBLEDevice::getScan()->start(0, false);
    }
  }
}

```
</details>

<details>
  
<summary>Sender.ino</summary>
  
```bash

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

```
</details>
