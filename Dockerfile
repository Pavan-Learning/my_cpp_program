FROM alpine:3.20

# Keep the image minimal while including essential C++ and Python tooling.
RUN apk add --no-cache \
    gcc \
    g++ \
    make \
    musl-dev \
    cmake \
    ninja \
    python3 \
    py3-pip \
    py3-virtualenv \
    bash \
    bash-completion \
    git \
    gdb \
    lldb \
    binutils \
    strace \
    valgrind

# Common command aliases expected by many scripts.
RUN ln -sf python3 /usr/bin/python && ln -sf pip3 /usr/bin/pip

WORKDIR /workspaces/Cpp_programing

CMD ["bash"]