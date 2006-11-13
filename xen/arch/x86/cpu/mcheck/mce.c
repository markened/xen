/*
 * mce.c - x86 Machine Check Exception Reporting
 * (c) 2002 Alan Cox <alan@redhat.com>, Dave Jones <davej@codemonkey.org.uk>
 */

#include <xen/init.h>
#include <xen/types.h>
#include <xen/kernel.h>
#include <xen/config.h>
#include <xen/smp.h>

#include <asm/processor.h> 
#include <asm/system.h>

#include "mce.h"

int mce_disabled = 0;
int nr_mce_banks;

/* Handle unconfigured int18 (should never happen) */
static fastcall void unexpected_machine_check(struct cpu_user_regs * regs, long error_code)
{	
	printk(KERN_ERR "CPU#%d: Unexpected int18 (Machine Check).\n", smp_processor_id());
}

/* Call the installed machine check handler for this CPU setup. */
void fastcall (*machine_check_vector)(struct cpu_user_regs *, long error_code) = unexpected_machine_check;

/* This has to be run for each processor */
void mcheck_init(struct cpuinfo_x86 *c)
{
	if (mce_disabled==1)
		return;

	switch (c->x86_vendor) {
		case X86_VENDOR_AMD:
			if (c->x86==6 || c->x86==15)
				amd_mcheck_init(c);
			break;

		case X86_VENDOR_INTEL:
#ifndef CONFIG_X86_64
			if (c->x86==5)
				intel_p5_mcheck_init(c);
			if (c->x86==6)
				intel_p6_mcheck_init(c);
#endif
			if (c->x86==15)
				intel_p4_mcheck_init(c);
			break;

#ifndef CONFIG_X86_64
		case X86_VENDOR_CENTAUR:
			if (c->x86==5)
				winchip_mcheck_init(c);
			break;
#endif

		default:
			break;
	}
}

static int __init mcheck_disable(char *str)
{
	mce_disabled = 1;
	return 0;
}

static int __init mcheck_enable(char *str)
{
	mce_disabled = -1;
	return 0;
}

custom_param("nomce", mcheck_disable);
custom_param("mce", mcheck_enable);
