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

#ifndef SMM_FUNCTIONS_H
#define SMM_FUNCTIONS_H

#include <pci/pci.h>
#include <sys/io.h>   /* inl(), outl()  */


/* values valid for ICH5, ICH3M */
#define LPCB_DOMAIN (0x0)
#define LPCB_BUS    (0x0)
#define LPCB_DEV    (0x1f)
#define LPCB_FUNC   (0x0)
#define SMI_EN_OFFSET  (0x30)
#define SMI_STS_OFFSET (0x34)
/* bits in SMI_EN (not all) */
/* TODO all bits here write */
#define APMC_EN_BIT    (0x01 << 5)  /* bit 5 -- set -- writes to APM_CNT will cause SMI# */
#define GBL_SMI_EN_BIT (0x01)       /* bit 0 -- set -- global enable SMI# */


/* bits in SMRAM, valid for 865G/865GV/865P/865PE/850 (all bits here) */
#define RESERVED0_BIT   (0x01 << 7)
#define D_OPEN_BIT      (0x01 << 6)
#define D_CLS_BIT       (0x01 << 5)
#define D_LCK_BIT       (0x01 << 4)
#define G_SMRAME_BIT    (0x01 << 3)
#define C_BASE_SEG2_BIT (0x01 << 2) /* readonly */
#define C_BASE_SEG1_BIT (0x01 << 1) /* readonly */
#define C_BASE_SEG0_BIT (0x01)      /* readonly */

#ifdef I830
#define SMRAM_OFFSET (0x90) /* 830M */
#else
#define SMRAM_OFFSET (0x9d) /* 865, on groucho */ 
#endif

#define MEMDEVICE "/dev/mem"
#define MAPPEDAREASIZE (4096)    /* this is the size of the mapped SMRAM area (in bytes */               
#define APM_CNT_IO   (0xb2)      /* io address of APM_CNT register */

#define SMBASE (0xa0000)
#define SMIINSTADDRESS (SMBASE + 0x8000)

#define APM_CNT_IO   (0xb2)      /* io address of APM_CNT register */

u8 show_smram(struct pci_dev* smram_dev, u8 bits_to_show);
u16 get_pmbase(void);
u16 get_smi_en_iop(void);
u16 get_smi_sts_iop(void);

int enable_smi_gbl(u16 smi_en_iop);
int disable_smi_gbl(u16 smi_en_iop);
int enable_smi_on_apm(u16 smi_en_iop);
int disable_smi_on_apm(u16 smi_en_iop);

int open_smram(void);
int close_smram(void);
int lock_smram(void);

void write_to_apm_cnt(void);


/* 
 * TODO
 * 1. test write_to_apm_cnt 
 */

#endif /* SMM_FUNCTIONS_H */
