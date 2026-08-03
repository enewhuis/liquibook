#!/bin/bash
# Activate virtual environment if exists
if [ -d "/opt/venv" ]; then
    source /opt/venv/bin/activate
fi

# Start bash shell
cd /app/liquibook/python/examples/LiquiBookSandbox 
exec python3 LiquiBookSandbox.py
#exec bash
