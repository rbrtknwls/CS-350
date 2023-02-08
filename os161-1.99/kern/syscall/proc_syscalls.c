#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/wait.h>
#include <lib.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <thread.h>
#include <addrspace.h>
#include <copyinout.h>
#include <mips/trapframe.h>
#include <clock.h>

#include "opt-A1.h"

  /* this implementation of sys__exit does not do anything with the exit code */
  /* this needs to be fixed to get exit() and waitpid() working properly */

void sys__exit(int exitcode) {

  struct addrspace *as;
  struct proc *p = curproc;
  /* for now, just include this to keep the compiler from complaining about
     an unused variable */
  (void)exitcode;

  DEBUG(DB_SYSCALL,"Syscall: _exit(%d)\n",exitcode);

  KASSERT(curproc->p_addrspace != NULL);
  as_deactivate();
  /*
   * clear p_addrspace before calling as_destroy. Otherwise if
   * as_destroy sleeps (which is quite possible) when we
   * come back we'll be calling as_activate on a
   * half-destroyed address space. This tends to be
   * messily fatal.
   */
  as = curproc_setas(NULL);
  as_destroy(as);

  /* detach this thread from its process */
  /* note: curproc cannot be used after this call */
  proc_remthread(curthread);

  /* if this is the last user process in the system, proc_destroy()
     will wake up the kernel menu thread */
  proc_destroy(p);
  
  thread_exit();
  /* thread_exit() does not return, so we should never get here */
  panic("return from thread_exit in sys_exit\n");
}

#ifdef OPT_A1
int
sys_fork(pid_t *retval, struct trapframe *tf)
{
   DEBUG(DB_THREADS,"===FORKING A NEW PROCESS==\n");
   struct proc *child = proc_create_runprogram("child");
   struct trapframe *trapframe_for_child = kmalloc(sizeof(struct trapframe));

   *trapframe_for_child = *tf;

   as_copy(curproc_getas(), &child->p_addrspace);

   DEBUG(DB_THREADS,"Trapframe is now updated\n");

   thread_fork("child_thread",
               child,
               &enter_forked_process,
               &trapframe_for_child,
               0);

   *retval = child->p_pid;

   kfree(trapframe_for_child);

   clocksleep(1);
   DEBUG(DB_THREADS,"===FORKING IS DONE===\n");
   return 0;
}
#endif

/* stub handler for getpid() system call                */
int
sys_getpid(pid_t *retval)
{
  #ifdef OPT_A1
     DEBUG(DB_THREADS,"Getting pid for proc: %s \n", curproc->p_name);
     *retval = curproc->p_pid;
  #else
     *retval = 1;
  #endif
  return(0);
}

/* stub handler for waitpid() system call                */

int
sys_waitpid(pid_t pid,
	    userptr_t status,
	    int options,
	    pid_t *retval)
{
  int exitstatus;
  int result;

  /* this is just a stub implementation that always reports an
     exit status of 0, regardless of the actual exit status of
     the specified process.   
     In fact, this will return 0 even if the specified process
     is still running, and even if it never existed in the first place.

     Fix this!
  */

  if (options != 0) {
    return(EINVAL);
  }
  /* for now, just pretend the exitstatus is 0 */
  exitstatus = 0;
  result = copyout((void *)&exitstatus,status,sizeof(int));
  if (result) {
    return(result);
  }
  *retval = pid;
  return(0);
}

