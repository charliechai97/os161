#ifndef _SYSCALL_H_
#define _SYSCALL_H_

/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */

int sys_reboot(int code);

void
md_forkentry(void *tf, unsigned long addr);

int write(struct trapframe *tf, int *retval);
int fork(struct trapframe *tf, int *retval);
int exit(struct trapframe *tf);
int sys_read(struct trapframe *trapFrame, int* retval);
int sys_waitpid(pid_t pid,int *status,int options, int* retval);
int sys_getpid(void);
void sys__exit(unsigned iexitCode);
int execv(struct trapframe *tf);
int sys___time(userptr_t secondsPtr, userptr_t nsecondsPtr, int * retval);
int sys_sbrk(intptr_t amount, int * retval);


#endif /* _SYSCALL_H_ */
