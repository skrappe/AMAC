# create_db.py

from database import Base, engine

# Hvis dine modeller ikke automatisk importeres i database.py, skal du evt. importere dem her:
# from database import Drawer

print("Attempting to create database tables...")
Base.metadata.create_all(engine)
print("Database tables creation attempted.")

# Hvis du vil droppe og genskabe tabeller (ADVARSEL: sletter data!)
#try:
   # print("Attempting to drop all database tables...")
   # Base.metadata.drop_all(engine)
   # print("Database tables dropped.")
#except Exception as e:
    #print(f"Error dropping tables: {e}")

#try:
   # print("Attempting to create database tables again...")
    #Base.metadata.create_all(engine)
   # print("Database tables created.")
#except Exception as e:
    #print(f"Error creating tables: {e}")
