#!/bin/bash

# ==========================================
# Configuration Variables
# ==========================================
SOURCE_DIR="$HOME/Desktop/coderbot/coderbot"
TARGET_USER="coderbot"
TARGET_HOST="emb-cb01.Home"
TARGET_DIR="cb_filo"

# ==========================================
# Transfer command
# ==========================================
echo "Starting transfer using tar over ssh..."

# 1. CD into the source dir to avoid creating absolute paths
# 2. Tar and compress (-czf -) the current directory (.)
# 3. Pipe to SSH, create the target dir, and extract (-xzf -)
tar -czf - \
  --exclude-vcs \
  --exclude-vcs-ignores \
  -C "$SOURCE_DIR" . | ssh "${TARGET_USER}@${TARGET_HOST}" "mkdir -p '${TARGET_DIR}' && cd '${TARGET_DIR}' && tar -xzf -"

echo "Transfer complete!"
