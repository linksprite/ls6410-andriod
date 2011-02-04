#include <common.h>

#ifdef CONFIG_MOVINAND

#if defined(CONFIG_SMDK6410)
#include <s3c6410.h>
#elif defined(CONFIG_SMDK6430)
#include <s3c6430.h>
#endif

#include <movi.h>
#include <asm/io.h>

uint movi_hc = 0;

void movi_set_capacity(void)
{
#if defined(CONFIG_S3C6400)
	if (MOVI_HIGH_CAPACITY == 2)
#else
	if (MOVI_HIGH_CAPACITY & 0x1)
#endif
		movi_hc = 1;
}

int movi_set_ofs(uint last)
{
	int changed = 0;

	if (ofsinfo.last != last) {
		ofsinfo.last = last - (eFUSE_SIZE / MOVI_BLKSIZE);
		ofsinfo.bl1	 = ofsinfo.last - MOVI_BL1_BLKCNT;
		ofsinfo.env	 = ofsinfo.bl1 - MOVI_ENV_BLKCNT;
		ofsinfo.bl2	 = ofsinfo.bl1 - (MOVI_BL2_BLKCNT + MOVI_ENV_BLKCNT);
		ofsinfo.zimage	= ofsinfo.bl2 - MOVI_ZIMAGE_BLKCNT;
		changed = 1;
	}

	return changed;
}

int movi_init(void)
{
	hsmmc_set_gpio();
	hsmmc_reset();

	if (hsmmc_init()) {
		printf("\nCard Initialization failed.\n");
		return -1;
	}

	return 1;
}

void movi_write_env(ulong addr)
{
	movi_write((uint)addr, ofsinfo.env, MOVI_ENV_BLKCNT);
}

void movi_read_env(ulong addr)
{
	movi_read((uint)addr, ofsinfo.env, MOVI_ENV_BLKCNT);
}

void movi_bl2_copy(void)
{
#if defined(CONFIG_S3C6400)
	CopyMovitoMem(MOVI_BL2_POS, MOVI_BL2_BLKCNT, (uint *)BL2_BASE, CONFIG_SYS_CLK_FREQ, MOVI_INIT_REQUIRED);
#else
	writel(readl(HM_CONTROL4) | (0x3 << 16), HM_CONTROL4);
	CopyMovitoMem(HSMMC_CHANNEL, MOVI_BL2_POS, MOVI_BL2_BLKCNT, (uint *)BL2_BASE, MOVI_INIT_REQUIRED);
#endif
}

#endif
