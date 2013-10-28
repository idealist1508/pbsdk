/* drivers.c
 * (c) 2002 Mikulas Patocka
 * This file is a part of the Links program, released under GPL
 */

#include "cfg.h"

#ifdef G

#include "links.h"

int F = 0;

struct graphics_driver *drv = NULL;

#ifdef GRDRV_X
extern struct graphics_driver x_driver;
#endif
#ifdef GRDRV_SVGALIB
extern struct graphics_driver svga_driver;
#endif
#ifdef GRDRV_FB
extern struct graphics_driver fb_driver;
#endif
#ifdef GRDRV_DIRECTFB
extern struct graphics_driver directfb_driver;
#endif
#ifdef GRDRV_PMSHELL
extern struct graphics_driver pmshell_driver;
#endif
#ifdef GRDRV_ATHEOS
extern struct graphics_driver atheos_driver;
#endif
#ifdef GRDRV_SDL
extern struct graphics_driver sdl_driver;
#endif

/*
 * On SPAD you must test first svgalib and then X (because X test is slow).
 * On other systems you must test first X and then svgalib (because svgalib
 *	would work in X too and it's undesirable).
 */

struct graphics_driver *graphics_drivers[] = {
#ifdef GRDRV_PMSHELL
	&pmshell_driver,
#endif
#ifdef GRDRV_ATHEOS
	&atheos_driver,
#endif
#ifndef SPAD
#ifdef GRDRV_X
	&x_driver,
#endif
#endif
#ifdef GRDRV_DIRECTFB
	&directfb_driver,
#endif
#ifdef GRDRV_FB
	&fb_driver,
#endif
#ifdef GRDRV_SVGALIB
	&svga_driver,
#endif
#ifdef SPAD
#ifdef GRDRV_X
	&x_driver,
#endif
#endif
#ifdef GRDRV_SDL
	&sdl_driver,
#endif
	NULL
};

int dummy_block(struct graphics_device *dev)
{
	return 0;
}

int dummy_unblock(struct graphics_device *dev)
{
	return 0;
}

#if 0
static unsigned char *list_graphics_drivers(void)
{ 
	unsigned char *d = init_str();
	int l = 0;
	struct graphics_driver **gd;
	for (gd = graphics_drivers; *gd; gd++) {
		if (l) add_to_str(&d, &l, " ");
		add_to_str(&d, &l, (*gd)->name);
	}
	return d;
}
#endif

/* Driver je jednorazovy argument, kterej se preda grafickymu driveru, nikde se dal
 * neuklada.  Param se skladuje v default_driver param a uklada se do konfiguraku. Pred
 * ukoncenim grafickeho driveru se nastavi default_driver_param podle
 * drv->get_driver_param.
 */
static unsigned char *init_graphics_driver(struct graphics_driver *gd, unsigned char *param, unsigned char *display)
{
	unsigned char *r;
	unsigned char *p = param;
	struct driver_param *dp=get_driver_param(gd->name);
	if (!param || !*param) p = dp->param;
	gd->codepage=dp->codepage;
	gd->shell=mem_calloc(MAX_STR_LEN);
	if (dp->shell) strncpy(gd->shell,dp->shell,MAX_STR_LEN);
	gd->shell[MAX_STR_LEN-1]=0;
	r = gd->init_driver(p,display);
	if (r) mem_free(gd->shell), gd->shell = NULL;
	return r;
}

unsigned char *init_graphics(unsigned char *driver, unsigned char *param, unsigned char *display)
{
	unsigned char *s = init_str();
	int l = 0;
	struct graphics_driver **gd;

	for (gd = graphics_drivers; *gd; gd++) {
		if (!driver || !*driver || !strcasecmp((*gd)->name, driver)) {
			unsigned char *r;
			if ((!driver || !*driver) && (*gd)->flags & GD_NOAUTO) continue;
			if (!(r = init_graphics_driver(*gd, param, display))) {
				mem_free(s);
				drv = *gd;
				F = 1;
				return NULL;
			}
			if (!l) {
				if (!driver || !*driver) add_to_str(&s, &l, "Could not initialize any graphics driver. Tried the following drivers:\n");
				else add_to_str(&s, &l, "Could not initialize graphics driver ");
			}
			add_to_str(&s, &l, (*gd)->name);
			add_to_str(&s, &l, ":\n");
			add_to_str(&s, &l, r);
			mem_free(r);
		}
	}
	if (!l) {
		add_to_str(&s, &l, "Unknown graphics driver ");
		if (driver) add_to_str(&s, &l, driver);
		add_to_str(&s, &l, ".\nThe following graphics drivers are supported:\n");
		for (gd = graphics_drivers; *gd; gd++) {
			if (gd != graphics_drivers) add_to_str(&s, &l, ", ");
			add_to_str(&s, &l, (*gd)->name);
		}
		add_to_str(&s, &l, "\n");
	}
	return s;
}

void shutdown_graphics(void)
{
	if (drv)
	{
		if (drv->shell) mem_free(drv->shell);
		drv->shutdown_driver();
	}
}

void update_driver_param(void)
{
	if (drv)
	{
		struct driver_param *dp=get_driver_param(drv->name);
		dp->codepage=drv->codepage;
		if (dp->param) mem_free(dp->param);
		dp->param=stracpy(drv->get_driver_param());
		if (dp->shell) mem_free(dp->shell);
		dp->shell=stracpy(drv->shell);
		dp->nosave = 0;
	}
}

#endif
