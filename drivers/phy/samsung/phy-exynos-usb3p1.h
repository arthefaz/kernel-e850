/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com
 *
 * Author: Sung-Hyun Na <sunghyun.na@samsung.com>
 *
 * Chip Abstraction Layer for USB PHY
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef DRIVER_USB_USBPHY_CAL_PHY_EXYNOS_USB3P1_H_
#define DRIVER_USB_USBPHY_CAL_PHY_EXYNOS_USB3P1_H_

/* initialted */
extern void phy_exynos_usb_v3p1_enable(struct exynos_usbphy_info *info);
extern void phy_exynos_usb_v3p1_disable(struct exynos_usbphy_info *info);
extern void phy_exynos_usb_v3p1_link_sw_reset(struct exynos_usbphy_info *info);

/* USB/DP PHY control */
extern void phy_exynos_usb_v3p1_g2_pma_ready(struct exynos_usbphy_info *info);
extern void phy_exynos_usb_v3p1_pipe_ovrd(struct exynos_usbphy_info *info);

/* UART/JTAG over USB */
void phy_exynos_usb3p1_set_fsv_out_en(struct exynos_usbphy_info *usbphy_info, u32 fsv_out_en);

#endif /* DRIVER_USB_USBPHY_CAL_PHY_EXYNOS_USB3P1_H_ */
