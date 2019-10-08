/*
 * Sample/test code for running a user program.  You can use this for
 * reference when implementing the execv() system call. Remember though
 * that execv() needs to do more than this function does.
 */

#include <types.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <thread.h>
#include <curthread.h>
#include <vm.h>
#include <vfs.h>
#include <test.h>

/*
 * Load program "progname" and start running it in usermode.
 * Does not return except on error.
 *
 * Calls vfs_open on progname and thus may destroy it.
 */
int
runprogram(char *progname, void *ptr, unsigned long nargs)
{
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;

	/* Open the file. */
	result = vfs_open(progname, O_RDONLY, &v);
	if (result) {
		return result;
	}

	/* We should be a new thread. */
	assert(curthread->t_vmspace == NULL);

	/* Create a new address space. */
	curthread->t_vmspace = as_create();
	if (curthread->t_vmspace==NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Activate it. */
	as_activate(curthread->t_vmspace);

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(curthread->t_vmspace, &stackptr);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		return result;
	}

	int argc = (int) nargs;
	userptr_t userptr[argc];
	int i;
	char **args = ptr;
	//kprintf("program %d\n", nargs);

	for(i = argc; i >= 0; i--){
		//kprintf("i: %d\n", i);
		if(i == argc){
			userptr[i] = NULL;
		}
		else{
			int length = strlen(args[i]) + 1;
			stackptr = stackptr - (length + (4 - (length % 4)));
			copyout(args[i],stackptr,length);
			userptr[i] = stackptr;
		}
	}
	//kprintf("2\n");

	for(i = argc; i >= 0; i--){
		stackptr -= 4;
		copyout(&userptr[i], stackptr, 4);
	}
	//kprintf("argc = %d\n",argc);

	/* Warp to user mode. */
	md_usermode(argc /*argc*/, stackptr /*userspace addr of argv*/,
		    stackptr, entrypoint);
	
	/* md_usermode does not return */
	panic("md_usermode returned\n");
	return EINVAL;
}

