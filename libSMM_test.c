/*
 *
 *    Copyleft (C) 2008
 *      Andon Chauschev (D0nAnd0n) <andon.chauschev *noSPAM* rwth-aachen.de>
 *      Rodrigo Rubira Branco (BSDaemon) <rodrigo *noSPAM* risesecurity.org>
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
#include <stdlib.h>
#include "lib/libSMM.h"

int main(void)
{
	unsigned int PMBASE;
	PMBASE = get_pmbase();

	printf( "OK -- get_pmbase(): PMBASE extracted value: 0x%08x\n", PMBASE);

	u16 smi_en_iop;
	smi_en_iop = get_smi_en_iop();
	printf("OK -- get_smi_en_iop(): SMI_EN_IO extracted value: 0x%08x\n", smi_en_iop);

	u16 smi_sts_iop;
	smi_sts_iop = get_smi_sts_iop();
	printf("OK -- get_smi_sts_iop(): SMI_STS_IO extracted value: 0x%08x\n", smi_sts_iop);

	iopl(3);
	u32 smi_en_origvalue;
	int ret_disable_smi_gbl;
	smi_en_origvalue = inl(smi_en_iop);
	ret_disable_smi_gbl = disable_smi_gbl(smi_en_iop);

	if(ret_disable_smi_gbl < 0) {
		printf("NOT OK -- disable_smi_gbl(): returned value is negative.(original value in SMI_EN: 0x%08x)\n", smi_en_origvalue);
		exit(EXIT_FAILURE);
	} else {
		printf("OK -- disable_smi_gbl(): returned value is not negative.(original value in SMI_EN: 0x%08x)\n", smi_en_origvalue);
	}
	
	u32 smi_en_tmpvalue;
	smi_en_tmpvalue = inl(smi_en_iop);

	if(smi_en_tmpvalue &  GBL_SMI_EN_BIT) {
		printf("NOT OK -- disable_smi_gbl(): gbl_smi_en not really disabled. (value in SMI_EN: 0x%08x)\n", smi_en_tmpvalue);
		exit(EXIT_FAILURE);
	} else {
		printf("OK -- disable_smi_gbl(): gbl_smi_en really disabled. (value in SMI_EN: 0x%08x)\n", smi_en_tmpvalue);
	}

	int ret_enable_smi_gbl;
	ret_enable_smi_gbl = enable_smi_gbl(smi_en_iop);

	if(ret_enable_smi_gbl < 0) {
		printf("NOT OK -- enable_smi_gbl(): returned value is negative\n");
		exit(EXIT_FAILURE);
	} else {
		printf("OK -- enable_smi_gbl(): returned value is not negative\n");
	}

	smi_en_tmpvalue = inl(smi_en_iop);

	if(smi_en_tmpvalue &  GBL_SMI_EN_BIT) {
		printf("OK -- enable_smi_gbl(): gbl_smi_en is really enabled. (value in SMI_EN: 0x%08x)\n", smi_en_tmpvalue);
	} else {
		printf("NOT OK -- enable_smi_gbl(): gbl_smi_en is not really enabled. (value in SMI_EN: 0x%08x)\n", smi_en_tmpvalue);
		exit(EXIT_FAILURE);
	}


	int ret_disable_smi_on_apm;
	ret_disable_smi_on_apm = disable_smi_on_apm(smi_en_iop);

	if(ret_disable_smi_on_apm < 0) {
		printf("NOT OK -- disable_smi_on_apm(): returned value is negative\n");
		exit(EXIT_FAILURE);
	} else {
		printf("OK -- disable_smi_on_apm(): returned value is not negative\n");
	}

	smi_en_tmpvalue = inl(smi_en_iop);

	if(smi_en_tmpvalue &  APMC_EN_BIT) {
		printf("NOT OK -- disable_smi_en_apm(): apmc_en not really disabled. (value in SMI_EN: 0x%08x)\n", smi_en_tmpvalue);
		exit(EXIT_FAILURE);
	} else {
		printf("OK -- disable_smi_en_apm(): apmc_en really disabled. (value in SMI_EN: 0x%08x)\n", smi_en_tmpvalue);
	}

	
	int ret_enable_smi_on_apm;
	ret_enable_smi_on_apm = enable_smi_on_apm(smi_en_iop);

	if(ret_enable_smi_on_apm < 0) {
		printf("NOT OK -- enable_smi_gbl(): returned value is negative\n");
		exit(EXIT_FAILURE);
	} else {
		printf("OK -- enable_smi_gbl(): returned value is not negative\n");
	}

	smi_en_tmpvalue = inl(smi_en_iop);

	if(smi_en_tmpvalue &  APMC_EN_BIT) {
		printf("OK -- enable_smi_on_apm(): apmc_en is really enabled. (value in SMI_EN: 0x%08x)\n", smi_en_tmpvalue);
	} else {
		printf("NOT OK -- enable_smi_on_apm(): apmc_en is not really enabled. (value in SMI_EN: 0x%08x)\n", smi_en_tmpvalue);
		exit(EXIT_FAILURE);
	}

	outl(smi_en_origvalue, smi_en_iop);
	smi_en_tmpvalue = inl(smi_en_iop);

	if(smi_en_origvalue == smi_en_tmpvalue) {
		printf("OK -- original value (0x%08x) saved back to SMI_EN\n", smi_en_origvalue);
	} else {
		printf("NOT OK -- original value (0x%08x) not saved back to SMI_EN\n", smi_en_origvalue);
		exit(EXIT_FAILURE);
	}

	struct pci_access *pacc_test;
	struct pci_dev    *smram_dev_test;
	u8 smram_tmpvalue, smram_origvalue;
	int ret_open_smram, ret_close_smram;
	
	pacc_test = pci_alloc();
	pci_init(pacc_test);
	smram_dev_test = pci_get_dev(pacc_test, 0, 0, 0, 0);
	smram_origvalue = pci_read_byte(smram_dev_test, SMRAM_OFFSET); 

	ret_open_smram = open_smram();

	if(ret_open_smram < 0) {
		printf("NOT OK -- open_smram(): returned value is negative (SMRAM_OFFSET: 0x%x, D_OPEN_BIT: 0x%x, orig. value in SMRAM: 0x%x\n", 
			SMRAM_OFFSET, D_OPEN_BIT ,smram_origvalue);
		exit(EXIT_FAILURE);
	} else {
		printf("OK -- open_smram(): returned value is not negative (SMRAM_OFFSET: 0x%x, D_OPEN_BIT: 0x%x, orig. value in SMRAM: 0x%x\n", 
			SMRAM_OFFSET, D_OPEN_BIT, smram_origvalue);
	}

	smram_tmpvalue = pci_read_byte(smram_dev_test, SMRAM_OFFSET); 

	if(smram_tmpvalue & D_OPEN_BIT) {
		printf("OK -- open_smram(): SMRAM is opened(value in SMRAM: 0x%x\n", smram_tmpvalue);
	} else {
		printf("NOT OK -- open_smram(): SMRAM is not opened(value in SMRAM: 0x%x\n", smram_tmpvalue);
		exit(EXIT_FAILURE);
	}
	
	ret_close_smram = close_smram();

	if(ret_close_smram < 0) {
		printf("NOT OK -- close_smram(): returned value is negative\n");
		exit(EXIT_FAILURE);
	} else {
		printf("OK -- close_smram(): returned value is not negative\n");
	}

	smram_tmpvalue = pci_read_byte(smram_dev_test, SMRAM_OFFSET); 

	if(smram_tmpvalue & D_OPEN_BIT) {
		printf("NOT OK -- close_smram(): SMRAM is not closed(value in SMRAM: 0x%x\n", smram_tmpvalue);
		exit(EXIT_FAILURE);
	} else {
		printf("OK -- close_smram(): SMRAM is closed(value in SMRAM: 0x%x\n", smram_tmpvalue);
	}

	pci_write_byte(smram_dev_test, SMRAM_OFFSET, smram_origvalue);
	smram_tmpvalue = pci_read_byte(smram_dev_test, SMRAM_OFFSET); 

	if(smram_tmpvalue != smram_origvalue) {
		printf("NOT OK -- original value could not be saved in SMRAM. current value: 0x%x\n", smram_tmpvalue);
		exit(EXIT_FAILURE);
	} else {
		printf("OK -- original value saved back to SMRAM.\n");
	}

	/* now testing lock_smram as following: 
	 * at this point smram is closed.
	 * we call lock_smram, look at the return value.
	 * then we call open_smram.
	 * we look if smram is opened. when smram is not opened, that means, that lock_smram works correct
	 */
	int ret_lock_smram;
	ret_lock_smram = lock_smram();

	if (ret_lock_smram < 0) {
		printf("NOT OK -- lock_smram(): return value is negative\n");
		exit(EXIT_FAILURE);
	} else {
		printf("OK -- lock_smram(): return value is not negative\n");
	}

	open_smram();
	smram_tmpvalue = pci_read_byte(smram_dev_test, SMRAM_OFFSET);

	if(smram_tmpvalue & D_OPEN_BIT) {
		printf("NOT OK -- lock_smram(): open_smram() opened SMRAM -- SMRAM not locked. (value in SMRAM: 0x%x)\n", smram_tmpvalue);
		exit(EXIT_FAILURE);
	} else {
		printf("OK -- lock_smram(): open_smram() did not open SMRAM -- SMRAM is locked. (value in SMRAM: 0x%x)\n", smram_tmpvalue);
	}
		
	printf("\nSeems that everything is working... Dont forget to restart (SMRAM is locked). Good luck!\n");
	return 0;
}
