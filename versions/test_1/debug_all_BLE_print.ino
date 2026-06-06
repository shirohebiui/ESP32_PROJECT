#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

BLEScan* pBLEScan;

// 모든 광고 정보를 출력하는 콜백
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {

    Serial.println("\n===== BLE DEVICE FOUND =====");

    // 주소
    Serial.print("Address: ");
    Serial.println(advertisedDevice.getAddress().toString().c_str());

    // 이름
    Serial.print("Name: ");
    Serial.println(advertisedDevice.getName().c_str());

    // RSSI (신호 세기)
    Serial.print("RSSI: ");
    Serial.println(advertisedDevice.getRSSI());

    // Manufacturer Data (핵심)
    std::string raw = advertisedDevice.getManufacturerData();
    if (raw.length() > 0) {
      Serial.print("Manufacturer Data (ASCII): ");
      for (int i = 0; i < raw.length(); i++) {
        Serial.print((char)raw[i]);
      }
      Serial.println();

      Serial.print("Manufacturer Data (HEX): ");
      for (int i = 0; i < raw.length(); i++) {
        if ((uint8_t)raw[i] < 0x10) Serial.print("0");
        Serial.print((uint8_t)raw[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    } else {
      Serial.println("Manufacturer Data: NONE");
    }

    // Service UUID
    if (advertisedDevice.haveServiceUUID()) {
      Serial.print("Service UUID: ");
      Serial.println(advertisedDevice.getServiceUUID().toString().c_str());
    }

    // 전체 raw 정보
    Serial.print("RAW: ");
    Serial.println(advertisedDevice.toString().c_str());

    Serial.println("============================");
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Scanner (DEBUG MODE)...");

  BLEDevice::init("");

  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());

  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

void loop() {
  Serial.println("\n--- SCANNING START ---");

  BLEScanResults* foundDevices = pBLEScan->start(5, false);

  Serial.println("--- SCANNING END ---");

  delay(2000);
}
