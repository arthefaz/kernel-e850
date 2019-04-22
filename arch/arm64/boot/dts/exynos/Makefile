# SPDX-License-Identifier: GPL-2.0
dtb-$(CONFIG_ARCH_EXYNOS) += \
	exynos5433-tm2.dtb	\
	exynos5433-tm2e.dtb	\
	exynos7-espresso.dtb

dtb-$(CONFIG_SOC_EXYNOS9610) += exynos9610.dtb
dtb-$(CONFIG_SOC_EXYNOS9610) += \
	exynos9610-erd9610.dtbo
dtb-$(CONFIG_SOC_EXYNOS9630) += exynos9630.dtb \
				exynos9630-smdk9630.dtbo

always		:= $(dtb-y) $(dtbo-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb *.dtbo
