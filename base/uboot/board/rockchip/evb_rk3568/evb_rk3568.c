// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 Rockchip Electronics Co., Ltd
 */
#include <common.h>
#include <dm.h>
#include <adc.h>
#include <asm/global_data.h>

#define SARADC_ADDR	"saradc@fe720000"
#define HW_ID_CHANNEL	1
#define BOM_ID_CHANNEL	2

#define countof(x) (sizeof(x) / sizeof(x[0]))

struct variant_def {
	char *compatible;
	unsigned int hw_id_lower_bound;
	unsigned int hw_id_upper_bound;
	unsigned int bom_id_lower_bound;
	unsigned int bom_id_upper_bound;
	char *fdtfile;
};

#ifdef CONFIG_ID_EEPROM
static struct variant_def variants[] = {
	{"radxa,zero3",   230, 270, 0, -1, "rockchip/rk3566-radxa-zero-3w-aic8800ds2.dtb"},
	{"radxa,zero3",   400, 450, 0, -1, "rockchip/rk3566-radxa-zero-3e.dtb"},
	{"radxa,zero3",   451, 510, 0, -1, "rockchip/rk3566-radxa-zero-3w-ap6212.dtb"},
	{"radxa,rock-3c", 300, 360, 0, -1, "rockchip/rk3566-rock-3c-aic8800ds2.dtb"},
};

static void set_fdtfile(void)
{
	int i, ret;
	unsigned int hw_id, bom_id;
	struct variant_def *v;

	ret = adc_channel_single_shot(SARADC_ADDR, HW_ID_CHANNEL, &hw_id);
	if (ret) {
		pr_err("%s: adc_channel_single_shot fail for HW_ID: %i!\n", __func__, ret);
		return;
	}
	ret = adc_channel_single_shot(SARADC_ADDR, BOM_ID_CHANNEL, &bom_id);
	if (ret) {
		pr_err("%s: adc_channel_single_shot fail for BOM_ID: %i!\n", __func__, ret);
		return;
	}

	for(i = 0; i < countof(variants); i++) {
		v = &variants[i];
		if (of_machine_is_compatible(v->compatible) &&
		    hw_id >= v->hw_id_lower_bound &&
		    hw_id <= v->hw_id_upper_bound &&
		    bom_id >= v->bom_id_lower_bound &&
		    bom_id <= v->bom_id_upper_bound) {
			printf("Override default fdtfile to %s\n", v->fdtfile);
			env_set("fdtfile", v->fdtfile);
			break;
		}
	}
}

/**
 * mac_read_from_eeprom() - read the MAC address & the serial number in EEPROM
 *
 * This function reads the MAC address and the serial number from EEPROM and
 * sets the appropriate environment variables for each one read.
 *
 * The environment variables are only set if they haven't been set already.
 * This ensures that any user-saved variables are never overwritten.
 *
 * If CONFIG_ID_EEPROM is enabled, this function will be called in
 * "static init_fnc_t init_sequence_r[]" of u-boot/common/board_r.c.
 */
int mac_read_from_eeprom(void)
{
	set_fdtfile();
	return 0;
}

int do_mac(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	printf("This device does not support user programmable EEPROM.\n");
	return -1;
}
#endif
