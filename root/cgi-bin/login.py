import cgi
import csv
import os
import sys

form = cgi.FieldStorage()
username = form.getvalue("username")
password = form.getvalue("password")

csv_path = os.path.join(os.path.dirname(__file__), "../../backend/users.csv")

if not username or not password:
    print("Status: 302")
    print("Content-Type: text/html")
    print("Location: http://" + os.environ.get("SERVER_NAME", "") + ":" + os.environ.get("SERVER_PORT", "") + "/docs/field_error.html")
    sys.stdout.write("\r\n\r\n")
else:
    found = False
    updated_rows = []

    session_id = os.environ.get("SESSION_ID", "")

    with open(csv_path, "r", newline='') as f:
        reader = csv.reader(f)
        for row in reader:
            if len(row) < 4:
                updated_rows.append(row)
                continue
            stored_username, stored_email, stored_password, stored_session_id = row
            if username == stored_username and password == stored_password:
                row[3] = session_id
                found = True
            updated_rows.append(row)

    if found:
        with open(csv_path, "w", newline='') as f:
            writer = csv.writer(f)
            writer.writerows(updated_rows)

        print("Status: 302")
        print("Content-Type: text/html")
        print("Location: http://" + os.environ.get("SERVER_NAME", "") + ":" + os.environ.get("SERVER_PORT", "") + "/docs/valid_login.html")
        sys.stdout.write("\r\n\r\n")
    else:
        print("Status: 302")
        print("Content-Type: text/html")
        print("Location: http://" + os.environ.get("SERVER_NAME", "") + ":" + os.environ.get("SERVER_PORT", "") + "/docs/invalid_login.html")
        sys.stdout.write("\r\n\r\n")
