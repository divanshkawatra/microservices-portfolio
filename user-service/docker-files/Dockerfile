# --- Stage 1: Build Environment ---
# We start from an official Ubuntu Linux base image. This ensures a clean, consistent build environment.
FROM ubuntu:22.04 AS builder

# Set a working directory inside the image. All subsequent commands will run from here.
WORKDIR /app

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install all the dependencies needed to COMPILE our C++ code.
# - build-essential: Includes the C++ compiler (g++), make, etc.
# - cmake: Our build system generator.
# - libsqlite3-dev: The development headers for the SQLite3 library.
# - libpthread-stubs0-dev: Provides threading support needed by httplib/argon2.
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    libsqlite3-dev \
    libpthread-stubs0-dev

# Copy our ENTIRE project directory into the image's /app directory.
# The first '.' is our local project folder. The second '.' is the WORKDIR inside the image.
COPY . .

# Create a build directory, run CMake, and then compile the project with Make.
RUN mkdir build && cd build && cmake .. && make

# --- Stage 2: Final Production Image ---
# Start from a fresh, minimal Ubuntu image to keep our final image small.
FROM ubuntu:22.04

# Set the working directory
WORKDIR /app

# Install ONLY the runtime dependencies. We don't need the compiler or cmake anymore.
# We only need the SQLite library file itself.
RUN apt-get update && \
    apt-get install -y --no-install-recommends libsqlite3-0 && \
    rm -rf /var/lib/apt/lists/*

# Copy ONLY the compiled executable from the 'builder' stage.
# This is the magic of multi-stage builds! Our final image is tiny.
COPY --from=builder /app/build/user_service .
# copy data folder with DB inside final docker container
COPY ./data /app/data

# Create a directory for logs, as our Logger expects it.
RUN mkdir -p /app/logs

# Tell Docker that our application will listen on port 8001.
# This is just documentation; it doesn't actually open the port.
EXPOSE 8001

# Set the environment variable for the C++ application to read
# > Setting DOCKER_ENV environment variable, to know in the program that the program
# running in Docker container - and changing config according to it, like IP address
ENV DOCKER_ENV=TRUE

# The command to run when a container is started from this image.
# We run our compiled service and tell it where to create the database.
# Note: The database will be created INSIDE the container.
CMD ["./user_service", "data/user_db.db"]