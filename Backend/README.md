# ESP32 Drawer Monitoring System

This project provides a backend and dashboard for monitoring electronic component drawers using ESP32 Wi-Fi-enabled sensors.

## 📦 Features

- FastAPI backend to receive drawer updates
- SQLite database for storing current drawer status
- HTML/JS frontend dashboard served from the same server
- Simulation script to generate fake ESP32 drawer data

---

## 🚀 Getting Started

### 1. Clone the Repository


git clone <your-repo-url>
cd <your-repo-folder>

### 2. Create a Virtual Environment (Optional but Recommended)
python -m venv venv
source venv/bin/activate         # On Windows: venv\Scripts\activate

### 3. Install Dependencies
pip install -r requirements.txt

### 4. Run the Backend Server
uvicorn main:app --reload

Visit the dashboard in your browser:
👉 http://127.0.0.1:8000/

API docs (Swagger):
👉 http://127.0.0.1:8000/docs

### 5. Simulate Sensor Data (Optional)
This script mimics ESP32 sensors sending updates:

python simulate_updates.py
It will send multiple POST requests to /api/drawers/update with randomized data.

### PlatformIO 
pio run --target upload 
pio device monitor