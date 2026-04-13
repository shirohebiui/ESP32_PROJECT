
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

