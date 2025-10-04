FROM fedora:42

# Dockerfile for image: mug-os:2.2
# See the documentation for usage informations

ENV DEPENDENCIES="make wget git clang nasm lld gdisk mtools dosfstools"

# Install dependencies
RUN dnf update -y
RUN dnf install -y ${DEPENDENCIES}

# mugOS files (shall be volume-bound)
WORKDIR /srv/mugOS

# Build the toolchain (note: now EXCLUDES cross-compilers)
COPY Makefile .
COPY BuildScripts ./BuildScripts/
RUN make toolchain

# Compile
CMD ["make"]
