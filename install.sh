#!/bin/bash
REPO_URL="https://github.com/Tencent/rapidjson.git"
SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
git clone $REPO_URL