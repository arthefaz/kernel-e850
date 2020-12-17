#!/bin/sh

set -e

board=
config=

print_usage() {
	echo "Usage: $0 <board> <config>"
	echo
	echo "Params:"
	echo "  - board: erd850 or e850-96"
	echo "  - config: linux, android-base or android-full"
}

parse_params() {
	if [ $# -ne 2 ]; then
		echo "Error: Invalid argument count" >&2
		print_usage $*
		exit 1
	fi

	board="$1"
	config="$2"

	if [ "$board" != "erd850" -a "$board" != "e850-96" ]; then
		echo "Error: Invalid board specified: $board" >&2
		print_usage $*
		exit 1
	fi

	if [ "$config" != "linux" -a "$config" != "android-base" -a \
	     "$config" != "android-full" ]; then
		echo "Error: Invalid config specified: $config" >&2
		print_usage $*
		exit 1
	fi
}

if [ ! -d fragments ]; then
	echo "Error: Script must be ran from kernel root dir" >&2
	exit 1
fi

parse_params $*

case "$config" in
	"linux")
		./scripts/kconfig/merge_config.sh			\
			fragments/exynos3830-base.config		\
			fragments/linux.config
		;;
	"android-base")
		./scripts/kconfig/merge_config.sh			\
			fragments/exynos3830-base.config		\
			fragments/android-base.config			\
			fragments/android-base-conditional.config	\
			fragments/android-recommended-arm64.config	\
			fragments/android-recommended.config
		;;
	"android-full")
		./scripts/kconfig/merge_config.sh			\
			fragments/exynos3830-full-$board.config		\
			fragments/android-base.config			\
			fragments/android-base-conditional.config	\
			fragments/android-recommended-arm64.config	\
			fragments/android-recommended.config
		;;
	*)
		echo "Error: Invalid config: $config" >&2
		print_usage $*
		exit 1
		;;
esac
