#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <BLERemoteCharacteristic.h>
#include <BLEAdvertisedDevice.h>

#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-1234567890ab"

static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

bool doConnect = false;
bool connected = false;

// 🔥 모든 BLE 장치 출력 + 이름으로 필터링
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {

    Serial.println("---- FOUND DEVICE ----");
    Serial.println(advertisedDevice.toString().c_str());

    // 👉 이름으로 찾기 (중요)
    if (advertisedDevice.getName() == "ESP32_BLE_SERVER") {
      Serial.println(">>> TARGET FOUND <<<");

      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;

      BLEDevice::getScan()->stop();
    }
  }
};

// 🔥 notify 수신 콜백
void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {

  // 🔥 수신 데이터 문자열로 변환
  String received = "";
  for (int i = 0; i < length; i++) {
    received += (char)pData[i];
  }

  Serial.print(">>> Raw: ");
  Serial.println(received);

  // 🔥 값 해석
  if (received == "0") {
    Serial.println(">>> 상태: 어두움");
  } 
  else if (received == "1") {
    Serial.println(">>> 상태: 밝음");
  } 
  else {
    Serial.println(">>> 알 수 없는 값");
  }
}

// 🔥 서버 연결
bool connectToServer() {

  Serial.println("Connecting to server...");

  BLEClient* pClient = BLEDevice::createClient();
  pClient->connect(myDevice);

  Serial.println("Connected!");

  BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);

  if (pRemoteService == nullptr) {
    Serial.println("❌ Service NOT found");
    return false;
  }

  Serial.println("Service found");

  pRemoteCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID);

  if (pRemoteCharacteristic == nullptr) {
    Serial.println("❌ Characteristic NOT found");
    return false;
  }

  Serial.println("Characteristic found");

  // notify 등록
  pRemoteCharacteristic->registerForNotify(notifyCallback);

  connected = true;
  Serial.println(">>> READY TO RECEIVE <<<");

  return true;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("===== CLIENT START =====");

  BLEDevice::init("");

  BLEScan* pScan = BLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());

  pScan->setInterval(100);
  pScan->setWindow(99);
  pScan->setActiveScan(true);

  Serial.println("Scanning...");
  pScan->start(0, nullptr); // 🔥 무한 스캔
}

void loop() {

  if (doConnect) {
    if (connectToServer()) {
      Serial.println(">>> CONNECT SUCCESS <<<");
    } else {
      Serial.println(">>> CONNECT FAIL <<<");
    }
    doConnect = false;
  }

  delay(1000);
}
