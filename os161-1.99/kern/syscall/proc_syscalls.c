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
#include "opt-A2.h"

  /* this implementation of sys__exit does not do anything with the exit code */
  /* this needs to be fixed to get exit() and waitpid() working properly */

void sys__exit(int exitcode) {

  struct addrspace *as;
  struct proc *p = curproc;
  /* for now, just include this to keep the compiler from complaining about
     an unused variable */

  DEBUG(DB_SYSCALL,"Syscall: _exit(%d)\n",exitcode);

  //DEBUG(DB_THREADS,"===(START) DELETE PROCESS===\n");

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

  spinlock_acquire(&p->p_lock);
  DEBUG(DB_THREADS,"Updating proc: %s's %d children\n", p->p_name, p->p_children->num);
  spinlock_release(&curproc->p_lock);
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
    DEBUG(DB_THREADS,"===(END) PROCESS MARKED TO EXIT===\n");
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

   spinlock_acquire(&curproc->p_lock);
   DEBUG(DB_THREADS,"===(START) FORK PROCESS: %d ===\n", child->p_pid);
   spinlock_release(&curproc->p_lock);

   child->p_parent = curproc;

   array_add(curproc->p_children, child, NULL);

   struct trapframe *trapframe_for_child = kmalloc(sizeof(struct trapframe));

   *trapframe_for_child = *tf;
   spinlock_acquire(&curproc->p_lock);
   DEBUG(DB_THREADS,"Parent TF epc: %d | v0: %d | mem: %p \n", tf->tf_epc, tf->tf_v0, tf);
   DEBUG(DB_THREADS,"Child TF  epc: %d | v0: %d | mem: %p \n", trapframe_for_child->tf_epc, trapframe_for_child->tf_v0, trapframe_for_child);
   spinlock_release(&curproc->p_lock);

   as_copy(curproc_getas(), &child->p_addrspace);

   thread_fork("child_thread",
               child,
               &enter_forked_process,
               trapframe_for_child,
               0);

   *retval = child->p_pid;

   clocksleep(1);

   spinlock_acquire(&curproc->p_lock);
   DEBUG(DB_THREADS,"===(END) FORK PROCESS==\n");
   spinlock_release(&curproc->p_lock);
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

    spinlock_acquire(&curproc->p_lock);
    DEBUG(DB_THREADS,"===(START) WAITING FOR PROCESS: %d===\n", pid);
    spinlock_release(&curproc->p_lock);

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
        idx++;
    }
    if (!foundChild) {
        DEBUG(DB_THREADS,"No child with pid: %d \n", pid);
        exitstatus = _MKWAIT_EXIT(ECHILD);
        return(-1);
    } else {
        spinlock_acquire(&temp_child->p_lock);
        DEBUG(DB_THREADS,"Found child %d, waiting for them to exit...\n", temp_child->p_pid);
        while (!temp_child->p_exitstatus == P_running) {

            spinlock_release(&temp_child->p_lock);
            DEBUG(DB_THREADS,".");
            clocksleep(1);
            spinlock_acquire(&temp_child->p_lock);
        }
        DEBUG(DB_THREADS,"child has exited with an exit status of: %d\n", temp_child->p_exitcode);
        spinlock_release(&temp_child->p_lock);
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
  // DEBUG(DB_THREADS,"===(END) WAITING FOR PROCESS: %d===\n", pid);
  if (result) {
    return(result);
  }
  *retval = pid;
  return(0);
}

#ifdef OPT_A2
int sys_exec(char *progname, char **argv) {
    DEBUG(DB_THREADS,"=== Starting sys_exec program === \n");

	/*struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;*/
	(void) argv;


	char *kernprogname = kmalloc((strlen(progname) + 1) * sizeof(char*));

	copyin((const_userptr_t) progname, (void *) kernprogname, (strlen(progname) + 1) * sizeof(char*));
    DEBUG(DB_THREADS,"Kernal progname is: %s\n", kernprogname);
    DEBUG(DB_THREADS,"progname is: %s\n", progname);
    /*
	result = vfs_open(kernprogname, O_RDONLY, 0, &v);

	if (result) {
		return result;
	}


	KASSERT(curproc_getas() == NULL);

	as = as_create();
	if (as ==NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	curproc_setas(as);
	as_activate();

	result = load_elf(v, &entrypoint);
	if (result) {
		vfs_close(v);
		return result;
	}

	vfs_close(v);

	result = as_define_stack(as, &stackptr);
	if (result) {
		return result;
	}


    int spaceAlc = (argc+1) * sizeof(vaddr_t);

    vaddr_t *argv = kmalloc(spaceAlc);

    for (int i = 0; i < argc; i++) {
       argv[i] = argcopy_out(&stackptr, args[i]);

    }

    argv[argc] = (vaddr_t) NULL;

    stackptr = (stackptr/4)*4 - spaceAlc;

    copyout(argv, (userptr_t)stackptr, spaceAlc);


	enter_new_process(argc , (userptr_t) stackptr,
			  stackptr, entrypoint);

	kfree(argv);
	panic("enter_new_process returned\n");
	*/
	return EINVAL;
}
#endif

