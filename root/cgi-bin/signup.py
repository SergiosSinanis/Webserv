import cgi
import csv
import os
import sys

# cgi script receives the "names of this form"
# <form action="/cgi-bin/signup.py" method="post">
# <input type="text" id="new-username" name="username" required placeholder="Enter desired username">
# <input type="password" id="new-password" name="password" required placeholder="Enter secure password">
# <button type="submit" class="btn">[CREATE ACCOUNT]</button>

form = cgi.FieldStorage()  # read the form sent to the cgi and save it in a form/FiledStorage object

# save the 4 data from the forms name attribute and save it in variables
username = form.getvalue("username")
email = form.getvalue("email")
password = form.getvalue("password")
confirm_password = form.getvalue("confirm-password")
session_id = "0"

# save to a variable the full path to the csv file from the cgi-bin/signup.py
csv_path = os.path.join(os.path.dirname(__file__), "../../backend/users.csv")

# check missing fields or passwords validation match
if not username or not email or not password or not confirm_password:
    print("Status: 302")
    print("Content-Type: text/html")
    print("Location: http://" + os.environ.get("SERVER_NAME", "") + ":" + os.environ.get("SERVER_PORT", "") + "/docs/field_error.html")
    sys.stdout.write("\r\n")  # blank lines to separate headers from body
    sys.stdout.write("\r\n")
elif password != confirm_password:
    print("Status: 302")
    print("Content-Type: text/html")
    print("Location: http://" + os.environ.get("SERVER_NAME", "") + ":" + os.environ.get("SERVER_PORT", "") + "/docs/password_error.html")
    sys.stdout.write("\r\n")  # blank lines to separate headers from body
    sys.stdout.write("\r\n")
else:
    user_exists = False
    try:
        with open(csv_path, newline='') as f:
            reader = csv.reader(f)
            for row in reader:
                if len(row) >= 1 and row[0] == username:
                    user_exists = True
                    break
    except FileNotFoundError:
        pass  # skip check

    if user_exists:
        print("Status: 302")
        print("Content-Type: text/html")
        print("Location: http://" + os.environ.get("SERVER_NAME", "") + ":" + os.environ.get("SERVER_PORT", "") + "/docs/invalid_signup.html")
        sys.stdout.write("\r\n")  # blank lines to separate headers from body
        sys.stdout.write("\r\n")
    else:
        with open(csv_path, "a", newline='') as f:
            writer = csv.writer(f)
            writer.writerow([username, email, password, session_id])
        print("Status: 302")
        print("Content-Type: text/html")
        print("Location: http://" + os.environ.get("SERVER_NAME", "") + ":" + os.environ.get("SERVER_PORT", "") + "/docs/valid_signup.html")
        sys.stdout.write("\r\n")  # blank lines to separate headers from body
        sys.stdout.write("\r\n")
