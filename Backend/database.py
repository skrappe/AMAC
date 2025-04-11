from sqlalchemy import create_engine, Column, String, Boolean, DateTime
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker
from datetime import datetime

DATABASE_URL = "sqlite:///./drawers.db"

engine = create_engine(DATABASE_URL, connect_args={"check_same_thread": False})
SessionLocal = sessionmaker(bind=engine)

Base = declarative_base()

class Drawer(Base):
    __tablename__ = "drawers"

    drawer_id = Column(String, primary_key=True, index=True)
    sensor_type = Column(String)
    status = Column(String)
    conductive = Column(Boolean)
    last_updated = Column(DateTime, default=datetime.utcnow)
