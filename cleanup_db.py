from sqlalchemy.orm import Session
from database import Drawer, SessionLocal

# Starter en session
db: Session = SessionLocal()

# Henter alle drawers undtagen drawer_011
drawers_to_delete = db.query(Drawer).filter(Drawer.drawer_id != "drawer_011").all()

for drawer in drawers_to_delete:
    print(f"Sletter {drawer.drawer_id}")
    db.delete(drawer)

db.commit()
print("✅ Rydning fuldført")
db.close()
