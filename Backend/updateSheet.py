# Backend/updateSheet.py
import os
import gspread
from google.oauth2.service_account import Credentials
from datetime import datetime, timedelta, timezone

time_zone = timezone(timedelta(hours=2))
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
    now = datetime.now(time_zone).strftime("%Y-%m-%d %H:%M:%S")
    all_records = sheet.get_all_records()

    row_found = False
    incoming_id = drawer_id.strip().lower()

    for idx, record in enumerate(all_records, start=2):  # Header i række 1
        existing_id = str(record.get('Drawer ID', '')).strip().lower()
        if existing_id == incoming_id:
            print(f"🔄 Opdaterer række {idx} for {drawer_id}")
            sheet.update(f"A{idx}:E{idx}", [[now, drawer_id, item_name, sr_code, status]])
            row_found = True
            break

    if not row_found:
        print(f"➕ Tilføjer ny række for {drawer_id}")
        sheet.append_row([now, drawer_id, item_name, sr_code, status])

