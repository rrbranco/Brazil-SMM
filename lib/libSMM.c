/*
 *
 *    Copyleft (C) 2008  
 *      Andon Chauschev (D0nAnd0n) <andon.chauschev *noSPAM* rwth-aachen.de>
 *	Rodrigo Rubira Branco (BSDaemon) <rodrigo *noSPAM* risesecurity.org>
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License as
 *    published by the Free Software Foundation; either version 2 of
 *    the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be
 *    useful, but WITHOUT ANY WARRANTY; without even the implied
 *    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *    PURPOSE.  See the GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this program; if not, write to the Free
 *    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 *    USA.
 *
 */ 

#include <stdio.h>
#include <pci/pci.h>
#include "libSMM.h"

u8 show_smram(struct pci_dev* smram_dev, u8 bits_to_show)
{
	struct pci_access *pacc;
	struct pci_dev *smram_dev_default;
        u8 current_value;

	if (! smram_dev) {
		pacc = pci_alloc();
        	pci_init(pacc);
		smram_dev_default = pci_get_dev(pacc, 0, 0, 0, 0);
	} else smram_dev_default=smram_dev;

        current_value = pci_read_byte(smram_dev_default, SMRAM_OFFSET);

        printf("Current value in SMRAM: 0x%04x\n", current_value);

	if(RESERVED0_BIT & bits_to_show) 
		printf("RESERVED0_BIT: %d\n", (current_value & RESERVED0_BIT) ? 1 : 0);

        if(D_OPEN_BIT & bits_to_show) 
		printf("D_OPEN_BIT: %d\n", (current_value & D_OPEN_BIT) ? 1 : 0);

        if(D_CLS_BIT & bits_to_show) 
		printf("D_CLS_BIT: %d\n", (current_value & D_CLS_BIT) ? 1 : 0);

        if(D_LCK_BIT & bits_to_show) 
		printf("D_LCK_BIT: %d\n", (current_value & D_LCK_BIT) ? 1 : 0);

        if(G_SMRAME_BIT & bits_to_show) 
		printf("G_SMRAME_BIT: %d\n", (current_value & G_SMRAME_BIT) ? 1 : 0);

        if(C_BASE_SEG2_BIT & bits_to_show) 
		printf("C_BASE_SEG2_BIT: %d\n", (current_value & C_BASE_SEG2_BIT) ? 1 : 0);

        if(C_BASE_SEG1_BIT & bits_to_show) 
		printf("C_BASE_SEG1_BIT: %d\n", (current_value & C_BASE_SEG1_BIT) ? 1 : 0);

        if(C_BASE_SEG0_BIT & bits_to_show) 
		printf("C_BASE_SEG0_BIT: %d\n", (current_value & C_BASE_SEG0_BIT) ? 1 : 0);

        return current_value;
}

u16 get_pmbase(void) 
{
	struct pci_access *pacc;
	struct pci_dev    *LPCBridge;
	u8 val1, val2;
	u16 badr;
	
	pacc = pci_alloc();
	pci_init(pacc);
	
	LPCBridge = pci_get_dev(pacc, LPCB_DOMAIN, LPCB_BUS, LPCB_DEV, LPCB_FUNC);
	val1 = pci_read_byte(LPCBridge, 0x40); /* see linux kernel, drivers/char/watchdog/i8xx_tco.c */
	val2 = pci_read_byte(LPCBridge, 0x41);
	badr = ((val2 << 1) | (val1 >> 7)) << 7;
	
	pci_cleanup(pacc); /* close everything */

	return badr;
}

u16 get_smi_en_iop(void)
{
	u16 smi_en_iop, tmp;

	tmp = get_pmbase();
	smi_en_iop = tmp + SMI_EN_OFFSET;

	return smi_en_iop;
}

u16 get_smi_sts_iop(void)
{
	u16 smi_sts_iop, tmp;

	tmp = get_pmbase();
	smi_sts_iop = tmp + SMI_STS_OFFSET;

	return smi_sts_iop;
}

int enable_smi_gbl(u16 smi_en_iop)
{
	u32 smi_en_value;

	iopl(3);

	smi_en_value = inl(smi_en_iop);

	if(smi_en_value & GBL_SMI_EN_BIT) {
		/* gbl_smi_en was set */
		return 0;
	} else {
		outl(smi_en_value | GBL_SMI_EN_BIT, smi_en_iop);
		smi_en_value = inl(smi_en_iop);

		if(smi_en_value & GBL_SMI_EN_BIT) {
			/* gbl_smi_en is set */
			return 1;
		} else {
			/* gbl_smi_en cannot be set */
			return -1;
		}
	}

	/* we should never reach this */
	return -1;
}

int disable_smi_gbl(u16 smi_en_iop)
{
	u32 smi_en_value;

	iopl(3);

	smi_en_value = inl(smi_en_iop);

	if(smi_en_value & GBL_SMI_EN_BIT) {
		/* is enabled, must be disabled */
		outl(smi_en_value & ~GBL_SMI_EN_BIT, smi_en_iop);
		smi_en_value = inl(smi_en_iop);

		if(smi_en_value & GBL_SMI_EN_BIT) {
			/* gbl_smi_en is still set */
			return -1;
		} else {
			/* gbl_smi_en is not set */
			return 0;
		}
	} else {
		/* gbl_smi_en was not set */
		return 0;
	}

	/* we should never reach this */
	return -1;
}

int enable_smi_on_apm(u16 smi_en_iop)
{
	u32 smi_en_value;

	iopl(3);

	smi_en_value = inl(smi_en_iop);

	if(smi_en_value & APMC_EN_BIT) {
		/* apmc_en was set */
		return 0;
	} else {
		outl(smi_en_value | APMC_EN_BIT, smi_en_iop);
		smi_en_value = inl(smi_en_iop);

		if(smi_en_value & APMC_EN_BIT) {
			/* apmc_en is set */
			return 1;
		} else {
			/* apmc_en cannot be set */
			return -1;
		}
	}

	/* we should never reach this */
	return -1;
}

int disable_smi_on_apm(u16 smi_en_iop)
{
	u32 smi_en_value;

	iopl(3);

	smi_en_value = inl(smi_en_iop);

	if(smi_en_value & APMC_EN_BIT) {
		/* is enabled, must be disabled */
		outl(smi_en_value & ~APMC_EN_BIT, smi_en_iop);
		smi_en_value = inl(smi_en_iop);

		if(smi_en_value & APMC_EN_BIT) {
			/* is still set */
			return -1;
		} else {
			/* is not set */
			return 0;
		}
	} else {
		/* was not set */
		return 0;
	}

	/* we should never reach this */
	return -1;
}

int open_smram(void)
{
	struct pci_access *pacc;
	struct pci_dev *smram_dev;
	u8 current_value;

	pacc = pci_alloc();
	pci_init(pacc);

	smram_dev = pci_get_dev(pacc, 0, 0, 0, 0);

	current_value = pci_read_byte(smram_dev, SMRAM_OFFSET); 

	if(current_value & D_OPEN_BIT) {
		/* D_OPEN_BIT was set */
		pci_cleanup(pacc); /* close everything */
		return 0;
	} else {
		/* D_OPEN_BIT is not set, we must set it */
		pci_write_byte(smram_dev, SMRAM_OFFSET, (current_value | D_OPEN_BIT));
		current_value =  pci_read_byte(smram_dev, SMRAM_OFFSET);

		if(current_value & D_OPEN_BIT) {
			/* D_OPEN_BIT was set */
			pci_cleanup(pacc); /* close everything */
			return 1;
		} else {
			/* it was not able not set D_OPEN */
			pci_cleanup(pacc);
			return -1;
		}
	}

	return -1;
}


int close_smram(void)
{
	struct pci_access *pacc;
	struct pci_dev *smram_dev;
	u8 current_value;

	pacc = pci_alloc();
	pci_init(pacc);

	smram_dev = pci_get_dev(pacc, 0, 0, 0, 0);

	current_value = pci_read_byte(smram_dev, SMRAM_OFFSET); 

	if(current_value & D_OPEN_BIT) {
		/* D_OPEN_BIT was set, we must set it off */
		pci_write_byte(smram_dev, SMRAM_OFFSET, (current_value & ~D_OPEN_BIT));
		current_value =  pci_read_byte(smram_dev, SMRAM_OFFSET);

		if(current_value & D_OPEN_BIT) {
			/* D_OPEN_BIT could not be set off */
			pci_cleanup(pacc); /* close everything */
			return -1;
		} else {
			/* D_OPEN successfully turned off */
			pci_cleanup(pacc);
			return 0;
		}
	} else {
		/* D_OPEN_BIT was not set */
		pci_cleanup(pacc); /* close everything */
		return 0;
	}

	return -1;
}

int lock_smram(void)
{
	struct pci_access *pacc;
	struct pci_dev *smram_dev;
	u8 current_value, orig_value;

	pacc = pci_alloc();
	pci_init(pacc);

	smram_dev = pci_get_dev(pacc, 0, 0, 0, 0);

	current_value = pci_read_byte(smram_dev, SMRAM_OFFSET); 
	orig_value = current_value;

	/* lock it if not locked */
	if(!(current_value & D_LCK_BIT))  
		pci_write_byte(smram_dev, SMRAM_OFFSET, (current_value | D_LCK_BIT));

	/* then try to unlock it */
	pci_write_byte(smram_dev, SMRAM_OFFSET, (current_value & ~D_LCK_BIT));
	
	current_value = pci_read_byte(smram_dev, SMRAM_OFFSET);
	
	/* is locked and cannot be unlocked */
	if(current_value & D_LCK_BIT)
		return 0;

	/* D_LCK_BIT is not set and could be not set  */
	return -1;
}

void write_to_apm_cnt(void)
{
	iopl(3);
	outb (0xff, APM_CNT_IO);
}
