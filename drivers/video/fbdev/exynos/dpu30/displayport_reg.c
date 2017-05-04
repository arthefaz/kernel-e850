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

	displayport_write_mask(SW_Reset, ~0, DP_TX_SW_RESET);

	do {
		state = displayport_read(SW_Reset) & DP_TX_SW_RESET;
		cnt--;
		udelay(1);
	} while (state && cnt);

	if (!cnt)
		displayport_err("%s is timeout.\n", __func__);
}

void displayport_reg_phy_reset(u32 en)
{
	if (en)
		displayport_write_mask(Phy_Resetn, 0, CMN_PHY_RSTN);
	else
		displayport_write_mask(Phy_Resetn, ~0, CMN_PHY_RSTN);
}

void displayport_reg_turn_on_phy(struct displayport_device *displayport)
{
	u32 val = 0;
#if defined(CONFIG_CCIC_NOTIFIER)
	switch (displayport->ccic_notify_dp_conf) {
	case CCIC_NOTIFY_DP_PIN_UNKNOWN:
		displayport_dbg("CCIC_NOTIFY_DP_PIN_UNKNOWN\n");
		break;

	case CCIC_NOTIFY_DP_PIN_A:
	case CCIC_NOTIFY_DP_PIN_C:
	case CCIC_NOTIFY_DP_PIN_E:
		val = LN0_LANE_EN | LN1_LANE_EN | LN2_LANE_EN | LN3_LANE_EN;
		break;

	case CCIC_NOTIFY_DP_PIN_B:
	case CCIC_NOTIFY_DP_PIN_D:
	case CCIC_NOTIFY_DP_PIN_F:
		if (displayport->dp_sw_sel)
			val = LN0_LANE_EN | LN1_LANE_EN;
		else
			val = LN2_LANE_EN | LN3_LANE_EN;
		break;

	default:
		displayport_dbg("CCIC_NOTIFY_DP_PIN_UNKNOWN\n");
		break;
	}
#endif
	val |= 0xF0;

	displayport_write(Phy_Enable, val);
}

void displayport_reg_wait_phy_pll_lock(void)
{
	u32 cnt = 165;	/* wait for 150us + 10% margin */
	u32 state;

	do {
		state = displayport_read(Phy_Lock_Mon) & PLL_LOCK_DONE;
		cnt--;
		udelay(1);
	} while (!state && cnt);

	if (!cnt)
		displayport_err("%s is timeout.\n", __func__);
}

void displayport_reg_set_link_bw(u8 link_rate)
{
	displayport_write(DP_Main_Link_Bandwidth_Setting, link_rate);
}

u32 displayport_reg_get_link_bw(void)
{
	return displayport_read(DP_Main_Link_Bandwidth_Setting);
}

void displayport_reg_set_lane_count(u8 lane_cnt)
{
	displayport_write(DP_Main_Link_Lane_Count, lane_cnt);
}

u32 displayport_reg_get_lane_count(void)
{
	return displayport_read(DP_Main_Link_Lane_Count);
}

void displayport_reg_set_training_pattern(displayport_training_pattern pattern)
{
	displayport_write_mask(DP_Training_Pattern_Set, 0, LINK_QUAL_PATTERN_SET);
	displayport_write_mask(DP_Training_Pattern_Set, pattern, TRAINING_PATTERN_SET);

	if (pattern == NORAMAL_DATA)
		displayport_write_mask(DP_Training_Pattern_Set, 0, SCRAMBLING_DISABLE);
	else
		displayport_write_mask(DP_Training_Pattern_Set, 1, SCRAMBLING_DISABLE);
}

void displayport_reg_set_qual_pattern(displayport_qual_pattern pattern, displayport_scrambling scramble)
{
	displayport_write_mask(DP_Training_Pattern_Set, 0, TRAINING_PATTERN_SET);
	displayport_write_mask(DP_Training_Pattern_Set, pattern, LINK_QUAL_PATTERN_SET);
	displayport_write_mask(DP_Training_Pattern_Set, scramble, SCRAMBLING_DISABLE);
}

void displayport_reg_set_hbr2_scrambler_reset(u32 uResetCount)
{
	uResetCount /= 2;       /* only even value@Istor EVT1, ?*/
	displayport_write(HBR2_Eye_SR_Low, (uResetCount>>0)&0xff);
	displayport_write(HBR2_Eye_SR_High, (uResetCount>>8)&0xff);
}

void displayport_reg_set_pattern_PLTPAT(void)
{
	displayport_write(DP_Test_Pattern_0, 0xe0);     /* 11100000 */
	displayport_write(DP_Test_Pattern_1, 0x83);     /* 10000011 */
	displayport_write(DP_Test_Pattern_2, 0x0f);     /* 00001111 */
	displayport_write(DP_Test_Pattern_3, 0x3e);     /* 00111110 */
	displayport_write(DP_Test_Pattern_4, 0xf8);     /* 11111000 */
	displayport_write(DP_Test_Pattern_5, 0xe0);     /* 11100000 */
	displayport_write(DP_Test_Pattern_6, 0x83);     /* 10000011 */
	displayport_write(DP_Test_Pattern_7, 0x0f);     /* 00001111 */
	displayport_write(DP_Test_Pattern_8, 0x3e);     /* 00111110 */
	displayport_write(DP_Test_Pattern_9, 0xf8);     /* 11111000 */
}

void displayport_reg_set_voltage_and_pre_emphasis(u8 *voltage, u8 *pre_emphasis)
{
	u32 val = 0;

	val = voltage[0] | (pre_emphasis[0] << 3);
	displayport_write(DP_Lane_0_Link_Training_Control, val);

	val = voltage[1] | (pre_emphasis[1] << 3);
	displayport_write(DP_Lane_1_Link_Training_Control, val);

	val = voltage[2] | (pre_emphasis[2] << 3);
	displayport_write(DP_Lane_2_Link_Training_Control, val);

	val = voltage[3] | (pre_emphasis[3] << 3);
	displayport_write(DP_Lane_3_Link_Training_Control, val);
}

void displayport_reg_get_voltage_and_pre_emphasis_max_reach(u8 *max_reach_value)
{
	max_reach_value[0] = (u8)displayport_read_mask(DP_Lane_0_Link_Training_Control,
			MAX_PRE_REACH_0|MAX_DRIVE_REACH_0);
	displayport_dbg("DP_Lane_0_Link_Training_Control = %x\n", max_reach_value[0]);

	max_reach_value[1] = (u8)displayport_read_mask(DP_Lane_1_Link_Training_Control,
			MAX_PRE_REACH_1|MAX_DRIVE_REACH_1);
	displayport_dbg("DP_Lane_1_Link_Training_Control = %x\n", max_reach_value[1]);

	max_reach_value[2] = (u8)displayport_read_mask(DP_Lane_2_Link_Training_Control,
			MAX_PRE_REACH_2|MAX_DRIVE_REACH_2);
	displayport_dbg("DP_Lane_2_Link_Training_Control = %x\n", max_reach_value[2]);

	max_reach_value[3] = (u8)displayport_read_mask(DP_Lane_3_Link_Training_Control,
			MAX_PRE_REACH_3|MAX_DRIVE_REACH_3);
	displayport_dbg("DP_Lane_3_Link_Training_Control = %x\n", max_reach_value[3]);
}

void displayport_reg_init_function_enable(void)
{
	displayport_write(Function_En_1, 0x9C);
	displayport_write(Function_En_2, 0xF8);
	displayport_write(Function_En_4, 0x01);
}

void displayport_reg_set_interrupt_mask(enum displayport_interrupt_mask param, u8 set)
{
	u32 val = set ? ~0 : 0;

	switch (param) {
	case VSYNC_DET_INT_MASK:
		displayport_write_mask(Interrupt_Mask_1, val, VSYNC_DET);
		break;

	case PLL_LOCK_CHG_INT_MASK:
		displayport_write_mask(Interrupt_Mask_2, val, PLL_LOCK_CHG);
		break;

	case VID_FORMAT_CHG_INT_MASK:
		displayport_write_mask(Interrupt_Mask_1, val, VID_FORMAT_CHG);
		break;

	case VID_CLK_CHG_INT_MASK:
		displayport_write_mask(Interrupt_Mask_1, val, VID_CLK_CHG);
		break;

	case HOTPLUG_CHG_INT_MASK:
		displayport_write_mask(Common_Interrupt_Mask_4, val, HPD_CHG);
		break;

	case HPD_LOST_INT_MASK:
		displayport_write_mask(Common_Interrupt_Mask_4, val, HPD_LOST);
		break;

	case PLUG_INT_MASK:
		displayport_write_mask(Common_Interrupt_Mask_4, val, PLUG);
		break;

	case INT_HPD_INT_MASK:
		displayport_write_mask(DP_Interrupt_Status_Mask_1, val, HOT_PLUG_DET_MASK);
		break;

	case RPLY_RECEIV_INT_MASK:
		displayport_write_mask(DP_Interrupt_Status_Mask_1, val, RPLY_RECEIV_MASK);
		break;

	case AUX_ERR_INT_MASK:
		displayport_write_mask(DP_Interrupt_Status_Mask_1, val, AUX_ERR_MASK);
		break;

	case HDCP_LINK_CHECK_INT_MASK:
		displayport_write_mask(Interrupt_Mask_2, val, R0_CHECK_FLAG);
		break;

	case HDCP_LINK_FAIL_INT_MASK:
		displayport_write_mask(Interrupt_Mask_2, val, HDCP_LINK_CHK_FAIL);
		break;

	case HW_HDCP_DONE_INT_MASK:
		displayport_write_mask(Interrupt_Mask_2, val, AUTH_DONE);
		break;

	case HW_AUTH_CHG_INT_MASK:
		displayport_write_mask(Interrupt_Mask_2, val, AUTH_STATE_CHG);
		break;

	case HDCP_R0_READY_INT_MASK:
		displayport_write_mask(Interrupt_Mask_2, val, R0_CHECK_FLAG);
		break;
/*
	case AUDIO_FIFO_UNDER_RUN_INT_MASK:
		displayport_write_mask(Interrupt_Mask_3, val, AFIFO_UNDER);
		break;

	case AUDIO_FIFO_OVER_RUN_INT_MASK:
		displayport_write_mask(Interrupt_Mask_3, val, AFIFO_OVER);
		break;
*/
	case ALL_INT_MASK:
		displayport_write_mask(Interrupt_Mask_1, val,
				VSYNC_DET | VID_FORMAT_CHG | VID_CLK_CHG);

		displayport_write_mask(Interrupt_Mask_2, val,
				PLL_LOCK_CHG | R0_CHECK_FLAG | HDCP_LINK_CHK_FAIL
				| AUTH_STATE_CHG | AUTH_DONE);
/*
		displayport_write_mask(Interrupt_Mask_3, val,
				AFIFO_UNDER | AFIFO_OVER);
*/
		displayport_write_mask(Common_Interrupt_Mask_4, val,
				HPD_CHG | HPD_LOST | PLUG);

		displayport_write_mask(DP_Interrupt_Status_Mask_1, val,
				SOFT_INTERRUPT_MASK | HOT_PLUG_DET_MASK
				| RPLY_RECEIV_MASK | AUX_ERR_MASK);
		break;
	}
}

void displayport_reg_set_interrupt(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write(DP_Interrupt_Status, ~0);
	displayport_write(Common_Interrupt_Status_2, ~0);
	displayport_write(Common_Interrupt_Status_4, ~0);

	displayport_reg_set_interrupt_mask(HOTPLUG_CHG_INT_MASK, val);
	displayport_reg_set_interrupt_mask(HPD_LOST_INT_MASK, val);
	displayport_reg_set_interrupt_mask(PLUG_INT_MASK, val);
	displayport_reg_set_interrupt_mask(INT_HPD_INT_MASK, val);
}

u32 displayport_reg_get_interrupt_and_clear(u32 interrupt_status_register)
{
	u32 val = displayport_read(interrupt_status_register);

	displayport_write(interrupt_status_register, ~0);

	return val;
}

void displayport_reg_set_daynamic_range(enum displayport_dynamic_range_type dynamic_range)
{
	displayport_write_mask(Video_Control_2, dynamic_range, IN_D_RANGE);
}

void displayport_reg_set_video_bist_mode(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write_mask(DP_System_Control_3, val, F_VALID|VALID_CTRL);
	displayport_write_mask(Video_Control_4, val, BIST_EN);
}

void displayport_reg_set_audio_bist_mode(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write_mask(Audio_BIST_Control_Register, 0x0F, SIN_AMPL);
	displayport_write_mask(Audio_BIST_Control_Register, val, AUD_BIST_EN);
}

void displayport_reg_video_format_register_setting(videoformat video_format)
{
	u32 val;

	val = videoformat_parameters[video_format].v_total & TOTAL_LINE_CFG_L;
	displayport_write_mask(Total_Line_Low_Byte, val, TOTAL_LINE_CFG_L);
	val = (videoformat_parameters[video_format].v_total >> 8) & TOTAL_LINE_CFG_H;
	displayport_write_mask(Total_Line_High_Byte, val, TOTAL_LINE_CFG_H);

	val = videoformat_parameters[video_format].v_active & ACTIVE_LINE_CFG_L;
	displayport_write_mask(Active_Line_Low_Byte, val, ACTIVE_LINE_CFG_L);
	val = (videoformat_parameters[video_format].v_active >> 8) & ACTIVE_LINE_CFG_H;
	displayport_write_mask(Active_Line_High_Byte, val, ACTIVE_LINE_CFG_H);

	val = videoformat_parameters[video_format].v_f_porch;
	displayport_write_mask(Vertical_Front_Porch, val, V_F_PORCH_CFG);

	val = videoformat_parameters[video_format].v_sync;
	displayport_write_mask(Vertical_Sync_Width, val, V_SYNC_CFG);

	val = videoformat_parameters[video_format].v_b_porch;
	displayport_write_mask(Vertical_Back_Porch, val, V_B_PORCH_CFG);

	val = videoformat_parameters[video_format].h_total & TOTAL_PIXEL_CFG_L;
	displayport_write_mask(Total_Pixel_Low_Byte, val, TOTAL_PIXEL_CFG_L);
	val = (videoformat_parameters[video_format].h_total >> 8) & TOTAL_PIXEL_CFG_H;
	displayport_write_mask(Total_Pixel_High_Byte, val, TOTAL_PIXEL_CFG_H);

	val = videoformat_parameters[video_format].h_active & ACTIVE_PIXEL_CFG_L;
	displayport_write_mask(Active_Pixel_Low_Byte, val, ACTIVE_PIXEL_CFG_L);
	val = (videoformat_parameters[video_format].h_active >> 8) & ACTIVE_PIXEL_CFG_H;
	displayport_write_mask(Active_Pixel_High_Byte, val, ACTIVE_PIXEL_CFG_H);

	val = videoformat_parameters[video_format].h_f_porch & H_F_PORCH_CFG_L;
	displayport_write_mask(Horizon_Front_Porch_Low_Byte, val, H_F_PORCH_CFG_L);
	val = (videoformat_parameters[video_format].h_f_porch >> 8) & H_F_PORCH_CFG_H;
	displayport_write_mask(Horizon_Front_Porch_High_Byte, val, H_F_PORCH_CFG_H);

	val = videoformat_parameters[video_format].h_sync & H_SYNC_CFG_L;
	displayport_write_mask(Horizon_Sync_Width_Low_Byte, val, H_SYNC_CFG_L);
	val = (videoformat_parameters[video_format].h_sync >> 8) & H_SYNC_CFG_H;
	displayport_write_mask(Horizon_Sync_Width_High_Byte, val, H_SYNC_CFG_H);

	val = videoformat_parameters[video_format].h_b_porch & H_B_PORCH_CFG_L;
	displayport_write_mask(Horizon_Back_Porch_Low_Byte, val, H_B_PORCH_CFG_L);
	val = (videoformat_parameters[video_format].h_b_porch >> 8) & H_B_PORCH_CFG_H;
	displayport_write_mask(Horizon_Back_Porch_High_Byte, val, H_B_PORCH_CFG_H);

	val = videoformat_parameters[video_format].v_sync_pol;
	displayport_write_mask(Video_Control_10, val, VSYNC_P_CFG);

	val = videoformat_parameters[video_format].h_sync_pol;
	displayport_write_mask(Video_Control_10, val, HSYNC_P_CFG);
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
	displayport_write_mask(Req_Op_En, 1, BUF_CLR);
}

void displayport_reg_aux_defer_ctrl(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write_mask(AUX_Ch_Defer_Ctrl, val, DEFER_CTRL_EN);
}

void displayport_reg_set_aux_reply_timeout(void)
{
	displayport_write_mask(AUX_Ch_MISC_Ctrl_1, AUX_TIMEOUT_1800us, AUX_TC);
}

void displayport_reg_aux_error_clr(void)
{
	displayport_write_mask(Read_Error_Ctrl, 1, ERROR_CLR);
}

void displayport_reg_set_aux_ch_command(enum displayport_aux_ch_command_type aux_ch_mode)
{
	displayport_write_mask(Req_Addr_0, aux_ch_mode, REG_REQ_COMM);
}

void displayport_reg_set_aux_ch_address(u32 aux_ch_address)
{
	displayport_write_mask(Req_Addr_0, aux_ch_address, REG_REQ_ADDR);
}

void displayport_reg_set_aux_ch_length(u32 aux_ch_length)
{
	displayport_write_mask(Req_Addr_0, aux_ch_length-1, REG_REQ_LENGTH);
}

void displayport_reg_aux_ch_send_buf(u8 *aux_ch_send_buf, u32 aux_ch_length)
{
	int i;

	for (i = 0; i < aux_ch_length; i++) {
		displayport_write_mask(SEND_DATA_0_1_2_3 + ((i / 4) * 4),
			aux_ch_send_buf[i], (0x000000FF << ((i % 4) * 8)));
	}
}

void displayport_reg_aux_ch_received_buf(u8 *aux_ch_received_buf, u32 aux_ch_length)
{
	int i;

	for (i = 0; i < aux_ch_length; i++) {
		aux_ch_received_buf[i] =
			(displayport_read_mask(RECEIVED_DATA_0_1_2_3+((i/4)*4),
			0xFF<<((i%4)*8)) >> (i%4)*8);
	}
}

int displayport_reg_set_aux_ch_operation_enable(void)
{
	u32 cnt = 5000;
	u32 state;
	u32 val0, val1;

	displayport_write_mask(Req_Op_En, 1, AUX_OP_EN);

	do {
		state = displayport_read(Req_Op_En) & AUX_OP_EN;
		cnt--;
		udelay(10);
	} while (state && cnt);

	if (!cnt) {
		displayport_err("AUX Req_Op_En waiting timeout.\n");
		return -ETIME;
	}

	val0 = displayport_read(AUX_Status_0);
	val1 = displayport_read(AUX_Status_1);

	if ((val0 & AUX_STATUS) != 0x00 || val1 != 0x00) {
		displayport_info("AUX stat 0:0x%X, 1:0x%X, 2:0x%X, 3:0x%X, err:0x%X\n",
				val0, val1, displayport_read(AUX_Status_2),
				displayport_read(AUX_Status_3),
				displayport_read(Read_Error_Ctrl));
		displayport_info("AUX ctrl 0:0x%X, 1:0x%X, 2:0x%X, def:0x%X, Addr:0x%X, OP_en:0x%X\n",
				displayport_read(AUX_Ch_MISC_Ctrl_0),
				displayport_read(AUX_Ch_MISC_Ctrl_1),
				displayport_read(AUX_Ch_MISC_Ctrl_2),
				displayport_read(AUX_Ch_Defer_Ctrl),
				displayport_read(Req_Addr_0),
				displayport_read(Req_Op_En));

		udelay(400);
		return -EIO;
	}

	return 0;
}

void displayport_reg_set_aux_ch_address_only_command(u32 en)
{
	displayport_write_mask(Req_Op_En, en, ADDR_ONLY);
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
	u32 val = lane3 << LANE3_MAP_BIT_POSISION
		| lane2 << LANE2_MAP_BIT_POSISION
		| lane1 << LANE1_MAP_BIT_POSISION
		| lane0 << LANE0_MAP_BIT_POSISION;

	displayport_write(Lane_Map, val);
}

void displayport_reg_set_lane_map_config(struct displayport_device *displayport)
{
#if defined(CONFIG_CCIC_NOTIFIER)
	switch (displayport->ccic_notify_dp_conf) {
	case CCIC_NOTIFY_DP_PIN_UNKNOWN:
		break;

	case CCIC_NOTIFY_DP_PIN_A:
		if (displayport->dp_sw_sel)
			displayport_reg_set_lane_map(2, 0, 1, 3);
		else
			displayport_reg_set_lane_map(3, 1, 0, 2);
		break;

	case CCIC_NOTIFY_DP_PIN_B:
		if (displayport->dp_sw_sel)
			displayport_reg_set_lane_map(1, 0, 2, 3);
		else
			displayport_reg_set_lane_map(3, 2, 0, 1);
		break;

	case CCIC_NOTIFY_DP_PIN_C:
	case CCIC_NOTIFY_DP_PIN_E:
	case CCIC_NOTIFY_DP_PIN_D:
	case CCIC_NOTIFY_DP_PIN_F:
		if (displayport->dp_sw_sel)
			displayport_reg_set_lane_map(0, 1, 2, 3);
		else
			displayport_reg_set_lane_map(3, 2, 1, 0);
		break;

	default:
		break;
	}
#endif
}

void displayport_reg_init(void)
{
	struct displayport_device *displayport = get_displayport_drvdata();

	displayport_reg_sw_reset();
	displayport_reg_turn_on_phy(displayport);
	udelay(110);	/* wait for 100us + 10% margin */
	displayport_reg_wait_phy_pll_lock();
	udelay(165);	/* wait for 150us + 10% margin */
	displayport_reg_init_function_enable();
	displayport_reg_set_interrupt(0);	/*disable irq*/
	displayport_reg_set_lane_map_config(displayport);
}

void displayport_reg_set_video_configuration(u8 bpc)
{
	displayport_reg_set_daynamic_range(CEA_RANGE);
	displayport_write_mask(Video_Control_2, (bpc)?1:0, IN_BPC);	/* 0:6bits, 1:8bits */
	displayport_write_mask(Video_Control_2, 0, IN_COLOR_F);		/* RGB */
	displayport_write_mask(Video_Control_10, 0, F_SEL);		/* Video Format Auto Calculation mode */
	displayport_write_mask(DP_System_Control_4, 0, FIX_M_VID);	/* M_VID Auto Calculation mode */
}

void displayport_reg_set_bist_video_configuration(videoformat video_format, u8 bpc, u8 type, u8 range)
{
	displayport_reg_set_daynamic_range((range)?CEA_RANGE:VESA_RANGE);
	displayport_write_mask(Video_Control_2, (bpc)?1:0, IN_BPC);	/* 0:6bits, 1:8bits */
	displayport_write_mask(Video_Control_2, 0, IN_COLOR_F);		/* RGB */
	displayport_write_mask(Video_Control_10, 0, F_SEL);		/* 0 in bist mode */
	/*displayport_write_mask(Video_Control_10, 1, VSYNC_P_CFG);*/	/*Vertical Pulse Polarity: negative*/
	/*displayport_write_mask(Video_Control_10, 1, HSYNC_P_CFG);*/	/*Horizontal Pulse Polarity: negative*/
	displayport_reg_video_format_register_setting(video_format);
	displayport_write_mask(DP_System_Control_4, 0, FIX_M_VID);	/* M_VID Auto Calculation mode */
	displayport_write_mask(Video_Control_4, type, BIST_TYPE);	/* Display BIST type */
	displayport_reg_set_video_bist_mode(1);

	displayport_info("set bist video config format:%d range:%d bpc:%d type:%d\n",
		video_format, (range)?1:0, (bpc)?1:0, type);
}

void displayport_reg_set_bist_video_configuration_for_blue_screen(videoformat video_format)
{
	displayport_reg_set_daynamic_range(CEA_RANGE);
	displayport_write_mask(Video_Control_2, 1, IN_BPC);		/* 8 bits */
	displayport_write_mask(Video_Control_2, 0, IN_COLOR_F);		/* RGB */
	displayport_write_mask(Video_Control_10, 0, F_SEL);		/* 0 in bist mode */
	displayport_reg_video_format_register_setting(video_format);
	displayport_write_mask(DP_System_Control_4, 0, FIX_M_VID);	/* M_VID Auto Calculation mode */
	displayport_write(HOST_BIST_DATA_R_ADD, 0x00);
	displayport_write(HOST_BIST_DATA_G_ADD, 0x00);
	displayport_write(HOST_BIST_DATA_B_ADD, 0xFF);
	displayport_write_mask(Video_Control_4, 1, USER_BIST_DATA_EN);
	displayport_reg_set_video_bist_mode(1);
	displayport_dbg("set bist video config for blue screen\n");
}

void displayport_reg_set_avi_infoframe(struct infoframe avi_infoframe)
{
	int i;

	for (i = 0; i < avi_infoframe.length; i++)
		displayport_write(AVI_infoFrame_Packet_Register_AVI_Data_Byte_1 + i * 4, avi_infoframe.data[i]);

	displayport_write_mask(Packet_Send_Control, 1, AVI_UD);
	displayport_write_mask(Packet_Send_Control, 1, AVI_EN);
}

void displayport_reg_set_audio_infoframe(struct infoframe audio_infoframe, u32 en)
{
	int i;

	for (i = 0; i < audio_infoframe.length; i++)
		displayport_write(Audio_infoFrame_Packet_Register_AVI_Data_Byte_1 + i * 4, audio_infoframe.data[i]);

	displayport_write_mask(Packet_Send_Control, en, AUDIO_INFO_UP);
	displayport_write_mask(Packet_Send_Control, en, AUDIO_INFOR_EN);
}

void displayport_reg_start(void)
{
	displayport_write_mask(Video_Control_1, 1, VIDEO_EN);
}


void displayport_reg_video_mute(u32 en)
{
/*	displayport_dbg("set mute %d\n", en);
 *	displayport_write_mask(Video_Control_1, en, VIDEO_MUTE);
 */
}

void displayport_reg_stop(void)
{
	displayport_write_mask(Video_Control_1, 0, VIDEO_EN);
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
	u32 val = en ? ~0 : 0;

	displayport_write_mask(DP_System_Control_3, val, F_VALID|VALID_CTRL);
	displayport_write_mask(Video_Control_4, val, BIST_EN);
}

void displayport_reg_set_sa_bist_video_configuration(videoformat video_format, u32 bist_type, u32 bist_width)
{
	displayport_reg_set_daynamic_range(VESA_RANGE);
	displayport_write_mask(Video_Control_2, 1, IN_BPC);		/* 8 bits */
	displayport_write_mask(Video_Control_2, 0, IN_COLOR_F);		/* RGB */
	displayport_write_mask(Video_Control_10, 0, F_SEL);		/* 0 in bist mode */
	displayport_reg_video_format_register_setting(video_format);
	displayport_write_mask(DP_System_Control_4, 0, FIX_M_VID);	/* M_VID Auto Calculation mode */
	displayport_write_mask(Video_Control_4, bist_type, BIST_TYPE);
	displayport_write_mask(Video_Control_4, bist_width, BIST_WIDTH);
	displayport_reg_set_sa_bist_mode(1);
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
		displayport_write_mask(DP_System_Control_4, 0, FIX_M_AUD);
	} else {
		m_value = audio_sync_m_n[0][array_set][audio_sampling_freq];
		n_value = audio_sync_m_n[1][array_set][audio_sampling_freq];
		displayport_write_mask(DP_System_Control_4, 1, FIX_M_AUD);
	}

	displayport_write(DP_Maud_Configure_Register_0, m_value&0xFF);
	displayport_write(DP_Maud_Configure_Register_1, (m_value>>8)&0xFF);
	displayport_write(DP_Maud_Configure_Register_2, (m_value>>16)&0xFF);

	displayport_write(DP_Naud_Configure_Register_0, n_value&0xFF);
	displayport_write(DP_Naud_Configure_Register_1, (n_value>>8)&0xFF);
	displayport_write(DP_Naud_Configure_Register_2, (n_value>>16)&0xFF);
}

void displayport_reg_set_audio_function_enable(u32 en)
{
	u32 val = en ? 0 : ~0; /* 0 is enable */

	displayport_write_mask(Function_En_1, val, AUDIO_FUNC_EN_N_STR0);
}

void displayport_reg_set_audio_master_mode(void)
{
	displayport_write_mask(General_Control, 0, AUDIO_MODE);
}

void displayport_reg_set_dma_burst_size(enum audio_dma_word_length word_length)
{
	displayport_write_mask(General_Control, word_length, DMA_BURST_SEL);
}

void displayport_reg_set_dma_pack_mode(enum audio_16bit_dma_mode dma_mode)
{
	displayport_write_mask(General_Control, dma_mode, AUDIO_BIT_MAPPING_TYPE);
}

void displayport_reg_set_pcm_size(enum audio_bit_per_channel audio_bit_size)
{
	displayport_write_mask(General_Control, audio_bit_size, PCM_SIZE);
}

void displayport_reg_set_audio_ch_status_same(u32 en)
{
	displayport_write_mask(General_Control, en, AUDIO_CH_STATUS_SAME);
}

void displayport_reg_set_audio_ch(u32 audio_ch_cnt)
{
	displayport_write_mask(Master_audio_Buffer_control_register,
				audio_ch_cnt - 1, MASTER_AUDIO_CHANNEL_COUNT);
}

void displayport_reg_set_audio_fifo_function_enable(u32 en)
{
	u32 val = en ? 0 : ~0; /* 0 is enable */

	displayport_write_mask(Function_En_1, val, AUDIO_FIFO_FUNC_EN_N_STR0);
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

	displayport_write(Maud_register_in_audio_master_mode, m_aud_master[audio_sampling_freq]);
	displayport_write(Naud_register_in_audio_master_mode, n_aud_master[n_aud_master_set]);
}

void displayport_reg_set_dp_audio_enable(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write_mask(DP_Audio_Control_Register, val, DP_AUDIO_EN);
}

void displayport_reg_set_audio_master_mode_enable(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write_mask(General_Control, val, AUDIO_MASTER_MODE_EN);
}

void displayport_reg_set_ch_status_ch_cnt(u32 audio_ch_cnt)
{
	displayport_write_mask(Audio_BIST_Channel_Status_Register_3,
				audio_ch_cnt, CH_NUM);

	displayport_write_mask(Audio_BIST_Channel_Status_Register_3,
				audio_ch_cnt, SOURCE_NUM);

	displayport_write_mask(Master_audio_Packet_GP0_Status_Register_2,
				audio_ch_cnt, CH_NUM);

	displayport_write_mask(Master_audio_Packet_GP0_Status_Register_2,
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

	displayport_write_mask(Audio_BIST_Channel_Status_Register_5,
		word_max, WORD_MAX);

	displayport_write_mask(Audio_BIST_Channel_Status_Register_5,
		sample_word_length, WORD_LENGTH);

	displayport_write_mask(Master_audio_Packet_GP0_Status_Register_4,
		word_max, WORD_MAX);

	displayport_write_mask(Master_audio_Packet_GP0_Status_Register_4,
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

	displayport_write_mask(Audio_BIST_Channel_Status_Register_4, fs_freq, FS_FREQ);
	displayport_write_mask(Master_audio_Packet_GP0_Status_Register_3, fs_freq, FS_FREQ);
}

void displayport_reg_set_ch_status_clock_accuracy(enum audio_clock_accuracy clock_accuracy)
{
	displayport_write_mask(Audio_BIST_Channel_Status_Register_4, clock_accuracy, CLK_ACCUR);
	displayport_write_mask(Master_audio_Packet_GP0_Status_Register_3, clock_accuracy, CLK_ACCUR);
}

void displayport_reg_set_hdcp22_system_enable(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write_mask(SEC_DP_HDCP22_SYS_EN_APB4, val, System_Enable);
}

void displayport_reg_set_hdcp22_mode(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write_mask(Function_En_3, val, HDCP22_MODE);
}

void displayport_reg_set_hdcp22_encryption_enable(u32 en)
{
	u32 val = en ? ~0 : 0;

	displayport_write_mask(Function_En_3, val, HDCP22_ENC_EN);
}

void displayport_reg_set_aux_pn_inv(u32 val)
{
	displayport_write_mask(AUX_Ch_MISC_Ctrl_0, val, AUX_PN_INV);
}
