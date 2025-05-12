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
    empty = [d for d in drawers if d.status.lower() == "empty"]
    filled = [d for d in drawers if d.status.lower() == "item_detected"]

    html = """
    <html><head>
        <style>
            body {{ font-family: 'Segoe UI', sans-serif; background-color: #f9f9f9; padding: 40px; color: #333; }}
            h2 {{ color: #2c3e50; }}
            h3 {{ margin-top: 30px; }}
            p {{ font-size: 16px; }}
            table {{ border-collapse: collapse; width: 100%; margin-top: 10px; background: #fff; }}
            th, td {{ padding: 12px; border: 1px solid #ddd; text-align: left; }}
            th {{ background-color: #2c3e50; color: white; }}
            tr:nth-child(even) {{ background-color: #f2f2f2; }}
            .empty {{ color: #e74c3c; font-weight: bold; }}
            .item_detected {{ color: #27ae60; font-weight: bold; }}
        </style>
    </head><body>
        <h2>🗄️ Monthly Drawer Status</h2>

        <p>
            ✅ <strong>{filled_count}</strong> skuffe(r) har dele<br>
            ⚠️ <strong>{empty_count}</strong> skuffe(r) mangler dele
        </p>
    """.format(filled_count=len(filled), empty_count=len(empty))

    def render_table(title, drawers_list):
        if not drawers_list:
            return "<p>Ingen skuffer i denne kategori.</p>"
        rows = "".join(f"""
            <tr>
                <td>{d.item_name}</td>
                <td class="{d.status.lower()}">{d.status}</td>
                <td>{d.last_updated.strftime('%Y-%m-%d %H:%M')}</td>
            </tr>
        """ for d in drawers_list)
        return f"""
            <h3>{title}</h3>
            <table>
                <tr><th>Item</th><th>Status</th><th>Last update</th></tr>
                {rows}
            </table>
        """

    html += render_table("⚠️ Skuffer der mangler dele", empty)
    html += render_table("✅ Skuffer med dele", filled)

    html += "</body></html>"
    return html


# --- Send Email ---
def send_monthly_email(html_content):
    print("✅ SendGrid KEY found:", os.getenv("SENDGRID_API_KEY") is not None)
    message = Mail(
        from_email='skrappe9@hotmail.com',
        to_emails=['skrappe9@gmail.com','toft814@gmail.com'],
        subject='Monthly Drawer Status Report',
        html_content=html_content,
        plain_text_content="Se status for alle skuffer i den vedhæftede HTML-tabel."
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



# This file generates and sends monthly email reports for the ESP32 Drawer Monitoring System.
# We have used AI, specifically OpenAI's ChatGPT, to refine the structure of the code,
# improve documentation, and ensure consistency in the logic.
# All code complies with ITU's instructions regarding Generative AI.