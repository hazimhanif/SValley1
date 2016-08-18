#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/fs.h>      // Needed by filp
#include <asm/uaccess.h> 
#include <linux/string.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <asm/unistd.h>
#include <linux/delay.h>
int already_found = 0;




static void print_mem(struct task_struct *task)
{
        struct mm_struct *mm;
        struct vm_area_struct *vma;
	int nr_pages;
	unsigned long end,start;
        mm = task->mm;
	//printk("\n");
        //printk(" OKKKK  %s[%d]\n", task->comm, task->pid);
             //   mm->locked_vm=1;
		//mm->locked_vm += nr_pages;

 for (vma = mm->mmap ; vma ; vma = vma->vm_next) {
           /*     printk (" Vma number %d: \n", ++count);
                printk("  Starts at 0x%lx, Ends at 0x%lx\n",
                          vma->vm_start, vma->vm_end); */
		start=vma->vm_start;
		end=vma->vm_end;
		nr_pages = (end - start) >> PAGE_SHIFT;
		mm->locked_vm += nr_pages;
		mm->pinned_vm += nr_pages;
        }
	//printk("\n");
        printk(" Code  Segment start = 0x%lx, end = 0x%lx \n"
                 "Data  Segment start = 0x%lx, end = 0x%lx\n"
                 "Stack Segment start = 0x%lx\n",
                 mm->start_code, mm->end_code,
                 mm->start_data, mm->end_data,
                 mm->start_stack); 
		printk(" pgt %lu \n",mm->pgd->pgd);
		printk("total_vm %lu %lu %lu \n",mm->total_vm,
			mm->locked_vm,mm->pinned_vm);
             //   mm->locked_vm=1;


}




static int mm_exp_load(void){
        struct task_struct *task;
	struct file *f;
        char buf[128];
        mm_segment_t fs;
        int i;
       char *check="nginx";
       //char *check="ingatan";
	printk("welcome\n");
    // Init the buffer with 0
       for(i=0;i<128;i++)
       buf[i] = 0;
    f = filp_open("/etc/priority", O_RDONLY, 0);
    if(f == NULL){
        printk(KERN_ALERT "filp_open error!!.\n");
	goto exit;
    }else{
        // Get current segment descriptor
        fs = get_fs();
        // Set segment descriptor associated to kernel space
        set_fs(get_ds());
        // Read the file
        f->f_op->read(f, buf, 128, &f->f_pos);
        // Restore segment descriptor
        set_fs(fs);
        // See what we read from file
        printk(KERN_INFO "buf:%s\n",buf);
        printk(KERN_INFO "lenth::%u\n",(unsigned)strlen(buf));
    }
    filp_close(f,NULL);
buf[strlen(buf)]='\0';
lookit_again:
	
        for_each_process(task) {
		//if(strncmp(buf,task->comm,strlen(buf)) ==0){
		if(strcmp(check,task->comm) ==0){
		printk("found it!! \n");
		already_found = 1;
                 printk("%s %d \n", task->comm, task->pid);
                        printk("%s[%d]\n", task->comm, task->pid);
                        print_mem(task);
                }
        }
	if (msleep_interruptible(1000))
			flush_signals(current);
	if(already_found==0)
		goto lookit_again;
        return 0;
exit:
        return 1;
}

static void mm_exp_unload(void)
{
        printk("Print segment information module exiting.\n");
}
module_init(mm_exp_load);
module_exit(mm_exp_unload);
MODULE_AUTHOR("Mohamed Azran bin Aziz"); 
MODULE_VERSION("1.0"); 
MODULE_LICENSE("GPL2");

