from fastapi import FastAPI, Depends
from sqlalchemy.orm import Session
from database import Base, engine, SessionLocal, Drawer
from pydantic import BaseModel
from datetime import datetime
from typing import Optional

app = FastAPI()
Base.metadata.create_all(bind=engine)

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

@app.post("/api/drawers/update")
def update_drawer(update: DrawerUpdate, db: Session = Depends(get_db)):
    drawer = db.query(Drawer).filter(Drawer.drawer_id == update.drawer_id).first()
    if drawer:
        drawer.sensor_type = update.sensor_type
        drawer.status = update.status
        drawer.conductive = update.conductive
        drawer.last_updated = update.last_updated or datetime.utcnow()
    else:
        drawer = Drawer(
            drawer_id=update.drawer_id,
            sensor_type=update.sensor_type,
            status=update.status,
            conductive=update.conductive,
            last_updated=update.last_updated or datetime.utcnow()
        )
        db.add(drawer)
    db.commit()
    return {"message": f"Drawer {update.drawer_id} updated"}
