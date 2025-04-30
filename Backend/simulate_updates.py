import requests
import random
from datetime import datetime, timezone, timedelta
import time

# API endpoint
API_URL = "http://127.0.0.1:8000/api/drawers/update"
time_zone = timezone(timedelta(hours=2))
# Simulate 10 random drawer updates (scale up to 100 as needed)
for i in range(1, 11):
    drawer_id = f"drawer_{i:03d}"
    sensor_type = random.choice(["capacitive", "infrared", "strain_gauge", "ultrasound"])
    status = random.choice(["empty", "item_detected"])
    conductive = random.choice([True, False])
    payload = {
        "drawer_id": drawer_id,
        "sensor_type": sensor_type,
        "status": status,
        "conductive": conductive,
        "last_updated": datetime.now(time_zone)
    }

    response = requests.post(API_URL, json=payload)
    print(f"[{drawer_id}] Status: {response.status_code} - {response.json()}")
    time.sleep(0.2)  # short delay to simulate network interval
