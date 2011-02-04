
#include <common.h>

#ifdef CONFIG_BB

#include <command.h>

int do_bb(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	char *cmd;

	cmd = argv[1];	

	//burn qtopia_base system
	if (strcmp(cmd, "qtopia") == 0)
	{
		//burn u-boot
		if(run_command ("tftp c0008000 u-boot_qtopia.bin", flag) < 0)
			goto usage;
		if(run_command ("nand erase 0 40000", flag) < 0)
			goto usage;
		if(run_command ("nand write c0008000 0 40000", flag) < 0)
			goto usage;
		//burn cramfs
		if(run_command ("tftp c0008000 rootfs_qtopia.cramfs", flag) < 0)
			goto usage;
		if(run_command ("nand erase 400000 400000", flag) < 0)
			goto usage;
		if(run_command ("nand write c0008000 400000 400000", flag) < 0)
			goto usage;	
		//burn zImage		
		if(run_command ("tftp c0008000 zImage_qtopia", flag) < 0)
			goto usage;
		if(run_command ("nand erase 40000 300000", flag) < 0)
			goto usage;
		if(run_command ("nand write c0008000 40000 300000", flag) < 0)
			goto usage;	

		if(run_command ("setenv bootargs noinitrd root=/dev/mtdblock0 console=ttySAC0 init=/linuxrc qtopia", flag) < 0)
			goto usage;
	}
	//burn android_base system
	else if (strcmp(cmd, "android") == 0)
	{
		if(run_command ("tftp c0008000 u-boot_android.bin", flag) < 0)
			goto usage;
		if(run_command ("nand erase 0 40000", flag) < 0)
			goto usage;
		if(run_command ("nand write c0008000 0 40000", flag) < 0)
			goto usage;
		//burn cramfs
		if(run_command ("tftp c0008000 rootfs_android.cramfs", flag) < 0)
			goto usage;
		if(run_command ("nand erase 400000 400000", flag) < 0)
			goto usage;
		if(run_command ("nand write c0008000 400000 400000", flag) < 0)
			goto usage;	
		//burn zImage		
		if(run_command ("tftp c0008000 zImage_android", flag) < 0)
			goto usage;
		if(run_command ("nand erase 40000 300000", flag) < 0)
			goto usage;
		if(run_command ("nand write c0008000 40000 300000", flag) < 0)
			goto usage;	

		if(run_command ("setenv bootargs noinitrd root=/dev/mtdblock0 console=ttySAC0 init=/linuxrc android", flag) < 0)
			goto usage;	
	}
	//calibrate the touchscreen for android system
	else if (strcmp(cmd, "ts") == 0)
	{
		if(run_command ("nand read c0008000 40000 300000", flag) < 0)
			goto usage;
		if(run_command ("setenv bootargs noinitrd root=/dev/mtdblock0 console=ttySAC0 init=/linuxrc ts_calibrate", flag) < 0)
			goto usage;	
	}
	else
		goto usage;	
	
	if(run_command ("bootm c0008000", flag) < 0)
		goto usage;	
	
	return 1;

usage:
	printf("Usage:\n%s\n", cmdtp->usage);
	return -1;
}

U_BOOT_CMD(
	bb,	4,	0,	do_bb,
	"bb\t- Bulk burn \n",
	"bb android | qtopia\n"
);

#endif /* (CONFIG_COMMANDS & CFG_CMD_BB) */

