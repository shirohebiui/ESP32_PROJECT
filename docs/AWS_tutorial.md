# API Gateway 생성

```bash
https://eu-north-1.console.aws.amazon.com/apigateway/main/welcome?region=eu-north-1
```

<details>
  
<summary>아두이노 코드</summary>

<details>
  
<summary>RECEIVER.ino / WiFi명+비밀번호, API URL 수정필요</summary>

```bash

#include <NimBLEDevice.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// ===== WiFi 설정 =====
#define WIFI_SSID     "WIFI명" // <<수정필요
#define WIFI_PASSWORD "WIFI비밀번호" // <<수정필요

// ===== AWS API 설정 =====
#define AWS_URL "/data" //<- 자신의 API URL  ex)"https://scku0upgj8.execute-api.us-east-1.amazonaws.com/data"

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

```

</details>

<details>
  
<summary>SENDER</summary>

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

</details>

<details open>
  
<summary>1.API 생성</summary>

ESP32모듈과 AWS서버를 연결하는 창구

<img width="1345" height="350" alt="AWS_tutorial_API_1" src="https://github.com/user-attachments/assets/997d0b3b-01c2-4361-ae56-4021f9fbe7ff" />

<img width="1301" height="339" alt="AWS_tutorial_API_2" src="https://github.com/user-attachments/assets/dffc8381-b36d-40d8-8f50-dddae16bdb99" />

<img width="1329" height="663" alt="AWS_tutorial_API_3" src="https://github.com/user-attachments/assets/3df1e7b8-808e-45ac-9595-7599ea53c6c8" />

<img width="1321" height="427" alt="AWS_tutorial_API_4" src="https://github.com/user-attachments/assets/e3a5b84e-71df-41fc-8305-77a5757df067" />

<img width="1323" height="501" alt="AWS_tutorial_API_5" src="https://github.com/user-attachments/assets/6437586b-4281-413f-81e8-5a27892e9b4e" />

<img width="1348" height="837" alt="AWS_tutorial_API_6" src="https://github.com/user-attachments/assets/bc785ab7-8e16-41e3-9435-7e48149a3610" />

<img width="1341" height="430" alt="AWS_tutorial_API_7" src="https://github.com/user-attachments/assets/0e1ca8dd-9e6e-4822-8543-5071ff71f704" />

</details>


```bash
https://eu-north-1.console.aws.amazon.com/lambda/home?region=eu-north-1#/begin
```

<details open>
  
<summary>2.Lambda 생성</summary>

<img width="1334" height="488" alt="AWS_tutorial_Lambda_1" src="https://github.com/user-attachments/assets/852bc2e3-0220-41f7-8ec6-73a45b3667ff" />

<img width="1340" height="1096" alt="AWS_tutorial_Lambda_2" src="https://github.com/user-attachments/assets/502dd84a-cacd-4680-a251-ebc092985881" />

<img width="1331" height="922" alt="AWS_tutorial_Lambda_3" src="https://github.com/user-attachments/assets/26028105-3a84-4cbd-b997-1c1edbfb18c7" />

</details>

<details open>
  
<summary>3.API → Lambda 연결</summary>

생성한 API  선택 
<img width="1347" height="434" alt="AWS_tutorial_Connect_1" src="https://github.com/user-attachments/assets/834fa5d0-3f5e-4345-9576-899f4b99564a" />

Develop > Routes 에서 “생선한 API”의 “생성” 클릭
<img width="1014" height="319" alt="AWS_tutorial_Connect_2" src="https://github.com/user-attachments/assets/3ca0ebfd-875e-43ee-ba37-a843057a4b62" />

경로 생성에서 [메서드 “POST”, 경로 “/data”] 생성
<img width="1001" height="318" alt="AWS_tutorial_Connect_3" src="https://github.com/user-attachments/assets/f4e5fb58-9883-4e99-b5b6-aff1bc2c26ac" />

Develop > Integrations 에서 통합 생성 및 연결 클릭
<img width="1347" height="521" alt="AWS_tutorial_Connect_4" src="https://github.com/user-attachments/assets/bdc6b942-4596-43f1-b531-3d388e6c8ee1" />

통합 유형에서 이전에 생성한 Lambda함수선택 후 생성
<img width="1341" height="479" alt="AWS_tutorial_Connect_5" src="https://github.com/user-attachments/assets/97907776-a432-4108-bac2-390b984e8562" />

<img width="1009" height="1017" alt="AWS_tutorial_Connect_6" src="https://github.com/user-attachments/assets/1219c518-cf43-42c9-977a-c0c68f157889" />

결과
<img width="1016" height="839" alt="AWS_tutorial_Connect_7" src="https://github.com/user-attachments/assets/76bbf896-5d17-4dfd-b5ba-acb8ca902cf6" />

검사

Deploy > Stages 에서 [URL호출주소]를 복사후
<img width="1322" height="484" alt="AWS_tutorial_Connect_8" src="https://github.com/user-attachments/assets/8b89daad-4759-4212-9821-ed1c9468d341" />

cmd/터미널 에서 아래 명령어 URL호출주소를 실행
```bash
curl -X POST 자신의URL/data -H "Content-Type: application/json" -d "{}"
```
예시
```bash
curl -X POST https://scku0upgj8.execute-api.us-east-1.amazonaws.com/data -H "Content-Type: application/json" -d "{}"
```
“Hello from Lambda” 가 출력되면 성공

<img width="1115" height="71" alt="AWS_tutorial_Connect_9" src="https://github.com/user-attachments/assets/b9fb40ff-938c-486d-8e52-83a894b81447" />

<img width="567" height="44" alt="AWS_tutorial_Connect_10" src="https://github.com/user-attachments/assets/d3f0b716-0e04-4928-960b-9329fee61b91" />


</details>
