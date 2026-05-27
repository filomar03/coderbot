#!/bin/bash

SOURCE_DIR=""
TARGET_USER="coderbot"
TARGET_HOST="emb-cb01.local"
TARGET_DIR="cb_filo"

echo "Starting transfer using tar over ssh..."

tar -czf - \
  --exclude-vcs \
  --exclude-vcs-ignores \
  -C "$SOURCE_DIR" . | ssh "${TARGET_USER}@${TARGET_HOST}" "mkdir -p '${TARGET_DIR}' && cd '${TARGET_DIR}' && tar -xzf -"

echo "Transfer complete!"
