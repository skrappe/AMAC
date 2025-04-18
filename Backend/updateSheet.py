# Backend/updateSheet.py
import os
import gspread
from google.oauth2.service_account import Credentials
from datetime import datetime

# --- Credentials setup ---
CREDENTIALS_PATH = os.getenv("GOOGLE_CREDENTIALS_PATH", "Backend/credentials/amac-credentials.json").strip()
scopes = ["https://www.googleapis.com/auth/spreadsheets"]

credentials = Credentials.from_service_account_file(CREDENTIALS_PATH, scopes=scopes)
gc = gspread.authorize(credentials)

# --- Spreadsheet config ---
SPREADSHEET_ID = "13REM3pUdVp14TV_xDA7fzTK30Rzu5vsCnQXgik9GZgs"
SHEET_NAME = "AMAC - Parts to buy"
sheet = gc.open_by_key(SPREADSHEET_ID).worksheet(SHEET_NAME)

# --- Logging function ---
def log_drawer_update(drawer_id, item_name, sr_code, status):
    now = datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S")
    new_row = [now, drawer_id, item_name, sr_code, status]
    print(f"Logging update: {new_row}")
    sheet.append_row(new_row)

