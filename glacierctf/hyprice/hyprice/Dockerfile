# Debian Bookworm 12.11 (29.10.2025)
# GCC 12.2.0 (Debian 12.2.0-14+deb12u1)
# Clang 14.0.6 (Debian)
# GLIBC 2.36 (Debian GLIBC 2.36-9+deb12u10)

##################
# Builder system #
##################
FROM docker.io/library/debian:stable@sha256:5bdced7b0c02512895dd702dfc903f3249755de86f72ab86e12b514ca9e4505e AS builder

# Install apt dependencies if needed
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y patchelf meson cmake pkg-config xz-utils unzip wget tar make gcc git && apt-get clean

# NOTE: If you want to use a precompiled binary, uncomment
# comment the build binary section and uncomment the use
# precompiled binary section

### Build Binary
WORKDIR /tmp

# Build pixman 0.46.4
RUN wget https://cairographics.org/releases/pixman-0.46.4.tar.xz &&\
    tar xf pixman-0.46.4.tar.xz &&\
    cd pixman-0.46.4/ &&\
    meson setup build &&\
    meson compile -C build -j$(nproc) &&\
    meson install -C build

# Build hyprutils 0.10.2
RUN wget https://github.com/hyprwm/hyprutils/archive/0a28f35c002397bc21ddc9d3ef9771b3d80e65ba.zip &&\
    unzip 0a28f35c002397bc21ddc9d3ef9771b3d80e65ba.zip &&\
    cd hyprutils* &&\
    cmake -DBUILD_TESTING=0 -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_INSTALL_PREFIX:PATH=/usr -S . -B ./build &&\
    cmake --build ./build --config Release --target hyprutils -j$(nproc 2>/dev/null || getconf NPROCESSORS_CONF) &&\
    cmake --install build &&\
    patchelf --set-rpath ./lib ./build/libhyprutils.so.0.10.2 # For running with ./lib later

COPY . /tmp/build/
RUN make -C /tmp/build/ challenge
SHELL ["/bin/sh", "-c"]
ARG CACHEBUST=1
CMD cp /tmp/build/challenge /dist/challenge;\
    cp /tmp/hyprutils*/build/libhyprutils.so.0.10.2 /dist/lib/libhyprutils.so.9;\
    cp /usr/local/lib/x86_64-linux-gnu/libpixman-1.so.0.46.4 /dist/lib/libpixman-1.so.0;\
    cp /usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.33 /dist/lib/libstdc++.so.6;\
    cp /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2 /dist/lib/;\
    cp /usr/lib/x86_64-linux-gnu/libc.so.6 /dist/lib/;\
    cp /usr/lib/x86_64-linux-gnu/libm.so.6 /dist/lib/;\
    cp /usr/lib/x86_64-linux-gnu/libgcc_s.so.1 /dist/lib

### Use precompiled binary
# COPY ./challenge /tmp/build/challenge
# COPY ./challenge /dist/challenge

###############
# Base system #
###############
FROM docker.io/library/debian:stable@sha256:de77ae1b03425c79cd5c829687b3db9f1c62e25af09b34aed09f684043f0285a AS base

# Install apt dependencies if needed
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y coreutils && apt-get clean

#################
# Runner system #
#################
# Opensuse Tumbleweed (only for running nsjail)
FROM docker.io/opensuse/tumbleweed@sha256:7272a708ee245cc28546bf45767b690176a0fc88fe02b3364d8bdb449c5067bf

# Install apk dependencies if needed
RUN zypper -n install nsjail && zypper -n clean --all

# Copy base filesystem
COPY --from=base / /jail

# Copy challenge required files
RUN mkdir -p /jail/app
COPY --from=builder /tmp/build/challenge /jail/app/challenge
COPY --from=builder /tmp/hyprutils*/build/libhyprutils.so.0.10.2 /jail/app/lib/libhyprutils.so.9
COPY --from=builder /usr/local/lib/x86_64-linux-gnu/libpixman-1.so.0.46.4 /jail/app/lib/libpixman-1.so.0
COPY --from=builder /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2 /jail/app/lib/ld-linux-x86-64.so.2
COPY --from=builder /usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.33 /jail/app/lib/libstdc++.so.6
COPY --from=builder /usr/lib/x86_64-linux-gnu/libc.so.6 /jail/app/lib/libc.so.6
COPY --from=builder /usr/lib/x86_64-linux-gnu/libm.so.6 /jail/app/lib/libm.so.6
COPY --from=builder /usr/lib/x86_64-linux-gnu/libgcc_s.so.1 /jail/app/lib/libgcc_s.so.1
RUN chmod +x /jail/app/challenge
COPY entrypoint.sh /jail/app/entrypoint.sh
COPY flag.txt /jail/flag.txt

# add "-m none:/DESTPATH:tmpfs:size=N" before --cwd on nsjail args to have a tmpfs-backed writable DESTPATH of N bytes
# remember that /DESTPATH cannot contain any files coming from /jail (as its a mount). If you want
# pre-created/copied files in /DESTPATH you should manually copy them in entrypoint.sh
# Note: /DESTPATH should not contain /jail as a prefix

SHELL ["/bin/sh", "-c"]
CMD nsjail \
    --mode l \
    --disable_proc \
    --time_limit ${TIMEOUT} \
    --bindhost 0.0.0.0 \
    --port 1337 \
    --bindmount_ro /jail:/ \
    -m none:/dev:tmpfs:mode=555,size=1,uid=65534,gid=65534 \
    --bindmount /dev \
    --cwd /app/ \
    -u 1337:1337:1 \
    -g 1337:1337:1 \
    -u 65534:65534:1 \
    -g 65534:65534:1 \
    -- /bin/sh -c "/app/entrypoint.sh"
