FROM fedora:39

ENV DEPENDENCIES="make nasm gcc gcc-c++ dosfstools mtools bison flex gmp-devel libmpc-devel mpfr-devel texinfo wget guestfs-tools"

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
