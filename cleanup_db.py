from sqlalchemy.orm import Session
from database import Drawer, SessionLocal
from datetime import datetime, timedelta

# Start database session
db: Session = SessionLocal()

# Beregn cutoff-tidspunkt: 31 dage tilbage fra nu
cutoff = datetime.utcnow() - timedelta(days=31)

# Find drawers der ikke er blevet opdateret siden cutoff
drawers_to_delete = db.query(Drawer).filter(Drawer.last_updated < cutoff).all()

print(f"🧹 Skuffer der ikke er opdateret siden {cutoff.isoformat()}:")
if not drawers_to_delete:
    print("Ingen gamle skuffer fundet.")
else:
    for drawer in drawers_to_delete:
        print(f"🗑️ Sletter {drawer.drawer_id} (sidst opdateret: {drawer.last_updated})")
        db.delete(drawer)
    db.commit()
    print("✅ Oprydning fuldført.")

# Luk session
db.close()
