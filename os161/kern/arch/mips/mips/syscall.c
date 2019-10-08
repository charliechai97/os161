#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <machine/pcb.h>
#include <machine/spl.h>
#include <machine/trapframe.h>
#include <kern/callno.h>
#include <syscall.h>
#include <thread.h>
#include <curthread.h>
#include <addrspace.h>
#include <synch.h>
#include <vnode.h>
#include <kern/unistd.h>
#include <vm.h>

#include <clock.h>


extern struct page *coremap;
extern int totalPages;

/*
 * System call handler.
 *
 * A pointer to the trapframe created during exception entry (in
 * exception.S) is passed in.
 *
 * The calling conventions for syscalls are as follows: Like ordinary
 * function calls, the first 4 32-bit arguments are passed in the 4
 * argument registers a0-a3. In addition, the system call number is
 * passed in the v0 register.
 *
 * On successful return, the return value is passed back in the v0
 * register, like an ordinary function call, and the a3 register is
 * also set to 0 to indicate success.
 *
 * On an error return, the error code is passed back in the v0
 * register, and the a3 register is set to 1 to indicate failure.
 * (Userlevel code takes care of storing the error code in errno and
 * returning the value -1 from the actual userlevel syscall function.
 * See src/lib/libc/syscalls.S and related files.)
 *
 * Upon syscall return the program counter stored in the trapframe
 * must be incremented by one instruction; otherwise the exception
 * return code will restart the "syscall" instruction and the system
 * call will repeat forever.
 *
 * Since none of the OS/161 system calls have more than 4 arguments,
 * there should be no need to fetch additional arguments from the
 * user-level stack.
 *
 * Watch out: if you make system calls that have 64-bit quantities as
 * arguments, they will get passed in pairs of registers, and not
 * necessarily in the way you expect. We recommend you don't do it.
 * (In fact, we recommend you don't use 64-bit quantities at all. See
 * arch/mips/include/types.h.)
 */

extern struct threadnode processList[200];

void
mips_syscall(struct trapframe *tf) {
    int callno;
    int32_t retval;
    int err;

    assert(curspl == 0);

    callno = tf->tf_v0;

    /*
     * Initialize retval to 0. Many of the system calls don't
     * really return a value, just 0 for success and -1 on
     * error. Since retval is the value returned on success,
     * initialize it to 0 by default; thus it's not necessary to
     * deal with it except for calls that return other values,
     * like write.
     */

    retval = 0;

    switch (callno) {
        case SYS_reboot:
            err = sys_reboot(tf->tf_a0);
            break;

            /* Add stuff here */

        case SYS_write:
            err = write(tf,&retval);
            break;

	case SYS_read:
		err = sys_read(tf,&retval);
		break;
            
        case SYS_fork:
            err = fork(tf,&retval);
            break;
	
	case SYS_waitpid:
	    err = sys_waitpid((pid_t)tf->tf_a0,(int*)tf->tf_a1,tf->tf_a2, &retval);
	    break;
	case SYS_getpid:
	    retval = sys_getpid();
	err = 0;
	    break;
	case SYS__exit:
	     sys__exit(tf->tf_a0);
	     break;
	case SYS_execv:
		//kprintf("start\n");
		err = execv(tf);
		break;
	case SYS___time:
		err = sys___time((userptr_t)tf->tf_a0,(userptr_t)tf->tf_a1, &retval);
		break;
	case SYS_sbrk:
		err = sbrk(tf, &retval);

        default:
            //kprintf("Unknown syscall %d\n", callno);
            err = ENOSYS;
            break;
    }


    if (err) {
        /*
         * Return the error code. This gets converted at
         * userlevel to a return value of -1 and the error
         * code in errno.
         */
        tf->tf_v0 = err;
        tf->tf_a3 = 1; /* signal an error */
    } else {
        /* Success. */
        tf->tf_v0 = retval;
        tf->tf_a3 = 0; /* signal no error */
    }

    /*
     * Now, advance the program counter, to avoid restarting
     * the syscall over and over again.
     */

    tf->tf_epc += 4;

    /* Make sure the syscall code didn't forget to lower spl */
    assert(curspl == 0);
}

void
md_forkentry(void *tf, unsigned long addr) {
    /*
     * This function is provided as a reminder. You need to write
     * both it and the code that calls it.
     *
     * Thus, you can trash it and do things another way if you prefer.
     */
	struct trapframe *trapFrame = (struct trapframe *) tf;
	struct trapframe tempTf;	

	curthread->t_vmspace = (struct addrspace*) addr;
	as_activate(curthread->t_vmspace);	

	trapFrame->tf_v0 = 0;
	trapFrame->tf_a3 = 0;
	trapFrame->tf_epc += 4;
	tempTf = *trapFrame;
	kfree(trapFrame);

	mips_usermode(&tempTf);
}

int write(struct trapframe *tf, int *retval) {
	int fd = tf->tf_a0;
	void* test = (void*) tf->tf_a1;
	int nbytes = tf->tf_a2;

	char *buf = kmalloc((nbytes+1)*sizeof(char));
	int check = copyin(test, buf, nbytes);
	if(check != 0)
		return EFAULT;

	if (fd != 1 && fd != 2)
		return EBADF;
	if (nbytes == 0) {
		*retval = 0;
		return 0;
	}

	int spl = splhigh();

	int i;
	for(i = 0; i < nbytes; i++)
	{
		if(*buf != '\0')
			kprintf("%c", *buf);
		buf++;
	}
	*retval = nbytes;

	splx(spl);

	return 0;
}

int sys_read(struct trapframe *trapFrame, int* retval){
	int fd = trapFrame -> tf_a0;
	char* test = (char*) trapFrame -> tf_a1;
	int numBytes = trapFrame -> tf_a2;
	
	if (fd < 0 || fd > 0){       //not valid file descriptor
		return EBADF;
	}
	
	char *buffer = kmalloc((numBytes+1)*sizeof(char));

	int tmpBytes = numBytes;
	int i = 0;
	while (numBytes > 0)
	{
		buffer[i] = getch();   //grab a character
		numBytes--; //decrease num bytes	
		i++;
	}

	int check = copyout(buffer, test, (sizeof(char))*tmpBytes);
	if(check != 0)
		return EFAULT;
	
	*retval = tmpBytes;
	return 0;	
}

int fork(struct trapframe *tf, int *retval) {
	//kprintf("about to fork\n");
	struct thread *child;
	struct addrspace *childAddr;
	struct trapframe *childTf;
	//pid_t childPid;
    
	int spl = splhigh();
    
	int check = as_copy(curthread->t_vmspace, &childAddr);
	if (check){
		splx(spl);
		return ENOMEM;
	}
	as_activate(curthread->t_vmspace);

	childTf = kmalloc(sizeof (struct trapframe));
	if (childTf == NULL){
		splx(spl);
		return ENOMEM;
	}
	memcpy(childTf, tf, sizeof(struct trapframe));
	//*childTf = *tf;
    
	check = thread_fork(curthread->t_name, childTf, 
		(unsigned long) childAddr, (void*) md_forkentry, &child);
    
	if (check) {
		splx(spl);
        	return check;
	}
    
	*retval = child->pid;
	//kprintf("child pid is %d\n",child->pid);
    
	splx(spl);

	return 0;
}

//int exit(struct trapframe *tf){
//	thread_exit();
//	return 0;
//}

int sys_getpid(void){
    return curthread->pid;
}

int sys_waitpid(pid_t pid,int *status,int options, int* retval){
	
	//kprintf("starting wait\n");
	//kprintf("starting wait\n");
	//pid_t pid = (pid_t) trapFrame -> tf_a0;
	//int *status = (int*)trapFrame -> tf_a1;
	//int options = trapFrame -> tf_a2;
	
	//kprintf("\n");
	//kprintf("I am %d waiting on %d\n", curthread->pid, pid);

	//kprintf("11111\n");
	if(status == NULL)
		return EFAULT;
	if (options != 0){
		//kprintf("#");
		return EINVAL;
	}
	//if(curthread->children == NULL)
	//	return EINVAL;

	if(pid < 0){
		//kprintf("*");
		return EINVAL;
	}
	//kprintf("22222\n");

	struct thread *currentThread = curthread;
	if (pid == currentThread->pid){
		//kprintf("0");
		return EINVAL;
	}
	
	int spl;
	spl = splhigh();

	//here check if process exists

	struct thread * theThread = getThread(pid);
	//kprintf("iam: %d", theThread->pid);
	
	if (theThread == NULL){
	//kprintf("1");
	splx(spl);
	return EINVAL;
	}
	
	//if doesnt exist return
	/*if (theThread->beenWaitedOn == 1 && theThread->exited == 1){
		//kprintf("2");
		splx(spl);
		return EINVAL;
	}*/
	if (processList[pid].beenWaitedOn == 1 && processList[pid].exited == 1){
		//kprintf("2");
		splx(spl);
		return EINVAL;
	}
			
			/*struct thread *threadFound = doesThreadExist(pid);

			if (threadFound == NULL){
				splx(spl);
				return EINVAL;
			}*/
	

	//if it does exist
	//check if your child
	int isChild = 0;
	int i;
	for(i = 0;i < 100; i++){
		if(currentThread -> children[i] != NULL)
			/*if (pid == currentThread -> children[i] -> pid){
				isChild = 1;
			}*/
			if (currentThread == processList[pid].parent){
				isChild = 1;
			}
	} 
	
	if (isChild == 0){
		//kprintf("3");
		//kprintf("parent is: %d", theThread->parent->pid);
		splx(spl);
		return EINVAL; //this isn't your child
	}

	
	  //if is your child
	  //if already exited
	int a;
	/*if (theThread -> exited == 1){
		*retval = pid;
		a = getExit(pid);
		copyout(&a,status,4);
		//*status = theThread->exitCode;		//kern,user, space
		splx(spl);
		return 0;
	} */
	if (processList[pid].exited == 1){
		*retval = pid;
		a = getExit(pid);
		copyout(&a,status,4);
		//*status = theThread->exitCode;		//kern,user, space
		splx(spl);
		return 0;
	} 
	

			/*if (threadFound->exited == 1){	
				*retval = pid;
				*status = threadFound->exitCode;  
		
				splx(spl);
				return 0;
			}*/	
	
		

	
	//if has not exited

//	P(theThread->waitOnMe);
	P(processList[pid].waitOnMe);
	
	//theThread -> beenWaitedOn = 1;
	processList[pid].beenWaitedOn=1;
	*retval = pid;
	//*status = theThread->exitCode; 
	a = getExit(pid);
	copyout(&a,status,4);
		//if has not exited, sleep till exit
	
		/*threadFound -> beenWaitedOn = 1;
		kprintf("\n");
		kprintf("I am %d, %d is waiting on me ", threadFound->pid, curthread->pid);
		P(threadFound -> waitOnMe);

		*retval = pid;
		*status = threadFound->exitCode; 
		if(threadFound -> beenWaitedOn == 1)
			//V(threadFound->need2Check);


		//splx(spl);
	
		//*status = threadFound->exitCode;
		*/

	splx(spl);
	return 0;
}

void sys__exit(unsigned iexitCode){
	//kprintf("exiting\n");
	//kprintf("exiting\n");
	//int iexitCode = trapFrame->tf_a0;
	int spl = splhigh();
	//kprintf("1111\n");
	//curthread->exitCode = iexitCode;
	processList[curthread->pid].exitCode=iexitCode;
	setExit(iexitCode,curthread->pid);
	//curthread->exited = 1;
	processList[curthread->pid].exited=1;
	//V(curthread->waitOnMe);
	V(processList[curthread->pid].waitOnMe);
	//kprintf("2222\n");
	
	//kprintf("3333\n");
	
	//kprintf("4444\n");	

	//if (curthread->beenWaitedOn == 1)
	//	thread_sleep(curthread->slpaddr);
	
	//if (curthread->beenWaitedOn == 1)
	//	P(curthread->need2Check);

	//P(curthread->parent->imParent);
	//V(curthread->parent->imParent);
	//V(curthread->imParent);
	
	//while (curthread->beenWaitedOn != 1){
	//kprintf("fuck this");
	//}
	//curthread->exited = 1;
	//V(curthread->waitOnMe);
	splx(spl);
	thread_exit();
}

int execv(struct trapframe *tf){
	//kprintf("1\n");
	char *program = (char*) tf->tf_a0;
	char **args = (char**) tf->tf_a1;

	if(program == NULL)
		return ENOENT;
	if(args == NULL)
		return EFAULT;

	int check;
	size_t programSize = 1024;
	char *kProgram = (char*) kmalloc(programSize);

	int spl = splhigh();
	//kprintf("2\n");

	//copy from user space to kernel space
	check = copyinstr(program,kProgram,programSize,NULL);
	if(check){
		splx(spl);
		return EFAULT;
	}

	int i = 0;
	int argc;
	while(args[i] != '\0'){
		i++;
	}
	argc = i;
	//kprintf("%d\n",argc);

	//allocating space and copying into argv
	char **argv = (char**)kmalloc(sizeof(char*)*(argc+1));
	for(i = 0; i < argc; i++){
		argv[i] = (char*)kmalloc(sizeof(char)*programSize);
	}
	for(i = 0; i < argc; i++){
		if(i < argc){
			check = copyinstr((userptr_t)args[i], argv[i], programSize, NULL);
			//kprintf("check: %d %s\n",check,args[i]);
			if(check){
				splx(spl);
				return EFAULT;
			}
		}
		else
			argv[i] = NULL;
	}
	
	//kprintf("4\n");

	//running program
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;

	/* Open the file. */
	result = vfs_open(kProgram, O_RDONLY, &v);
	if (result) {
		splx(spl);
		return result;
	}

	//kprintf("5\n");

	/* We should be a new thread. */
	if(curthread->t_vmspace != NULL){
		curthread->t_vmspace = NULL;
	}

	//kprintf("6\n");

	/* Create a new address space. */
	curthread->t_vmspace = as_create();
	if (curthread->t_vmspace==NULL) {
		vfs_close(v);
		splx(spl);
		return ENOMEM;
	}

	/* Activate it. */
	as_activate(curthread->t_vmspace);

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		vfs_close(v);
		splx(spl);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(curthread->t_vmspace, &stackptr);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		splx(spl);
		return result;
	}

	//kprintf("7\n");

	//copy back to kernal space
	userptr_t userptr[argc];
	//int offset = 0;
	//for(i = 0; i < argc; i++)
		//kprintf("argv: %s\n",argv[i]);

	for(i = argc; i >= 0; i--){
		if(i == argc){
			userptr[i] = NULL;
		}
		else{
			int length = strlen(argv[i]) + 1;
			stackptr = stackptr - (length + (4 - (length % 4)));
			check = copyoutstr((userptr_t)argv[i],stackptr,length,NULL);
			if(check){
				splx(spl);
				return EFAULT;
			}
			userptr[i] = stackptr;
		}
	}

	for(i = argc; i >= 0; i--){
		stackptr -= 4;
		check = copyout(&userptr[i], stackptr, 4);
		if(check){
			splx(spl);
			return EFAULT;
		}
	}
	//kprintf("8\n");

	splx(spl);
	/* Warp to user mode. */
	md_usermode(argc /*argc*/, stackptr /*userspace addr of argv*/,
		    stackptr, entrypoint);


	return EINVAL;
}



int sys___time(userptr_t secondsPtr, userptr_t nsecondsPtr, int *retval)
{
	time_t seconds;
	u_int32_t nseconds;
	int result;
	
	//kprintf("RETVAL IS %d\n", *retval);

	gettime(&seconds, &nseconds);
	
	if(nsecondsPtr == NULL){
		return 0;
	}
	if (secondsPtr == NULL)
		goto skip;


	result = copyout(&seconds, secondsPtr, sizeof(time_t));
	if (result!=0) {
		*retval = -1;
		//kprintf("\n");
		//kprintf("sec fail%d\n", *retval);
		return EFAULT;
	}
	
	skip:

	result = copyout(&nseconds, nsecondsPtr, sizeof(u_int32_t));
	if (result != 0) {
		//kprintf("\n");
		//kprintf("nsec fail%d\n", *retval);
		*retval = -1;
		return EFAULT;
	}
	kprintf("\n");
	kprintf("%d\n", seconds);
	*retval = (int)seconds;

	return 0;
}


int sbrk(struct trapframe *tf, int * retval){
	vaddr_t old_end;
	vaddr_t new_end;
	vaddr_t old_start;
	struct addrspace *as = curthread->t_vmspace;
	int num = tf->tf_a0;

	old_end = as->heapend;
	old_start = as->heapstart;

	if(num % 4 != 0){
		*retval = -1;
		return EINVAL;
	}
	if(num == 0){
		*retval = old_end;
		return 0;
	}

	new_end = old_end + num;
	if(new_end >= (USERSTACK - 500 * PAGE_SIZE) || new_end < old_start)
		return EINVAL;

	int heappages = (new_end - old_start + PAGE_SIZE)/PAGE_SIZE;
	if(heappages >= 1024)
		return ENOMEM;

	int need;
	if(num%PAGE_SIZE != 0){
		need = num%PAGE_SIZE + 1;
	}
	else{
		need = num%PAGE_SIZE;
	}

	int i;
	int pagesTaken;
	struct page* coremap_page;
	for(i = 0; i < totalPages; i++){
		coremap_page = coremap + i;
		if(coremap_page->taken == 1)
			pagesTaken++;
	}

	if(need > pagesTaken)
		return ENOMEM;

	as->heapend = new_end;
	*retval = old_end;
	return 0;
}
