AWS API <-> Labmda <-> DB
DB에 저장된 데이터에 접근

1.DB에 접근하여 데이터를 가져오기위한 Lambda함수생성
Lambda함수 페이지에서 함수생성

<img width="1045" height="509" alt="Lambda1" src="https://github.com/user-attachments/assets/c3608dbb-b51c-4bb5-b805-1222ce083cde" />

함수이름 설정, 런타임 Python3.12버전 설정 후 함수생성

<img width="1026" height="1135" alt="Lambda2" src="https://github.com/user-attachments/assets/7840d5d9-6a7e-43a8-a3ef-fdc58dbabba9" />


동작할 코드를 입력
'테이블이름'과 '리전(region_name)'확인 자신에 맞는 이름과 지역으로 수정

<img width="1026" height="1137" alt="Lambda3" src="https://github.com/user-attachments/assets/ada19f24-6085-4754-851c-d8ffa760fe35" />


<details>
  
<summary>Lambda함수 (python3.12)</summary>

```bash

import json
import boto3

dynamodb = boto3.resource('dynamodb', region_name='us-east-1')  # 서울 리전
table = dynamodb.Table('sensor_data')  # ← 본인 테이블명으로 변경

def lambda_handler(event, context):
    response = table.scan()  # 전체 데이터 가져오기
    items = response['Items']
    
    return {
        'statusCode': 200,
        'headers': {
            'Access-Control-Allow-Origin': '*',  # CORS 허용
            'Content-Type': 'application/json'
        },
        'body': json.dumps(items, default=str)  # 날짜형식 등 자동 변환
    }

```
</details>

2.DB접근권한 부여
IAM정책페이지 검색 및 이동

<img width="1263" height="208" alt="IAM1" src="https://github.com/user-attachments/assets/75a68d0e-5dab-475a-ba64-c686586f0ccd" />


액세스관리 -> 역할 -> DB에 연결하기 위해 생성한 함수 선택

<img width="1374" height="592" alt="IAM2" src="https://github.com/user-attachments/assets/22e7d0ba-f083-4088-b687-27b339dd39aa" />


권한추가 -> 정책연결

<img width="1377" height="701" alt="IAM3" src="https://github.com/user-attachments/assets/c78c99fb-363f-43e5-a008-c28d71ae8582" />


AmazonDynamoOBReadOnlyAccess 권한추가

<img width="1374" height="590" alt="IAM4" src="https://github.com/user-attachments/assets/044c7e47-5d1c-4ef7-972a-4100a1d3a247" />

결과 확인

<img width="1041" height="774" alt="IAM5" src="https://github.com/user-attachments/assets/e28eda44-13d4-4d85-9edb-4af84955dcd9" />


3.API설정
기존 생성한 HTTP API를 활용

<img width="1375" height="354" alt="API1" src="https://github.com/user-attachments/assets/5d541732-c043-439b-a734-86e66859995b" />


경로 -> 생성

<img width="410" height="426" alt="API2" src="https://github.com/user-attachments/assets/adaf2a9e-d96b-49d4-a709-774e3744021f" />


메서드 GET, 경로 /data 생성

<img width="1052" height="363" alt="API3" src="https://github.com/user-attachments/assets/b60e1815-5d71-47b9-a67e-733ffec56a23" />


GET->통합연결

<img width="1031" height="616" alt="API4" src="https://github.com/user-attachments/assets/7b6ac32a-48d9-475f-90ec-b85f89d15e3e" />


통합 생성 및 연결

<img width="1039" height="622" alt="API5" src="https://github.com/user-attachments/assets/f235ee49-a2f6-4590-ada6-5dc38059a444" />


통합유형 'Lambda함수' -> DB연결을 위해 생성한 함수 선택 후 생성

<img width="1051" height="1053" alt="API6" src="https://github.com/user-attachments/assets/75bbba10-fd88-41c6-b770-082114d78b32" />


결과 확인

<img width="1044" height="934" alt="API7" src="https://github.com/user-attachments/assets/f0985349-b50a-47cd-a569-d7fb2a1d5d9d" />

4.(선택)github Page기능 활용하여 DB를 시각화하여 조회

gitHub설정










































