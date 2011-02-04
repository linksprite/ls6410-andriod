/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <regs.h>

/* ------------------------------------------------------------------------- */
#define CS8900_Tacs	(0x0)	// 0clk		address set-up
#define CS8900_Tcos	(0x0)	// 4clk		chip selection set-up
#define CS8900_Tacc	(0x7)	// 14clk	access cycle
#define CS8900_Tcoh	(0x0)	// 1clk		chip selection hold
#define CS8900_Tah	(0x0)	// 4clk		address holding time
#define CS8900_Tacp	(0x0)	// 6clk		page mode access cycle
#define CS8900_PMC	(0x0)	// normal(1data)page mode configuration

static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n" "bne 1b":"=r" (loops):"0"(loops));
}

/*
 * Miscellaneous platform dependent initialisations
 */

static void cs8900_pre_init(void)
{
	SROM_BW_REG &= ~(0xf << 4);
	SROM_BW_REG |= (1<<4);
	SROM_BC1_REG = ((CS8900_Tacs<<28)+(CS8900_Tcos<<24)+(CS8900_Tacc<<16)+(CS8900_Tcoh<<12)+(CS8900_Tah<<8)+(CS8900_Tacp<<4)+(CS8900_PMC));
}

int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	cs8900_pre_init();

	gd->bd->bi_arch_number = MACH_TYPE;
	gd->bd->bi_boot_params = (PHYS_SDRAM_1+0x100);

#if 0
	icache_enable();
	dcache_enable();
#endif
	return 0;
}

int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

/*
 *Here we configure the PMU(AXP181) through iic.
 *Figo (sagres_2004@163.com)
 */
void pmu_axp181_init(void)
{
	uchar buffer = 0xff;
	//trun on sw1/sw2/LDO4/LDO5
	if(i2c_write (0x16, 0x10, 1, &buffer, 1) != 0)
	{
		printf("Write PMU(AXP181) failed\n");
		return;
	}
	//todo:configure more ...
}

/*
 *Here we turn on the GPRS through pull low GPIO C2.
 *Figo (sagres_2004@163.com)
 */
void gprs_init(void)
{
#if 1
	GPCCON_REG &= ~(0xf000000);
	GPCCON_REG |= 0x1000000;
	GPCPUD_REG &= ~(0x3000);
	GPCPUD_REG |= 0x2000;
	GPCDAT_REG &= ~(0x40);
	GPCDAT_REG |= 0x40;
#else
	GPCCON_REG &= ~(0xf00);
	GPCCON_REG |= 0x100;
	GPCPUD_REG &= ~(0x30);
	GPCPUD_REG |= 0x20;
	GPCDAT_REG &= ~(0x4);
	GPCDAT_REG |= 0x4;
#endif
}

#ifdef BOARD_LATE_INIT
#if defined(CONFIG_BOOT_NAND)
int board_late_init (void)
{
	uint *magic = (uint*)(PHYS_SDRAM_1);
	char boot_cmd[100];
	
	gprs_init();
//	pmu_axp181_init();
	if ((0x24564236 == magic[0]) && (0x20764316 == magic[1])) {
		sprintf(boot_cmd, "nand erase 0 40000;nand write %08x 0 40000", PHYS_SDRAM_1 + 0x8000);
		magic[0] = 0;
		magic[1] = 0;
		printf("\nready for self-burning U-Boot image\n\n");
		setenv("bootdelay", "0");
		setenv("bootcmd", boot_cmd);
	}

	return 0;
}
#elif defined(CONFIG_BOOT_MOVINAND)
int board_late_init (void)
{
	uint *magic = (uint*)(PHYS_SDRAM_1);
	char boot_cmd[100];
	int hc;

	gprs_init();
//	pmu_axp181_init();
	hc = (magic[2] & 0x1) ? 1 : 0;

	if ((0x24564236 == magic[0]) && (0x20764316 == magic[1])) {
		sprintf(boot_cmd, "movi init %d %d;movi write u-boot %08x", magic[3], hc, PHYS_SDRAM_1 + 0x8000);
		magic[0] = 0;
		magic[1] = 0;
		printf("\nready for self-burning U-Boot image\n\n");
		setenv("bootdelay", "0");
		setenv("bootcmd", boot_cmd);
	}

	return 0;
}
#else
int board_late_init (void)
{
	gprs_init();
//	pmu_axp181_init();
	return 0;
}
#endif
#endif

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("Board:   SMDK6410\n");
	return (0);
}
#endif

#ifdef CONFIG_ENABLE_MMU
ulong virt_to_phy_smdk6410(ulong addr)
{
	if ((0xc0000000 <= addr) && (addr < 0xc8000000))
		return (addr - 0xc0000000 + 0x50000000);
	else
		printf("do not support this address : %08lx\n", addr);

	return addr;
}
#endif

#if (CONFIG_COMMANDS & CFG_CMD_NAND) && defined(CFG_NAND_LEGACY)
#include <linux/mtd/nand.h>
extern struct nand_chip nand_dev_desc[CFG_MAX_NAND_DEVICE];
void nand_init(void)
{
	nand_probe(CFG_NAND_BASE);
        if (nand_dev_desc[0].ChipID != NAND_ChipID_UNKNOWN) {
                print_size(nand_dev_desc[0].totlen, "\n");
        }
}
#endif
