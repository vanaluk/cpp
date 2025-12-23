#!/bin/bash

set -e

echo "=== C++ Interview Demo Project - Installation ==="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to check command
check_command() {
    if command -v "$1" &> /dev/null; then
        echo -e "${GREEN}✓${NC} $1 installed"
        return 0
    else
        echo -e "${RED}✗${NC} $1 not installed"
        return 1
    fi
}

# Function to install dependencies for Ubuntu/Debian
install_dependencies_ubuntu() {
    echo "Installing dependencies for Ubuntu/Debian..."
    sudo apt-get update
    sudo apt-get install -y \
        build-essential \
        cmake \
        git \
        python3 \
        python3-pip \
        python3-dev \
        libboost-all-dev \
        libpq-dev \
        postgresql-client \
        perf
}

# Function to install dependencies for Fedora/RHEL
install_dependencies_fedora() {
    echo "Installing dependencies for Fedora/RHEL..."
    sudo dnf install -y \
        gcc-c++ \
        cmake \
        git \
        python3 \
        python3-pip \
        python3-devel \
        boost-devel \
        libpq-devel \
        postgresql \
        perf
}

# Check Docker
if check_command docker && check_command docker-compose; then
    echo ""
    echo -e "${GREEN}Docker detected! Using Docker for build.${NC}"
    echo ""
    
    # Build Docker image
    echo "Building Docker image..."
    docker-compose build
    
    echo ""
    echo -e "${GREEN}✓ Installation completed successfully!${NC}"
    echo ""
    echo "To run the project use:"
    echo "  docker-compose up"
    echo ""
    echo "Or to run Python script directly:"
    echo "  docker-compose run --rm app python3 python/run.py"
    exit 0
fi

# Local installation
echo ""
echo -e "${YELLOW}Docker not detected. Checking local dependencies...${NC}"
echo ""

# Check required commands
MISSING_DEPS=()

check_command g++ || MISSING_DEPS+=("g++")
check_command cmake || MISSING_DEPS+=("cmake")
check_command python3 || MISSING_DEPS+=("python3")
check_command pip3 || MISSING_DEPS+=("pip3")

# Check libraries
if ! pkg-config --exists libpq; then
    MISSING_DEPS+=("libpq-dev (PostgreSQL)")
fi

if ! pkg-config --exists libboost_system; then
    MISSING_DEPS+=("libboost-all-dev (Boost)")
fi

# If there are missing dependencies
if [ ${#MISSING_DEPS[@]} -ne 0 ]; then
    echo ""
    echo -e "${RED}Missing dependencies detected:${NC}"
    for dep in "${MISSING_DEPS[@]}"; do
        echo "  - $dep"
    done
    echo ""
    
    # Determine distribution
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        case $ID in
            ubuntu|debian)
                echo "To install dependencies run:"
                echo "  sudo apt-get update"
                echo "  sudo apt-get install -y build-essential cmake git python3 python3-pip python3-dev libboost-all-dev libpq-dev postgresql-client perf"
                ;;
            fedora|rhel|centos)
                echo "To install dependencies run:"
                echo "  sudo dnf install -y gcc-c++ cmake git python3 python3-pip python3-devel boost-devel libpq-devel postgresql perf"
                ;;
            *)
                echo "Please install missing dependencies manually."
                ;;
        esac
    else
                echo "Please install missing dependencies manually."
    fi
    
    exit 1
fi

# Install Python dependencies
echo ""
echo "Installing Python dependencies..."
pip3 install --user -r python/requirements.txt

# Build project
echo ""
echo "Building project..."
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
cd ..

echo ""
echo -e "${GREEN}✓ Installation completed successfully!${NC}"
echo ""
echo "To run the project use:"
echo "  python3 python/run.py"
echo ""
