import gspread
from google.oauth2.service_account import Credentials
from datetime import datetime

# Angiv sti til din credentials JSON
CREDENTIALS_PATH = "Backend/credentials/amac-credentials.json"

# Sheet ID og ark navn
SPREADSHEET_ID = "13REM3pUdVp14TV_xDA7fzTK30Rzu5vsCnQXgik9GZgs"
SHEET_NAME = "AMAC - Parts to buy"

# Setup adgang til Google Sheets
scopes = ["https://www.googleapis.com/auth/spreadsheets"]
credentials = Credentials.from_service_account_file(CREDENTIALS_PATH, scopes=scopes)
gc = gspread.authorize(credentials)

# Åbn arket
spreadsheet = gc.open_by_key(SPREADSHEET_ID)
sheet = spreadsheet.worksheet(SHEET_NAME)

# Eksempel på hvad vi vil tilføje
drawer_id = "drawer_ultra_s3"
item_name = "ESP32 WROOM"
sr_code = "SR-123456"
status = "TOM"
timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

# Tilføj ny række
sheet.append_row([drawer_id, item_name, sr_code, status, timestamp])

print("✅ Skuffestatus sendt til arket!")
