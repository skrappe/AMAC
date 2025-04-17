# Backend/updateSheet.py

import gspread
from google.oauth2.service_account import Credentials
from datetime import datetime

# Constants
CREDENTIALS_PATH = "Backend/credentials/amac-credentials.json"
SPREADSHEET_ID = "13REM3pUdVp14TV_xDA7fzTK30Rzu5vsCnQXgik9GZgs"
SHEET_NAME = "AMAC - Parts to buy"

# Setup credentials once (can be reused)
scopes = ["https://www.googleapis.com/auth/spreadsheets"]
credentials = Credentials.from_service_account_file(CREDENTIALS_PATH, scopes=scopes)
gc = gspread.authorize(credentials)
spreadsheet = gc.open_by_key(SPREADSHEET_ID)
sheet = spreadsheet.worksheet(SHEET_NAME)

def log_drawer_update(drawer_id, item_name, sr_code, status):
    print(f"Logging update: {drawer_id}, {item_name}, {sr_code}, {status}")
    return  # <- Fjern alt andet for at isolere fejlen

