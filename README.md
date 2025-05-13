<!-- This README file was developed to document the ESP32 Drawer Monitoring System. -->
<!-- We have used AI, specifically OpenAI's ChatGPT, to assist with refining the structure, -->
<!-- improving readability, including comments, and ensuring consistency in the documentation. -->
<!-- All content complies with ITU's instructions regarding Generative AI. -->

# ESP32 Drawer Monitoring System
This project provides a backend and dashboard system for monitoring the occupancy status of component drawers using WiFi-enabled ESP32 microcontrollers. It was developed as part of a bachelor thesis at the IT University of Copenhagen.

## Project Overview

The system receives occupancy data from ultrasonic sensors mounted on drawers. The data is processed and stored in a cloud hosted PostgreSQL database. A web based dashboard provides real time visibility, and monthly summary reports are sent automatically via email. Status changes are also logged to an external Google Sheet to support existing workflows, aswell as a psysical notification via LED lights, that indicate weather the drawers are empty or not.

The goal was to build a simple, scalable, and low maintenance system suitable for lab environments such as ITU's REAL Lab.

## Features

- FastAPI backend to receive and store data from ESP32 devices
- PostgreSQL database hosted on Render for persistent storage
- Static HTML and JavaScript frontend dashboard
- ESP32 integration with ultrasonic sensors and LED indicators
- Automated monthly status report emails (using SendGrid + GitHub Actions)
- Google Sheets integration for external log visibility

## Repository Structure

AMAC/
├── .github/
│   └── workflows/
│       └── monthly_report.yml       
├── Backend/
│   ├── __pycache__/                 
│   ├── .venv/                       
│   ├── credentials/                 
│   ├── static/
│   │   └── index.html               
│   ├── cleanup_db.py                
│   ├── create_db.py                 
│   ├── database.py                  
│   ├── main.py                      
│   ├── monthly_report.py            
│   ├── Overview.md                  
│   ├── README.md                    
│   ├── requirements.txt             
│   ├── simulate_updates.py          
│   └── updateSheet.py               
├── LED_breadboard_setup/
│   ├── platformio.ini               
│   ├── src/
│   │   └── main.cpp                 
│   └── test/                        
├── multi_hc-sr04_sensor_setup/
│   ├── platformio.ini               
│   ├── src/
│   │   └── multi_hc_sr04_ultralyd.cpp 
│   └── test/                        
├── single_hc-sr04_sensor_setup/
│   ├── platformio.ini               
│   ├── src/
│   │   └── main.cpp                 
│   └── test/                        
└── requirements.txt                 


## Installation and Setup

### 1. Clone the Repository

git clone <repo url>
cd <cd repo folder>


### 2. Create a Virtual Environment (optional but recommended. For global installation: pip install -r requirements.txt)

# on Mac: 
python -m venv venv
source venv/bin/activate 
# On Windows: 
venv\Scripts\activate

### 3. Install Dependencies
pip install -r requirements.txt


### 4. Set Environment Variables
The following variables are required:

SENDGRID_API_KEY=<api-key>
DATABASE_URL=<postgresql-database-url>


We do not use a .env file locally to store any secret keys etc. Instead all enviorment variables are securely managed via Render, Github secrets and Google cloud IAM. 

### 5. Run the Backend Server

uvicorn Backend.main:app --reload

The local dashboard will now be available:
- Dashboard: `http://127.0.0.1:8000/`

In our case, and when running through render, is is available on: https://amac.onredner.com/

## Monthly Email Reporting

The system sends a monthly email summary on the first day of each month at 12:00 PM UTC. This is handled using a GitHub Actions workflow that runs the `monthly_report.py` script and sends the email via SendGrid.

To run the script manually:
python Backend/monthly_report.py


## Google Sheets Integration

Drawer updates are mirrored to a shared Google Sheet using the Google Sheets API.
Access and sharing are controlled using a Google service account with read and write permissions.

## Dashboard

The dashboard is a static HTML page served directly from the backend. It:

- Displays all drawer statuses in a table format
- Automatically refreshes every 5 seconds
- Uses red/green color indicators for empty or occupied drawers

The dashboard is located in `Backend/static/index.html` and requires no additional setup.

## ESP32 Integration
Each ESP32 microcontroller:

- Connects to Wi-Fi and sends POST requests to the backend
- Reads data using an HC-SR04 ultrasonic sensor
- Controls red/green LEDs to provide local status feedback
- Operates independently, requiring no central coordination
- ESP32 firmware is developed using PlotformIO 

#### Steps to Upload Firmware:
1. Install PlatformIO:
   pip install platformio
   cd <desired project>
   pio run --target upload
   pio device monitor

## Technology Stack
- Python 3.11+
- FastAPI
- SQLAlchemy
- PostgreSQL
- Uvicorn
- SendGrid
- Google Sheets API
- HTML/CSS/JavaScript
- GitHub Actions

## Functional Highlights
- Real time drawer status via HTTP API
- Low latency polling dashboard with no frontend framework
- Scalable backend
- Scheduled email reports without maintaining an internal scheduler

## Usage Notes
- Network connectivity is assumed for both ESP32 devices and the backend
- For production use, security features such as authentication and HTTPS should be added

## License
This project was developed for academic purposes. License and reuse terms can be added here as needed.
