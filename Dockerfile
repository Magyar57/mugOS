FROM fedora:42

ENV DEPENDENCIES="make wget git clang nasm lld gdisk mtools dosfstools"

# Install dependencies
RUN dnf update -y
RUN dnf install -y ${DEPENDENCIES}

# mugOS files (shall be volume-binded)
WORKDIR /srv/mugOS

# Build cross-compilers: binutils & gcc
COPY BuildScripts /srv/mugOS/BuildScripts
COPY Makefile /srv/mugOS
RUN make toolchain && make clean-toolchain

# Compile
CMD ["make"]
