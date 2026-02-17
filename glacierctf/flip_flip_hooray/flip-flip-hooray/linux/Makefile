.PHONY: build

KERNEL=linux-6.17.8
URL=https://cdn.kernel.org/pub/linux/kernel/v6.x/${KERNEL}.tar.xz

build:
# Download kernel if not available
ifeq (,$(wildcard ${KERNEL}))
	wget ${URL}
	tar xvf ${KERNEL}.tar.xz
	rm ${KERNEL}.tar.xz
endif
	patch -p1 -d ${KERNEL} < flip-flip-hooray.patch
	cp config ${KERNEL}/.config
	make -j $(shell nproc) -C ${KERNEL} ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-
	cp ${KERNEL}/arch/arm64/boot/Image .
