#!/bin/bash
set -e

if [ ! -d "venv" ]; then
    python3 -m venv venv
    echo "Venv created!"
fi

source venv/bin/activate
echo "Venv activated!"

check_platformio() {
    if ! python -m platformio --version &>/dev/null; then
        echo "PlatformIO is not installed!"
        return 1
    fi
    echo "PlatformIO $(python -m platformio --version) installed!"
    return 0
}

if ! check_platformio; then
    if [ -f "requirements.txt" ]; then
        pip install -r requirements.txt
    else
        pip install platformio
    fi

    if ! check_platformio; then
        echo "Can't install platformio!"
        exit 1
    fi
    echo "PlatformIO installed!"
fi

echo "Compiling..."
pio run