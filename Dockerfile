FROM fedora:38

# Install dependencies
RUN dnf update -y
RUN dnf install make -y && dnf install nasm -y && dnf install dosfstools -y  && dnf install mtools -y && dnf install gcc -y
RUN dnf install qemu-system-x86 -y

# mugOS files (shall be volume-binded)
WORKDIR /srv/mugOS

# Watcom compiler
COPY Compilers/watcom /usr/bin/watcom/
ENV PATH="$PATH:/usr/bin/watcom/binl64/"

# Compile
CMD ["make"]
