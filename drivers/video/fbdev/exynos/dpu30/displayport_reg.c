/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * SFR access functions for Samsung EXYNOS SoC DisplayPort driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include "displayport.h"

videoformat_info videoformat_parameters[] = {
	{v640x480p_60Hz,       800,  640,   16,  96,  48,  525,  480, 10,  2, 33, 60,  25200000,   1, SYNC_NEGATIVE, SYNC_NEGATIVE},
	{v720x480p_60Hz,       858,  720,   16,  62,  60,  525,  480,  9,  6, 30, 59,  27027000,   2, SYNC_NEGATIVE, SYNC_NEGATIVE},
	{v720x576p_50Hz,       864,  720,   12,  64,  68,  625,  576,  5,  5, 39, 50,  27000000,  17, SYNC_NEGATIVE, SYNC_NEGATIVE},
	{v1280x720p_50Hz,     1980, 1280,  440,  40, 220,  750,  720,  5,  5, 20, 50,  74250000,  19, SYNC_POSITIVE, SYNC_POSITIVE},
	{v1280x720p_60Hz,     1650, 1280,  110,  40, 220,  750,  720,  5,  5, 20, 60,  74250000,   4, SYNC_POSITIVE, SYNC_POSITIVE},
	{v1280x800p_RB_60Hz,  1440, 1280,   48,  32,  80,  823,  800,  3,  6, 14, 60,  71000000,   0, SYNC_NEGATIVE, SYNC_POSITIVE},
	{v1280x1024p_60Hz,    1688, 1280,   48, 112, 248, 1066, 1024,  1,  3, 38, 60, 108000000,   4, SYNC_POSITIVE, SYNC_POSITIVE},
	{v1920x1080p_24Hz,    2750, 1920,  638,  44, 148, 1125, 1080,  4,  5, 36, 24,  74250000,  32, SYNC_POSITIVE, SYNC_POSITIVE},
	{v1920x1080p_25Hz,    2640, 1920,  528,  44, 148, 1125, 1080,  4,  5, 36, 25,  74250000,  33, SYNC_POSITIVE, SYNC_POSITIVE},
	{v1920x1080p_30Hz,    2200, 1920,   88,  44, 148, 1125, 1080,  4,  5, 36, 30,  74250000,  34, SYNC_POSITIVE, SYNC_POSITIVE},
	{v1920x1080p_59Hz,    2080, 1920,   48,  44,  68, 1111, 1080,  3,  5, 23, 59, 138500000,   0, SYNC_POSITIVE, SYNC_POSITIVE},
	{v1920x1080p_50Hz,    2640, 1920,  528,  44, 148, 1125, 1080,  4,  5, 36, 50, 148500000,  31, SYNC_POSITIVE, SYNC_POSITIVE},
	{v1920x1080p_60Hz,    2200, 1920,   88,  44, 148, 1125, 1080,  4,  5, 36, 60, 148500000,  16, SYNC_POSITIVE, SYNC_POSITIVE},
	{v1920x1440p_60Hz,    2600, 1920,  128, 208, 344, 1500, 1440,  1,  3, 56, 60, 234000000,  16, SYNC_POSITIVE, SYNC_POSITIVE},
	{v2560x1440p_59Hz,    2720, 2560,   48,  32,  80, 1481, 1440,  3,  5, 33, 59, 241500000,   0, SYNC_POSITIVE, SYNC_POSITIVE},
	{v2560x1440p_60Hz,    3488, 2560,  192, 272, 464, 1493, 1440,  3,  5, 45, 60, 312000000,   0, SYNC_POSITIVE, SYNC_POSITIVE},
	{v3840x2160p_24Hz,    5500, 3840, 1276,  88, 296, 2250, 2160,  8, 10, 72, 24, 297000000,  93, SYNC_POSITIVE, SYNC_POSITIVE},
	{v3840x2160p_25Hz,    5280, 3840, 1056,  88, 296, 2250, 2160,  8, 10, 72, 25, 297000000,  94, SYNC_POSITIVE, SYNC_POSITIVE},
	{v3840x2160p_30Hz,    4400, 3840,  176,  88, 296, 2250, 2160,  8, 10, 72, 30, 297000000,  95, SYNC_POSITIVE, SYNC_POSITIVE},
	{v3840x2160p_50Hz,    5280, 3840, 1056,  88, 296, 2250, 2160,  8, 10, 72, 50, 594000000,  96, SYNC_POSITIVE, SYNC_POSITIVE},
	{v3840x2160p_RB_59Hz, 4000, 3840,   48,  32,  80, 2222, 2160,  3,  5, 54, 59, 533000000,   0, SYNC_POSITIVE, SYNC_POSITIVE},
	{v3840x2160p_60Hz,    4400, 3840,  176,  88, 296, 2250, 2160,  8, 10, 72, 60, 594000000,  97, SYNC_POSITIVE, SYNC_POSITIVE},
	{v4096x2160p_24Hz,    5500, 4096, 1020,  88, 296, 2250, 2160,  8, 10, 72, 24, 297000000,  98, SYNC_POSITIVE, SYNC_POSITIVE},
	{v4096x2160p_25Hz,    5280, 4096,  968,  88, 128, 2250, 2160,  8, 10, 72, 25, 297000000,  99, SYNC_POSITIVE, SYNC_POSITIVE},
	{v4096x2160p_30Hz,    4400, 4096,   88,  88, 128, 2250, 2160,  8, 10, 72, 30, 297000000, 100, SYNC_POSITIVE, SYNC_POSITIVE},
	{v4096x2160p_50Hz,    5280, 4096,  968,  88, 128, 2250, 2160,  8, 10, 72, 50, 594000000, 101, SYNC_POSITIVE, SYNC_POSITIVE},
	{v4096x2160p_60Hz,    4400, 4096,   88,  88, 128, 2250, 2160,  8, 10, 72, 60, 594000000, 102, SYNC_POSITIVE, SYNC_POSITIVE},
	{sa_crc_640x10_60Hz,   800,  640,   16,  96,  48,   26,   10,  2,  2, 12, 60,  27000000,   0, SYNC_POSITIVE, SYNC_POSITIVE},
};

const int videoformat_parameters_cnt = ARRAY_SIZE(videoformat_parameters);

u32 audio_async_m_n[2][3][7] = {
	{	/* M value set */
		{3314, 4567, 4971, 9134, 9942, 18269, 19884},
		{1988, 2740, 2983, 5481, 5695, 10961, 11930},
		{ 994, 1370, 1491, 2740, 2983,  5481,  5965},
	},
	{	/* N value set */
		{32768, 32768, 32768, 32768, 32768, 32768, 32768},
		{32768, 32768, 32768, 32768, 32768, 32768, 32768},
		{32768, 32768, 32768, 32768, 32768, 32768, 32768},
	}
};

u32 audio_sync_m_n[2][3][7] = {
	{	/* M value set */
		{1024, 784, 512, 1568, 1024, 3136, 2048},
		{1024, 784, 512, 1568, 1024, 3136, 2048},
		{1024, 784, 512,  784,  512, 1568, 1024},
	},
	{	/* N value set */
		{10125,  5625,  3375, 5625, 3375, 5625, 3375},
		{16875,  9375,  5625, 9375, 5625, 9375, 5625},
		{33750, 18750, 11250, 9375, 5625, 9375, 5625},
	}
};

u32 m_aud_master[7] = {32000, 44100, 48000, 88200, 96000, 176000, 192000};

u32 n_aud_master[3] = {81000000, 135000000, 270000000};

void displayport_reg_sw_reset(void)
{
	u32 cnt = 10;
	u32 state;

	displayport_write_mask(SYSTEM_SW_RESET_CONTROL, ~0, SW_RESET);

	do {
		state = displayport_read(SYSTEM_SW_RESET_CONTROL) & SW_RESET;
		cnt--;
		udelay(1);
	} while (state && cnt);

	if (!cnt)
		displayport_err("%s is timeout.\n", __func__);
}

void displayport_reg_phy_reset(u32 en)
{
	if (en)
		displayport_phy_write_mask(DP_REG_1, 0, CMN_INIT_RSTN);
	else
		displayport_phy_write_mask(DP_REG_1, ~0, CMN_INIT_RSTN);
}

void displayport_reg_phy_txclk_source_setting(u8 lane_num)
{
	displayport_phy_write_mask(DP_REG_B, lane_num, LN_TXCLK_SOURCE_LANE);
}

void displayport_reg_phy_mode_setting(void)
{
#if defined(CONFIG_CCIC_NOTIFIER)
	struct displayport_device *displayport = get_displayport_drvdata();
#endif
	u32 val = 0;

	displayport_phy_write_mask(CMN_REG2C, 1, MAN_USBDP_MODE_EN);

#if defined(CONFIG_CCIC_NOTIFIER)
	switch (displayport->ccic_notify_dp_conf) {
	case CCIC_NOTIFY_DP_PIN_UNKNOWN:
		displayport_dbg("CCIC_NOTIFY_DP_PIN_UNKNOWN\n");
		break;

	case CCIC_NOTIFY_DP_PIN_A:
	case CCIC_NOTIFY_DP_PIN_C:
	case CCIC_NOTIFY_DP_PIN_E:
		displayport_phy_write_mask(CMN_REG2C, 0x02, MAN_USBDP_MODE);

		displayport_phy_write_mask(CMN_REG2D, 0, USB_TX1_SEL);
		displayport_phy_write_mask(CMN_REG2D, 0, USB_TX3_SEL);

		displayport_phy_write_mask(DP_REG_B3, 0x02, CMN_DUMMY_CTRL_1_0);
		displayport_phy_write_mask(DP_REG_B3, 0x00, CMN_DUMMY_CTRL_7_6);

		val = LN0_LANE_EN | LN1_LANE_EN | LN2_LANE_EN | LN3_LANE_EN;
		displayport_reg_phy_txclk_source_setting(0);
		break;

	case CCIC_NOTIFY_DP_PIN_B:
	case CCIC_NOTIFY_DP_PIN_D:
	case CCIC_NOTIFY_DP_PIN_F:
		if (displayport->dp_sw_sel) {
			displayport_phy_write_mask(CMN_REG2C, 0x00, MAN_USBDP_MODE);

			displayport_phy_write_mask(CMN_REG2D, 0, USB_TX1_SEL);
			displayport_phy_write_mask(CMN_REG2D, 1, USB_TX3_SEL);

			displayport_phy_write_mask(DP_REG_B3, 0x00, CMN_DUMMY_CTRL_1_0);
			displayport_phy_write_mask(DP_REG_B3, 0x02, CMN_DUMMY_CTRL_7_6);

			val = LN0_LANE_EN | LN1_LANE_EN;
			displayport_reg_phy_txclk_source_setting(0);
		} else {
			displayport_phy_write_mask(CMN_REG2C, 0x03, MAN_USBDP_MODE);

			displayport_phy_write_mask(CMN_REG2D, 1, USB_TX1_SEL);
			displayport_phy_write_mask(CMN_REG2D, 0, USB_TX3_SEL);

			displayport_phy_write_mask(DP_REG_B3, 0x03, CMN_DUMMY_CTRL_1_0);
			displayport_phy_write_mask(DP_REG_B3, 0x01, CMN_DUMMY_CTRL_7_6);

			val = LN2_LANE_EN | LN3_LANE_EN;
			displayport_reg_phy_txclk_source_setting(3);
		}
		break;

	default:
		displayport_dbg("CCIC_NOTIFY_DP_PIN_UNKNOWN\n");
		break;
	}
#endif

	val |= 0xF0;
	displayport_phy_write(DP_REG_0, val);
}

void displayport_reg_wait_phy_pll_lock(void)
{
	u32 cnt = 165;	/* wait for 150us + 10% margin */
	u32 state;

	do {
		state = displayport_read(SYSTEM_PLL_LOCK_CONTROL) & PLL_LOCK_STATUS;
		cnt--;
		udelay(1);
	} while (!state && cnt);

	if (!cnt)
		displayport_err("%s is timeout.\n", __func__);
}

void displayport_reg_set_link_bw(u8 link_rate)
{
	displayport_write(SYSTEM_MAIN_LINK_BANDWIDTH, link_rate);
}

u32 displayport_reg_get_link_bw(void)
{
	return displayport_read(SYSTEM_MAIN_LINK_BANDWIDTH);
}

void displayport_reg_set_lane_count(u8 lane_cnt)
{
	displayport_write(SYSTEM_MAIN_LINK_LANE_COUNT, lane_cnt);
}

u32 displayport_reg_get_lane_count(void)
{
	return displayport_read(SYSTEM_MAIN_LINK_LANE_COUNT);
}

void displayport_reg_set_training_pattern(displayport_training_pattern pattern)
{
	displayport_write_mask(PCS_TEST_PATTERN_CONTROL, 0, LINK_QUALITY_PATTERN_SET);
	displayport_write_mask(PCS_CONTROL, pattern, LINK_TRAINING_PATTERN_SET);

	if (pattern == NORAMAL_DATA)
		displayport_write_mask(PCS_CONTROL, 0, SCRAMBLE_BYPASS);
	else
		displayport_write_mask(PCS_CONTROL, 1, SCRAMBLE_BYPASS);
}

void displayport_reg_set_qual_pattern(displayport_qual_pattern pattern, displayport_scrambling scramble)
{
	displayport_write_mask(PCS_CONTROL, 0, LINK_TRAINING_PATTERN_SET);
	displayport_write_mask(PCS_TEST_PATTERN_CONTROL, pattern, LINK_QUALITY_PATTERN_SET);
	displayport_write_mask(PCS_CONTROL, scramble, SCRAMBLE_BYPASS);
}

void displayport_reg_set_hbr2_scrambler_reset(u32 uResetCount)
{
	uResetCount /= 2;       /* only even value@Istor EVT1, ?*/
	displayport_write_mask(PCS_HBR2_EYE_SR_CONTROL, uResetCount, HBR2_EYE_SR_COUNT);
}

void displayport_reg_set_pattern_PLTPAT(void)
{
	displayport_write(PCS_TEST_PATTERN_SET0, 0x3E0F83E0);	/* 00111110 00001111 10000011 11100000 */
	displayport_write(PCS_TEST_PATTERN_SET1, 0x0F83E0F8);	/* 00001111 10000011 11100000 11111000 */
	displayport_write(PCS_TEST_PATTERN_SET2, 0x0000F83E);	/* 11111000 00111110 */
}

void displayport_reg_set_voltage_and_pre_emphasis(u8 *voltage, u8 *pre_emphasis)
{
	u32 val = 0;

	val = (voltage[0] << LN0_TX_AMP_CTRL_BIT_POS) | (voltage[1] << LN1_TX_AMP_CTRL_BIT_POS)
		| (voltage[2] << LN2_TX_AMP_CTRL_BIT_POS) | (voltage[3] << LN3_TX_AMP_CTRL_BIT_POS);
	displayport_phy_write(DP_REG_3, val);

	val = (pre_emphasis[0] << LN0_TX_EMP_CTRL_BIT_POS) | (pre_emphasis[1] << LN1_TX_EMP_CTRL_BIT_POS)
		| (pre_emphasis[2] << LN2_TX_EMP_CTRL_BIT_POS) | (pre_emphasis[3] << LN3_TX_EMP_CTRL_BIT_POS);
	displayport_phy_write(DP_REG_4, val);
}

void displayport_reg_function_enable(void)
{
	displayport_write_mask(SYSTEM_COMMON_FUNCTION_ENABLE, 1, PCS_FUNC_EN);
	displayport_write_mask(SYSTEM_COMMON_FUNCTION_ENABLE, 1, AUX_FUNC_EN);
	displayport_write_mask(SYSTEM_SST1_FUNCTION_ENABLE, 1, SST1_VIDEO_FUNC_EN);
}

void displayport_reg_set_interrupt_mask(enum displayport_interrupt_mask param, u8 set)
{
	u32 val = set ? ~0 : 0;

	switch (param) {
	case HOTPLUG_CHG_INT_MASK:
		displayport_write_mask(SYSTEM_IRQ_COMMON_STATUS_MASK, val, HPD_CHG_MASK);
		break;

	case HPD_LOST_INT_MASK:
		displayport_write_mask(SYSTEM_IRQ_COMMON_STATUS_MASK, val, HPD_LOST_MASK);
		break;

	case PLUG_INT_MASK:
		displayport_write_mask(SYSTEM_IRQ_COMMON_STATUS_MASK, val, HPD_PLUG_MASK);
		break;

	case HPD_IRQ_INT_MASK:
		displayport_write_mask(SYSTEM_IRQ_COMMON_STATUS_MASK, val, HPD_IRQ_MASK);
		break;

	case RPLY_RECEIV_INT_MASK:
		displayport_write_mask(SYSTEM_IRQ_COMMON_STATUS_MASK, val, AUX_REPLY_RECEIVED_MASK);
		break;

	case AUX_ERR_INT_MASK:
		displayport_write_mask(SYSTEM_IRQ_COMMON_STATUS_MASK, val, AUX_ERR_MASK);
		break;

	case HDCP_LINK_CHECK_INT_MASK:
		displayport_write_mask(SYSTEM_IRQ_COMMON_STATUS_MASK, val, HDCP_R0_CHECK_FLAG_MASK);
		break;

	case HDCP_LINK_FAIL_INT_MASK:
		displayport_write_mask(SYSTEM_IRQ_COMMON_STATUS_MASK, val, HDCP_LINK_CHK_FAIL_MASK);
		break;

	case HDCP_R0_READY_INT_MASK:
		displayport_write_mask(SYSTEM_IRQ_COMMON_STATUS_MASK, val, HDCP_R0_CHECK_FLAG_MASK);
		break;

	case PLL_LOCK_CHG_INT_MASK:
		displayport_write_mask(SYSTEM_IRQ_COMMON_STATUS_MASK, val, PLL_LOCK_CHG_MASK);
		break;

	case VIDEO_FIFO_UNDER_FLOW_MASK:
		displayport_write_mask(SST1_INTERRUPT_MASK_SET0, val, MAPI_FIFO_UNDER_FLOW_MASK);
		break;

	case VSYNC_DET_INT_MASK:
		displayport_write_mask(SST1_INTERRUPT_MASK_SET0, val, VSYNC_DET_MASK);
		break;

	case AUDIO_FIFO_UNDER_RUN_INT_MASK:
		displayport_write_mask(SST1_INTERRUPT_STATUS_SET1, val, AFIFO_UNDER);
		break;

	case AUDIO_FIFO_OVER_RUN_INT_MASK:
		displayport_write_mask(SST1_INTERRUPT_STATUS_SET1, val, AFIFO_OVER);
		break;

	case ALL_INT_MASK:
		displayport_write(SYSTEM_IRQ_COMMON_STATUS_MASK, 0xFF);
		displayport_write(SST1_INTERRUPT_MASK_SET0, 0xFF);
		displayport_write(SST1_INTERRUPT_STATUS_SET1, 0xFF);
		break;
	}
}

void displayport_reg_set_interrupt(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write(SYSTEM_IRQ_COMMON_STATUS, ~0);
	displayport_write(SST1_INTERRUPT_STATUS_SET0, ~0);
	displayport_write(SST1_INTERRUPT_STATUS_SET1, ~0);
	displayport_write(SST2_INTERRUPT_STATUS_SET0, ~0);
	displayport_write(SST2_INTERRUPT_STATUS_SET1, ~0);

	displayport_reg_set_interrupt_mask(HPD_IRQ_INT_MASK, val);
	displayport_reg_set_interrupt_mask(HOTPLUG_CHG_INT_MASK, val);
	displayport_reg_set_interrupt_mask(HPD_LOST_INT_MASK, val);
	displayport_reg_set_interrupt_mask(PLUG_INT_MASK, val);
	displayport_reg_set_interrupt_mask(VIDEO_FIFO_UNDER_FLOW_MASK, val);
	displayport_reg_set_interrupt_mask(AUDIO_FIFO_UNDER_RUN_INT_MASK, val);
}

u32 displayport_reg_get_interrupt_and_clear(u32 interrupt_status_register)
{
	u32 val = displayport_read(interrupt_status_register);

	displayport_write(interrupt_status_register, ~0);

	return val;
}

void displayport_reg_set_daynamic_range(enum displayport_dynamic_range_type dynamic_range)
{
	displayport_write_mask(SST1_VIDEO_CONTROL, dynamic_range, DYNAMIC_RANGE_MODE);
}

void displayport_reg_set_video_bist_mode(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write_mask(SST1_VIDEO_CONTROL, val, STRM_VALID_FORCE | STRM_VALID_CTRL);
	displayport_write_mask(SST1_VIDEO_BIST_CONTROL, val, BIST_EN);
}

void displayport_reg_set_audio_bist_mode(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write_mask(SST1_AUDIO_BIST_CONTROL, 0x0F, SIN_AMPL);
	displayport_write_mask(SST1_AUDIO_BIST_CONTROL, val, AUD_BIST_EN);
}

void displayport_reg_video_format_register_setting(videoformat video_format)
{
	u32 val;

	val = videoformat_parameters[video_format].v_total;
	displayport_write(SST1_VIDEO_VERTICAL_TOTAL_PIXELS, val);

	val = videoformat_parameters[video_format].h_total;
	displayport_write(SST1_VIDEO_HORIZONTAL_TOTAL_PIXELS, val);

	val = videoformat_parameters[video_format].v_active;
	displayport_write(SST1_VIDEO_VERTICAL_ACTIVE, val);

	val = videoformat_parameters[video_format].v_f_porch;
	displayport_write(SST1_VIDEO_VERTICAL_FRONT_PORCH, val);

	val = videoformat_parameters[video_format].v_b_porch;
	displayport_write(SST1_VIDEO_VERTICAL_BACK_PORCH, val);

	val = videoformat_parameters[video_format].h_active;
	displayport_write(SST1_VIDEO_HORIZONTAL_ACTIVE, val);

	val = videoformat_parameters[video_format].h_f_porch;
	displayport_write(SST1_VIDEO_HORIZONTAL_FRONT_PORCH, val);

	val = videoformat_parameters[video_format].h_b_porch;
	displayport_write(SST1_VIDEO_HORIZONTAL_BACK_PORCH, val);

	val = videoformat_parameters[video_format].v_sync_pol;
	displayport_write_mask(SST1_VIDEO_CONTROL, val, VSYNC_POLARITY);

	val = videoformat_parameters[video_format].h_sync_pol;
	displayport_write_mask(SST1_VIDEO_CONTROL, val, HSYNC_POLARITY);
}

u32 displayport_reg_get_video_clk(void)
{
	struct displayport_device *displayport = get_displayport_drvdata();

	return videoformat_parameters[displayport->current_videoformat].pixel_clock;
}

u32 displayport_reg_get_ls_clk(void)
{
	u32 val;
	u32 ls_clk;

	val = displayport_reg_get_link_bw();

	if (val == LINK_RATE_5_4Gbps)
		ls_clk = 540000000;
	else if (val == LINK_RATE_2_7Gbps)
		ls_clk = 270000000;
	else /* LINK_RATE_1_62Gbps */
		ls_clk = 162000000;

	return ls_clk;
}

void displayport_reg_set_video_clock(void)
{
	u32 stream_clk = 0;
	u32 ls_clk = 0;
	u32 mvid_master = 0;
	u32 nvid_master = 0;

	stream_clk = displayport_reg_get_video_clk() / 1000;
	ls_clk = displayport_reg_get_ls_clk() / 1000;

	mvid_master = stream_clk >> 1;
	nvid_master = ls_clk;

	displayport_write(SST1_MVID_MASTER_MODE, mvid_master);
	displayport_write(SST1_NVID_MASTER_MODE, nvid_master);

	displayport_write_mask(SST1_MAIN_CONTROL, 1, MVID_MODE);

	displayport_write(SST1_MVID_SFR_CONFIGURE, stream_clk);
	displayport_write(SST1_NVID_SFR_CONFIGURE, ls_clk);
}

void displayport_reg_set_active_symbol(void)
{
	u64 TU_off = 0;	/* TU Size when FEC is off*/
	u64 TU_on = 0;	/* TU Size when FEC is on*/
	u32 bpp = 0;	/* Bit Per Pixel */
	u32 lanecount = 0;
	u32 bandwidth = 0;
	u32 integer_fec_off = 0;
	u32 fraction_fec_off = 0;
	u32 threshold_fec_off = 0;
	u32 integer_fec_on = 0;
	u32 fraction_fec_on = 0;
	u32 threshold_fec_on = 0;
	u32 clk = 0;
	struct displayport_device *displayport = get_displayport_drvdata();

	displayport_write_mask(SST1_ACTIVE_SYMBOL_MODE_CONTROL, 1, ACTIVE_SYMBOL_MODE_CONTROL);
	displayport_write_mask(SST1_ACTIVE_SYMBOL_THRESHOLD_SEL_FEC_OFF, 1, ACTIVE_SYMBOL_THRESHOLD_SEL_FEC_OFF);
	displayport_write_mask(SST1_ACTIVE_SYMBOL_THRESHOLD_SEL_FEC_ON, 1, ACTIVE_SYMBOL_THRESHOLD_SEL_FEC_ON);

	switch (displayport->bpc) {
	case BPC_8:
		bpp = 24;
		break;
	case BPC_10:
		bpp = 30;
		break;
	default:
		bpp = 18;
		break;
	} /* if DSC on, bpp / 3 */

	/* change to Mbps from bps of pixel clock*/
	clk = displayport_reg_get_video_clk() / 1000;

	bandwidth = displayport_reg_get_ls_clk() / 1000;
	lanecount = displayport_reg_get_lane_count();

	TU_off = ((clk * bpp * 32) * 10000000000) / (lanecount * bandwidth * 8);
	TU_on = (TU_off * 1000) / 976;

	integer_fec_off = (u32)(TU_off / 10000000000);
	fraction_fec_off = (u32)((TU_off - (integer_fec_off * 10000000000)) / 10);
	integer_fec_on = (u32)(TU_on / 10000000000);
	fraction_fec_on = (u32)((TU_on - (integer_fec_on * 10000000000)) / 10);

	if (integer_fec_off <= 2)
		threshold_fec_off = 7;
	else if (integer_fec_off > 2 && integer_fec_off <= 5)
		threshold_fec_off = 8;
	else if (integer_fec_off > 5)
		threshold_fec_off = 9;

	if (integer_fec_on <= 2)
		threshold_fec_on = 7;
	else if (integer_fec_on > 2 && integer_fec_on <= 5)
		threshold_fec_on = 8;
	else if (integer_fec_on > 5)
		threshold_fec_on = 9;

	displayport_info("integer_fec_off = %d\n", integer_fec_off);
	displayport_info("fraction_fec_off = %d\n", fraction_fec_off);
	displayport_info("threshold_fec_off = %d\n", threshold_fec_off);
	displayport_info("integer_fec_on = %d\n", integer_fec_on);
	displayport_info("fraction_fec_on = %d\n", fraction_fec_on);
	displayport_info("threshold_fec_on = %d\n", threshold_fec_on);

	displayport_write_mask(SST1_ACTIVE_SYMBOL_INTEGER_FEC_OFF, integer_fec_off, ACTIVE_SYMBOL_INTEGER_FEC_OFF);
	displayport_write_mask(SST1_ACTIVE_SYMBOL_FRACTION_FEC_OFF, fraction_fec_off, ACTIVE_SYMBOL_FRACTION_FEC_OFF);
	displayport_write_mask(SST1_ACTIVE_SYMBOL_THRESHOLD_FEC_OFF, threshold_fec_off, ACTIVE_SYMBOL_FRACTION_FEC_OFF);

	displayport_write_mask(SST1_ACTIVE_SYMBOL_INTEGER_FEC_ON, integer_fec_on, ACTIVE_SYMBOL_INTEGER_FEC_ON);
	displayport_write_mask(SST1_ACTIVE_SYMBOL_FRACTION_FEC_ON, fraction_fec_on, ACTIVE_SYMBOL_FRACTION_FEC_OFF);
	displayport_write_mask(SST1_ACTIVE_SYMBOL_THRESHOLD_FEC_ON, threshold_fec_on, ACTIVE_SYMBOL_THRESHOLD_FEC_ON);
}

void displayport_reg_enable_interface_crc(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write_mask(IF_CRC_Control_1, val, IF_CRC_EN);
	displayport_write_mask(IF_CRC_Control_1, val, IF_CRC_SW_COMPARE);

	if (val == 0) {
		displayport_write_mask(IF_CRC_Control_1, 1, IF_CRC_CLEAR);
		displayport_write_mask(IF_CRC_Control_1, 0, IF_CRC_CLEAR);
	}
}

void displayport_reg_get_interface_crc(u32 *crc_r_result, u32 *crc_g_result, u32 *crc_b_result)
{
	*crc_r_result = displayport_read_mask(IF_CRC_Control_2, IF_CRC_R_RESULT);
	*crc_g_result = displayport_read_mask(IF_CRC_Control_3, IF_CRC_G_RESULT);
	*crc_b_result = displayport_read_mask(IF_CRC_Control_4, IF_CRC_B_RESULT);
}

void displayport_reg_enable_stand_alone_crc(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write_mask(SA_CRC_Control_1, val,
		SA_CRC_LN0_EN|SA_CRC_LN1_EN|SA_CRC_LN2_EN|SA_CRC_LN3_EN);
	displayport_write_mask(SA_CRC_Control_1, val, SA_CRC_SW_COMPARE);

	if (val == 0) {
		displayport_write_mask(SA_CRC_Control_1, 1, SA_CRC_CLEAR);
		displayport_write_mask(SA_CRC_Control_1, 0, SA_CRC_CLEAR);
	}
}

void displayport_reg_get_stand_alone_crc(u32 *ln0, u32 *ln1, u32 *ln2, u32 *ln3)
{
	*ln0 = displayport_read_mask(SA_CRC_Control_2, SA_CRC_LN0_RESULT);
	*ln1 = displayport_read_mask(SA_CRC_Control_3, SA_CRC_LN1_RESULT);
	*ln2 = displayport_read_mask(SA_CRC_Control_4, SA_CRC_LN2_RESULT);
	*ln3 = displayport_read_mask(SA_CRC_Control_5, SA_CRC_LN3_RESULT);
}

void displayport_reg_aux_ch_buf_clr(void)
{
	displayport_write_mask(AUX_BUFFER_CLEAR, 1, AUX_BUF_CLR);
}

void displayport_reg_aux_defer_ctrl(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write_mask(AUX_COMMAND_CONTROL, val, DEFER_CTRL_EN);
}

void displayport_reg_set_aux_reply_timeout(void)
{
	displayport_write_mask(AUX_CONTROL, AUX_TIMEOUT_1800us, AUX_REPLY_TIMER_MODE);
}

void displayport_reg_set_aux_ch_command(enum displayport_aux_ch_command_type aux_ch_mode)
{
	displayport_write_mask(AUX_REQUEST_CONTROL, aux_ch_mode, REQ_COMM);
}

void displayport_reg_set_aux_ch_address(u32 aux_ch_address)
{
	displayport_write_mask(AUX_REQUEST_CONTROL, aux_ch_address, REQ_ADDR);
}

void displayport_reg_set_aux_ch_length(u32 aux_ch_length)
{
	displayport_write_mask(AUX_REQUEST_CONTROL, aux_ch_length - 1, REQ_LENGTH);
}

void displayport_reg_aux_ch_send_buf(u8 *aux_ch_send_buf, u32 aux_ch_length)
{
	int i;

	for (i = 0; i < aux_ch_length; i++) {
		displayport_write_mask(AUX_TX_DATA_SET0 + ((i / 4) * 4),
			aux_ch_send_buf[i], (0x000000FF << ((i % 4) * 8)));
	}
}

void displayport_reg_aux_ch_received_buf(u8 *aux_ch_received_buf, u32 aux_ch_length)
{
	int i;

	for (i = 0; i < aux_ch_length; i++) {
		aux_ch_received_buf[i] =
			(displayport_read_mask(AUX_RX_DATA_SET0 + ((i / 4) * 4),
			0xFF << ((i % 4) * 8)) >> (i % 4) * 8);
	}
}

int displayport_reg_set_aux_ch_operation_enable(void)
{
	u32 cnt = 5000;
	u32 state;
	u32 val0, val1;

	displayport_write_mask(AUX_TRANSACTION_START, 1, AUX_TRAN_START);

	do {
		state = displayport_read(AUX_TRANSACTION_START) & AUX_TRAN_START;
		cnt--;
		udelay(10);
	} while (state && cnt);

	if (!cnt) {
		displayport_err("AUX_TRAN_START waiting timeout.\n");
		return -ETIME;
	}

	val0 = displayport_read(AUX_MONITOR_1);
	val1 = displayport_read(AUX_MONITOR_2);

	if ((val0 & AUX_CMD_STATUS) != 0x00 || val1 != 0x00) {
		displayport_info("AUX_MONITOR_1 : 0x%X, AUX_MONITOR_2 : 0x%X\n", val0, val1);
		displayport_info("AUX_CONTROL : 0x%X, AUX_REQUEST_CONTROL : 0x%X, AUX_COMMAND_CONTROL : 0x%X\n",
				displayport_read(AUX_CONTROL),
				displayport_read(AUX_REQUEST_CONTROL),
				displayport_read(AUX_COMMAND_CONTROL));

		udelay(400);
		return -EIO;
	}

	return 0;
}

void displayport_reg_set_aux_ch_address_only_command(u32 en)
{
	displayport_write_mask(AUX_ADDR_ONLY_COMMAND, en, ADDR_ONLY_CMD);
}

int displayport_reg_dpcd_write(u32 address, u32 length, u8 *data)
{
	int ret;
	int retry_cnt = AUX_RETRY_COUNT;
	struct displayport_device *displayport = get_displayport_drvdata();

	mutex_lock(&displayport->aux_lock);
	while(retry_cnt > 0) {
		displayport_reg_aux_ch_buf_clr();
		displayport_reg_aux_defer_ctrl(1);
		displayport_reg_set_aux_reply_timeout();
		displayport_reg_set_aux_ch_command(DPCD_WRITE);
		displayport_reg_set_aux_ch_address(address);
		displayport_reg_set_aux_ch_length(length);
		displayport_reg_aux_ch_send_buf(data, length);
		ret = displayport_reg_set_aux_ch_operation_enable();
		if (ret == 0)
			break;

		retry_cnt--;
	}

	mutex_unlock(&displayport->aux_lock);

	return ret;
}

int displayport_reg_dpcd_read(u32 address, u32 length, u8 *data)
{
	int ret;
	struct displayport_device *displayport = get_displayport_drvdata();
	int retry_cnt = AUX_RETRY_COUNT;

	mutex_lock(&displayport->aux_lock);
	while(retry_cnt > 0) {
		displayport_reg_set_aux_ch_command(DPCD_READ);
		displayport_reg_set_aux_ch_address(address);
		displayport_reg_set_aux_ch_length(length);
		displayport_reg_aux_ch_buf_clr();
		displayport_reg_aux_defer_ctrl(1);
		displayport_reg_set_aux_reply_timeout();
		ret = displayport_reg_set_aux_ch_operation_enable();

		if (ret == 0)
			break;
		retry_cnt--;
	}

	if (ret == 0)
		displayport_reg_aux_ch_received_buf(data, length);

	mutex_unlock(&displayport->aux_lock);

	return ret;
}

int displayport_reg_dpcd_write_burst(u32 address, u32 length, u8 *data)
{
	int ret = 0;
	u32 i, buf_length, length_calculation;

	length_calculation = length;
	for (i = 0; i < length; i += AUX_DATA_BUF_COUNT) {
		if (length_calculation >= AUX_DATA_BUF_COUNT) {
			buf_length = AUX_DATA_BUF_COUNT;
			length_calculation -= AUX_DATA_BUF_COUNT;
		} else {
			buf_length = length % AUX_DATA_BUF_COUNT;
			length_calculation = 0;
		}

		ret = displayport_reg_dpcd_write(address + i, buf_length, data + i);
		if (ret != 0) {
			displayport_err("displayport_reg_dpcd_write_burst fail\n");
			break;
		}
	}

	return ret;
}

int displayport_reg_dpcd_read_burst(u32 address, u32 length, u8 *data)
{
	int ret = 0;
	u32 i, buf_length, length_calculation;

	length_calculation = length;

	for (i = 0; i < length; i += AUX_DATA_BUF_COUNT) {
		if (length_calculation >= AUX_DATA_BUF_COUNT) {
			buf_length = AUX_DATA_BUF_COUNT;
			length_calculation -= AUX_DATA_BUF_COUNT;
		} else {
			buf_length = length % AUX_DATA_BUF_COUNT;
			length_calculation = 0;
		}

		ret = displayport_reg_dpcd_read(address + i, buf_length, data + i);

		if (ret != 0) {
			displayport_err("displayport_reg_dpcd_read_burst fail\n");
			break;
		}
	}

	return ret;
}

int displayport_reg_edid_write(u8 edid_addr_offset, u32 length, u8 *data)
{
	u32 i, buf_length, length_calculation;
	int ret;
	int retry_cnt = AUX_RETRY_COUNT;

	while(retry_cnt > 0) {
		displayport_reg_aux_ch_buf_clr();
		displayport_reg_aux_defer_ctrl(1);
		displayport_reg_set_aux_reply_timeout();
		displayport_reg_set_aux_ch_command(I2C_WRITE);
		displayport_reg_set_aux_ch_address(EDID_ADDRESS);
		displayport_reg_set_aux_ch_length(1);
		displayport_reg_aux_ch_send_buf(&edid_addr_offset, 1);
		ret = displayport_reg_set_aux_ch_operation_enable();

		if (ret == 0) {
			length_calculation = length;


			/*	displayport_write_mask(AUX_Ch_MISC_Ctrl_1, 0x3, 3 << 6); */
			for (i = 0; i < length; i += AUX_DATA_BUF_COUNT) {
				if (length_calculation >= AUX_DATA_BUF_COUNT) {
					buf_length = AUX_DATA_BUF_COUNT;
					length_calculation -= AUX_DATA_BUF_COUNT;
				} else {
					buf_length = length%AUX_DATA_BUF_COUNT;
					length_calculation = 0;
				}

				displayport_reg_set_aux_ch_length(buf_length);
				displayport_reg_aux_ch_send_buf(data+((i/AUX_DATA_BUF_COUNT)*AUX_DATA_BUF_COUNT), buf_length);
				ret = displayport_reg_set_aux_ch_operation_enable();

				if (ret == 0)
					break;
			}
		}

		if (ret == 0) {
			displayport_reg_set_aux_ch_address_only_command(1);
			ret = displayport_reg_set_aux_ch_operation_enable();
			displayport_reg_set_aux_ch_address_only_command(0);
		}
		if (ret == 0)
			break;

		retry_cnt--;
	}


	return ret;
}

int displayport_reg_edid_read(u8 edid_addr_offset, u32 length, u8 *data)
{
	u32 i, buf_length, length_calculation;
	int ret;
	struct displayport_device *displayport = get_displayport_drvdata();
	int retry_cnt = AUX_RETRY_COUNT;

	mutex_lock(&displayport->aux_lock);

	while(retry_cnt > 0) {
		displayport_reg_set_aux_ch_command(I2C_WRITE);
		displayport_reg_set_aux_ch_address(EDID_ADDRESS);
		displayport_reg_set_aux_ch_address_only_command(1);
		ret = displayport_reg_set_aux_ch_operation_enable();
		displayport_reg_set_aux_ch_address_only_command(0);
		displayport_dbg("1st address only request in EDID read\n");

		displayport_reg_aux_ch_buf_clr();
		displayport_reg_aux_defer_ctrl(1);
		displayport_reg_set_aux_reply_timeout();
		displayport_reg_set_aux_ch_address_only_command(0);
		displayport_reg_set_aux_ch_command(I2C_WRITE);
		displayport_reg_set_aux_ch_address(EDID_ADDRESS);
		displayport_reg_set_aux_ch_length(1);
		displayport_reg_aux_ch_send_buf(&edid_addr_offset, 1);
		ret = displayport_reg_set_aux_ch_operation_enable();

		displayport_dbg("EDID address command in EDID read\n");

		if (ret == 0) {
			displayport_reg_set_aux_ch_command(I2C_READ);
			length_calculation = length;

			for (i = 0; i < length; i += AUX_DATA_BUF_COUNT) {
				if (length_calculation >= AUX_DATA_BUF_COUNT) {
					buf_length = AUX_DATA_BUF_COUNT;
					length_calculation -= AUX_DATA_BUF_COUNT;
				} else {
					buf_length = length%AUX_DATA_BUF_COUNT;
					length_calculation = 0;
				}

				displayport_reg_set_aux_ch_length(buf_length);
				displayport_reg_aux_ch_buf_clr();
				ret = displayport_reg_set_aux_ch_operation_enable();

				if (ret == 0) {
					displayport_reg_aux_ch_received_buf(data+((i/AUX_DATA_BUF_COUNT)*AUX_DATA_BUF_COUNT), buf_length);
					displayport_dbg("AUX buffer read count = %d in EDID read\n", i);
				} else {
					displayport_dbg("AUX buffer read fail in EDID read\n");
					break;
				}
			}
		}

		if (ret == 0) {
			displayport_reg_set_aux_ch_command(I2C_WRITE);
			displayport_reg_set_aux_ch_address(EDID_ADDRESS);
			displayport_reg_set_aux_ch_address_only_command(1);
			ret = displayport_reg_set_aux_ch_operation_enable();
			displayport_reg_set_aux_ch_address_only_command(0);

			displayport_dbg("2nd address only request in EDID read\n");
		}

		if (ret == 0)
			break;

		retry_cnt--;
	}

	mutex_unlock(&displayport->aux_lock);

	return ret;
}

void displayport_reg_set_lane_map(u32 lane0, u32 lane1, u32 lane2, u32 lane3)
{
	displayport_write_mask(PCS_LANE_CONTROL, lane0, LANE0_MAP);
	displayport_write_mask(PCS_LANE_CONTROL, lane1, LANE1_MAP);
	displayport_write_mask(PCS_LANE_CONTROL, lane2, LANE2_MAP);
	displayport_write_mask(PCS_LANE_CONTROL, lane3, LANE3_MAP);
}

void displayport_reg_set_lane_map_config(void)
{
#if defined(CONFIG_CCIC_NOTIFIER)
	struct displayport_device *displayport = get_displayport_drvdata();

	switch (displayport->ccic_notify_dp_conf) {
	case CCIC_NOTIFY_DP_PIN_UNKNOWN:
		break;

	case CCIC_NOTIFY_DP_PIN_A:
		if (displayport->dp_sw_sel)
			displayport_reg_set_lane_map(1, 2, 3, 0);
		else
			displayport_reg_set_lane_map(0, 3, 2, 1);
		break;

	case CCIC_NOTIFY_DP_PIN_B:
		if (displayport->dp_sw_sel)
			displayport_reg_set_lane_map(3, 2, 1, 0);
		else
			displayport_reg_set_lane_map(0, 1, 2, 3);
		break;

	case CCIC_NOTIFY_DP_PIN_C:
	case CCIC_NOTIFY_DP_PIN_E:
	case CCIC_NOTIFY_DP_PIN_D:
	case CCIC_NOTIFY_DP_PIN_F:
		if (displayport->dp_sw_sel)
			displayport_reg_set_lane_map(2, 3, 1, 0);
		else
			displayport_reg_set_lane_map(0, 1, 3, 2);
		break;

	default:
		break;
	}
#endif
}

void displayport_reg_lh_p_ch_power(u32 en)
{
	u32 cnt = 1000;	/* wait 1ms */
	u32 state;

	if (en) {
		displayport_write_mask(SYSTEM_SST1_FUNCTION_ENABLE, 1, SST1_LH_PWR_ON);

		do {
			state = displayport_read_mask(SYSTEM_SST1_FUNCTION_ENABLE, SST1_LH_PWR_ON_STATUS);
			cnt--;
			udelay(1);
		} while (!state && cnt);

		if (!cnt)
			displayport_err("%s is timeout.\n", __func__);
	} else
		displayport_write_mask(SYSTEM_SST1_FUNCTION_ENABLE, 0, SST1_LH_PWR_ON);
}

void displayport_reg_sw_function_en(void)
{
	displayport_write_mask(SYSTEM_SW_FUNCTION_ENABLE, 1, SW_FUNC_EN);
}

void displayport_reg_init(void)
{
	displayport_reg_sw_reset();

	/* phy initialization */
	displayport_reg_phy_reset(1);
	displayport_reg_phy_mode_setting();
	displayport_reg_phy_reset(0);
	displayport_reg_wait_phy_pll_lock();

	/* link initialization */
	displayport_reg_function_enable();
	displayport_reg_lh_p_ch_power(1);
	displayport_reg_sw_function_en();

	displayport_reg_set_interrupt(0);	/*disable irq*/
	displayport_reg_set_lane_map_config();
}

void displayport_reg_set_video_configuration(videoformat video_format, u8 bpc, u8 range)
{
	displayport_reg_set_daynamic_range((range)?CEA_RANGE:VESA_RANGE);
	displayport_write_mask(SST1_VIDEO_CONTROL, bpc, BPC);	/* 0 : 6bits, 1 : 8bits */
	displayport_write_mask(SST1_VIDEO_CONTROL, 0, COLOR_FORMAT);	/* RGB */
	displayport_reg_video_format_register_setting(video_format);
	displayport_reg_set_video_clock();
	displayport_reg_set_active_symbol();
	displayport_write_mask(SST1_VIDEO_MASTER_TIMING_GEN, 1, VIDEO_MASTER_TIME_GEN);
	displayport_write_mask(SST1_MAIN_CONTROL, 0, VIDEO_MODE);
}

void displayport_reg_set_bist_video_configuration(videoformat video_format, u8 bpc, u8 type, u8 range)
{
	displayport_reg_set_video_configuration(video_format, bpc, range);
	displayport_write_mask(SST1_VIDEO_BIST_CONTROL, type, BIST_TYPE);	/* Display BIST type */
	displayport_reg_set_video_bist_mode(1);

	displayport_info("set bist video config format:%d range:%d bpc:%d type:%d\n",
		video_format, (range)?1:0, (bpc)?1:0, type);
}

void displayport_reg_set_bist_video_configuration_for_blue_screen(videoformat video_format)
{
	displayport_reg_set_video_configuration(video_format, BPC_8, CEA_RANGE); /* 8 bits */
	displayport_write(SST1_VIDEO_BIST_USER_DATA_R, 0x00);
	displayport_write(SST1_VIDEO_BIST_USER_DATA_G, 0x00);
	displayport_write(SST1_VIDEO_BIST_USER_DATA_B, 0xFF);
	displayport_write_mask(SST1_VIDEO_BIST_CONTROL, 1, BIST_USER_DATA_EN);
	displayport_reg_set_video_bist_mode(1);

	displayport_dbg("set bist video config for blue screen\n");
}

void displayport_reg_set_avi_infoframe(struct infoframe avi_infoframe)
{
	u32 avi_infoframe_data = 0;

	avi_infoframe_data = ((u32)avi_infoframe.data[3] << 24) || ((u32)avi_infoframe.data[2] << 16)
			|| ((u32)avi_infoframe.data[1] << 8) || (u32)avi_infoframe.data[0];
	displayport_write(SST1_INFOFRAME_AVI_PACKET_DATA_SET0, avi_infoframe_data);

	avi_infoframe_data = ((u32)avi_infoframe.data[7] << 24) || ((u32)avi_infoframe.data[6] << 16)
			|| ((u32)avi_infoframe.data[5] << 8) || (u32)avi_infoframe.data[4];
	displayport_write(SST1_INFOFRAME_AVI_PACKET_DATA_SET1, avi_infoframe_data);

	avi_infoframe_data = ((u32)avi_infoframe.data[11] << 24) || ((u32)avi_infoframe.data[10] << 16)
			|| ((u32)avi_infoframe.data[9] << 8) || (u32)avi_infoframe.data[8];
	displayport_write(SST1_INFOFRAME_AVI_PACKET_DATA_SET2, avi_infoframe_data);

	avi_infoframe_data = (u32)avi_infoframe.data[12];
	displayport_write(SST1_INFOFRAME_AVI_PACKET_DATA_SET3, avi_infoframe_data);

	displayport_write_mask(SST1_INFOFRAME_UPDATE_CONTROL, 1, AVI_INFO_UPDATE);
	displayport_write_mask(SST1_INFOFRAME_SEND_CONTROL, 1, AVI_INFO_SEND);
}

void displayport_reg_set_audio_infoframe(struct infoframe audio_infoframe, u32 en)
{
	u32 audio_infoframe_data = 0;

	audio_infoframe_data = ((u32)audio_infoframe.data[3] << 24) || ((u32)audio_infoframe.data[2] << 16)
			|| ((u32)audio_infoframe.data[1] << 8) || (u32)audio_infoframe.data[0];
	displayport_write(SST1_INFOFRAME_AUDIO_PACKET_DATA_SET0, audio_infoframe_data);

	audio_infoframe_data = ((u32)audio_infoframe.data[7] << 24) || ((u32)audio_infoframe.data[6] << 16)
			|| ((u32)audio_infoframe.data[5] << 8) || (u32)audio_infoframe.data[4];
	displayport_write(SST1_INFOFRAME_AUDIO_PACKET_DATA_SET1, audio_infoframe_data);

	audio_infoframe_data = ((u32)audio_infoframe.data[10] << 8) || (u32)audio_infoframe.data[9];
	displayport_write(SST1_INFOFRAME_AUDIO_PACKET_DATA_SET2, audio_infoframe_data);

	displayport_write_mask(SST1_INFOFRAME_UPDATE_CONTROL, en, AUDIO_INFO_UPDATE);
	displayport_write_mask(SST1_INFOFRAME_SEND_CONTROL, en, AUDIO_INFO_SEND);
}

void displayport_reg_start(void)
{
	displayport_write_mask(SST1_VIDEO_ENABLE, 1, VIDEO_EN);
}

void displayport_reg_video_mute(u32 en)
{
/*	displayport_dbg("set mute %d\n", en);
 *	displayport_write_mask(Video_Control_1, en, VIDEO_MUTE);
 */
}

void displayport_reg_stop(void)
{
	displayport_write_mask(SST1_VIDEO_ENABLE, 0, VIDEO_EN);
}

/* Set SA CRC, For Sorting Vector */
void displayport_reg_set_stand_alone_crc(u32 crc_ln0_ref, u32 crc_ln1_ref, u32 crc_ln2_ref, u32 crc_ln3_ref)
{
	displayport_write_mask(SA_CRC_Control_2, crc_ln0_ref, SA_CRC_LN0_REF);
	displayport_write_mask(SA_CRC_Control_3, crc_ln1_ref, SA_CRC_LN1_REF);
	displayport_write_mask(SA_CRC_Control_4, crc_ln2_ref, SA_CRC_LN2_REF);
	displayport_write_mask(SA_CRC_Control_5, crc_ln3_ref, SA_CRC_LN3_REF);
}

void displayport_reg_enable_stand_alone_crc_hw(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write_mask(SA_CRC_Control_1, val,
		SA_CRC_LN0_EN|SA_CRC_LN1_EN|SA_CRC_LN2_EN|SA_CRC_LN3_EN);
	displayport_write_mask(SA_CRC_Control_1, 0, SA_CRC_SW_COMPARE);	/* use H/W compare */
}

void displayport_reg_clear_stand_alone_crc(void)
{
	displayport_write_mask(SA_CRC_Control_1, 1, SA_CRC_CLEAR);
	displayport_write_mask(SA_CRC_Control_1, 0, SA_CRC_CLEAR);
}

int displayport_reg_get_stand_alone_crc_result(void)
{
	u32 val;
	int err = 0;

	val = displayport_read_mask(SA_CRC_Control_1, 0x00000FF0);
	val = val>>4;

	if (val == 0xf0) {
		displayport_info(" Display Port SA CRC Pass !!!\n");
	} else {
		err = -1;
		displayport_err(" Display Port SA CRC Fail : 0x%02X !!!\n", val);
	}

	return  err;
}

/* Set Bist enable, Color bar */
/* D_range change from CEA_RANGE to VESA_RANGE */
void displayport_reg_set_sa_bist_mode(u32 en)
{
}

void displayport_reg_set_sa_bist_video_configuration(videoformat video_format, u32 bist_type, u32 bist_width)
{
}

/* SA CRC Condition : 8bpc, 4lane, 640x10 size, BIST_TYPE=0, BIST_WIDTH =0 */
int displayport_reg_stand_alone_crc_sorting(void)
{
	int ret;

	displayport_reg_set_stand_alone_crc(0x135E, 0x135E, 0x135E, 0x135E);
	displayport_reg_enable_stand_alone_crc_hw(1);
	displayport_reg_set_sa_bist_video_configuration(sa_crc_640x10_60Hz, 0, 0);
	displayport_reg_start();

	mdelay(10); /* wait for 10ms + 10% margin */

	ret =  displayport_reg_get_stand_alone_crc_result();

	return ret;
}

void displayport_reg_set_audio_m_n(audio_sync_mode audio_sync_mode,
		enum audio_sampling_frequency audio_sampling_freq)
{
	u32 link_bandwidth_set;
	u32 array_set;
	u32 m_value;
	u32 n_value;

	link_bandwidth_set = displayport_reg_get_link_bw();
	if (link_bandwidth_set == LINK_RATE_1_62Gbps)
		array_set = 0;
	else if (link_bandwidth_set == LINK_RATE_2_7Gbps)
		array_set = 1;
	else/* if (link_bandwidth_set == LINK_RATE_5_4Gbps)*/
		array_set = 2;

	if (audio_sync_mode == ASYNC_MODE) {
		m_value = audio_async_m_n[0][array_set][audio_sampling_freq];
		n_value = audio_async_m_n[1][array_set][audio_sampling_freq];
		displayport_write_mask(SST1_MAIN_CONTROL, 0, MAUD_MODE);
	} else {
		m_value = audio_sync_m_n[0][array_set][audio_sampling_freq];
		n_value = audio_sync_m_n[1][array_set][audio_sampling_freq];
		displayport_write_mask(SST1_MAIN_CONTROL, 1, MAUD_MODE);
	}

	displayport_write(SST1_MAUD_SFR_CONFIGURE, m_value);
	displayport_write(SST1_NAUD_SFR_CONFIGURE, n_value);
}

void displayport_reg_set_audio_function_enable(u32 en)
{
	u32 val = en ? 0 : ~0; /* 0 is enable */

	displayport_write_mask(SYSTEM_SST1_FUNCTION_ENABLE, val, SST1_AUDIO_FIFO_FUNC_EN);
	displayport_write_mask(SYSTEM_SST1_FUNCTION_ENABLE, val, SST1_AUDIO_FUNC_EN);
}

void displayport_reg_set_dma_burst_size(enum audio_dma_word_length word_length)
{
	displayport_write_mask(SST1_AUDIO_CONTROL, word_length, DMA_BURST_SEL);
}

void displayport_reg_set_dma_pack_mode(enum audio_16bit_dma_mode dma_mode)
{
	displayport_write_mask(SST1_AUDIO_CONTROL, dma_mode, AUDIO_BIT_MAPPING_TYPE);
}

void displayport_reg_set_pcm_size(enum audio_bit_per_channel audio_bit_size)
{
	displayport_write_mask(SST1_AUDIO_CONTROL, audio_bit_size, PCM_SIZE);
}

void displayport_reg_set_audio_ch_status_same(u32 en)
{
	displayport_write_mask(SST1_AUDIO_CONTROL, en, AUDIO_CH_STATUS_SAME);
}

void displayport_reg_set_audio_ch(u32 audio_ch_cnt)
{
	displayport_write_mask(SST1_AUDIO_BUFFER_CONTROL,
				audio_ch_cnt - 1, MASTER_AUDIO_CHANNEL_COUNT);
}

void displayport_reg_set_audio_ch_mapping(u8 pkt_1, u8 pkt_2, u8 pkt_3, u8 pkt_4,
						u8 pkt_5, u8 pkt_6, u8 pkt_7, u8 pkt_8)
{
	displayport_write_mask(SST1_AUDIO_CHANNEL_1_4_REMAP, pkt_1, AUD_CH_01_REMAP);
	displayport_write_mask(SST1_AUDIO_CHANNEL_1_4_REMAP, pkt_2, AUD_CH_02_REMAP);
	displayport_write_mask(SST1_AUDIO_CHANNEL_1_4_REMAP, pkt_3, AUD_CH_03_REMAP);
	displayport_write_mask(SST1_AUDIO_CHANNEL_1_4_REMAP, pkt_4, AUD_CH_04_REMAP);

	displayport_write_mask(SST1_AUDIO_CHANNEL_1_4_REMAP, pkt_5, AUD_CH_05_REMAP);
	displayport_write_mask(SST1_AUDIO_CHANNEL_1_4_REMAP, pkt_6, AUD_CH_06_REMAP);
	displayport_write_mask(SST1_AUDIO_CHANNEL_1_4_REMAP, pkt_7, AUD_CH_07_REMAP);
	displayport_write_mask(SST1_AUDIO_CHANNEL_1_4_REMAP, pkt_8, AUD_CH_08_REMAP);

	displayport_dbg("audio 1~4 channel mapping = 0x%X\n",
			displayport_read(SST1_AUDIO_CHANNEL_1_4_REMAP));
	displayport_dbg("audio 5~8 channel mapping = 0x%X\n",
			displayport_read(SST1_AUDIO_CHANNEL_5_8_REMAP));
}

void displayport_reg_set_audio_fifo_function_enable(u32 en)
{
	u32 val = en ? 0 : ~0; /* 0 is enable */

	displayport_write_mask(SYSTEM_SST1_FUNCTION_ENABLE, val, SST1_AUDIO_FIFO_FUNC_EN);
}

void displayport_reg_set_audio_sampling_frequency(enum audio_sampling_frequency audio_sampling_freq)
{
	u32 link_bandwidth_set;
	u32 n_aud_master_set;

	link_bandwidth_set = displayport_reg_get_link_bw();
	if (link_bandwidth_set == LINK_RATE_1_62Gbps)
		n_aud_master_set = 0;
	else if (link_bandwidth_set == LINK_RATE_2_7Gbps)
		n_aud_master_set = 1;
	else/* if (link_bandwidth_set == LINK_RATE_5_4Gbps)*/
		n_aud_master_set = 2;

	displayport_write(SST1_MAUD_MASTER_MODE, m_aud_master[audio_sampling_freq]);
	displayport_write(SST1_NAUD_MASTER_MODE, n_aud_master[n_aud_master_set]);
}

void displayport_reg_set_dp_audio_enable(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write_mask(SST1_AUDIO_ENABLE, val, AUDIO_EN);
}

void displayport_reg_set_audio_master_mode_enable(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write_mask(SST1_AUDIO_MASTER_TIMING_GEN, val, AUDIO_MASTER_TIME_GEN);
}

void displayport_reg_set_ch_status_ch_cnt(u32 audio_ch_cnt)
{
	displayport_write_mask(SST1_AUDIO_BIST_CHANNEL_STATUS_SET0,
				audio_ch_cnt, CH_NUM);

	displayport_write_mask(SST1_AUDIO_BIST_CHANNEL_STATUS_SET0,
				audio_ch_cnt, SOURCE_NUM);

	displayport_write_mask(SST1_AUDIO_CHANNEL_1_2_STATUS_CTRL_0,
				audio_ch_cnt, CH_NUM);

	displayport_write_mask(SST1_AUDIO_CHANNEL_1_2_STATUS_CTRL_0,
				audio_ch_cnt, SOURCE_NUM);
}

void displayport_reg_set_ch_status_word_length(enum audio_bit_per_channel audio_bit_size)
{
	u32 word_max = 0;
	u32 sample_word_length = 0;

	switch (audio_bit_size) {
	case AUDIO_24_BIT:
		word_max = 1;
		sample_word_length = 0x05;
		break;

	case AUDIO_16_BIT:
		word_max = 0;
		sample_word_length = 0x01;
		break;

	case AUDIO_20_BIT:
		word_max = 0;
		sample_word_length = 0x05;
		break;

	default:
		word_max = 0;
		sample_word_length = 0x00;
		break;
	}

	displayport_write_mask(SST1_AUDIO_BIST_CHANNEL_STATUS_SET1,
		word_max, WORD_MAX);

	displayport_write_mask(SST1_AUDIO_BIST_CHANNEL_STATUS_SET1,
		sample_word_length, WORD_LENGTH);

	displayport_write_mask(SST1_AUDIO_CHANNEL_1_2_STATUS_CTRL_1,
		word_max, WORD_MAX);

	displayport_write_mask(SST1_AUDIO_CHANNEL_1_2_STATUS_CTRL_1,
		sample_word_length, WORD_LENGTH);
}

void displayport_reg_set_ch_status_sampling_frequency(enum audio_sampling_frequency audio_sampling_freq)
{
	u32 fs_freq = 0;

	switch (audio_sampling_freq) {
	case FS_32KHZ:
		fs_freq = 0x03;
		break;
	case FS_44KHZ:
		fs_freq = 0x00;
		break;
	case FS_48KHZ:
		fs_freq = 0x02;
		break;
	case FS_88KHZ:
		fs_freq = 0x08;
		break;
	case FS_96KHZ:
		fs_freq = 0x0A;
		break;
	case FS_176KHZ:
		fs_freq = 0x0C;
		break;
	case FS_192KHZ:
		fs_freq = 0x0E;
		break;
	default:
		fs_freq = 0x00;
		break;
	}

	displayport_write_mask(SST1_AUDIO_BIST_CHANNEL_STATUS_SET0, fs_freq, FS_FREQ);
	displayport_write_mask(SST1_AUDIO_CHANNEL_1_2_STATUS_CTRL_0, fs_freq, FS_FREQ);
}

void displayport_reg_set_ch_status_clock_accuracy(enum audio_clock_accuracy clock_accuracy)
{
	displayport_write_mask(SST1_AUDIO_BIST_CHANNEL_STATUS_SET0, clock_accuracy, CLK_ACCUR);
	displayport_write_mask(SST1_AUDIO_CHANNEL_1_2_STATUS_CTRL_0, clock_accuracy, CLK_ACCUR);
}

void displayport_reg_set_hdcp22_system_enable(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write_mask(HDCP22_SYS_EN, val, SYSTEM_ENABLE);
}

void displayport_reg_set_hdcp22_mode(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write_mask(SYSTEM_COMMON_FUNCTION_ENABLE, val, HDCP22_FUNC_EN);
}

void displayport_reg_set_hdcp22_encryption_enable(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write_mask(HDCP22_CONTROL, val, HDCP22_ENC_EN);
}

void displayport_reg_set_aux_pn_inv(u32 val)
{
	displayport_write_mask(AUX_CONTROL, val, AUX_PN_INV);
}
