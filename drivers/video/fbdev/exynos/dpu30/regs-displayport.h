/* linux/drivers/video/fbdev/exynos/dpu/regs-displayport.h
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Register definition file for Samsung vpp driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef DISPLAYPORT_REGS_H_
#define DISPLAYPORT_REGS_H_

#define Function_En_1                           (0x18)
#define GTC_FUNC_EN_N			        (1 << 7)
#define VIDEO_CAPTURE_FUNC_EN_N_STR0		(1 << 6)
#define VIDEO_FUNC_EN_N_STR0		        (1 << 5)
#define AUDIO_FIFO_FUNC_EN_N_STR0		(1 << 4)
#define AUDIO_FUNC_EN_N_STR0		        (1 << 3)
#define HDCP_FUNC_EN_N				(1 << 2)
#define SOFTWARE_FUNC_EN_N			(1 << 0)

#define SYSTEM_COMMON_FUNCTION_ENABLE		(0x0018)
#define HDCP22_FUNC_EN				(0x01 << 4)
#define HDCP13_FUNC_EN				(0x01 << 3)
#define GTC_FUNC_EN				(0x01 << 2)
#define PCS_FUNC_EN				(0x01 << 1)
#define AUX_FUNC_EN				(0x01 << 0)

#define Function_En_2                           (0x1C)
#define VIDEO_CAPTURE_FUNC_EN_N_STR1		(1 << 6)
#define VIDEO_FUNC_EN_N_STR1			(1 << 5)
#define AUDIO_FIFO_FUNC_EN_N_STR1               (1 << 4)
#define AUDIO_FUNC_EN_N_STR1		        (1 << 3)
#define AUX_FUNC_EN_N		                (1 << 2)
#define SERDES_FIFO_FUNC_EN_N			(1 << 1)
#define LS_CLOCK_DOMAIN_FUNC_EN_N		(1 << 0)

#define Function_En_4                           (0x24)
#define STRM_CLK_EN		                (1 << 0)

#define SYSTEM_SST1_FUNCTION_ENABLE		(0x001C)
#define SST1_LH_PWR_ON_STATUS			(0x01 << 5)
#define SST1_LH_PWR_ON				(0x01 << 4)
#define SST1_AUDIO_FIFO_FUNC_EN			(0x01 << 2)
#define SST1_AUDIO_FUNC_EN			(0x01 << 1)
#define SST1_VIDEO_FUNC_EN			(0x01 << 0)

#define SYSTEM_PLL_LOCK_CONTROL			(0x002C)
#define PLL_LOCK_STATUS				(0x01 << 4)
#define PLL_LOCK_FORCE				(0x01 << 3)
#define PLL_LOCK_FORCE_EN			(0x01 << 2)

#define SW_Reset                                (0x100)
#define DP_TX_SW_RESET		                (1 << 0)

#define Env_Control                             (0x104)
#define I2C_CLK_SEL		                (1 << 0)

#define Lane_Map				(0x35C)
#define LANE3_MAP				(3 << 6)
#define LANE3_MAP_BIT_POSISION			(6)
#define LANE2_MAP				(3 << 4)
#define LANE2_MAP_BIT_POSISION			(4)
#define LANE1_MAP				(3 << 2)
#define LANE1_MAP_BIT_POSISION			(2)
#define LANE0_MAP				(3 << 0)
#define LANE0_MAP_BIT_POSISION			(0)

#define Interrupt_Status_Top                    (0x3C0)
#define STR1_INT_STATE		                (1 << 2)
#define STR2_INT_STATE				(1 << 1)
#define INT_STATE		                (1 << 0)

#define Common_Interrupt_Status_2               (0x3C8)
#define PLL_LOCK_CHG		                (1 << 7)
#define ENC_EN_CHG				(1 << 6)
#define HDCP_LINK_CHK_FAIL		        (1 << 5)
#define R0_CHECK_FLAG		                (1 << 4)
#define BKSV_RDY				(1 << 3)
#define SHA_DONE		                (1 << 2)
#define AUTH_STATE_CHG				(1 << 1)
#define AUTH_DONE		                (1 << 0)

#define Common_Interrupt_Status_4               (0x3D0)
#define HPD_CHG                                 (1 << 2)
#define HPD_LOST                                (1 << 1)
#define PLUG                                    (1 << 0)

#define DP_Interrupt_Status                     (0x3DC)
#define HOT_PLUG_DET				(1 << 6)
#define TRAINING_FINISH				(1 << 5)
#define SOFT_INTERRUPT		                (1 << 4)
#define SINK_LOST_CLEAR				(1 << 3)
#define LINK_LOST_CLEAR		                (1 << 2)
#define RPLY_RECEIV				(1 << 1)
#define AUX_ERR					(1 << 0)

#define Interrupt_Mask_2                        (0x3E4)
#define PLL_LOCK_CHG		                (1 << 7)
#define ENC_EN_CHG				(1 << 6)
#define HDCP_LINK_CHK_FAIL		        (1 << 5)
#define R0_CHECK_FLAG		                (1 << 4)
#define BKSV_RDY				(1 << 3)
#define SHA_DONE		                (1 << 2)
#define AUTH_STATE_CHG				(1 << 1)
#define AUTH_DONE		                (1 << 0)

#define Common_Interrupt_Mask_4                 (0x3EC)
#define HPD_CHG                                 (1 << 2)
#define HPD_LOST                                (1 << 1)
#define PLUG                                    (1 << 0)

#define DP_Interrupt_Status_Mask_1              (0x3F8)
#define SOFT_INTERRUPT_MASK                     (1 << 7)
#define HOT_PLUG_DET_MASK                       (1 << 6)
#define TRAINING_FINISH_MASK		        (1 << 5)
#define SINK_LOST_MASK                          (1 << 3)
#define LINK_LOST_MASK		                (1 << 2)
#define RPLY_RECEIV_MASK			(1 << 1)
#define AUX_ERR_MASK		                (1 << 0)

#define Interrupt_Control                       (0x3FC)
#define SOFT_INT_CTRL                           (1 << 2)
#define INT_POL                                 (1 << 0)

#define DP_System_Control_HPD                   (0x608)
#define HPD_EVENT_UNPLUG                        (1 << 10)
#define HPD_EVENT_PLUG                          (1 << 9)
#define HPD_EVENT_IRQ                           (1 << 8)
#define HPD_EVENT_CTRL                          (1 << 7)
#define PD_STATUS                               (1 << 6)
#define F_HPD                                   (1 << 5)
#define HPD_CTRL                                (1 << 4)
#define HPD_HDCP		                (1 << 3)

#define DP_MISC_Control                         (0x648)
#define MISC_CTRL_EN                            (1 << 7)
#define HDCP_HPD_RST                            (1 << 6)
#define LINK_CLK_SEL                            (3 << 4)

#define DP_Main_Link_Bandwidth_Setting          (0x680)
#define LINK_BW_SET                             (0x1F << 0)

#define DP_Main_Link_Lane_Count                 (0x684)
#define LANE_COUNT_SET                          (7 << 0)

#define DP_Training_Pattern_Set                 (0x688)
#define SCRAMBLING_DISABLE                      (1 << 5)
#define LINK_QUAL_PATTERN_SET                   (7 << 2)
#define TRAINING_PATTERN_SET                    (3 << 0)

#define DP_Lane_0_Link_Training_Control         (0x68C)
#define MAX_PRE_REACH_0                         (1 << 5)
#define PRE_EMPHASIS_SET_0                      (3 << 3)
#define MAX_DRIVE_REACH_0                       (1 << 2)
#define DRIVE_CURRENT_SET_0                     (3 << 0)

#define DP_Lane_1_Link_Training_Control         (0x690)
#define MAX_PRE_REACH_1                         (1 << 5)
#define PRE_EMPHASIS_SET_1                      (3 << 3)
#define MAX_DRIVE_REACH_1                       (1 << 2)
#define DRIVE_CURRENT_SET_1                     (3 << 0)

#define DP_Lane_2_Link_Training_Control         (0x694)
#define MAX_PRE_REACH_2                         (1 << 5)
#define PRE_EMPHASIS_SET_2                      (3 << 3)
#define MAX_DRIVE_REACH_2                       (1 << 2)
#define DRIVE_CURRENT_SET_2                     (3 << 0)

#define DP_Lane_3_Link_Training_Control         (0x698)
#define MAX_PRE_REACH_3                         (1 << 5)
#define PRE_EMPHASIS_SET_3                      (3 << 3)
#define MAX_DRIVE_REACH_3                       (1 << 2)
#define DRIVE_CURRENT_SET_3                     (3 << 0)

#define DP_HW_LINK_TRAINING_CONTROL_Register    (0x6A0)
#define TRAINING_ERROR_CODE                     (7 << 4)
#define TRAINING_EN                             (1 << 0)

#define DP_Debug_1                              (0x6C0)
#define PLL_LOCK                                (1 << 4)
#define F_PLL_LOCK                              (1 << 3)
#define PLL_LOCK_CTRL                           (1 << 2)
#define PN_INV                                  (1 << 0)

#define DP_HPD_Deglitch_Low_Byte                (0x6C4)
#define HPD_DEGLITCH_L                          (0xFF << 0)

#define DP_HPD_Deglitch_High_Byte               (0x6C8)
#define HPD_DEGLITCH_H                          (0x3F << 0)

#define DP_Link_Debug_Control                   (0x6E0)
#define TEST_CLK_SEL                            (7 << 5)
#define NEW_PRBS7                               (1 << 4)
#define DIS_FIFO_RST                            (1 << 3)
#define DISABLE_AUTO_RESET_ENCODER              (1 << 2)
#define PRBS31_EN                               (1 << 0)

#define DP_Scrambler_Reset_Counter_Value        (0x6E4)
#define SCRAMBER_COUNT_RESET_VALUE              (0x1FF << 0)

#define DP_Scrambler_Reset_Start_Ctrl           (0x6E8)
#define SCRAMBLER_RESET_START_CTRL              (1 << 0)

#define DP_Scrambler_Start_Number_Ctrl          (0x6EC)
#define R_SEND_SR_EN                            (1 << 10)
#define R_SR_START_NUMBER                       (0x1F << 0)

#define DP_TOP_GNS_Control                      (0x72C)
#define TPS3_EN                                 (1 << 7)
#define EQ_LOOP_COUNT                           (1 << 6)
#define SCRAMBLE_CTRL                           (1 << 4)
#define SCRAMBLE_IN_EX                          (1 << 3)
#define DISABLE_SERDES_FIFO_RSET                (1 << 2)
#define HRB2_EYE_SR_CTRL                        (3 << 0)

#define DP_Test_Pattern_0                       (0x784)
#define TEST_80BIT_PATTERN_0                    (0xFF << 0)

#define DP_Test_Pattern_1                       (0x788)
#define TEST_80BIT_PATTERN_1                    (0xFF << 0)

#define DP_Test_Pattern_2                       (0x78C)
#define TEST_80BIT_PATTERN_2                    (0xFF << 0)

#define DP_Test_Pattern_3                       (0x790)
#define TEST_80BIT_PATTERN_3                    (0xFF << 0)

#define DP_Test_Pattern_4                       (0x794)
#define TEST_80BIT_PATTERN_4                    (0xFF << 0)

#define DP_Test_Pattern_5                       (0x798)
#define TEST_80BIT_PATTERN_5                    (0xFF << 0)

#define DP_Test_Pattern_6                       (0x79C)
#define TEST_80BIT_PATTERN_6                    (0xFF << 0)

#define DP_Test_Pattern_7                       (0x7A0)
#define TEST_80BIT_PATTERN_7                    (0xFF << 0)

#define DP_Test_Pattern_8                       (0x7A4)
#define TEST_80BIT_PATTERN_8                    (0xFF << 0)

#define DP_Test_Pattern_9                       (0x7A8)
#define TEST_80BIT_PATTERN_9                    (0xFF << 0)

#define HBR2_Eye_SR_Low                         (0x7AC)
#define HBR2_EYE_SR_L                           (0xFF << 0)

#define HBR2_Eye_SR_High                        (0x7B0)
#define HBR2_EYE_SR_H                           (0xFF << 0)

#define Test_Pattern_Register                   (0x8A0)
#define TEST_PATTERN                            (0x3F << 0)

#define AUX_CONTROL				(0x1000)
#define AUX_POWER_DOWN				(0x01 << 16)
#define AUX_REPLY_TIMER_MODE			(0x03 << 12)
#define AUX_RETRY_TIMER				(0x07 << 8)
#define AUX_PN_INV				(0x01 << 1)
#define REG_MODE_SEL				(0x01 << 0)

#define AUX_TRANSACTION_START			(0x1004)
#define AUX_TRAN_START				(0x01 << 0)

#define AUX_BUFFER_CLEAR			(0x1008)
#define AUX_BUF_CLR				(0x01 << 0)

#define AUX_ADDR_ONLY_COMMAND			(0x100C)
#define ADDR_ONLY_CMD				(0x01 << 0)

#define AUX_REQUEST_CONTROL			(0x1010)
#define REQ_COMM				(0x0F << 28)
#define REQ_ADDR				(0xFFFFF << 8)
#define REQ_LENGTH				(0x3F << 0)

#define AUX_COMMAND_CONTROL			(0x1014)
#define DEFER_CTRL_EN				(0x01 << 8)
#define DEFER_COUNT				(0x7F << 0)

#define AUX_MONITOR_1				(0x1018)
#define AUX_BUF_DATA_COUNT			(0x7F << 24)
#define AUX_DETECTED_PERIOD_MON			(0x1FF << 12)
#define AUX_CMD_STATUS				(0x0F << 8)
#define AUX_RX_COMM				(0x0F << 4)
#define AUX_LAST_MODE				(0x01 << 3)
#define AUX_BUSY				(0x01 << 2)
#define AUX_REQ_WAIT_GRANT			(0x01 << 1)
#define AUX_REQ_SIGNAL				(0x01 << 0)

#define AUX_MONITOR_2				(0x101C)
#define AUX_ERROR_NUMBER			(0xFF << 0)

#define AUX_TX_DATA_SET0			(0x1030)
#define TX_DATA_3				(0xFF << 24)
#define TX_DATA_2				(0xFF << 16)
#define TX_DATA_1				(0xFF << 8)
#define TX_DATA_0				(0xFF << 0)

#define AUX_TX_DATA_SET1			(0x1034)
#define TX_DATA_7				(0xFF << 24)
#define TX_DATA_6				(0xFF << 16)
#define TX_DATA_5				(0xFF << 8)
#define TX_DATA_4				(0xFF << 0)

#define AUX_TX_DATA_SET2			(0x1038)
#define TX_DATA_11				(0xFF << 24)
#define TX_DATA_10				(0xFF << 16)
#define TX_DATA_9				(0xFF << 8)
#define TX_DATA_8				(0xFF << 0)

#define AUX_TX_DATA_SET3			(0x103C)
#define TX_DATA_15				(0xFF << 24)
#define TX_DATA_14				(0xFF << 16)
#define TX_DATA_13				(0xFF << 8)
#define TX_DATA_12				(0xFF << 0)

#define AUX_RX_DATA_SET0			(0x1040)
#define RX_DATA_3				(0xFF << 24)
#define RX_DATA_2				(0xFF << 16)
#define RX_DATA_1				(0xFF << 8)
#define RX_DATA_0				(0xFF << 0)

#define AUX_RX_DATA_SET1			(0x1044)
#define RX_DATA_7				(0xFF << 24)
#define RX_DATA_6				(0xFF << 16)
#define RX_DATA_5				(0xFF << 8)
#define RX_DATA_4				(0xFF << 0)

#define AUX_RX_DATA_SET2			(0x1048)
#define RX_DATA_11				(0xFF << 24)
#define RX_DATA_10				(0xFF << 16)
#define RX_DATA_9				(0xFF << 8)
#define RX_DATA_8				(0xFF << 0)

#define AUX_RX_DATA_SET3			(0x104C)
#define RX_DATA_15				(0xFF << 24)
#define RX_DATA_14				(0xFF << 16)
#define RX_DATA_13				(0xFF << 8)
#define RX_DATA_12				(0xFF << 0)

#define IF_CRC_Control_1                        (0x1114)
#define IF_CRC_CLEAR                            (1 << 13)
#define IF_CRC_PASS                             (1 << 12)
#define IF_CRC_FAIL                             (1 << 8)
#define IF_CRC_SW_COMPARE                       (1 << 4)
#define IF_CRC_EN                               (1 << 0)

#define IF_CRC_Control_2                        (0x1118)
#define IF_CRC_R_REF                            (0xFFFF << 16)
#define IF_CRC_R_RESULT                         (0xFFFF << 0)

#define IF_CRC_Control_3                        (0x111C)
#define IF_CRC_G_REF                            (0xFFFF << 16)
#define IF_CRC_G_RESULT                         (0xFFFF << 0)

#define IF_CRC_Control_4                        (0x1120)
#define IF_CRC_B_REF                            (0xFFFF << 16)
#define IF_CRC_B_RESULT                         (0xFFFF << 0)

#define SA_CRC_Control_1                        (0x1124)
#define SA_CRC_CLEAR                            (1 << 13)
#define SA_CRC_SW_COMPARE                       (1 << 12)
#define SA_CRC_LN3_PASS                         (1 << 11)
#define SA_CRC_LN2_PASS                         (1 << 10)
#define SA_CRC_LN1_PASS                         (1 << 9)
#define SA_CRC_LN0_PASS                         (1 << 8)
#define SA_CRC_LN3_FAIL                         (1 << 7)
#define SA_CRC_LN2_FAIL                         (1 << 6)
#define SA_CRC_LN1_FAIL                         (1 << 5)
#define SA_CRC_LN0_FAIL                         (1 << 4)
#define SA_CRC_LN3_EN                           (1 << 3)
#define SA_CRC_LN2_EN                           (1 << 2)
#define SA_CRC_LN1_EN                           (1 << 1)
#define SA_CRC_LN0_EN                           (1 << 0)

#define SA_CRC_Control_2                        (0x1128)
#define SA_CRC_LN0_REF                          (0xFFFF << 16)
#define SA_CRC_LN0_RESULT                       (0xFFFF << 0)

#define SA_CRC_Control_3                        (0x112C)
#define SA_CRC_LN1_REF                          (0xFFFF << 16)
#define SA_CRC_LN1_RESULT                       (0xFFFF << 0)

#define SA_CRC_Control_4                        (0x1130)
#define SA_CRC_LN2_REF                          (0xFFFF << 16)
#define SA_CRC_LN2_RESULT                       (0xFFFF << 0)

#define SA_CRC_Control_5                        (0x1134)
#define SA_CRC_LN3_REF                          (0xFFFF << 16)
#define SA_CRC_LN3_RESULT                       (0xFFFF << 0)

#define HOST_BIST_DATA_R_ADD                    (0x1138)
#define HOST_BIST_DATA_R                        (0xFF << 0)

#define HOST_BIST_DATA_G_ADD                    (0x113C)
#define HOST_BIST_DATA_G                        (0xFF << 0)

#define HOST_BIST_DATA_B_ADD			(0x1140)
#define HOST_BIST_DATA_B                        (0xFF << 0)

#define AVI_infoFrame_Packet_Register_AVI_Data_Byte_1   (0x11D0)
#define AVI_DB1                                 (0xFF << 0)

#define AVI_infoFrame_Packet_Register_AVI_Data_Byte_2   (0x11D4)
#define AVI_DB2                                 (0xFF << 0)

#define AVI_infoFrame_Packet_Register_AVI_Data_Byte_3   (0x11D8)
#define AVI_DB3                                 (0xFF << 0)

#define AVI_infoFrame_Packet_Register_AVI_Data_Byte_4   (0x11DC)
#define AVI_DB4                                 (0xFF << 0)

#define AVI_infoFrame_Packet_Register_AVI_Data_Byte_5   (0x11E0)
#define AVI_DB5                                 (0xFF << 0)

#define AVI_infoFrame_Packet_Register_AVI_Data_Byte_6   (0x11E4)
#define AVI_DB6                                 (0xFF << 0)

#define AVI_infoFrame_Packet_Register_AVI_Data_Byte_7   (0x11E8)
#define AVI_DB7                                 (0xFF << 0)

#define AVI_infoFrame_Packet_Register_AVI_Data_Byte_8   (0x11EC)
#define AVI_DB8                                 (0xFF << 0)

#define AVI_infoFrame_Packet_Register_AVI_Data_Byte_9   (0x11F0)
#define AVI_DB9                                 (0xFF << 0)

#define AVI_infoFrame_Packet_Register_AVI_Data_Byte_10  (0x11F4)
#define AVI_DB10                                (0xFF << 0)

#define AVI_infoFrame_Packet_Register_AVI_Data_Byte_11  (0x11F8)
#define AVI_DB11                                (0xFF << 0)

#define AVI_infoFrame_Packet_Register_AVI_Data_Byte_12  (0x11FC)
#define AVI_DB12                                (0xFF << 0)

#define AVI_infoFrame_Packet_Register_AVI_Data_Byte_13  (0x1200)
#define AVI_DB13                                (0xFF << 0)

#define Audio_infoFrame_Packet_Register_AVI_Data_Byte_1         (0x121C)
#define Audio_DB1                               (0xFF << 0)

#define Audio_infoFrame_Packet_Register_AVI_Data_Byte_2         (0x1220)
#define Audio_DB2                               (0xFF << 0)

#define Audio_infoFrame_Packet_Register_AVI_Data_Byte_3         (0x1224)
#define Audio_DB3                               (0xFF << 0)

#define Audio_infoFrame_Packet_Register_AVI_Data_Byte_4         (0x1228)
#define Audio_DB4                               (0xFF << 0)

#define Audio_infoFrame_Packet_Register_AVI_Data_Byte_5         (0x122C)
#define Audio_DB5                               (0xFF << 0)

#define Audio_infoFrame_Packet_Register_AVI_Data_Byte_6         (0x1230)
#define Audio_DB6                               (0xFF << 0)

#define Audio_infoFrame_Packet_Register_AVI_Data_Byte_7         (0x1234)
#define Audio_DB7                               (0xFF << 0)

#define Audio_infoFrame_Packet_Register_AVI_Data_Byte_8         (0x1238)
#define Audio_DB8                               (0xFF << 0)

#define Audio_infoFrame_Packet_Register_AVI_Data_Byte_9         (0x123C)
#define Audio_DB9                               (0xFF << 0)

#define Audio_infoFrame_Packet_Register_AVI_Data_Byte_10        (0x1240)
#define Audio_DB10                              (0xFF << 0)

#define Interrupt_Status                        (0x13C0)
#define INT_STATE                               (1 << 0)

#define Common_Interrupt_Status_1               (0x13C4)
#define VSYNC_DET                               (1 << 7)
#define VID_FORMAT_CHG                          (1 << 3)
#define AUD_CLK_CHG                             (1 << 2)
#define VID_CLK_CHG                             (1 << 1)

#define Common_Interrupt_Status_3               (0x13CC)
#define AFIFO_UNDER                             (1 << 7)
#define AFIFO_OVER                              (1 << 6)

#define Interrupt_Mask_1                        (0x13E0)
#define VSYNC_DET_MASK                          (1 << 7)
#define VID_FORMAT_CHG_MASK                     (1 << 3)
#define INT_AUDIO_CLK_CHANGE_MASK               (1 << 2)
#define VID_CLK_CHG_MASK                        (1 << 1)

#define Interrupt_Mask_3                        (0x13E8)
#define COMMON_INT_MASK_3                       (3 << 6)

#define DP_System_Control_1                     (0x1600)
#define DET_STA                                 (1 << 2)
#define FORCE_DET                               (1 << 1)
#define DET_CTRL                                (1 << 0)

#define DP_System_Control_2                     (0x1604)
#define CHA_CRI                                 (0xF << 4)
#define CHA_STA                                 (1 << 2)
#define FORCE_CHA                               (1 << 1)
#define CHA_CTRL                                (1 << 0)

#define DP_System_Control_3                     (0x1608)
#define STRM_VALID                              (1 << 2)
#define F_VALID                                 (1 << 1)
#define VALID_CTRL                              (1 << 0)

#define Packet_Send_Control                     (0x1640)
#define AUDIO_INFO_UP                           (1 << 7)
#define AVI_UD                                  (1 << 6)
#define MPEG_UD                                 (1 << 5)
#define SPD_INFO_UP                             (1 << 4)
#define AUDIO_INFOR_EN                          (1 << 3)
#define AVI_EN									(1 << 2)
#define MPEG_EN                                 (1 << 1)
#define SPD_INFO_EN                             (1 << 0)

#define DP_FIFO_Threshold                       (0x1730)
#define TH_CTRL                                 (1 << 5)
#define TH_VALUE                                (0x1F << 0)

#define DP_GNS_Control                          (0x1734)
#define VIDEO_MAP_CTRL                          (1 << 1)
#define RS_CTRL                                 (1 << 0)

#define CRC_Control_4                           (0x1890)
#define AUD_CRC_PK_NUM                          (0x3FF << 16)
#define AUD_CRC_FLUSH                           (1 << 3)
#define VID_CRC_FLUSH                           (1 << 2)
#define AUD_CRC_EN                              (1 << 1)
#define VID_CRC_EN                              (1 << 0)

#define CRC_Result                              (0x1894)
#define AUD_CRC_RESULT                          (0xFFFF << 16)
#define VID_CRC_RESULT                          (0xFFFF << 0)

#define DP_IRQ_VECTOR                           (0x30A0)
#define IRQ_VECTOR                              (0x3F << 0)

#define DP_LINK_STATUS_1                        (0x30A4)
#define SYMBOL_LOCK_1                           (1 << 6)
#define EQ_DONE_1                               (1 << 5)
#define CR_DONE_1                               (1 << 4)
#define SYMBOL_LOCK_0                           (1 << 2)
#define EQ_DONE_0                               (1 << 1)
#define CR_DONE_0                               (1 << 0)

#define DP_LINK_STATUS_2                        (0x30A8)
#define SYMBOL_LOCK_3                           (1 << 6)
#define EQ_DONE_3                               (1 << 5)
#define CR_DONE_3                               (1 << 4)
#define SYMBOL_LOCK_2                           (1 << 2)
#define EQ_DONE_2                               (1 << 1)
#define CR_DONE_2                               (1 << 0)

#define DP_SINK_COUNT                           (0x30AC)
#define CP_READY                                (1 << 6)
#define SINK_COUNT                              (0x3F << 0)

#define DP_SINK_STATUS                          (0x30B0)
#define SINK_STATUS_1                           (1 << 1)
#define SINK_STATUS_0                           (1 << 0)

#define DP_ALIGN_STATUS                         (0x30B4)
#define LINK_STATUS_UPDATED                     (1 << 7)
#define DOWNSTREAM_PORT_STATUS_CHANGED          (1 << 6)
#define INTERLANE_ALIGN_DONE                    (1 << 0)

#define TX_GTC_VAL1                             (0x30C0)
#define TX_GTC_VAL_1                            (0xFF << 0)

#define TX_GTC_VAL2                             (0x30C4)
#define TX_GTC_VAL_2                            (0xFF << 0)

#define TX_GTC_VAL3                             (0x30C8)
#define TX_GTC_VAL_3                            (0xFF << 0)

#define TX_GTC_VAL4                             (0x30CC)
#define TX_GTC_VAL_4                            (0xFF << 0)

#define RX_GTC_VAL1                             (0x30D0)
#define RX_GTC_VAL_1                            (0xFF << 0)

#define RX_GTC_VAL2                             (0x30D4)
#define RX_GTC_VAL_2                            (0xFF << 0)

#define RX_GTC_VAL3                             (0x30D8)
#define RX_GTC_VAL_3                            (0xFF << 0)

#define RX_GTC_VAL4                             (0x30DC)
#define RX_GTC_VAL_4                            (0xFF << 0)

#define GTC_TP_Control_Register_0               (0x3300)
#define I_SPEC_GTC_IMPLEMENTATION               (1 << 29)
#define TP_SEL                                  (3 << 27)
#define TP_FRACTIONAL                           (0x7FFFF << 8)
#define SYMBOL_LOCK_2                           (1 << 2)
#define TP_INT1                                 (0xF << 4)
#define TP_INT2                                 (0xF << 0)

#define HDCP13_STATUS				(0x4000)
#define REAUTH_REQUEST				(0x01 << 7)
#define AUTH_FAIL				(0x01 << 6)
#define HW_1ST_AUTHEN_PASS			(0x01 << 5)
#define BKSV_VALID				(0x01 << 3)
#define ENCRYPT					(0x01 << 2)
#define HW_AUTHEN_PASS				(0x01 << 1)
#define AKSV_VALID				(0x01 << 0)

#define HDCP13_CONTROL_0			(0x4004)
#define SW_STORE_AN				(0x01 << 7)
#define SW_RX_REPEATER				(0x01 << 6)
#define HW_RE_AUTHEN				(0x01 << 5)
#define SW_AUTH_OK				(0x01 << 4)
#define HW_AUTH_EN				(0x01 << 3)
#define HDCP13_ENC_EN				(0x01 << 2)
#define HW_1ST_PART_ATHENTICATION_EN		(0x01 << 1)
#define HW_2ND_PART_ATHENTICATION_EN		(0x01 << 0)

#define HDCP13_CONTROL_1			(0x4008)
#define DPCD_REV_1_2				(0x01 << 3)
#define HW_AUTH_POLLING_MODE			(0x01 << 1)
#define HDCP_INT				(0x01 << 0)

#define HDCP13_AKSV_0				(0x4010)
#define AKSV0					(0xFFFFFFFF << 0)

#define HDCP13_AKSV_1				(0x4014)
#define AKSV1					(0xFF << 0)

#define HDCP13_AN_0				(0x4018)
#define AN0					(0xFFFFFFFF << 0)

#define HDCP13_AN_1				(0x401C)
#define AN1					(0xFFFFFFFF << 0)

#define HDCP13_BKSV_0				(0x4020)
#define BKSV0					(0xFFFFFFFF << 0)

#define HDCP13_BKSV_1				(0x4024)
#define BKSV1					(0xFF << 0)

#define HDCP13_R0_REG				(0x4028)
#define R0					(0xFFFF << 0)

#define HDCP13_BCAPS				(0x4030)
#define BCAPS					(0xFF << 0)

#define HDCP13_BINFO_REG			(0x4034)
#define BINFO					(0xFF << 0)

#define HDCP13_DEBUG_CONTROL			(0x4038)
#define CHECK_KSV				(0x01 << 2)
#define REVOCATION_CHK_DONE			(0x01 << 1)
#define HW_SKIP_RPT_ZERO_DEV			(0x01 << 0)

#define HDCP13_AUTH_DBG				(0x4040)
#define DDC_STATE				(0x07 << 5)
#define AUTH_STATE				(0x1F << 0)

#define HDCP13_ENC_DBG				(0x4044)
#define ENC_STATE				(0x07 << 3)

#define HDCP13_AM0_0				(0x4048)
#define AM0_0					(0xFFFFFFFF << 0)

#define HDCP13_AM0_1				(0x4048)
#define AM0_1					(0xFFFFFFFF << 0)

#define HDCP13_WAIT_R0_TIME			(0x4054)
#define HW_WRITE_AKSV_WAIT			(0xFF << 0)

#define HDCP13_LINK_CHK_TIME			(0x4058)
#define LINK_CHK_TIMER				(0xFF << 0)

#define HDCP13_REPEATER_READY_WAIT_TIME		(0x405C)
#define HW_RPTR_RDY_TIMER			(0xFF << 0)

#define HDCP13_READY_POLL_TIME			(0x4060)
#define POLLING_TIMER_TH			(0xFF << 0)

#define HDCP13_STREAM_ID_ENCRYPTION_CONTROL	(0x4068)
#define STRM_ID_ENC_UPDATE			(0x01 << 7)
#define STRM_ID_ENC				(0x7F << 0)

#define HDCP22_SYS_EN				(0x4400)
#define SYSTEM_ENABLE				(0x01 << 0)

#define HDCP22_CONTROL				(0x4404)
#define HDCP22_BYPASS_MODE			(0x01 << 1)
#define HDCP22_ENC_EN				(0x01 << 0)

#define HDCP22_CONTROL				(0x4404)
#define HDCP22_BYPASS_MODE			(0x01 << 1)
#define HDCP22_ENC_EN				(0x01 << 0)

#define HDCP22_STREAM_TYPE			(0x4454)
#define STREAM_TYPE				(0x01 << 0)

#define HDCP22_LVP				(0x4460)
#define LINK_VERIFICATION_PATTERN		(0xFFFF << 0)

#define HDCP22_LVP_GEN				(0x4464)
#define LVP_GEN					(0x01 << 0)

#define HDCP22_LVP_CNT_KEEP			(0x4468)
#define LVP_COUNT_KEEP_ENABLE			(0x01 << 0)

#define HDCP22_LANE_DECODE_CTRL			(0x4470)
#define ENHANCED_FRAMING_MODE			(0x01 << 3)
#define LVP_EN_DECODE_ENABLE			(0x01 << 2)
#define ENCRYPTION_SIGNAL_DECODE_ENABLE		(0x01 << 1)
#define LANE_DECODE_ENABLE			(0x01 << 0)

#define HDCP22_SR_VALUE				(0x4480)
#define SR_VALUE				(0xFF << 0)

#define HDCP22_CP_VALUE				(0x4484)
#define CP_VALUE				(0xFF << 0)

#define HDCP22_BF_VALUE				(0x4488)
#define BF_VALUE				(0xFF << 0)

#define HDCP22_BS_VALUE				(0x448C)
#define BS_VALUE				(0xFF << 0)

#define HDCP22_RIV_XOR				(0x4490)
#define RIV_XOR_LOCATION			(0x01 << 0)

#define HDCP22_RIV_0				(0x4500)
#define RIV_KEY_0				(0xFFFFFFFF << 0)

#define HDCP22_RIV_1				(0x4504)
#define RIV_KEY_1				(0xFFFFFFFF << 0)

#define SST1_MAIN_CONTROL			(0x5000)
#define MVID_MODE				(0x01 << 11)
#define MAUD_MODE				(0x01 << 10)
#define MVID_UPDATE_RATE			(0x03 << 8)
#define VIDEO_MODE				(0x01 << 6)
#define ENHANCED_MODE				(0x01 << 5)
#define ODD_TU_CONTROL				(0x01 << 4)

#define SST1_MAUD_MASTER_MODE			(0x505C)
#define MAUD_MASTER				(0xFFFFFFFF << 0)

#define SST1_NAUD_MASTER_MODE			(0x5060)
#define NAUD_MASTER				(0xFFFFFF << 0)

#define SST1_MAUD_SFR_CONFIGURE			(0x5064)
#define MAUD_SFR_CONFIG				(0xFFFFFF << 0)

#define SST1_NAUD_SFR_CONFIGURE			(0x5068)
#define NAUD_SFR_CONFIG				(0xFFFFFF << 0)

#define SST1_VIDEO_CONTROL			(0x5400)
#define STRM_VALID_MON				(0x01 << 10)
#define STRM_VALID_FORCE			(0x01 << 9)
#define STRM_VALID_CTRL				(0x01 << 8)
#define DYNAMIC_RANGE_MODE			(0x01 << 7)
#define BPC					(0x07 << 4)
#define COLOR_FORMAT				(0x03 << 2)
#define VSYNC_POLARITY				(0x01 << 1)
#define HSYNC_POLARITY				(0x01 << 0)

#define SST1_VIDEO_ENABLE			(0x5404)
#define VIDEO_EN				(0x01 << 0)

#define SST1_VIDEO_MASTER_TIMING_GEN		(0x5408)
#define VIDEO_MASTER_TIME_GEN			(0x01 << 0)

#define SST1_VIDEO_MUTE				(0x540C)
#define VIDEO_MUTE				(0x01 << 0)

#define SST1_VIDEO_FIFO_THRESHOLD_CONTROL	(0x5410)
#define GL_FIFO_TH_CTRL				(0x01 << 5)
#define GL_FIFO_TH_VALUE			(0x1F << 0)

#define SST1_VIDEO_HORIZONTAL_TOTAL_PIXELS	(0x5414)
#define H_TOTAL_MASTER				(0xFFFFFFFF << 0)

#define SST1_VIDEO_VERTICAL_TOTAL_PIXELS	(0x5418)
#define V_TOTAL_MASTER				(0xFFFFFFFF << 0)

#define SST1_VIDEO_HORIZONTAL_FRONT_PORCH	(0x541C)
#define H_F_PORCH_MASTER			(0xFFFFFFFF << 0)

#define SST1_VIDEO_HORIZONTAL_BACK_PORCH	(0x5420)
#define H_B_PORCH_MASTER			(0xFFFFFFFF << 0)

#define SST1_VIDEO_HORIZONTAL_ACTIVE		(0x5424)
#define H_ACTIVE_MASTER				(0xFFFFFFFF << 0)

#define SST1_VIDEO_VERTICAL_FRONT_PORCH		(0x5428)
#define V_F_PORCH_MASTER			(0xFFFFFFFF << 0)

#define SST1_VIDEO_VERTICAL_BACK_PORCH		(0x542C)
#define V_B_PORCH_MASTER			(0xFFFFFFFF << 0)

#define SST1_VIDEO_VERTICAL_ACTIVE		(0x5430)
#define V_ACTIVE_MASTER				(0xFFFFFFFF << 0)

#define SST1_VIDEO_DSC_STREAM_CONTROL_0		(0x5434)
#define DSC_ENABLE				(0x01 << 4)
#define SLICE_COUNT_PER_LINE			(0x07 << 0)

#define SST1_VIDEO_DSC_STREAM_CONTROL_1		(0x5438)
#define CHUNK_SIZE_1				(0xFFFF << 16)
#define CHUNK_SIZE_0				(0xFFFF << 0)

#define SST1_VIDEO_DSC_STREAM_CONTROL_2		(0x543C)
#define CHUNK_SIZE_3				(0xFFFF << 16)
#define CHUNK_SIZE_2				(0xFFFF << 0)

#define SST1_VIDEO_BIST_CONTROL			(0x5450)
#define BIST_PRBS7_SEED				(0x7F << 8)
#define BIST_USER_DATA_EN			(0x01 << 4)
#define BIST_EN					(0x01 << 3)
#define BIST_WIDTH				(0x01 << 2)
#define BIST_TYPE				(0x03 << 0)

#define SST1_VIDEO_BIST_USER_DATA_R		(0x5454)
#define BIST_USER_DATA_R			(0x3FF << 0)

#define SST1_VIDEO_BIST_USER_DATA_G		(0x5458)
#define BIST_USER_DATA_G			(0x3FF << 0)

#define SST1_VIDEO_BIST_USER_DATA_B		(0x545C)
#define BIST_USER_DATA_B			(0x3FF << 0)

#define SST1_VIDEO_DEBUG_FSM_STATE		(0x5460)
#define DATA_PACK_FSM_STATE			(0x3F << 16)
#define LINE_FSM_STATE				(0x07 << 8)
#define PIXEL_FSM_STATE				(0x07 << 0)

#define SST1_VIDEO_DEBUG_MAPI			(0x5464)
#define MAPI_UNDERFLOW_STATUS			(0x01 << 0)

#define SST1_VIDEO_DEBUG_ACTV_SYM_STEP_CNTL	(0x5468)
#define ACTV_SYM_STEP_CNT_VAL			(0x3FF << 1)
#define ACTV_SYM_STEP_CNT_EN			(0x01 << 0)

#define SST1_VIDEO_DEBUG_HOR_BLANK_AUD_BW_ADJ	(0x546C)
#define HOR_BLANK_AUD_BW_ADJ			(0x01 << 0)

#define SST1_AUDIO_CONTROL			(0x5800)
#define SW_AUD_CODING_TYPE			(0x07 << 27)
#define AUD_DMA_IF_LTNCY_TRG_MODE		(0x01 << 26)
#define AUD_DMA_IF_MODE_CONFIG			(0x01 << 25)
#define AUD_ODD_CHANNEL_DUMMY			(0x01 << 24)
#define AUD_M_VALUE_CMP_SPD_MASTER		(0x07 << 21)
#define DMA_BURST_SEL				(0x07 << 18)
#define AUDIO_BIT_MAPPING_TYPE			(0x03 << 16)
#define PCM_SIZE				(0x03 << 13)
#define AUDIO_CH_STATUS_SAME			(0x01 << 5)
#define AUD_GTC_CHST_EN				(0x01 << 1)

#define SST1_AUDIO_ENABLE			(0x5804)
#define AUDIO_EN				(0x01 << 0)

#define SST1_AUDIO_MASTER_TIMING_GEN		(0x5808)
#define AUDIO_MASTER_TIME_GEN			(0x01 << 0)

#define SST1_AUDIO_DMA_REQUEST_LATENCY_CONFIG	(0x580C)
#define AUD_DMA_ACK_STATUS			(0x01 << 21)
#define AUD_DMA_FORCE_ACK			(0x01 << 20)
#define AUD_DMA_FORCE_ACK_SEL			(0x01 << 19)
#define AUD_DMA_REQ_STATUS			(0x01 << 18)
#define AUD_DMA_FORCE_REQ_VAL			(0x01 << 17)
#define AUD_DMA_FORCE_REQ_SEL			(0x01 << 16)
#define MASTER_DMA_REQ_LTNCY_CONFIG		(0xFF << 0)

#define SST1_AUDIO_MUTE_CONTROL			(0x5810)
#define AUD_MUTE_UNDRUN_EN			(0x01 << 5)
#define AUD_MUTE_OVFLOW_EN			(0x01 << 4)
#define AUD_MUTE_CLKCHG_EN			(0x01 << 1)

#define SST1_AUDIO_MARGIN_CONTROL		(0x5814)
#define FORCE_AUDIO_MARGIN			(0x01 << 16)
#define AUDIO_MARGIN				(0x1FFF << 0)

#define SST1_AUDIO_DATA_WRITE_FIFO		(0x5818)
#define AUDIO_DATA_FIFO				(0xFFFFFFFF << 0)

#define SST1_AUDIO_GTC_CONTROL			(0x5824)
#define AUD_GTC_DELTA				(0xFFFFFFFF << 0)

#define SST1_AUDIO_GTC_VALID_BIT_CONTROL	(0x5828)
#define AUDIO_GTC_VALID_CONTROL			(0x01 << 1)
#define AUDIO_GTC_VALID				(0x01 << 0)

#define SST1_AUDIO_3DLPCM_PACKET_WAIT_TIMER	(0x582C)
#define AUDIO_3D_PKT_WAIT_TIMER			(0x3F << 0)

#define SST1_AUDIO_BIST_CONTROL			(0x5830)
#define SIN_AMPL				(0x0F << 4)
#define AUD_BIST_EN				(0x01 << 0)

#define SST1_AUDIO_BIST_CHANNEL_STATUS_SET0	(0x5834)
#define CHNL_BIT1				(0x03 << 30)
#define CLK_ACCUR				(0x03 << 28)
#define FS_FREQ					(0x0F << 24)
#define CH_NUM					(0x0F << 20)
#define SOURCE_NUM				(0x0F << 16)
#define CAT_CODE				(0xFF << 8)
#define MODE					(0x03 << 6)
#define PCM_MODE				(0x07 << 3)
#define SW_CPRGT				(0x01 << 2)
#define NON_PCM					(0x01 << 1)
#define PROF_APP				(0x01 << 0)

#define SST1_AUDIO_BIST_CHANNEL_STATUS_SET1	(0x5838)
#define CHNL_BIT2				(0x0F << 4)
#define WORD_LENGTH				(0x07 << 1)
#define WORD_MAX				(0x01 << 0)

#define SST1_AUDIO_BUFFER_CONTROL		(0x583C)
#define MASTER_AUDIO_INIT_BUFFER_THRD		(0x7F << 24)
#define MASTER_AUDIO_BUFFER_THRD		(0x3F << 18)
#define MASTER_AUDIO_BUFFER_EMPTY_INT_MASK	(0x01 << 17)
#define MASTER_AUDIO_CHANNEL_COUNT		(0x1F << 12)
#define MASTER_AUDIO_BUFFER_LEVEL		(0x7F << 5)
#define AUD_DMA_NOISE_INT_MASK			(0x01 << 4)
#define AUD_DMA_NOISE_INT			(0x01 << 3)
#define AUD_DMA_NOISE_INT_EN			(0x01 << 2)
#define MASTER_AUDIO_BUFFER_EMPTY_INT		(0x01 << 1)
#define MASTER_AUDIO_BUFFER_EMPTY_INT_EN	(0x01 << 0)

#define SST1_AUDIO_CHANNEL_1_4_REMAP		(0x5840)
#define AUD_CH_04_REMAP				(0x3F << 24)
#define AUD_CH_03_REMAP				(0x3F << 16)
#define AUD_CH_02_REMAP				(0x3F << 8)
#define AUD_CH_01_REMAP				(0x3F << 0)

#define SST1_AUDIO_CHANNEL_5_8_REMAP		(0x5844)
#define AUD_CH_08_REMAP				(0x3F << 24)
#define AUD_CH_07_REMAP				(0x3F << 16)
#define AUD_CH_06_REMAP				(0x3F << 8)
#define AUD_CH_05_REMAP				(0x3F << 0)

#define SST1_AUDIO_CHANNEL_9_12_REMAP		(0x5848)
#define AUD_CH_12_REMAP				(0x3F << 24)
#define AUD_CH_11_REMAP				(0x3F << 16)
#define AUD_CH_10_REMAP				(0x3F << 8)
#define AUD_CH_09_REMAP				(0x3F << 0)

#define SST1_AUDIO_CHANNEL_13_16_REMAP		(0x584C)
#define AUD_CH_16_REMAP				(0x3F << 24)
#define AUD_CH_15_REMAP				(0x3F << 16)
#define AUD_CH_14_REMAP				(0x3F << 8)
#define AUD_CH_13_REMAP				(0x3F << 0)

#define SST1_AUDIO_CHANNEL_17_20_REMAP		(0x5850)
#define AUD_CH_20_REMAP				(0x3F << 24)
#define AUD_CH_19_REMAP				(0x3F << 16)
#define AUD_CH_18_REMAP				(0x3F << 8)
#define AUD_CH_17_REMAP				(0x3F << 0)

#define SST1_AUDIO_CHANNEL_21_24_REMAP		(0x5854)
#define AUD_CH_24_REMAP				(0x3F << 24)
#define AUD_CH_23_REMAP				(0x3F << 16)
#define AUD_CH_22_REMAP				(0x3F << 8)
#define AUD_CH_21_REMAP				(0x3F << 0)

#define SST1_AUDIO_CHANNEL_25_28_REMAP		(0x5858)
#define AUD_CH_28_REMAP				(0x3F << 24)
#define AUD_CH_27_REMAP				(0x3F << 16)
#define AUD_CH_26_REMAP				(0x3F << 8)
#define AUD_CH_25_REMAP				(0x3F << 0)

#define SST1_AUDIO_CHANNEL_29_32_REMAP		(0x585C)
#define AUD_CH_32_REMAP				(0x3F << 24)
#define AUD_CH_31_REMAP				(0x3F << 16)
#define AUD_CH_30_REMAP				(0x3F << 8)
#define AUD_CH_29_REMAP				(0x3F << 0)

#define SST1_AUDIO_CHANNEL_1_2_STATUS_CTRL_0	(0x5860)
#define MASTER_AUD_GP0_STA_3			(0xFF << 24)
#define MASTER_AUD_GP0_STA_2			(0xFF << 16)
#define MASTER_AUD_GP0_STA_1			(0xFF << 8)
#define MASTER_AUD_GP0_STA_0			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_1_2_STATUS_CTRL_1	(0x5864)
#define MASTER_AUD_GP0_STA_4			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_3_4_STATUS_CTRL_0	(0x5868)
#define MASTER_AUD_GP1_STA_3			(0xFF << 24)
#define MASTER_AUD_GP1_STA_2			(0xFF << 16)
#define MASTER_AUD_GP1_STA_1			(0xFF << 8)
#define MASTER_AUD_GP1_STA_0			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_3_4_STATUS_CTRL_1	(0x586C)
#define MASTER_AUD_GP1_STA_4			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_5_6_STATUS_CTRL_0	(0x5870)
#define MASTER_AUD_GP2_STA_3			(0xFF << 24)
#define MASTER_AUD_GP2_STA_2			(0xFF << 16)
#define MASTER_AUD_GP2_STA_1			(0xFF << 8)
#define MASTER_AUD_GP2_STA_0			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_5_6_STATUS_CTRL_1	(0x5874)
#define MASTER_AUD_GP2_STA_4			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_7_8_STATUS_CTRL_0	(0x5878)
#define MASTER_AUD_GP3_STA_3			(0xFF << 24)
#define MASTER_AUD_GP3_STA_2			(0xFF << 16)
#define MASTER_AUD_GP3_STA_1			(0xFF << 8)
#define MASTER_AUD_GP3_STA_0			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_7_8_STATUS_CTRL_1	(0x587C)
#define MASTER_AUD_GP3_STA_4			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_9_10_STATUS_CTRL_0	(0x5880)
#define MASTER_AUD_GP4_STA_3			(0xFF << 24)
#define MASTER_AUD_GP4_STA_2			(0xFF << 16)
#define MASTER_AUD_GP4_STA_1			(0xFF << 8)
#define MASTER_AUD_GP4_STA_0			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_9_10_STATUS_CTRL_1	(0x5884)
#define MASTER_AUD_GP4_STA_4			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_11_12_STATUS_CTRL_0	(0x5888)
#define MASTER_AUD_GP5_STA_3			(0xFF << 24)
#define MASTER_AUD_GP5_STA_2			(0xFF << 16)
#define MASTER_AUD_GP5_STA_1			(0xFF << 8)
#define MASTER_AUD_GP5_STA_0			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_11_12_STATUS_CTRL_1	(0x588C)
#define MASTER_AUD_GP5_STA_4			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_13_14_STATUS_CTRL_0	(0x5890)
#define MASTER_AUD_GP6_STA_3			(0xFF << 24)
#define MASTER_AUD_GP6_STA_2			(0xFF << 16)
#define MASTER_AUD_GP6_STA_1			(0xFF << 8)
#define MASTER_AUD_GP6_STA_0			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_13_14_STATUS_CTRL_1	(0x5894)
#define MASTER_AUD_GP6_STA_4			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_15_16_STATUS_CTRL_0	(0x5898)
#define MASTER_AUD_GP7_STA_3			(0xFF << 24)
#define MASTER_AUD_GP7_STA_2			(0xFF << 16)
#define MASTER_AUD_GP7_STA_1			(0xFF << 8)
#define MASTER_AUD_GP7_STA_0			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_15_16_STATUS_CTRL_1	(0x589C)
#define MASTER_AUD_GP7_STA_4			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_17_18_STATUS_CTRL_0	(0x58A0)
#define MASTER_AUD_GP8_STA_3			(0xFF << 24)
#define MASTER_AUD_GP8_STA_2			(0xFF << 16)
#define MASTER_AUD_GP8_STA_1			(0xFF << 8)
#define MASTER_AUD_GP8_STA_0			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_17_18_STATUS_CTRL_1	(0x58A4)
#define MASTER_AUD_GP8_STA_4			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_19_20_STATUS_CTRL_0	(0x58A8)
#define MASTER_AUD_GP9_STA_3			(0xFF << 24)
#define MASTER_AUD_GP9_STA_2			(0xFF << 16)
#define MASTER_AUD_GP9_STA_1			(0xFF << 8)
#define MASTER_AUD_GP9_STA_0			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_19_20_STATUS_CTRL_1	(0x58AC)
#define MASTER_AUD_GP9_STA_4			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_21_22_STATUS_CTRL_0	(0x58B0)
#define MASTER_AUD_GP10_STA_3			(0xFF << 24)
#define MASTER_AUD_GP10_STA_2			(0xFF << 16)
#define MASTER_AUD_GP10_STA_1			(0xFF << 8)
#define MASTER_AUD_GP10_STA_0			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_21_22_STATUS_CTRL_1	(0x58B4)
#define MASTER_AUD_GP10_STA_4			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_23_24_STATUS_CTRL_0	(0x58B8)
#define MASTER_AUD_GP11_STA_3			(0xFF << 24)
#define MASTER_AUD_GP11_STA_2			(0xFF << 16)
#define MASTER_AUD_GP11_STA_1			(0xFF << 8)
#define MASTER_AUD_GP11_STA_0			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_23_24_STATUS_CTRL_1	(0x58BC)
#define MASTER_AUD_GP11_STA_4			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_25_26_STATUS_CTRL_0	(0x58C0)
#define MASTER_AUD_GP12_STA_3			(0xFF << 24)
#define MASTER_AUD_GP12_STA_2			(0xFF << 16)
#define MASTER_AUD_GP12_STA_1			(0xFF << 8)
#define MASTER_AUD_GP12_STA_0			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_25_26_STATUS_CTRL_1	(0x58C4)
#define MASTER_AUD_GP12_STA_4			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_27_28_STATUS_CTRL_0	(0x58C8)
#define MASTER_AUD_GP13_STA_3			(0xFF << 24)
#define MASTER_AUD_GP13_STA_2			(0xFF << 16)
#define MASTER_AUD_GP13_STA_1			(0xFF << 8)
#define MASTER_AUD_GP13_STA_0			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_27_28_STATUS_CTRL_1	(0x58CC)
#define MASTER_AUD_GP13_STA_4			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_29_30_STATUS_CTRL_0	(0x58D0)
#define MASTER_AUD_GP14_STA_3			(0xFF << 24)
#define MASTER_AUD_GP14_STA_2			(0xFF << 16)
#define MASTER_AUD_GP14_STA_1			(0xFF << 8)
#define MASTER_AUD_GP14_STA_0			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_29_30_STATUS_CTRL_1	(0x58D4)
#define MASTER_AUD_GP14_STA_4			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_31_32_STATUS_CTRL_0	(0x58D8)
#define MASTER_AUD_GP15_STA_3			(0xFF << 24)
#define MASTER_AUD_GP15_STA_2			(0xFF << 16)
#define MASTER_AUD_GP15_STA_1			(0xFF << 8)
#define MASTER_AUD_GP15_STA_0			(0xFF << 0)

#define SST1_AUDIO_CHANNEL_31_32_STATUS_CTRL_1	(0x58DC)
#define MASTER_AUD_GP15_STA_4			(0xFF << 0)

#define CMN_REG2C				(0x00B0)
#define MAN_USBDP_MODE				(0x03 << 1)
#define MAN_USBDP_MODE_EN			(0x01 << 0)

#define CMN_REG2D				(0x00B4)
#define USB_TX1_SEL				(0x01 << 5)
#define USB_TX3_SEL				(0x01 << 4)

#define DP_REG_0				(0x0800)
#define AUX_EN                                  (1 << 7)
#define BGR_EN                                  (1 << 6)
#define BIAS_EN                                 (1 << 5)
#define ROPLL_EN				(1 << 4)
#define LN0_LANE_EN                             (1 << 3)
#define LN1_LANE_EN                             (1 << 2)
#define LN2_LANE_EN                             (1 << 1)
#define LN3_LANE_EN                             (1 << 0)

#define DP_REG_1				(0x0804)
#define CMN_INIT_RSTN				(0x01 << 0)

#define DP_REG_3				(0x080C)
#define LN0_TX_AMP_CTRL				(3 << 6)
#define LN0_TX_AMP_CTRL_BIT_POS			(6)
#define LN1_TX_AMP_CTRL				(3 << 4)
#define LN1_TX_AMP_CTRL_BIT_POS			(4)
#define LN2_TX_AMP_CTRL				(3 << 2)
#define LN2_TX_AMP_CTRL_BIT_POS			(2)
#define LN3_TX_AMP_CTRL				(3 << 0)
#define LN3_TX_AMP_CTRL_BIT_POS			(0)

#define DP_REG_4				(0x0810)
#define LN0_TX_EMP_CTRL				(3 << 6)
#define LN0_TX_EMP_CTRL_BIT_POS			(6)
#define LN1_TX_EMP_CTRL				(3 << 4)
#define LN1_TX_EMP_CTRL_BIT_POS			(4)
#define LN2_TX_EMP_CTRL				(3 << 2)
#define LN2_TX_EMP_CTRL_BIT_POS			(2)
#define LN3_TX_EMP_CTRL				(3 << 0)
#define LN3_TX_EMP_CTRL_BIT_POS			(0)

#define DP_REG_B				(0x082C)
#define LN_TXCLK_SOURCE_LANE			(0x03 << 0)

#define DP_REG_13				(0x084C)
#define DP_REG_16				(0x0858)
#define DP_REG_17				(0x085C)
#define DP_REG_18				(0x0860)
#define DP_REG_19				(0x0864)
#define DP_REG_1A				(0x0868)
#define DP_REG_1B				(0x086C)
#define DP_REG_1C				(0x0870)
#define DP_REG_1D				(0x0874)

#define DP_REG_B3				(0x0ACC)
#define CMN_DUMMY_CTRL_7_6			(0x03 << 6)
#define CMN_DUMMY_CTRL_1_0			(0x03 << 0)
#endif
