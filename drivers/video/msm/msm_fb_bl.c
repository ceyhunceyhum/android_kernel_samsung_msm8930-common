/* Copyright (c) 2008-2009, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fb.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/backlight.h>

#include "msm_fb.h"

static int msm_fb_bl_get_brightness(struct backlight_device *pbd)
{
	printk(KERN_INFO "msm_fb_bl_get_brightness %d\n", pbd->props.brightness);
	return pbd->props.brightness;
}

static int msm_fb_bl_update_status(struct backlight_device *pbd)
{
	struct msm_fb_data_type *mfd = bl_get_data(pbd);
	__u32 bl_lvl;

	printk(KERN_INFO "msm_fb_bl_update_status %d\n", pbd->props.brightness);
	bl_lvl = pbd->props.brightness;
#if !defined(CONFIG_MACH_CRATERTD_CHN_3G) && !defined(CONFIG_MACH_BAFFINVETD_CHN_3G)
	bl_lvl = mfd->fbi->bl_curve[bl_lvl];
#endif
	down(&mfd->sem);
	msm_fb_set_backlight(mfd, bl_lvl);
	up(&mfd->sem);
	return 0;
}

static struct backlight_ops msm_fb_bl_ops = {
	.get_brightness = msm_fb_bl_get_brightness,
	.update_status = msm_fb_bl_update_status,
};

void msm_fb_config_backlight(struct msm_fb_data_type *mfd)
{
	struct msm_fb_panel_data *pdata;
	struct backlight_device *pbd;
	struct fb_info *fbi;
	char name[16];
	struct backlight_properties props;

	fbi = mfd->fbi;
	pdata = (struct msm_fb_panel_data *)mfd->pdev->dev.platform_data;

	printk(KERN_INFO "msm_fb_config_backlight\n");
	if ((pdata) && (pdata->set_backlight)) {
		snprintf(name, sizeof(name), "msmfb_bl%d", mfd->index);
#if defined(CONFIG_MACH_CRATERTD_CHN_3G) || defined(CONFIG_MACH_BAFFINVETD_CHN_3G)
		props.max_brightness = FB_BACKLIGHT_MAX;
		props.brightness = FB_BACKLIGHT_MAX;
#else
		props.max_brightness = FB_BACKLIGHT_LEVELS - 1;
		props.brightness = FB_BACKLIGHT_LEVELS - 1;
#endif
		props.type = BACKLIGHT_PLATFORM;
		printk(KERN_INFO "name: %s\n", name);
		pbd =
		    backlight_device_register(name, fbi->dev, mfd,
					      &msm_fb_bl_ops, &props);
		if (!IS_ERR(pbd)) {
			fbi->bl_dev = pbd;
			fb_bl_default_curve(fbi,
					    0,
					    mfd->panel_info.bl_min,
					    mfd->panel_info.bl_max);
		} else {
			fbi->bl_dev = NULL;
			printk(KERN_ERR "msm_fb: backlight_device_register failed!\n");
		}
	}
}
