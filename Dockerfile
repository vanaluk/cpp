FROM ubuntu:22.04

# Build mode argument: Release (default, ULL optimizations) or Debug
ARG BUILD_TYPE=Release

# Create non-root user for security
RUN groupadd -r appgroup && useradd -r -g appgroup appuser

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
    linux-tools-generic \
    gdb \
    && rm -rf /var/lib/apt/lists/*

# Install Python dependencies
COPY python/requirements.txt /tmp/requirements.txt
RUN pip3 install --no-cache-dir -r /tmp/requirements.txt

# Working directory
WORKDIR /app

# Copy source files
COPY . .

# Build project with specified build type
RUN mkdir -p build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=${BUILD_TYPE} && \
    cmake --build . -j$(nproc) && \
    cmake --install . --prefix /usr/local

# Store build type for runtime info
RUN echo "BUILD_TYPE=${BUILD_TYPE}" > /app/.build_info

# Change ownership to non-root user
RUN chown -R appuser:appgroup /app

# Switch to non-root user
USER appuser

# Healthcheck to verify the container is working
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD python3 -c "import sys; sys.path.insert(0, 'build'); import benchmark_kit_bindings; print('OK')" || exit 1

# Run Python script by default
CMD ["python3", "python/run.py"]
