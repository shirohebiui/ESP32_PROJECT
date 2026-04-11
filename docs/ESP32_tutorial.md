환경
IDE : ArduinoIDE

Module : ESP32 CP2102


<img width="1009" height="646" alt="ESP32_CP20102_PIN_MAP" src="https://github.com/user-attachments/assets/ccaba6bb-5eda-4abc-b81e-f83096b9242d" />

USB 드라이버 : CP210x Universal Windows Driver
```bash
https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads
```


상단메뉴 → 기본설정 → 추가 보드 관리자 URL

아래 URL추가

```bash
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

<img width="798" height="530" alt="ESP32_setting_1" src="https://github.com/user-attachments/assets/a85c84c4-147f-4342-927b-dabd38bfd2b9" />

도구 → 보드 → 보드 매니저 → 검색 ( ESP32 ) → “esp32 by Espressif” 설치

<img width="221" height="440" alt="ESP32_setting_2" src="https://github.com/user-attachments/assets/013eee87-d091-407b-8e2f-ce08a4b6e4b3" />

도구 → 보드 → esp32 → ESP32 Dev Module 선택

<img width="900" height="147" alt="ESP32_setting_3" src="https://github.com/user-attachments/assets/773349bf-c888-4a0b-b0e6-08e169da2a88" />

시리얼 모니터 속도 115200baud설정 안할시 속도 불일치로 값을 제대로 못읽는다.
시리얼 모니터의 스피드를 1150200baud로 변경

<img width="714" height="256" alt="Serial_monitor_speed_1" src="https://github.com/user-attachments/assets/85f4982b-2fa6-4a9f-b1a9-381bfb79f9e1" />

<img width="1440" height="802" alt="Serial_monitor_speed_2" src="https://github.com/user-attachments/assets/a14f24cf-cd0e-421d-8f8a-6091613cca48" />




