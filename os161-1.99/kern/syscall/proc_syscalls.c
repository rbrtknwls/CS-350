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

  DEBUG(DB_SYSCALL,"Syscall: _exit(%d)\n",exitcode);

  DEBUG(DB_THREADS,"===(START) DELETE PROCESS===\n");

  KASSERT(p->p_addrspace != NULL);
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

#ifdef OPT_A1 // Loop through children and delete them

  DEBUG(DB_THREADS,"Updating proc: %s's %d children\n", p->p_name, p->p_children->num);
  while (p->p_children->num != 0) {
    struct proc *temp_child = array_get(p->p_children, 0);
    array_remove(p->p_children, 0);
    spinlock_acquire(&temp_child->p_lock);
    if (temp_child->p_exitstatus == P_exited) {
        spinlock_release(&temp_child->p_lock);
        proc_destroy(temp_child);
    } else {
        temp_child->p_parent = NULL;
        spinlock_release(&temp_child->p_lock);
    }
  }

#endif
  /* detach this thread from its process */
  /* note: curproc cannot be used after this call */
  proc_remthread(curthread);

  /* if this is the last user process in the system, proc_destroy()
     will wake up the kernel menu thread */
#ifdef OPT_A1
  spinlock_acquire(&p->p_lock);
  if (p->p_parent == NULL || p->p_parent->p_exitstatus == P_exited) { // Process is no longer running
    DEBUG(DB_THREADS,"===(END) DELETE PROCESS===\n");
    spinlock_release(&p->p_lock);
    proc_destroy(p);
  } else {
    DEBUG(DB_THREADS,"===(END) PROCESS MARKED TO EXIT\n");
    p->p_exitstatus = P_exited;
    p->p_exitcode = exitcode;
    spinlock_release(&p->p_lock);
  }

#else
  proc_destroy(p);
#endif
  
  thread_exit();
  /* thread_exit() does not return, so we should never get here */
  panic("return from thread_exit in sys_exit\n");
}

#ifdef OPT_A1
int
sys_fork(pid_t *retval, struct trapframe *tf)
{
   struct proc *child = proc_create_runprogram("child");
   DEBUG(DB_THREADS,"===(START) FORK PROCESS: %d ===\n", child->p_pid);

   child->p_parent = curproc;

   array_add(curproc->p_children, child, NULL);

   struct trapframe *trapframe_for_child = kmalloc(sizeof(struct trapframe));

   *trapframe_for_child = *tf;
   DEBUG(DB_THREADS,"Parent TF epc: %d | v0: %d | mem: %p \n", tf->tf_epc, tf->tf_v0, tf);
   DEBUG(DB_THREADS,"Child TF  epc: %d | v0: %d | mem: %p \n", trapframe_for_child->tf_epc, trapframe_for_child->tf_v0, trapframe_for_child);


   as_copy(curproc_getas(), &child->p_addrspace);

   thread_fork("child_thread",
               child,
               &enter_forked_process,
               trapframe_for_child,
               0);

   *retval = child->p_pid;

   clocksleep(1);
   DEBUG(DB_THREADS,"===(END) FORK PROCESS==\n");
   return 0;
}
#endif

/* stub handler for getpid() system call                */
int
sys_getpid(pid_t *retval)
{
  #ifdef OPT_A1
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

  if (options != 0) {
      return(EINVAL);
    }

  #ifdef OPT_A1
    DEBUG(DB_THREADS,"===(START) WAITING FOR PROCESS: %d===\n", pid);
    unsigned int idx = 0; // Stores the current index of the child we are looking for
    bool foundChild = false;
    struct proc *temp_child;

    while (idx < curproc->p_children->num) {
        temp_child = array_get(curproc->p_children, idx);
        if (temp_child->p_pid == pid) {
            array_remove(curproc->p_children, idx);
            foundChild = true;
            break;
        }
    }
    if (!foundChild) {
        DEBUG(DB_THREADS,"No child with pid: %d \n", pid);
        exitstatus = _MKWAIT_EXIT(ECHILD);
    } else {
        spinlock_acquire(&temp_child->p_lock);
        DEBUG(DB_THREADS,"Found child, waiting for them to exit...\n");
        while (!temp_child->p_exitstatus == P_running) {
            spinlock_release(&temp_child->p_lock);
            clocksleep(1);
            spinlock_acquire(&temp_child->p_lock);
        }
        spinlock_release(&temp_child->p_lock);
        DEBUG(DB_THREADS,"child has exited with an exist status of: %d\n", temp_child->p_exitcode);
        exitstatus = _MKWAIT_EXIT(temp_child->p_exitcode);
        proc_destroy(temp_child);

    }
  #else

  /* this is just a stub implementation that always reports an
     exit status of 0, regardless of the actual exit status of
     the specified process.   
     In fact, this will return 0 even if the specified process
     is still running, and even if it never existed in the first place.

     Fix this!
  */

  /* for now, just pretend the exitstatus is 0 */
  exitstatus = 0;
  #endif

  result = copyout((void *)&exitstatus,status,sizeof(int));
   DEBUG(DB_THREADS,"===(END) WAITING FOR PROCESS: %d===\n", pid);
  if (result) {
    return(result);
  }
  *retval = pid;
  return(0);
}

