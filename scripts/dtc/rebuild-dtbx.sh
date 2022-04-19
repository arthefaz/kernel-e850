#!/bin/sh

# Script for rebuilding E850-96 dtb/dtbo files for Android DTBO image

set -e

cmd_cc=clang
dtc=scripts/dtc/dtc
dtc_flags="-@ -a 4 -Wno-unit_address_vs_reg"

# Build dtb/dtbo file suitable for Android DTBO image (with "-@ -a 4" flags)
# $1 - dts filename (without extension)
# $2 - "dtb" or "dtbo"
build_dtbx() {
	dts_filename=arch/arm64/boot/dts/exynos/${1}.dts

	echo "---> Building ${1}.${2}..."

	$cmd_cc -E -P -nostdinc -I./scripts/dtc/include-prefixes \
		-undef -D__DTS__ -x assembler-with-cpp \
		-o arch/arm64/boot/dts/exynos/.${1}.${2}.dts.tmp \
		$dts_filename
	$dtc -O dtb \
		-o arch/arm64/boot/dts/exynos/${1}.${2} \
		-b 0 \
		-iarch/arm64/boot/dts/exynos/ \
		-i./scripts/dtc/include-prefixes \
		$dtc_flags \
		arch/arm64/boot/dts/exynos/.${1}.${2}.dts.tmp
}

build_dtbx exynos3830 dtb
build_dtbx exynos3830-e850-96-reva dtbo
build_dtbx exynos3830-e850-96-revb dtbo
build_dtbx exynos3830-e850-96-revc dtbo
