# monthly_report.py
import os
from sqlalchemy import create_engine
from database import Drawer, Base
from sqlalchemy.orm import sessionmaker
from sendgrid import SendGridAPIClient
from sendgrid.helpers.mail import Mail

# --- DB Setup ---
DATABASE_URL = os.getenv("DATABASE_URL", "postgresql://amac_user:GhQAElY3mBVbSUaovKGUdat2ReC2J3Wf@dpg-d05mmnili9vc738vc69g-a.frankfurt-postgres.render.com/amac_db")  
engine = create_engine(DATABASE_URL)
SessionLocal = sessionmaker(bind=engine)

# --- Hent Skuffe Status ---
def get_drawer_statuses():
    db = SessionLocal()
    drawers = db.query(Drawer).all()
    db.close()
    return drawers

# --- Lav HTML Rapport ---
def generate_html(drawers):
    html = "<h2>Monthly Drawer Status</h2>"
    html += "<table border='1' cellpadding='5'><tr><th>Drawer ID</th><th>Status</th><th>Last update</th></tr>"
    for d in drawers:
        html += f"<tr><td>{d.drawer_id}</td><td>{d.status}</td><td>{d.last_updated.strftime('%Y-%m-%d %H:%M')}</td></tr>"
    html += "</table>"
    return html

# --- Send Email ---
def send_monthly_email(html_content):
    message = Mail(
        from_email='monie@itu.dk',
        to_emails='monie@itu.dk',
        subject='Monthly Drawer Status Report',
        html_content=html_content
    )
    try:
        sg = SendGridAPIClient(os.getenv('SENDGRID_API_KEY'))
        response = sg.send(message)
        print(f"✅ Monthly mail sent! Status: {response.status_code}")
    except Exception as e:
        print(f"❌ Error on send: {e}")

# --- Main ---
if __name__ == "__main__":
    drawers = get_drawer_statuses()
    html = generate_html(drawers)
    send_monthly_email(html)
