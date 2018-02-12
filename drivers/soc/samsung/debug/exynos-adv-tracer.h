#ifndef __EXYNOS_ADV_TRACER_H__
#define __EXYNOS_ADV_TRACER_H__

struct adv_tracer_info {
	unsigned int plugin_num;
	struct device *dev;
	unsigned int enter_wfi;
};
extern void *adv_tracer_memcpy_align_4(void *dest, const void *src, unsigned int n);
#endif
