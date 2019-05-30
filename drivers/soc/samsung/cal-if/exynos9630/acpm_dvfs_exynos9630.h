
enum acpm_dvfs_id {
	dvfs_mif = ACPM_VCLK_TYPE,
	dvfs_int,
	dvfs_cpucl0,
	dvfs_cpucl1,
	dvfs_mfc,
	dvfs_npu,
	dvfs_disp,
	dvfs_dsp,
	dvfs_aud,
	dvs_cp,
	dvfs_g3d,
	dvfs_intcam,
	dvfs_cam,
	dvfs_tnr,
	dvfs_dnc,
};

struct vclk acpm_vclk_list[] = {
	CMUCAL_ACPM_VCLK(dvfs_mif, NULL, NULL, NULL, NULL, MARGIN_MIF),
	CMUCAL_ACPM_VCLK(dvfs_int, NULL, NULL, NULL, NULL, MARGIN_INT),
	CMUCAL_ACPM_VCLK(dvfs_cpucl0, NULL, NULL, NULL, NULL, MARGIN_LIT),
	CMUCAL_ACPM_VCLK(dvfs_cpucl1, NULL, NULL, NULL, NULL, MARGIN_BIG),
	CMUCAL_ACPM_VCLK(dvfs_mfc, NULL, NULL, NULL, NULL, MARGIN_MFC),
	CMUCAL_ACPM_VCLK(dvfs_npu, NULL, NULL, NULL, NULL, MARGIN_NPU),
	CMUCAL_ACPM_VCLK(dvfs_disp, NULL, NULL, NULL, NULL, MARGIN_DISP),
	CMUCAL_ACPM_VCLK(dvfs_dsp, NULL, NULL, NULL, NULL, MARGIN_DSP),
	CMUCAL_ACPM_VCLK(dvfs_aud, NULL, NULL, NULL, NULL, MARGIN_AUD),
	CMUCAL_ACPM_VCLK(dvs_cp, NULL, NULL, NULL, NULL, MARGIN_CP),
	CMUCAL_ACPM_VCLK(dvfs_g3d, NULL, NULL, NULL, NULL, MARGIN_G3D),
	CMUCAL_ACPM_VCLK(dvfs_intcam, NULL, NULL, NULL, NULL, MARGIN_INTCAM),
	CMUCAL_ACPM_VCLK(dvfs_cam, NULL, NULL, NULL, NULL, MARGIN_CAM),
	CMUCAL_ACPM_VCLK(dvfs_tnr, NULL, NULL, NULL, NULL, MARGIN_TNR),
	CMUCAL_ACPM_VCLK(dvfs_dnc, NULL, NULL, NULL, NULL, MARGIN_DNC),
};

unsigned int acpm_vclk_size = ARRAY_SIZE(acpm_vclk_list);
