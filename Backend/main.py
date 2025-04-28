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
class LogMessage(BaseModel):
    log: str

class DrawerUpdate(BaseModel):
    drawer_id: str
    item_name: str
    sr_code: str
    status: str
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

@app.post("/api/log")
async def receive_log(log_entry: LogMessage):
    """
    Receives a simple log message from a client (e.g., microcontroller)
    and outputs it to the application logs.
    """
    # This is where you output the received log message to standard output/error
    # so it appears in your Render logs dashboard.
    print(f"MICROCONTROLLER LOG: {log_entry.log}")

    # You can add more sophisticated logging here using the 'logging' module if you prefer
    # import logging
    # logger = logging.getLogger(__name__)
    # logger.info(f"Microcontroller Log: {log_entry.log}")


    # You can return a simple success response
    return {"status": "Log message received successfully"}    


@app.post("/api/drawers/update", response_model=DrawerUpdate)
def update_drawer(update: DrawerUpdate, db: Session = Depends(get_db)):
    print("Received update:", update)

    drawer = db.query(Drawer).filter(Drawer.drawer_id == update.drawer_id).first()
    if drawer:
        print("Drawer found, updating...")
        drawer.status = update.status
        drawer.item_name = update.item_name
        drawer.sr_code = update.sr_code
        drawer.last_updated = update.last_updated or datetime.utcnow()
    else:
        print("Drawer not found, creating new...")
        drawer = Drawer(
            drawer_id=update.drawer_id,
            item_name=update.item_name,
            sr_code=update.sr_code,
            status=update.status,
            last_updated=update.last_updated or datetime.utcnow()
        )
        db.add(drawer)
    db.commit()

    print("Calling log_drawer_update...")
    log_drawer_update(
        drawer_id=update.drawer_id,
        item_name=update.item_name,
        sr_code=update.sr_code,
        status=update.status
    )

    print("Update complete")
    return DrawerUpdate(
    drawer_id=drawer.drawer_id,
    item_name=drawer.item_name,
    sr_code=update.sr_code,
    status=drawer.status,
    last_updated=drawer.last_updated
)



@app.get ("/api/drawers", response_model=List[DrawerUpdate])
def get_all_drawers(db: Session = Depends(get_db)):
        drawers = db.query(Drawer).all()
        return drawers