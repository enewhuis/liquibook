# =========================
# Stage 1: Builder
# =========================
FROM python:3.13-slim AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    git \
    python3-venv \
    swig \
    wget \
 && rm -rf /var/lib/apt/lists/*

# Install latest CMake from PyPI
RUN pip install --no-cache-dir cmake build

# Set working directory
WORKDIR /app

# Clone Liquibook source
RUN git clone -b master https://github.com/mkipnis/liquibook liquibook
WORKDIR /app/liquibook

# Build Liquibook C++ core
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
 && cmake --build build --target install

# Build Python wheel
RUN python3 -m build --wheel

# Create virtualenv and install wheel + runtime dependencies
RUN python3 -m venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"

RUN pip install --no-cache-dir --upgrade pip setuptools wheel \
 && pip install --no-cache-dir dist/*.whl \
    dash==3.0.4 \
    dash-bootstrap-components==2.0.3 \
    dash_ag_grid==31.3.1

# Copy entrypoint script
COPY entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh

# Expose Dash port
EXPOSE 8050

# =========================
# Stage 2: Runtime
# =========================
FROM python:3.13-slim AS runtime

# Copy virtualenv from builder
COPY --from=builder /opt/venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"

# Copy entrypoint script
COPY --from=builder /app/liquibook /app/liquibook
COPY --from=builder /usr/local/bin/entrypoint.sh /usr/local/bin/entrypoint.sh

# Expose Dash port
EXPOSE 8050

# Run Liquibook entrypoint
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
