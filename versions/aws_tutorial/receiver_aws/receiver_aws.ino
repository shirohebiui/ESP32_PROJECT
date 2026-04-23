#include <NimBLEDevice.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// ===== WiFi 설정 =====
#define WIFI_SSID     "olleh_WiFi_C75D" // <<수정필요
#define WIFI_PASSWORD "0000000985" // <<수정필요

// ===== AWS API 설정 =====
#define AWS_URL "https://scku0upgj8.execute-api.us-east-1.amazonaws.com/data" //<- 자신의 API URL  ex)"https://scku0upgj8.execute-api.us-east-1.amazonaws.com/data"

// ===== BLE UUID =====
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-1234567890ab"

static NimBLEAdvertisedDevice* myDevice = nullptr;
static NimBLEClient* pClient = nullptr;
volatile bool doConnect = false;

// ===== 큐 =====
QueueHandle_t dataQueue;

// ===== WiFi 연결 =====
void connectToWiFi() {
    Serial.print("WiFi 연결 중...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi 연결 성공 : " + WiFi.localIP().toString());
}

// ===== AWS 전송 =====
void sendToAWS(String data) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi 끊김 — 재연결 시도");
        connectToWiFi();
    }

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.begin(client, AWS_URL);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"data\": \"" + data + "\"}";
    int responseCode = http.POST(payload);

    if (responseCode > 0) {
        Serial.println("전송 성공 [" + String(responseCode) + "] : " + http.getString());
    } else {
        Serial.println("전송 실패 : " + String(responseCode));
    }

    http.end();
}

// ===== 데이터 수신 (notify) =====
String lastData = "";  // 이전 데이터 저장
void notifyCallback(
    NimBLERemoteCharacteristic* pChar,
    uint8_t* pData,
    size_t length,
    bool isNotify) {
    String received = "";
    for (size_t i = 0; i < length; i++) {
        received += (char)pData[i];
    }
    Serial.println("수신 : " + received);

    if (received == lastData) {
        Serial.println("변화 없음 — 전송 생략");
        return;
    }
    lastData = received;
    xQueueSend(dataQueue, &received, 0);
    Serial.println("변화 감지 — 전송");
}

// ===== BLE 장치 탐색 =====
class MyAdvertisedDeviceCallbacks : public NimBLEScanCallbacks {
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
        if (advertisedDevice->getName() == "ESP32_SENDER") {
            NimBLEDevice::getScan()->stop();
            myDevice = const_cast<NimBLEAdvertisedDevice*>(advertisedDevice);
            doConnect = true;
        }
    }
};

// ===== BLE 연결 =====
void connectToServer() {
    pClient = NimBLEDevice::createClient();

    if (!pClient->connect(myDevice)) {
        Serial.println("BLE 연결 실패 — 재시도 대기");
        NimBLEDevice::deleteClient(pClient);
        pClient = nullptr;
        doConnect = true;
        return;
    }
    Serial.println("BLE 연결 성공");

    NimBLERemoteService* pService = pClient->getService(SERVICE_UUID);
    if (!pService) {
        Serial.println("서비스 없음");
        pClient->disconnect();
        return;
    }

    NimBLERemoteCharacteristic* pRemoteChar = pService->getCharacteristic(CHARACTERISTIC_UUID);
    if (!pRemoteChar) {
        Serial.println("특성 없음");
        pClient->disconnect();
        return;
    }

    if (pRemoteChar->canNotify()) {
        pRemoteChar->subscribe(true, notifyCallback);
        Serial.println("Notify 등록 완료");
    }
}

// ===== BLE 연결 태스크 =====
void bleConnectTask(void* param) {
    connectToServer();
    vTaskDelete(NULL);
}

void setup() {
    Serial.begin(115200);

    dataQueue = xQueueCreate(10, sizeof(String));

    connectToWiFi();

    NimBLEDevice::init("");

    // ===== 2.x 스캔 방식 =====
    NimBLEScan* scan = NimBLEDevice::getScan();
    scan->setScanCallbacks(new MyAdvertisedDeviceCallbacks());
    scan->setActiveScan(true);
    scan->start(0, false);  // 무한 스캔
    Serial.println("BLE 스캔 시작...");
}

void loop() {
    if (doConnect) {
        doConnect = false;
        xTaskCreate(
            bleConnectTask, "ble_connect", 4096, NULL, 1, NULL
        );
    }

    String receivedData;
    if (xQueueReceive(dataQueue, &receivedData, 0) == pdTRUE) {
        sendToAWS(receivedData);
    }

    delay(1000);
}
