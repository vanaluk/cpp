#!/bin/bash

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Parse arguments
NO_DOCKER=false
SHOW_HELP=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --local)
            NO_DOCKER=true
            shift
            ;;
        -h|--help)
            SHOW_HELP=true
            shift
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            SHOW_HELP=true
            shift
            ;;
    esac
done

if [ "$SHOW_HELP" = true ]; then
    echo "Usage: ./install.sh [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --local      Force local build without Docker"
    echo "  -h, --help   Show this help message"
    echo ""
    echo "Examples:"
    echo "  ./install.sh          # Auto-detect Docker, use if available"
    echo "  ./install.sh --local  # Force local build"
    exit 0
fi

echo "=== C++ Benchmark Kit - Installation ==="
echo ""

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

# Check Docker (skip if --local flag is set)
if [ "$NO_DOCKER" = true ]; then
    echo -e "${YELLOW}Docker disabled by --local flag. Using local build.${NC}"
    echo ""
elif check_command docker && check_command docker-compose; then
    echo ""
    echo -e "${GREEN}Docker detected! Using Docker for build.${NC}"
    echo ""
    
    # Build Docker image (uses cache if available)
    echo "Building Docker image (using cache if available)..."
    docker-compose build
    
    echo ""
    echo -e "${GREEN}✓ Installation completed successfully!${NC}"
    echo ""
    echo "To run the interactive console:"
    echo "  docker-compose run --rm app"
    exit 0
fi

# Local installation
echo ""
if [ "$NO_DOCKER" = true ]; then
    echo -e "${YELLOW}Local build mode. Checking dependencies...${NC}"
else
    echo -e "${YELLOW}Docker not detected. Checking local dependencies...${NC}"
fi
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

# Check Boost by looking for header file (pkg-config doesn't always work for Boost)
if [ ! -f /usr/include/boost/version.hpp ] && ! pkg-config --exists boost; then
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
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build . -j$(nproc)
cd ..

# Create symlink for compile_commands.json (for clangd/IDE support)
if [ -f build/compile_commands.json ]; then
    ln -sf build/compile_commands.json compile_commands.json
    echo -e "${GREEN}✓${NC} Created symlink for compile_commands.json (IDE support)"
fi

echo ""
echo -e "${GREEN}✓ Installation completed successfully!${NC}"
echo ""
echo "To run the project use:"
echo "  python3 python/run.py"
echo ""
echo "For IDE support (clangd), compile_commands.json is available in project root."
echo ""
