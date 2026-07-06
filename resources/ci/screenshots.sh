#!/bin/bash
set -e

PROCESS_PATH="build/bt/bt"
SCREENSHOTS_DIR="screenshots"

mkdir -p "$SCREENSHOTS_DIR"

"$PROCESS_PATH" discover &
sleep 3

"$PROCESS_PATH" &
BT_PID=$!
sleep 3
import -window root "$SCREENSHOTS_DIR/config.png"
kill $BT_PID 2>/dev/null || true

"$PROCESS_PATH" pick "https://bbc.co.uk" &
PICK_PID=$!
sleep 3
import -window root "$SCREENSHOTS_DIR/picker.png"

kill $PICK_PID 2>/dev/null || true
kill $BT_PID 2>/dev/null || true
