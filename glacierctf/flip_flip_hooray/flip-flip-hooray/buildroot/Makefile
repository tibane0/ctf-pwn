.PHONY: build

BUILDROOT=buildroot-2025.02.7

build:
# Download buildroot if not available
ifeq (,$(wildcard ${BUILDROOT}))
	wget https://www.buildroot.org/downloads/${BUILDROOT}.tar.gz
	tar xvzf ${BUILDROOT}.tar.gz
	rm ${BUILDROOT}.tar.gz
endif
	cp config ${BUILDROOT}/.config
	make -C ${BUILDROOT} -j$(shell nproc) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-
	cp ${BUILDROOT}/output/images/rootfs.ext2 .
	qemu-img convert -f raw -O qcow2 rootfs.ext2 rootfs.qcow2
	rm rootfs.ext2
