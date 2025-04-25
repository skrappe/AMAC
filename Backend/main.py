from fastapi import FastAPI, Depends
from sqlalchemy.orm import Session
from database import Base, engine, SessionLocal, Drawer
from pydantic import BaseModel
from datetime import datetime
from typing import Optional, List
from fastapi.staticfiles import StaticFiles
from fastapi.responses import FileResponse
from updateSheet import log_drawer_update
import os
from sendgrid import SendGridAPIClient
from sendgrid.helpers.mail import Mail

# --- SendGrid Email Funktion ---
def send_notification_email(drawer_id, status):
    message = Mail(
        from_email='monie@itu.dk',  
        to_emails='monie@itu.dk',             
        subject=f'Skuffe {drawer_id} status opdateret',
        html_content=f'<strong>Status: {status}</strong>'
    )
    try:
        sg = SendGridAPIClient(os.getenv('SENDGRID_API_KEY'))
        response = sg.send(message)
        print(f"✅ Email sendt: Status {response.status_code}")
    except Exception as e:
        print(f"❌ Email fejl: {str(e)}")

# --- FastAPI App ---
app = FastAPI()
Base.metadata.create_all(bind=engine)
app.mount("/static", StaticFiles(directory="static"), name="static")

# --- Data Model ---
class DrawerUpdate(BaseModel):
    drawer_id: str
    sensor_type: str
    status: str
    conductive: bool
    last_updated: Optional[datetime] = None

# --- DB Dependency ---
def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()

# --- Frontend ---
@app.get("/")
def serve_frontend():
    return FileResponse("static/index.html")

# --- API Endpoint ---
@app.post("/api/drawers/update", response_model=DrawerUpdate)
def update_drawer(update: DrawerUpdate, db: Session = Depends(get_db)):
    print("Received update:", update)

    drawer = db.query(Drawer).filter(Drawer.drawer_id == update.drawer_id).first()
    if drawer:
        print("Drawer found, updating...")
        drawer.sensor_type = update.sensor_type
        drawer.status = update.status
        drawer.conductive = update.conductive
        drawer.last_updated = update.last_updated or datetime.utcnow()
    else:
        print("Drawer not found, creating new...")
        drawer = Drawer(
            drawer_id=update.drawer_id,
            sensor_type=update.sensor_type,
            status=update.status,
            conductive=update.conductive,
            last_updated=update.last_updated or datetime.utcnow()
        )
        db.add(drawer)
    db.commit()

    # --- Sheets Log ---
    print("Calling log_drawer_update...")
    log_drawer_update(
        drawer_id=update.drawer_id,
        item_name="ESP32 WROOM",
        sr_code="SR-123456",
        status=update.status
    )

    # --- Send Email ---
    send_notification_email(update.drawer_id, update.status)

    print("Update complete")
    return DrawerUpdate(
        drawer_id=drawer.drawer_id,
        sensor_type=drawer.sensor_type,
        status=drawer.status,
        conductive=drawer.conductive,
        last_updated=drawer.last_updated
    )

# --- Get All Drawers ---
@app.get("/api/drawers", response_model=List[DrawerUpdate])
def get_all_drawers(db: Session = Depends(get_db)):
    drawers = db.query(Drawer).all()
    return drawers
