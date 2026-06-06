변경 기록을 저장할 DynamoDB 테이블 생성


<img width="1571" height="401" alt="update_history1" src="https://github.com/user-attachments/assets/adc27407-23ba-497c-9623-e9e4095759e7" />
<img width="1881" height="1120" alt="update_history2" src="https://github.com/user-attachments/assets/3934a66f-49b6-4cb6-9695-61eccb38d40b" />
<img width="1553" height="279" alt="update_history3" src="https://github.com/user-attachments/assets/c0f8e859-a57d-4e2f-98e2-62c74dd52496" />


data_conver_to_Int 함수 수정
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