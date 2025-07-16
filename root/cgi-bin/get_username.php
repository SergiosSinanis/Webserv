#!/usr/bin/env php
<?php

$csv_path = __DIR__ . '/../../backend/users.csv';
$session_id = getenv("SESSION_ID") ?: "";
$username = "";
$found = false;

if (($handle = fopen($csv_path, "r")) !== false) {
    while (($row = fgetcsv($handle)) !== false) {
        if (count($row) < 4) {
            continue;
        }

        list($stored_username, $stored_email, $stored_password, $stored_session_id) = $row;

        if ($session_id === $stored_session_id) {
            $username = $stored_username;
            $found = true;
            break;
        }
    }
    fclose($handle);
}

header("Content-Type: application/json");

if ($found) {
    echo json_encode(["username" => $username]);
} else {
    echo json_encode(["error" => "Invalid session"]);
}
