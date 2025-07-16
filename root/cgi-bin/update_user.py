import cgi
import csv
import os
import sys

form = cgi.FieldStorage()
method = form.getvalue("_method", "").upper()
csv_path = os.path.join(os.path.dirname(__file__), "../../backend/users.csv")

if os.environ["REQUEST_METHOD"] != "POST" or method != "PATCH":
    print("<h2>> Invalid request method</h2>")
    exit()

# get form fields from form to py
session_id = os.environ.get("SESSION_ID", "")
username = ""

with open(csv_path, "r", newline='') as f:
    reader = csv.reader(f)
    for row in reader:
        if len(row) < 4:
            continue
        stored_username, stored_email, stored_password, stored_session_id = row
        if session_id == stored_session_id:
            username = stored_username
            break

new_email = form.getvalue("email", "").strip()
new_password = form.getvalue("password", "").strip()
confirm_password = form.getvalue("confirm-password", "").strip()

csv_path = os.path.join(os.path.dirname(__file__), "../../backend/users.csv")

if new_password and new_password != confirm_password:
    print("Status: 302")
    print("Content-Type: text/html")
    print("Location: http://" + os.environ.get("SERVER_NAME", "") + ":" + os.environ.get("SERVER_PORT", "") + "/docs/password_error.html")
    sys.stdout.write("\r\n")  # blank lines to separate headers from body
    sys.stdout.write("\r\n")

users = [] # create new list
user_found = False

try:
    with open(csv_path, "r", newline='') as f:
        reader = csv.reader(f)
        for row in reader:
            if len(row) != 4:
                continue  # skip invalid rows AGAIN!!
            user, email, password, session_id = row
            if user == username:
                # update only the new fields
                email = new_email if new_email else email
                password = new_password if new_password else password
                user_found = True
            users.append([user, email, password, session_id]) # add current user in the users list, even if no match
except FileNotFoundError:
    print("Status: 302")
    print("Content-Type: text/html")
    print("Location: http://" + os.environ.get("SERVER_NAME", "") + ":" + os.environ.get("SERVER_PORT", "") + "/docs/database_error.html")
    sys.stdout.write("\r\n")  # blank lines to separate headers from body
    sys.stdout.write("\r\n")

# overwrite csv file with new data, entire csv is rewritten but with the new users' data changed
if user_found:
    with open(csv_path, "w", newline='') as f:
        writer = csv.writer(f)
        writer.writerows(users)
    print("Status: 302")
    print("Content-Type: text/html")
    print("Location: http://" + os.environ.get("SERVER_NAME", "") + ":" + os.environ.get("SERVER_PORT", "") + "/docs/success_update.html")
    sys.stdout.write("\r\n")  # blank lines to separate headers from body
    sys.stdout.write("\r\n")
else:
    print("Status: 302")
    print("Content-Type: text/html")
    print("Location: http://" + os.environ.get("SERVER_NAME", "") + ":" + os.environ.get("SERVER_PORT", "") + "/docs/unknown_user.html")
    sys.stdout.write("\r\n")  # blank lines to separate headers from body
    sys.stdout.write("\r\n")
