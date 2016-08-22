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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/hugetlb.h>  
#include <linux/rcupdate.h>
#include <linux/swap.h> 
#include <linux/mmdebug.h>
#include <linux/page-flags.h>
#include <linux/delay.h>
#include <linux/string.h>


int already_found = 0;

static char *proc_name="mysqld"; //Process name of the desired process that needs locking.
static struct task_struct *task;
/*
static int do_lock(int flags)
{
    int nr_pages;
    struct vm_area_struct * vma;
    int lock=0;

    if (flags & MCL_FUTURE)
        task->mm->def_flags |= VM_LOCKED;
    else
        task->mm->def_flags &= ~VM_LOCKED;
    if (flags == MCL_FUTURE)
        goto out;

    for (vma = task->mm->mmap; vma ; vma = vma ->vm_next)
    {
        vm_flags_t newflags;

        newflags = vma->vm_flags | VM_LOCKED;
        if (!(flags & MCL_CURRENT))
            newflags &= ~VM_LOCKED;

        //Fixup part started.
        
        lock = !!(newflags & VM_LOCKED);


        nr_pages = (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;

        if (!lock)
            nr_pages = -nr_pages;

        task->mm->locked_vm += nr_pages;
        vma->vm_flags = newflags;


        //Fixup part ended.

        cond_resched_rcu_qs();

    }

    printk("VM Total:%lu ,VM Locked:%lu, Pinned VM:%lu\n",task->mm->total_vm,task->mm->locked_vm,task->mm->pinned_vm);
    printk("VM_LOCKED:%d\n",VM_LOCKED);
    printk("DEFFLASGS:%lu\n",task->mm->def_flags);

out:
    return 0;

}
*/



static int print_mem(int flags)
{
	 int lock=0;
        struct mm_struct *mm;
        struct vm_area_struct *vma;
	int nr_pages;
	unsigned long end,start;
	int r;        
	mm = task->mm;

	//printk("\n");
        //printk(" OKKKK  %s[%d]\n", task->comm, task->pid);
             //   mm->locked_vm=1;
		//mm->locked_vm += nr_pages;

 for (vma = mm->mmap ; vma ; vma = vma->vm_next) {
           /*     printk (" Vma number %d: \n", ++count);
                printk("  Starts at 0x%lx, Ends at 0x%lx\n",
                          vma->vm_start, vma->vm_end); */
vm_flags_t newflags;	
r=0;

        

        
if(vma->vm_flags & VM_READ) {
            printk("r");
	r=1;
        } else {
            printk("-");
        }
        if(vma->vm_flags & VM_WRITE) {
            printk("w");
	r=r+2;
        } else {
            printk("-");
        }
        if(vma->vm_flags & VM_EXEC) {
            printk("x");
	r=r+3;
        } else {
            printk("-");
        }

if(r==3){
printk("data\n");
newflags = vma->vm_flags | VM_LOCKED;
        if (!(flags & MCL_CURRENT))
            newflags &= ~VM_LOCKED;

        //Fixup part started.
        
        lock = !!(newflags & VM_LOCKED);


        nr_pages = (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;

        if (!lock)
            nr_pages = -nr_pages;

        task->mm->locked_vm += nr_pages;
        vma->vm_flags = newflags;


        //Fixup part ended.

        cond_resched_rcu_qs();
}
if(r==4)
printk("code\n");

//printk("%u\n",vma->vm_file->f_path.dentry->d_iname);
//printk("%u\n",vma->vm_file->f_flags);
       printk("\n");


  printk (" vm flags 0x%lu: \n", vma->vm_flags);
		start=vma->vm_start;
		end=vma->vm_end;
		nr_pages = (end - start) >> PAGE_SHIFT;
		//mm->locked_vm += nr_pages;
		//mm->pinned_vm += nr_pages;
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

return 0;
}





static int lock_test(int flags)
{

    unsigned long lock_limit;
    int ret = -EINVAL;

    printk("Start locking process:%s\n",proc_name);

    //Preparation for do_lock() function. Follows as in /linux/mm/mlock.c

    if (!flags || (flags & ~(MCL_CURRENT | MCL_FUTURE)))
        goto out;
 
    ret = -EPERM;
    if (!can_do_mlock())
        goto out;

    lock_limit = rlimit(RLIMIT_MEMLOCK);
    lock_limit >>= PAGE_SHIFT;

    ret = -ENOMEM;
    down_write(&task->mm->mmap_sem);

    if (!(flags & MCL_CURRENT) || (task->mm->total_vm <= lock_limit) || capable(CAP_IPC_LOCK))
	print_mem(flags);        
	//do_lock(flags);

    printk("Locking done!\n");

    up_write(&task->mm->mmap_sem);
    if (!ret && (flags & MCL_CURRENT))
        mm_populate(0, TASK_SIZE);

out:
    return ret;
}









static int mm_exp_load(void){
        struct task_struct *t;
	struct file *f;
        char buf[128];
        mm_segment_t fs;
        int i;
       char *check="mysqld";
       //char *check="ingatan";
	printk("welcome\n");
    // Init the buffer with 0
/*       for(i=0;i<128;i++)
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
buf[strlen(buf)]='\0'; */
lookit_again:
	
        for_each_process(t) {
		//if(strncmp(buf,task->comm,strlen(buf)) ==0){
		if(strcmp(check,t->comm) ==0){
		printk("found it!! \n");
		already_found = 1;
                 printk("%s %d \n", t->comm, t->pid);
                        printk("%s[%d]\n", t->comm, t->pid);
			task = t;                
		        lock_test(MCL_CURRENT);
		
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
MODULE_LICENSE("GPL v2");

