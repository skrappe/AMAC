# This code snippet is for demonstration purposes,
# you need to decide where to put and execute it.

from database import Base, engine
# You might also need to import your models to ensure they are
# registered with Base.metadata if they aren't already imported
# in database.py in a way that makes them accessible here.
# from database import Drawer # Example

print("Attempting to create database tables...")
Base.metadata.create_all(engine)
print("Database tables creation attempted.")

# If you need to drop tables before creating (BE CAREFUL - DATA LOSS!)
try:
    print("Attempting to drop all database tables...")
    Base.metadata.drop_all(engine)
    print("Database tables dropped.")
except Exception as e:
    print(f"Error dropping tables: {e}")

try:
    print("Attempting to create database tables again...")
    Base.metadata.create_all(engine)
    print("Database tables created.")
except Exception as e:
    print(f"Error creating tables: {e}")