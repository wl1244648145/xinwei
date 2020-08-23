#include <stdio.h>
#include <stdlib.h>
#include "../../../com_inc/bsp_types.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#define FLAG_BLOCK		"/dev/mtdblock1"
#define BUFF_LEN		128

#define UBOOT_FILE_NAME		"u-boot.bin"
#define KERNEL_FILE_NAME	"uImage"
#define DTB_FILE_NAME		"p2041bbu.dtb"
#define FILESYSTEM_FILE_NAME	"rootfs.bbu.uboot"

#define BOOTUP_BACK_TWO		0x0003002c			//¿¿¿¿2¿¿
#define BOOTUP_CNT_BASE		0x012c0000			//¿¿¿¿¿¿¿¿¿¿¿¿0¿¿¿ff¿¿
#define BOOTUP_UPDATA_FLAG	0x0102c0e0			//¿¿¿¿

#define MAX_RETRY 5

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <mtd/mtd-user.h>
#include <getopt.h>

typedef int bool;
#define true 1
#define false 0

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

/* for debugging purposes only */
#ifdef DEBUG
#undef DEBUG
#define DEBUG(fmt,args...) { log_printf (LOG_ERROR,"%d: ",__LINE__); log_printf (LOG_ERROR,fmt,## args); }
#else
#undef DEBUG
#define DEBUG(fmt,args...)
#endif

#define KB(x) ((x) / 1024)
#define PERCENTAGE(x,total) (((x) * 100) / (total))

/* size of read/write buffer */
#define BUFSIZE (256 * 1024)

/* cmd-line flags */
#define FLAG_NONE		0x00
#define FLAG_VERBOSE	0x01
#define FLAG_HELP		0x02
#define FLAG_FILENAME	0x04
#define FLAG_DEVICE		0x08

/* error levels */
#define LOG_NORMAL	1
#define LOG_ERROR	2

static void log_printf (int level,const char *fmt, ...)
{
	FILE *fp = level == LOG_NORMAL ? stdout : stderr;
	va_list ap;
	va_start (ap,fmt);
	vfprintf (fp,fmt,ap);
	va_end (ap);
	fflush (fp);
}

static int safe_open (const char *pathname,int flags)
{
	int fd;

	fd = open (pathname,flags);
	if (fd < 0)
	{
		log_printf (LOG_ERROR,"While trying to open %s",pathname);
		if (flags & O_RDWR)
			log_printf (LOG_ERROR," for read/write access");
		else if (flags & O_RDONLY)
			log_printf (LOG_ERROR," for read access");
		else if (flags & O_WRONLY)
			log_printf (LOG_ERROR," for write access");
		log_printf (LOG_ERROR,": %m\n");
		return (EXIT_FAILURE);
	}

	return (fd);
}

static void safe_read (int fd,const char *filename,void *buf,size_t count,bool verbose)
{
	ssize_t result;

	result = read (fd,buf,count);
	if (count != result)
	{
		if (verbose) log_printf (LOG_NORMAL,"\n");
		if (result < 0)
		{
			log_printf (LOG_ERROR,"While reading data from %s: %m\n",filename);
			return (EXIT_FAILURE);
		}
		log_printf (LOG_ERROR,"Short read count returned while reading from %s\n",filename);
		return (EXIT_FAILURE);
	}
}

static void safe_rewind (int fd,const char *filename)
{
	if (lseek (fd,0L,SEEK_SET) < 0)
	{
		log_printf (LOG_ERROR,"While seeking to start of %s: %m\n",filename);
		return (EXIT_FAILURE);
	}
}

/******************************************************************************/

static int dev_fd = -1,fil_fd = -1;
static 	unsigned char src[BUFSIZE],dest[BUFSIZE];
int flashcp(const char *device, const char *filename)
{

	int i;
	ssize_t result;
	size_t size,written;
	struct mtd_info_user mtd;
	struct erase_info_user erase;
	struct stat filestat;

	/* get some info about the flash device */
	dev_fd = safe_open (device,O_SYNC | O_RDWR);
	if (ioctl (dev_fd,MEMGETINFO,&mtd) < 0)
	{
		DEBUG("ioctl(): %m\n");
		log_printf (LOG_ERROR,"This doesn't seem to be a valid MTD flash device!\n");
		return (EXIT_FAILURE);
	}

	/* get some info about the file we want to copy */
	fil_fd = safe_open (filename,O_RDONLY);
	if (fstat (fil_fd,&filestat) < 0)
	{
		log_printf (LOG_ERROR,"While trying to get the file status of %s: %m\n",filename);
		return (EXIT_FAILURE);
	}

	/* does it fit into the device/partition? */
	if (filestat.st_size > mtd.size)
	{
		log_printf (LOG_ERROR,"%s won't fit into %s!\n",filename,device);
		return (EXIT_FAILURE);
	}

	/*****************************************************
	 * erase enough blocks so that we can write the file *
	 *****************************************************/

	size = (size_t)filestat.st_size;
	written = 0;
	i = mtd.erasesize;
	erase.start = 0;
	erase.length = mtd.erasesize;
	while (size > 0) {
			
		/* if not, erase the whole chunk in one shot */
		if (ioctl(dev_fd, MEMERASE, &erase) < 0)
		{
			log_printf (LOG_ERROR,
					"While erasing blocks from 0x%.8x-0x%.8x on %s: %m\n",
					(unsigned int) erase.start,(unsigned int) (erase.start + erase.length),device);
			return (EXIT_FAILURE);
		}

		
		/* read from filename */
		if (size < mtd.erasesize)
			i = size;

		safe_read (fil_fd,filename,src,i, 0);
		result = write (dev_fd,src,i);
		if (i != result)
		{
			if (result < 0)
			{
				log_printf (LOG_ERROR,
						"While writing data to 0x%.8x-0x%.8x on %s: %m\n",
						written,written + i,device);
				return (EXIT_FAILURE);
			}
			log_printf (LOG_ERROR,
					"Short write count returned while writing to x%.8x-0x%.8x on %s: %d/%lu bytes written to flash\n",
					written,written + i,device,written + result,filestat.st_size);
			return (EXIT_FAILURE);
		}

		lseek (fil_fd,erase.start,SEEK_SET);
		lseek (dev_fd,erase.start,SEEK_SET);

		safe_read (dev_fd,device,dest,i,0);

		/* compare buffers */
		if (memcmp (src,dest,i)) {
			lseek (dev_fd,erase.start,SEEK_SET);
			continue;
		}
		
		written += i;
		size -= i;
		erase.start += i;

		lseek (fil_fd,erase.start,SEEK_SET);
		lseek (dev_fd,erase.start,SEEK_SET);
		printf(".");
		fflush(stdout);
	}
	

	printf("OK\n");
	return 0;
}

UINT32 read_flag(void)
{
	UINT32  rd;
	int fd = open(FLAG_BLOCK, O_RDONLY);	

	if(read(fd, &rd,sizeof(rd)) == sizeof(rd))
	{
		close(fd);
		return rd;
	}

	close(fd);

	printf("read flag:0x%08lx\n", rd);
	return 0;
}

UINT32	write_flag(UINT32 flag_val)
{
	UINT32  len;

	int fd = open(FLAG_BLOCK, O_WRONLY);	
	if((len=write(fd, &flag_val,sizeof(flag_val))) == sizeof(flag_val))
	{
		close(fd);
		return len;
	}
	close(fd);
	return 0;
}
#if 0
int upgrade_mtd(int n, char *file)
{
	int ret, i = 0, j = 0;
	char cmd[100];

retry:
	sprintf(cmd, "flash_eraseall /dev/mtd%d\n", n);
	for (i = 0; i < MAX_RETRY; i++) {
		ret = system(cmd);
		if (ret == 0)
			break;
	}

	if (i == MAX_RETRY) {
		printf("MTD%d couldn't be erased\n", n);
		return BSP_ERROR;
	}

	sprintf(cmd, "flashcp -v %s /dev/mtd%d\n", file, n);

	ret = system(cmd);
	if (ret != 0) {
		if (j == MAX_RETRY) {
			printf("MTD couldn't be programe\n");
			return BSP_ERROR;
		}
		else {
			j++;
			goto retry;
		}
	}

	return BSP_OK;
}
#endif

#define BUFFER_SIZE	(10 * 1024 * 1024)
static u8 sector_buf[BUFFER_SIZE];
static u8 verify_buf[BUFFER_SIZE];

#define IH_NMLEN		32	/* Image Name Length		*/

typedef struct image_header {
	uint32_t	ih_magic;	/* Image Header Magic Number	*/
	uint32_t	ih_hcrc;	/* Image Header CRC Checksum	*/
	uint32_t	ih_time;	/* Image Creation Timestamp	*/
	uint32_t	ih_size;	/* Image Data Size		*/
	uint32_t	ih_load;	/* Data	 Load  Address		*/
	uint32_t	ih_ep;		/* Entry Point Address		*/
	uint32_t	ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t		ih_os;		/* Operating System		*/
	uint8_t		ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
	uint8_t		ih_name[IH_NMLEN];	/* Image Name		*/
} image_header_t;


typedef struct fdt_header {
	uint32_t magic;			 /* magic word FDT_MAGIC */
	uint32_t totalsize;		 /* total size of DT block */
	uint32_t off_dt_struct;		 /* offset to structure */
	uint32_t off_dt_strings;	 /* offset to strings */
	uint32_t off_mem_rsvmap;	 /* offset to memory reserve map */
	uint32_t version;		 /* format version */
	uint32_t last_comp_version;	 /* last compatible version */

	/* version 2 fields below */
	uint32_t boot_cpuid_phys;	 /* Which physical CPU id we're
					    booting on */
	/* version 3 fields below */
	uint32_t size_dt_strings;	 /* size of the strings block */

	/* version 17 fields below */
	uint32_t size_dt_struct;	 /* size of the structure block */
}fdt_header_t;

static int version_compare(char *file1, char *file2)
{
	int fp1, fp2, ret;
	image_header_t imgh1 = {0}, imgh2 = {0};
	fdt_header_t fdth1 = {0}, fdth2 = {0};
	
	fp1 = fopen(file1, "r");
	if (fp1 == NULL) {
		printf("Couldn't find %s\n", file1);
		return -1;
	}
	
	fp2 = fopen(file2, "r");
	if (fp2 == NULL) {
		printf("fCouldn't find %s\n", file2);
		return -1;
	}
	
	fread(&fdth1, sizeof(fdt_header_t), 1, fp1);	
	fread(&fdth2, sizeof(fdt_header_t), 1, fp2);	
	ret = memcmp(&fdth1, &fdth2, sizeof(fdt_header_t));
		
	if (ret != 0) {
		fread(&imgh1, sizeof(image_header_t), 1, fp1);	
		fread(&imgh2, sizeof(image_header_t), 1, fp2);	
		ret = memcmp(&imgh1, &imgh2, sizeof(image_header_t));
		if (ret != 0)
			return 1;
	}
	printf(".OK\n");
	return 0;
}

static int upgrade_mtd(char *mtd, char *filename)
{
	return flashcp(mtd, filename);
}

#define REPEAT_TIME 3

int upgrade_p2041(void)
{
	int i;

	printf("\n\nStart to update cpu firmware\n");
	
	printf("update dtb");
	if (version_compare("/mnt/btsa/p2041bbu.dtb", "/mnt/btsb/p2041bbu.dtb") == 1) {
		for (i = 0; i < REPEAT_TIME; i++) {
			if (!upgrade_mtd("/dev/mtd3", "/mnt/btsb/p2041bbu.dtb"))
				break;
		}
	}

	if (i == REPEAT_TIME) {
		printf("try to update %d times, fail! flash is broken\n", REPEAT_TIME);
		return BSP_ERROR;
	}

	printf("update uImage");
	if (version_compare("/mnt/btsa/uImage", "/mnt/btsb/uImage") == 1) {
		for (i = 0; i < REPEAT_TIME; i++) {
			if (!upgrade_mtd("/dev/mtd7", "/mnt/btsb/uImage"))
				break;
		}
	}
	
	if (i == REPEAT_TIME) {
		printf("try to update %d times, fail! flash is broken\n", REPEAT_TIME);
		return BSP_ERROR;
	}

	printf("update rootfs");
	if (version_compare("/mnt/btsa/rootfs.bbu.uboot", "/mnt/btsb/rootfs.bbu.uboot") == 1) {
		for (i = 0; i < REPEAT_TIME; i++) {
			if (!upgrade_mtd("/dev/mtd5", "/mnt/btsb/rootfs.bbu.uboot"))
				break;
		}
	}

	if (i == REPEAT_TIME) {
		printf("try to update %d times, fail! flash is broken\n", REPEAT_TIME);
		return BSP_ERROR;
	}

	printf("\n\nupdate cpu firmware complete\n");

	return BSP_OK;
}

int upgrade_firmware(void)
{
	int i;

	printf("update dtb");
	for (i = 0; i < REPEAT_TIME; i++) {
		if (!upgrade_mtd("/dev/mtd3", "/mnt/btsa/p2041bbu.dtb"))
			break;
	}

	if (i == REPEAT_TIME) {
		printf("try to update %d times, fail! flash is broken\n", REPEAT_TIME);
		return BSP_ERROR;
	}

	printf("update uImage");

	for (i = 0; i < REPEAT_TIME; i++) {
		if (!upgrade_mtd("/dev/mtd7", "/mnt/btsa/uImage"))
			break;
	}

	
	if (i == REPEAT_TIME) {
		printf("try to update %d times, fail! flash is broken\n", REPEAT_TIME);
		return BSP_ERROR;
	}

	printf("update rootfs");
	for (i = 0; i < REPEAT_TIME; i++) {
		if (!upgrade_mtd("/dev/mtd5", "/mnt/btsa/rootfs.bbu.uboot"))
			break;
	}

	if (i == REPEAT_TIME) {
		printf("try to update %d times, fail! flash is broken\n", REPEAT_TIME);
		return BSP_ERROR;
	}

	printf("\n\nupdate cpu firmware complete\n");

	return BSP_OK;
}

int restore_default(void)
{
	write_flag(BOOTUP_BACK_TWO);
}

int do_boot_flag(void)
{
	if (read_flag() == BOOTUP_BACK_TWO) {
		printf("Ö÷·ÖÇøËð»µ£¬½øÈë¾ÈÔ®Ä£Ê½£¬½øÐÐ°æ±¾¹Ì¼þÉý¼¶...\n");
		upgrade_firmware();
		write_flag(BOOTUP_CNT_BASE);
		system("reboot");
	}
		
	write_flag(BOOTUP_CNT_BASE);
	return 0;
}

int boot_from_major(void)
{
	write_flag(BOOTUP_CNT_BASE);
}
