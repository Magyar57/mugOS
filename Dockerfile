FROM fedora:39

ENV DEPENDENCIES="make nasm gcc gcc-c++ dosfstools mtools bison flex gmp-devel libmpc-devel mpfr-devel texinfo wget guestfs-tools"
ENV TEMPFILES_BINUTILS="toolchain/binutils.tar.xz toolchain/binutils-src toolchain/binutils-build" 
ENV TEMPFILES_GCC="toolchain/gcc.tar.xz toolchain/gcc-src toolchain/gcc-build"

# Install dependencies
RUN dnf update -y
RUN dnf install -y ${DEPENDENCIES}

# mugOS files (shall be volume-binded)
WORKDIR /srv/mugOS

# Build cross-compilers: binutils & gcc
COPY BuildScripts /srv/mugOS/BuildScripts
COPY Makefile /srv/mugOS
RUN make toolchain && rm -rf ${TEMPFILES_BINUTILS} ${TEMPFILES_GCC}

# Compile
CMD ["make"]
