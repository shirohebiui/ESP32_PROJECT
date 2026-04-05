#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// ✅ 조도센서 핀 추가 (DO 연결한 핀)
int sensorPin = 25;

// UUID (클라이언트와 반드시 동일)
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-1234567890ab"

// 연결 상태 콜백
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println(">>> Client Connected");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println(">>> Client Disconnected");
    BLEDevice::startAdvertising();
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("===== SERVER START =====");

  // ✅ 센서 핀 입력 설정
  pinMode(sensorPin, INPUT);

  // BLE 초기화
  BLEDevice::init("ESP32_BLE_SERVER");

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
  Serial.println(">>> BLE Advertising Started");
}

void loop() {

  // ✅ 조도센서 값 읽기
  int lightValue = digitalRead(sensorPin);

  // 시리얼 출력
  Serial.print("Light: ");
  Serial.println(lightValue);  // 0 또는 1

  if (deviceConnected) {
    // 문자열로 변환해서 전송
    String msg = String(lightValue);

    pCharacteristic->setValue(msg.c_str());
    pCharacteristic->notify();

    Serial.print(">>> Sent: ");
    Serial.println(msg);
  }

  delay(1000);  // 1초마다
}
