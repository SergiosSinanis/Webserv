import cgi
import csv
import os
import sys

#save data to local variables
csv_path = os.path.join(os.path.dirname(__file__), "../../backend/users.csv")
session_id = os.environ.get("SESSION_ID", "")

found = False                  # flag
with open(csv_path, "r", newline='') as f:  # open in read mode
    reader = csv.reader(f)     # create reader object
    for row in reader:         # iterate each row
        if len(row) < 4:       # skip rows missing infos
            continue
        stored_username, stored_email, stored_password, stored_session_id = row  # read and save the row values
        if session_id == stored_session_id:
            found = True      # a match
            break             # found, break for loop

if found:
    print("Status: 302")
    print("Content-Type: text/html")
    print("Location: " + "http://" + os.environ.get("SERVER_NAME", "") + ":" + os.environ.get("SERVER_PORT", "") + "/docs/dashboard.html")
    print()
    sys.stdout.write("\r\n")  # blank lines to separate headers from body
    sys.stdout.write("\r\n")
else:
    print("Status: 302")
    print("Content-Type: text/html")
    print("Location: http://" + os.environ.get("SERVER_NAME", "") + ":" + os.environ.get("SERVER_PORT", "") + "/docs/login.html")
    sys.stdout.write("\r\n")  # blank lines to separate headers from body
    sys.stdout.write("\r\n")
