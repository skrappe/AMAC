from fastapi import FastAPI, Depends
from sqlalchemy.orm import Session
from database import Base, engine, SessionLocal, Drawer
from pydantic import BaseModel
from datetime import datetime
from typing import Optional
from typing import List
from fastapi.staticfiles import StaticFiles
from fastapi.responses import FileResponse
from updateSheet import log_drawer_update




app = FastAPI()
Base.metadata.create_all(bind=engine)
# Serve static files (like frontend)
app.mount("/static", StaticFiles(directory="static"), name="static")


class DrawerUpdate(BaseModel):
    drawer_id: str
    sensor_type: str
    status: str
    conductive: bool
    last_updated: Optional[datetime] = None

# Dependency to get DB session
def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()

@app.get("/")
def serve_frontend():
    return FileResponse("static/index.html")        


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

    print("Calling log_drawer_update...")
    log_drawer_update(
        drawer_id=update.drawer_id,
        item_name="ESP32 WROOM",
        sr_code="SR-123456",
        status=update.status
    )

    print("Update complete")
    return {"message": f"Drawer {update.drawer_id} updated"}


@app.get ("/api/drawers", response_model=List[DrawerUpdate])
def get_all_drawers(db: Session = Depends(get_db)):
        drawers = db.query(Drawer).all()
        return drawers