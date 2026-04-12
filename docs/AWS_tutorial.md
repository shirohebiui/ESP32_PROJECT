# API Gateway 생성

```bash
https://eu-north-1.console.aws.amazon.com/apigateway/main/welcome?region=eu-north-1
```



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
