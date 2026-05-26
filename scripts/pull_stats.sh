#!/bin/bash

TARGET_USER="coderbot"
TARGET_HOST="emb-cb01.Home"
TARGET_PATH="cb_filo/stats"
DEST_PATH="$HOME/Desktop/coderbot/coderbot/stats"

scp "${TARGET_USER}@${TARGET_HOST}:${TARGET_PATH}" "${DEST_PATH}"
