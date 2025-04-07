import serial
import requests
from datetime import datetime

PORT = "/dev/cu.usbserial-0001"  # ← skift hvis din port er anderledes!
BAUD = 115200
API_URL = "http://127.0.0.1:8000/api/drawers/update"

ser = serial.Serial(PORT, BAUD, timeout=1)
print(f"🔌 Forbundet til {PORT} @ {BAUD}")

last_sent_status = None

while True:
    try:
        line = ser.readline().decode().strip()
        if not line:
            continue

        print(f"📥 ESP32 siger: {line}")

        if line != last_sent_status:
            payload = {
                "drawer_id": "test_switch_01",
                "sensor_type": "switch",
                "status": line,
                "conductive": False,
                "last_updated": datetime.utcnow().isoformat()
            }

            response = requests.post(API_URL, json=payload)
            print(f"✅ Sendt til API: {response.status_code}")
            last_sent_status = line

    except Exception as e:
        print(f"❌ Fejl: {e}")
