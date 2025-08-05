import json
import boto3
from datetime import datetime

timestamp = int(datetime.utcnow().timestamp())  # Send as UNIX timestamp (e.g., 1722247573)

dynamodb = boto3.resource('dynamodb')
table = dynamodb.Table('pillLogs')
sns = boto3.client('sns')

SNS_TOPIC_ARN = 'arn:aws:sns:ap-south-1:763240034378:PillboxAlerts'

def lambda_handler(event, context):
    print("Received event:", event)

    # Parse the incoming message
    payload = json.loads(event['Records'][0]['Sns']['Message']) if 'Records' in event else event

    device_id = payload.get('device_id', 'unknown')
    compartment = payload.get('compartment', 'N/A')
    status = payload.get('status', 'unknown')
    time_taken = payload.get('time', '00:00')
    # timestamp = datetime.utcnow().isoformat()
    timestamp = int(datetime.utcnow().timestamp())  # Send as UNIX timestamp (e.g., 1722247573)

    # Store in DynamoDB
    table.put_item(
    Item={
        'device_id': device_id,
        'timestamp': timestamp,  # No quotes! Send as number
        'compartment': compartment,
        'status': status,
        'time': time_taken
    }
)


    # Send alert
    if status.lower() == "missed":
        message = f"⚠️ Alert! Pill from Compartment {compartment} was missed at {time_taken}."
        sns.publish(
            TopicArn=SNS_TOPIC_ARN,
            Message=message,
            Subject="Pill Missed Alert"
        )
    else:
        message = f"Pill from Compartment {compartment} was taken at {time_taken}."
        sns.publish(
            TopicArn=SNS_TOPIC_ARN,
            Message=message,
            Subject="Pill Taken Alert"
        )

    return {
        'statusCode': 200,
        'body': json.dumps('Logged Successfully')
    }
