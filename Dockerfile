FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    python3 \
    python3-pip \
    python3-dev \
    libboost-all-dev \
    libpq-dev \
    postgresql-client \
    perf \
    && rm -rf /var/lib/apt/lists/*

# Install Python dependencies
COPY python/requirements.txt /tmp/requirements.txt
RUN pip3 install --no-cache-dir -r /tmp/requirements.txt

# Working directory
WORKDIR /app

# Copy source files
COPY . .

# Build project
RUN mkdir -p build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . -j$(nproc) && \
    cmake --install . --prefix /usr/local

# Run Python script by default
CMD ["python3", "python/run.py"]
