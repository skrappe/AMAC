from sqlalchemy import create_engine, Column, String, Boolean, DateTime
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker
from datetime import datetime
import os

DATABASE_URL = os.getenv("DATABASE_URL", "postgresql://amac_user:GhQAElY3mBVbSUaovKGUdat2ReC2J3Wf@dpg-d05mmnili9vc738vc69g-a.frankfurt-postgres.render.com/amac_db")
if not DATABASE_URL:
    raise ValueError("DATABASE_URL is not set")

engine = create_engine(DATABASE_URL)
SessionLocal = sessionmaker(bind=engine)

Base = declarative_base()

class Drawer(Base):
    __tablename__ = "drawers"

    drawer_id = Column(String, primary_key=True, index=True)
    sensor_type = Column(String)
    item_name = Column(String)
    sr_code = Column(String)
    status = Column(String)
    last_updated = Column(DateTime, default=datetime.utcnow)
