#!/bin/bash

TARGET_USER="coderbot"
TARGET_HOST="emb-cb01.local"
TARGET_PATH="cb_filo/stats"
DEST_PATH=""

scp "${TARGET_USER}@${TARGET_HOST}:${TARGET_PATH}" "${DEST_PATH}"
