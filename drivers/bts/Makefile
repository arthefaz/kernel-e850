#
# Makefile for BTS device driver
#

SOCMODEL			:= $(subst $\",,$(CONFIG_EXYNOS_SOC_NAME))
obj-$(CONFIG_EXYNOS_BTS)	+= exynos-bts.o
ifdef CONFIG_SOC_EXYNOS$(SOCMODEL)
obj-$(CONFIG_EXYNOS_BTS)	+= exynos-btsops$(SOCMODEL).o
endif
