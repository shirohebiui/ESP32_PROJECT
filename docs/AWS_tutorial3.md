변경 기록을 저장할 DynamoDB 테이블 생성

<img width="1571" height="401" alt="update_history1" src="https://github.com/user-attachments/assets/adc27407-23ba-497c-9623-e9e4095759e7" />
<img width="1881" height="1120" alt="update_history2" src="https://github.com/user-attachments/assets/3934a66f-49b6-4cb6-9695-61eccb38d40b" />
<img width="1553" height="279" alt="update_history3" src="https://github.com/user-attachments/assets/c0f8e859-a57d-4e2f-98e2-62c74dd52496" />



data_conver_to_Int 함수 수정

<img width="1683" height="1040" alt="data_conver_to_Int" src="https://github.com/user-attachments/assets/ec970e6b-6280-405d-922e-50541687020f" />

<details>
  
<summary>data_conver_to_Int 함수</summary>

```bash

import json
import boto3
import re
from datetime import datetime

dynamodb = boto3.resource('dynamodb')
table = dynamodb.Table('sensor_data')
history_table = dynamodb.Table('update_history')

def lambda_handler(event, context):
    try:
        body = json.loads(event.get('body', '{}'))
        raw  = body.get('data', '')

        # ===== {Ch:0,value:0} 문자열 파싱 =====
        ch_match    = re.search(r'Ch:(\d+)',    raw)
        value_match = re.search(r'value:(\d+)', raw)

        if not ch_match or not value_match:
            return {
                'statusCode': 400,
                'body': 'Parse Error : ' + raw
            }

        ch    = ch_match.group(1)
        value = value_match.group(1)
        timestamp = datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S')

        # ===== sensor_data =====
        table.put_item(
            Item={
                'id'        : ch,
                'value'     : value,
                'timestamp' : timestamp
            }
        )

        # ===== update_history =====
        history_table.put_item(
            Item={
                'id'        : ch,
                'value'     : value,
                'timestamp' : timestamp
            }
        )

        return {
            'statusCode': 200,
            'body': 'Save Success'
        }

    except Exception as e:
        return {
            'statusCode': 500,
            'body': 'Error : ' + str(e)
        }

```

</details>



Update_history에 접근하기위한 Lambda함수 생성

<img width="1606" height="329" alt="Updatehistory_Lambda_01" src="https://github.com/user-attachments/assets/73035259-1593-493e-89ee-d87579ab23b5" />
<img width="1599" height="1097" alt="Updatehistory_Lambda_02" src="https://github.com/user-attachments/assets/27f162a3-b28c-41f9-bf2d-2b56de5346d7" />
<img width="1597" height="1101" alt="Updatehistory_Lambda_03" src="https://github.com/user-attachments/assets/ecfb05a1-35bf-4c40-9a2d-add46aea6b54" />

<details>

<summary>access_to_DB_update_history 함수</summary>

```bash
import json
import boto3
from boto3.dynamodb.conditions import Key

dynamo = boto3.resource('dynamodb', region_name='us-east-1') # 지역
table = dynamo.Table('update_history') # <- 본인 테이블명으로 변경

def lambda_handler(event, context):
    params = event.get('queryStringParameters') or {}
    item_id = params.get('id', None)
    
    if item_id:
        result = table.query(
            KeyConditionExpression=Key('id').eq(item_id)
        )
    else:
        result = table.scan()
    
    return {
        'statusCode': 200,
        'headers': {
            'Access-Control-Allow-Origin': '*',
            'Content-Type': 'application/json'
        },
        'body': json.dumps(result['Items'], default=str)
    }
```

</details>
<img width="1590" height="734" alt="Updatehistory_Lambda_04" src="https://github.com/user-attachments/assets/3ef8faa3-99fd-4bac-86c3-6de92037fc2d" />
<img width="1591" height="755" alt="Updatehistory_Lambda_05" src="https://github.com/user-attachments/assets/05fe37e3-4740-4272-9396-1f65bc014a8e" />
<img width="1606" height="529" alt="Updatehistory_Lambda_06" src="https://github.com/user-attachments/assets/972f2812-08a3-47f2-8b57-c0dac6599866" />
<img width="1586" height="817" alt="Updatehistory_Lambda_07" src="https://github.com/user-attachments/assets/0df875a6-81e3-4152-be0a-6778ee849224" />
<img width="1588" height="503" alt="Updatehistory_Lambda_08" src="https://github.com/user-attachments/assets/f1d3e169-7648-4737-bea3-e46d9998f7c6" />
<img width="1605" height="1000" alt="Updatehistory_Lambda_09" src="https://github.com/user-attachments/assets/9362f9b3-d3e8-496e-8f54-b4c8df50c8e5" />
<img width="1593" height="575" alt="Updatehistory_Lambda_10" src="https://github.com/user-attachments/assets/d2431a41-2d86-4f22-9e1c-0df725ad898d" />

API GateWay 생성, Cors수정
<img width="1608" height="431" alt="Updatehistory_APIGateway_01" src="https://github.com/user-attachments/assets/fd2d8a38-bed6-41eb-808c-8fbf0a693d15" />
<img width="1605" height="306" alt="Updatehistory_APIGateway_02" src="https://github.com/user-attachments/assets/6c0f8d00-a752-4a71-a35d-0cc1aa1b2355" />
<img width="1607" height="414" alt="Updatehistory_APIGateway_03" src="https://github.com/user-attachments/assets/74677601-8fd4-4c94-bfe0-46c85c0a2e2a" />
<img width="1602" height="704" alt="Updatehistory_APIGateway_04" src="https://github.com/user-attachments/assets/39f2170e-baef-4ea3-8232-be945ce15298" />
<img width="1594" height="696" alt="Updatehistory_APIGateway_05" src="https://github.com/user-attachments/assets/36d237b8-858a-447b-b5e6-efa9f0b2a2bb" />
<img width="1607" height="688" alt="Updatehistory_APIGateway_06" src="https://github.com/user-attachments/assets/0dbc12ec-8828-4539-947f-042ab1e1c244" />
<img width="1703" height="827" alt="Updatehistory_APIGateway_07" src="https://github.com/user-attachments/assets/586a4fa5-57aa-4e87-b988-78bbce911f2a" />
<img width="1699" height="770" alt="Updatehistory_APIGateway_08" src="https://github.com/user-attachments/assets/dadf4346-81ae-4c23-8f0d-44310b3ca32e" />



