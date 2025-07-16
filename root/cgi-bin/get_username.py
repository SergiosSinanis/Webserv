#!/usr/bin/env python3
import os
import csv
import sys
import json

csv_path = os.path.join(os.path.dirname(__file__), "../../backend/users.csv")
session_id = os.environ.get("SESSION_ID", "")
username = ""

found = False
with open(csv_path, "r", newline='') as f:
    reader = csv.reader(f)
    for row in reader:
        if len(row) < 4:
            continue
        stored_username, stored_email, stored_password, stored_session_id = row
        if session_id == stored_session_id:
            username = stored_username
            found = True
            break

if found:
    response_data = {"username": username}
else:
    response_data = {"error": "Invalid session"}

response_json = json.dumps(response_data)

sys.stdout.write("Content-Type: application/json\r\n\r\n")
sys.stdout.write(response_json)
sys.stdout.flush()
