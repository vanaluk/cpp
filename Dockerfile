FROM ubuntu:22.04

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

# Change ownership to non-root user
RUN chown -R appuser:appgroup /app

# Switch to non-root user
USER appuser

# Healthcheck to verify the container is working
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD python3 -c "import sys; sys.path.insert(0, 'build'); import cpp_interview_bindings; print('OK')" || exit 1

# Run Python script by default
CMD ["python3", "python/run.py"]
