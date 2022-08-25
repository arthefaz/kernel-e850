/* linux/drivers/video/fbdev/exynos/dpu_9810/regs-dpp.h
 *
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Register definition file for Samsung dpp driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef DPP_REGS_H_
#define DPP_REGS_H_

/****************************[ DPU ]****************************/

/*
 * 1 - DMA.base
 *  Secure (G0 only)  : 0x128A_0000
 *  Non-secure        : 0x128B_0000
 */
#define DPU_DMA_VERSION			0x0000

#define DPU_DMA_CH_MAP				0x0004
#define DMA_CH_MAP(_v, _ch)			(((_v) & 0x3) << (2 * _ch))
#define DMA_CH_MAP_MASK(_ch)			(0x3 << (2 * _ch))

#define DPU_DMA_LPI_PERIOD			0x0008
#define DMA_LPI_PERIOD(_v)			((_v) << 0)
#define DMA_LPI_PERIOD_MASK			(0x1f << 0)

#define DPU_DMA_QCH_EN				0x000C
#define DMA_QCH_EN				(1 << 0)

#define DPU_DMA_SWRST				0x0010
#define DMA_CH2_SWRST				(1 << 3)
#define DMA_CH1_SWRST				(1 << 2)
#define DMA_CH0_SWRST				(1 << 1)
#define DMA_ALL_SWRST				(1 << 0)
#define DMA_CH_SWRST(_ch)			(1 << (1 + (_ch)))


#define DPU_DMA_GLB_CGEN			0x0014
#define DMA_SFR_CGEN(_v)			((_v) << 16)
#define DMA_SFR_CGEN_MASK			(1 << 16)
#define DMA_INT_CGEN(_v, _sb)			((_v) << (_sb))
#define DMA_INT_CGEN_MASK(_sb)		(0x1FFFFFFF << (_sb))
#define DMA_ALL_CGEN_MASK			(0x9FFFFFFF)

#define DPU_DMA_TEST_PATTERN0_3		0x0020
#define DPU_DMA_TEST_PATTERN0_2		0x0024
#define DPU_DMA_TEST_PATTERN0_1		0x0028
#define DPU_DMA_TEST_PATTERN0_0		0x002C
#define DPU_DMA_TEST_PATTERN1_3		0x0030
#define DPU_DMA_TEST_PATTERN1_2		0x0034
#define DPU_DMA_TEST_PATTERN1_1		0x0038
#define DPU_DMA_TEST_PATTERN1_0		0x003C
#define DPU_DMA_GLB_CONTROL		0x0040
#define DPU_DMA_GLB_DATA		0x0044

#define DPU_DMA_VR_MODE			0x0060
#define DPU_DMA_VR_MODE_CH2		(1 << 2)
#define DPU_DMA_VR_MODE_CH1		(1 << 1)
#define DPU_DMA_VR_MODE_CH0		(1 << 0)

#define DPU_DMA_QOS_LUT07_00	0x0070
#define DPU_DMA_QOS_LUT15_08	0x0074
#define DPU_DMA_PERFORMANCE_CON	0x0078
#define DPU_DMA_RECOVERY_NUM	0x0080
#define DPU_DMA_PSLV_ERR_CTRL	0x0090
#define DPU_DMA_DEBUG_CONTROL	0x0100
#define DPU_DMA_DEBUG_DATA		0x0104

/*
 * 1.1 - IDMA SFR
 * < DMA.offset >
 *  G0      G1      VG0     VG1     VGF0    VGF1    WB
 *  0x1000  0x2000  0x3000  0x4000  0x5000  0x6000  0x7000
 */
#define IDMA_ENABLE				0x0000
#define IDMA_SRSET				(1 << 24)
#define IDMA_SFR_CLOCK_GATE_EN		(1 << 10)
#define IDMA_SRAM_CLOCK_GATE_EN		(1 << 9)
#define IDMA_ALL_CLOCK_GATE_EN_MASK		(0x3 << 9)
#define IDMA_SFR_UPDATE_FORCE			(1 << 4)
#define IDMA_OP_STATUS				(1 << 2)
#define OP_STATUS_IDLE				(0)
#define OP_STATUS_BUSY				(1)

#define IDMA_IRQ				0x0004

#define IDMA_MO_CONFLICT_IRQ		(1 << 24)
#define IDMA_AFBC_TIMEOUT_IRQ			(1 << 23)
#define IDMA_RECOVERY_START_IRQ		(1 << 22)
#define IDMA_CONFIG_ERROR			(1 << 21)
#define IDMA_LOCAL_HW_RESET_DONE		(1 << 20)
#define IDMA_READ_SLAVE_ERROR			(1 << 19)
#define IDMA_STATUS_DEADLOCK_IRQ		(1 << 17)
#define IDMA_STATUS_FRAMEDONE_IRQ		(1 << 16)
#define IDMA_ALL_IRQ_CLEAR			(0x1FB << 16)
#define IDMA_MO_CONFLICT_MASK		(1 << 9)
#define IDMA_AFBC_TIMEOUT_MASK		(1 << 8)
#define IDMA_RECOVERY_START_MASK		(1 << 7)
#define IDMA_CONFIG_ERROR_MASK		(1 << 6)
#define IDMA_LOCAL_HW_RESET_DONE_MASK	(1 << 5)
#define IDMA_READ_SLAVE_ERROR_MASK		(1 << 4)
#define IDMA_IRQ_DEADLOCK_MASK		(1 << 2)
#define IDMA_IRQ_FRAMEDONE_MASK		(1 << 1)
#define IDMA_ALL_IRQ_MASK			(0x3F6 << 1)
#define IDMA_IRQ_ENABLE			(1 << 0)

#define IDMA_IN_CON				0x0008
#define IDMA_VR_MODE				(1 << 31)
#define IDMA_IN_IC_MAX(_v)			((_v) << 19)
#define IDMA_IN_IC_MAX_MASK			(0xFF << 19)
#define IDMA_IMG_FORMAT(_v)			((_v) << 11)
#define IDMA_IMG_FORMAT_MASK			(0x1f << 11)
#define IDMA_IMG_FORMAT_ARGB8888		(0)
#define IDMA_IMG_FORMAT_ABGR8888		(1)
#define IDMA_IMG_FORMAT_RGBA8888		(2)
#define IDMA_IMG_FORMAT_BGRA8888		(3)
#define IDMA_IMG_FORMAT_XRGB8888		(4)
#define IDMA_IMG_FORMAT_XBGR8888		(5)
#define IDMA_IMG_FORMAT_RGBX8888		(6)
#define IDMA_IMG_FORMAT_BGRX8888		(7)
#define IDMA_IMG_FORMAT_RGB565			(8)
#define IDMA_IMG_FORMAT_RGB1555			(12)
#define IDMA_IMG_FORMAT_RGB4444			(13)
#define IDMA_IMG_FORMAT_ARGB2101010		(16)
#define IDMA_IMG_FORMAT_ABGR2101010		(17)
#define IDMA_IMG_FORMAT_RGBA2101010		(18)
#define IDMA_IMG_FORMAT_BGRA2101010		(19)
#define IDMA_IMG_FORMAT_YUV420_2P		(24)
#define IDMA_IMG_FORMAT_YVU420_2P		(25)
#define IDMA_IMG_FORMAT_YUV420_4P		(26)	/*8bit + 2bit*/
#define IDMA_IMG_FORMAT_YVU420_4P		(27)	/*8bit + 2bit*/
#define IDMA_IMG_FORMAT_YUV420_P010		(28)
#define IDMA_IMG_FORMAT_YVU420_P010		(29)

#if defined(CONFIG_DPU_2_0_INTERFACE)
#define IDMA_ROTATION(_v)		((_v) << 8)
#define IDMA_ROTATION_MASK		(7 << 8)
#define IDMA_ROTATION_X_FLIP		(1 << 8)
#define IDMA_ROTATION_Y_FLIP		(2 << 8)
#define IDMA_ROTATION_180		(3 << 8)
#define IDMA_ROTATION_90		(4 << 8)
#define IDMA_ROTATION_90_X_FLIP		(5 << 8)
#define IDMA_ROTATION_90_Y_FLIP		(6 << 8)
#define IDMA_ROTATION_270		(7 << 8)
#else
#define IDMA_IN_FLIP(_v)			((_v) << 8)
#define IDMA_IN_FLIP_MASK			(0x3 << 8)
#endif

#define IDMA_AFBC_EN				(1 << 7)
#define IDMA_AFBC_TO_EN			(1 << 6)
#define IDMA_IN_CHROMINANCE_STRIDE_SEL	(1 << 4)
#define IDMA_BLOCK_EN				(1 << 3)
#define IDMA_SCANNING_ORDER_SLICE	(1 << 0)
#define IDMA_SCANNING_ORDER_RASTER	(0 << 0)

#define IDMA_OUT_CON				0x000C
#define IDMA_OUT_FRAME_ALPHA(_v)		((_v) << 24)
#define IDMA_OUT_FRAME_ALPHA_MASK		(0xff << 24)

#define IDMA_SRC_SIZE				0x0010
#define IDMA_SRC_HEIGHT(_v)			((_v) << 16)
#define IDMA_SRC_HEIGHT_MASK			(0x3FFF << 16)
#define IDMA_SRC_WIDTH(_v)			((_v) << 0)
#define IDMA_SRC_WIDTH_MASK			(0xFFFF << 0)

#define IDMA_SRC_OFFSET			0x0014
#define IDMA_SRC_OFFSET_Y(_v)			((_v) << 16)
#define IDMA_SRC_OFFSET_Y_MASK		(0x1FFF << 16)
#define IDMA_SRC_OFFSET_X(_v)			((_v) << 0)
#define IDMA_SRC_OFFSET_X_MASK		(0x1FFF << 0)

#define IDMA_IMG_SIZE				0x0018
#define IDMA_IMG_HEIGHT(_v)			((_v) << 16)
#define IDMA_IMG_HEIGHT_MASK			(0x1FFF << 16)
#define IDMA_IMG_WIDTH(_v)			((_v) << 0)
#define IDMA_IMG_WIDTH_MASK			(0x1FFF << 0)

#define IDMA_CHROMINANCE_STRIDE		0x0020
#define IDMA_CHROMA_STRIDE(_v)		((_v) << 0)
#define IDMA_CHROMA_STRIDE_MASK		(0xFFFF << 0)

#define IDMA_BLOCK_OFFSET			0x0024
#define IDMA_BLK_OFFSET_Y(_v)			((_v) << 16)
#define IDMA_BLK_OFFSET_Y_MASK		(0x1FFF << 16)
#define IDMA_BLK_OFFSET_X(_v)			((_v) << 0)
#define IDMA_BLK_OFFSET_X_MASK		(0x1FFF << 0)

#define IDMA_BLOCK_SIZE			0x0028
#define IDMA_BLK_HEIGHT(_v)			((_v) << 16)
#define IDMA_BLK_HEIGHT_MASK		(0x1FFF << 16)
#define IDMA_BLK_WIDTH(_v)			((_v) << 0)
#define IDMA_BLK_WIDTH_MASK			(0x1FFF << 0)

#define IDMA_2BIT_STRIDE		0x0030
#define IDMA_CHROM_2B_STRIDE(_v)	((-v) << 16)
#define IDMA_LUMA_2B_STRIDE(_v)		((-v) << 0)

/* _n: [0,7], _v: [0x0, 0xF] */
#define IDMA_IN_QOS_LUT07_00			0x0034
#define IDMA_IN_QOS_LUT15_08			0x0038
#define IDMA_IN_QOS_LUT(_n, _v)		((_v) << (4*(_n)))
#define IDMA_IN_QOS_LUT_MASK(_n)		(0xF << (4*(_n)))

#define IDMA_IN_BASE_ADDR_Y8			0x0040
#define IDMA_IN_BASE_ADDR_C8			0x0044
#define IDMA_IN_BASE_ADDR_Y2			0x0048
#define IDMA_IN_BASE_ADDR_C2			0x004C

#define IDMA_DEADLOCK_NUM			0x0050
#define IDMA_DEADLOCK_VAL(_v)			((_v) << 1)
#define IDMA_DEADLOCK_VAL_MASK		(0x7FFFFFFF << 1)
#define IDMA_DEADLOCK_EN			(1 << 0)

#define IDMA_BUS_CON				0x0054

#define IDMA_DYNAMIC_GATING_EN		0x0058
#define IDMA_DG_EN(_n, _v)			((_v) << (_n))
#define IDMA_DG_EN_MASK(_n)			(1 << (_n))
#define IDMA_DG_EN_ALL				(0x7FFFFFF << 0)

#define IDMA_RECOVERY_CTRL			0x005C
#define IDMA_RECOVERY_NUM(_v)			((_v) << 1)
#define IDMA_RECOVERY_NUM_MASK		(0x7FFFFFFF << 1)
#define IDMA_RECOVERY_EN			(1 << 0)

#define IDMA_CHAN_CONTROL				0x0060
#define IDMA_CHAN_DATA					0x0064

#define IDMA_CHAN_DEBUG_ENABLE		(1 << 0)

#define IDMA_IN_REQ_DEST			0x068

#define IDMA_MST_SECURITY			0x100
#define IDMA_SLV_SECURITY			0x104

#define IDMA_CFG_ERR_STATE			0x0870
#define IDMA_CFG_ERR_SRC_WIDTH		(1 << 10)
#define IDMA_CFG_ERR_CHROM_STRIDE		(1 << 9)
#define IDMA_CFG_ERR_BASE_ADDR_Y		(1 << 8)
#define IDMA_CFG_ERR_BASE_ADDR_C		(1 << 7)
#define IDMA_CFG_ERR_IMG_WIDTH_AFBC		(1 << 6)
#define IDMA_CFG_ERR_IMG_WIDTH		(1 << 5)
#define IDMA_CFG_ERR_IMG_HEIGHT_ROTATION	(1 << 4)
#define IDMA_CFG_ERR_IMG_HEIGHT		(1 << 3)
#define IDMA_CFG_ERR_BLOCKING			(1 << 2)
#define IDMA_CFG_ERR_SRC_OFFSET_X		(1 << 1)
#define IDMA_CFG_ERR_SRC_OFFSET_Y		(1 << 0)
#define IDMA_CFG_ERR_GET(_v)			(((_v) >> 0) & 0x7FF)

/*
 * 3 - DPP.base
 *  Secure (G0 only)  : 0x1283_0000
 *  Non-secure        : 0x1285_0000
 */
#define DPP_ENABLE				0x0000
#define DPP_SRSET				(1 << 24)
#define DPP_SFR_CLOCK_GATE_EN			(1 << 10)
#define DPP_SRAM_CLOCK_GATE_EN		(1 << 9)
#define DPP_INT_CLOCK_GATE_EN			(1 << 8)
#define DPP_ALL_CLOCK_GATE_EN_MASK		(0x7 << 8)
#define DPP_PSLVERR_EN				(1 << 5)
#define DPP_SFR_UPDATE_FORCE			(1 << 4)
#define DPP_QCHANNEL_EN			(1 << 3)
#define DPP_OP_STATUS				(1 << 2)
#define DPP_TZPC_FLAG_NONSECURE		(1 << 0)
#define DPP_TZPC_FLAG_SECURE		(0 << 0)

#define DPP_IRQ					0x0004
#define DPP_CONFIG_ERROR			(1 << 21)
#define DPP_STATUS_FRAMEDONE_IRQ		(1 << 16)
#define DPP_ALL_IRQ_CLEAR			(0x21 << 16)
#define DPP_CONFIG_ERROR_MASK			(1 << 6)
#define DPP_IRQ_FRAMEDONE_MASK		(1 << 1)
#define DPP_ALL_IRQ_MASK			(0x21 << 1)
#define DPP_IRQ_ENABLE				(1 << 0)

/* VG & VGF only */
#define DPP_IN_CON				0x0008
#define DPP_CSC_TYPE(_v)			((_v) << 18)
#define DPP_CSC_TYPE_MASK			(3 << 18)
#define DPP_CSC_RANGE(_v)			((_v) << 17)
#define DPP_CSC_RANGE_MASK			(1 << 17)
#define DPP_CSC_MODE(_v)			((_v) << 16)
#define DPP_CSC_MODE_MASK			(1 << 16)
#define DPP_DITH_MASK_SEL			(1 << 5)
#define DPP_DITH_MASK_SPIN			(1 << 4)
#define DPP_ALPHA_SEL(_v)			((_v) << 3)
#define DPP_ALPHA_SEL_MASK			(1 << 3)
#define DPP_IMG_FORMAT(_v)			((_v) << 0)
#define DPP_IMG_FORMAT_MASK			(7 << 0)

#define DPP_IMG_SIZE				0x0018
#define DPP_IMG_HEIGHT(_v)			((_v) << 16)
#define DPP_IMG_HEIGHT_MASK			(0x1FFF << 16)
#define DPP_IMG_WIDTH(_v)			((_v) << 0)
#define DPP_IMG_WIDTH_MASK			(0x1FFF << 0)

/* VGF only */
#define DPP_SCALED_IMG_SIZE			0x002C
#define DPP_SCALED_IMG_HEIGHT(_v)		((_v) << 16)
#define DPP_SCALED_IMG_HEIGHT_MASK		(0x1FFF << 16)
#define DPP_SCALED_IMG_WIDTH(_v)		((_v) << 0)
#define DPP_SCALED_IMG_WIDTH_MASK		(0x1FFF << 0)

/*
 * VG & VGF only
 * (00-01-02) : Reg0.L-Reg0.H-Reg1.L
 * (10-11-12) : Reg1.H-Reg2.L-Reg2.H
 * (20-21-22) : Reg3.L-Reg3.H-Reg4.L
 */
#define DPP_CSC_COEF0				0x0030
#define DPP_CSC_COEF1				0x0034
#define DPP_CSC_COEF2				0x0038
#define DPP_CSC_COEF3				0x003C
#define DPP_CSC_COEF4				0x0040
#define DPP_CSC_COEF_H(_v)			((_v) << 16)
#define DPP_CSC_COEF_H_MASK			(0xFFFF << 16)
#define DPP_CSC_COEF_L(_v)			((_v) << 0)
#define DPP_CSC_COEF_L_MASK			(0xFFFF << 0)
#define DPP_CSC_COEF_XX(_n, _v)		((_v) << (0 + (16 * (_n))))
#define DPP_CSC_COEF_XX_MASK(_n)		(0xFFFF << (0 + (16 * (_n))))

#define DPP_MAIN_H_RATIO			0x0044
#define DPP_H_RATIO(_v)			((_v) << 0)
#define DPP_H_RATIO_MASK			(0xFFFFFF << 0)

#define DPP_MAIN_V_RATIO			0x0048
#define DPP_V_RATIO(_v)			((_v) << 0)
#define DPP_V_RATIO_MASK			(0xFFFFFF << 0)

#define DPP_Y_VCOEF_0A				0x0200
#define DPP_Y_HCOEF_0A				0x0290
#define DPP_C_VCOEF_0A				0x0400
#define DPP_C_HCOEF_0A				0x0490
#define DPP_SCL_COEF(_v)			((_v) << 0)
#define DPP_SCL_COEF_MASK			(0x7FF << 0)
#define DPP_H_COEF(n, s, x)	(0x290 + (n) * 0x4 + (s) * 0x24 + (x) * 0x200)
#define DPP_V_COEF(n, s, x)	(0x200 + (n) * 0x4 + (s) * 0x24 + (x) * 0x200)

#define DPP_YHPOSITION				0x05B0
#define DPP_YVPOSITION				0x05B4
#define DPP_CHPOSITION				0x05B8
#define DPP_CVPOSITION				0x05BC
#define DPP_POS_I(_v)				((_v) << 20)
#define DPP_POS_I_MASK				(0xFFF << 20)
#define DPP_POS_I_GET(_v)			(((_v) >> 20) & 0xFFF)
#define DPP_POS_F(_v)				((_v) << 0)
#define DPP_POS_F_MASK				(0xFFFFF << 0)
#define DPP_POS_F_GET(_v)			(((_v) >> 0) & 0xFFFFF)

#define DPP_HDR_CON					0x0600
#define DPP_HDR_TM_ON				(1 << 3)
#define DPP_HDR_GM_ON				(1 << 2)
#define DPP_HDR_EOTF_ON				(1 << 1)
#define DPP_HDR_ON					(1 << 0)

#define DPP_HDR_EOTF_X_AXIS			0x0610
#define DPP_HDR_EOTF_Y_AXIS			0x0694
#define DPP_HDR_GM_COEF				0x0720
#define DPP_HDR_TM_X_AXIS			0x0750
#define DPP_HDR_TM_Y_AXIS			0x0794
#define DPP_HDR_EOTF_MASK			(0x3FF03FF << 0)
#define DPP_HDR_GM_COEF_MASK		(0x1FFFF << 0)
#define DPP_HDR_TM_MASK				(0x3FF03FF << 0)

#define DPP_ASHE_CON				0x0A00
#define DPP_ASHE_NR_OFF				(1 << 3)
#define DPP_ASHE_SC_OFF				(1 << 2)
#define DPP_ASHE_EC_OFF				(1 << 1)
#define DPP_ASHE_ASHE_ON			(1 << 0)

#define DPP_ASHE_PARAM_ED0			0x0A04
#define DPP_ASHE_GAIN_ED1(_v)		((_v) << 16)
#define DPP_ASHE_GAIN_ED0(_v)		((_v) << 0)
#define DPP_ASHE_PARAM_ED1			0x0A08
#define DPP_ASHE_GAIN_ED3(_v)		((_v) << 16)
#define DPP_ASHE_GAIN_ED2(_v)		((_v) << 0)
#define DPP_ASHE_PARAM_MASK			(0x7FF07FF << 0)

#define DPP_ASHE_PARAM_EC0			0x0A0C
#define DPP_ASHE_PARAM_EC1			0x0A10
#define DPP_ASHE_EC_TH_0(_v)		((_v) << 20)
#define DPP_ASHE_EC_TH_N16(_v)		((_v) << 10)
#define DPP_ASHE_EC_TH_N32(_v)		((_v) << 0)

#define DPP_ASHE_PARAM_SC			0x0A14
#define DPP_ASHE_SC_TH3_3RD(_v)		((_v) << 20)
#define DPP_ASHE_SC_TH_2ND(_v)		((_v) << 10)
#define DPP_ASHE_SC_TH_1ST(_v)		((_v) << 0)

#define DPP_ASHE_PARAM_NR			0x0A18
#define DPP_ASHE_NR_TH(_v)			((_v) << 10)
#define DPP_ASHE_NR_EC(_v)			((_v) << 0)

#define DPP_ASHE_PARAM_EC0			0x0A0C
#define DPP_SHE_ON(_v)				((_v) << 31)
#define DPP_SHE_ON_MASK			(1 << 31)
#define DPP_GAIN(_v)				((_v) << 16)
#define DPP_GAIN_MASK				(0x1FF << 16)
#define DPP_EC_EN(_v)				((_v) << 8)
#define DPP_EC_EN_MASK				(1 << 8)
#define DPP_EC_TH_N32(_v)			((_v) << 0)
#define DPP_EC_TH_N32_MASK			(0xFF << 0)

#define DPP_DYNAMIC_GATING_EN			0x0A54
#define DPP_DG_EN(_n, _v)			((_v) << (_n))
#define DPP_DG_EN_MASK(_n)			(1 << (_n))
#define DPP_DG_EN_ALL				(0x7F << 0)

#define DPP_CHAN_CONTROL			0x0C04
#define DPP_CHAN_DATA				0x0C10

#define DPP_LINECNT_CON			0x0d00
#define DPP_LC_CAPTURE(_v)			((_v) << 2)
#define DPP_LC_CAPTURE_MASK			(1 << 2)
#define DPP_LC_ENABLE(_v)			((_v) << 0)
#define DPP_LC_ENABLE_MASK			(1 << 0)

#define DPP_LINECNT_VAL			0x0d04
#define DPP_LC_COUNTER(_v)			((_v) << 0)
#define DPP_LC_COUNTER_MASK			(0x1FFF << 0)
#define DPP_LC_COUNTER_GET(_v)		(((_v) >> 0) & 0x1FFF)

#define DPP_CFG_ERR_STATE			0x0d08
/*
#define DPP_CFG_ERR_SCL_POS			(1 << 4)
#define DPP_CFG_ERR_SCALE_RATIO		(1 << 3)
#define DPP_CFG_ERR_ODD_SIZE			(1 << 2)
#define DPP_CFG_ERR_MAX_SIZE			(1 << 1)
#define DPP_CFG_ERR_MIN_SIZE			(1 << 0)
 */
#define DPP_CFG_ERR_GET(_v)			(((_v) >> 0) & 0x1F)

#endif
