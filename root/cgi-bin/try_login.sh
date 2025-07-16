#!/bin/bash

CSV_PATH="$(dirname "$0")/../../backend/users.csv"
SESSION_ID="${SESSION_ID:-}"

SERVER_NAME="${SERVER_NAME:-localhost}"
SERVER_PORT="${SERVER_PORT:-80}"

FOUND=false

# read csv file line by line
while IFS=, read -r username email password stored_session_id; do
    stored_session_id=$(echo "$stored_session_id" | tr -d '\r')
	if [ -n "$stored_session_id" ] && [ "$SESSION_ID" = "$stored_session_id" ]; then
    	FOUND=true
    	break
	fi

done < "$CSV_PATH"

printf "Status: 302 Found\r\n"
printf "Content-Type: text/html\r\n"

if [ "$FOUND" = true ]; then
    printf "Location: http://%s:%s/docs/dashboard.html\r\n" "$SERVER_NAME" "$SERVER_PORT"
else
    printf "Location: http://%s:%s/docs/login.html\r\n" "$SERVER_NAME" "$SERVER_PORT"
fi

# blank line to separate headers from body
printf "\r\n"

