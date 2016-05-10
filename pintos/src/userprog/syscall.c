#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

/* ---------------------------------------------------------- */
static void get_args (struct intr_frame *f, int *args, int n);
static void exit (int status);
static int write (/*struct file *f,*/ int fd, const void *buffer, unsigned size);

static tid_t exec (const char *cmd_line);
static int wait (tid_t pid);
/* ---------------------------------------------------------- */

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  // printf ("system call!\n");

   /* ---------------------------------------------------------- */
  // syscall num stored in (esp)
  int *f_esp = f->esp;
  int args[3];

  // printf("esp: %p\n", f_esp);
  // printf("esp: %d\n", *f_esp);
  // printf("esp+0x4: %d\n", *(f_esp+1));
  // printf("esp+0x8: %p\n", *(f_esp+2));
  // printf("esp+0xc: %u\n", *(f_esp+3));

  switch(*f_esp){

  	case SYS_WRITE:	// int write (int fd, const void *buffer, unsigned size)
  		get_args(f, &args[0], 3);
  		f->eax = write(/*f,*/ args[0], args[1], args[2]);
  		break;

  	case SYS_EXIT:
  		get_args(f, &args[0], 1);
  		exit(args[0]);
  		break;

  	case SYS_EXEC:
  		get_args(f, &args[0], 1);
  		f->eax = exec(args[0]);
  		break;

  	case SYS_WAIT:
  		get_args(f, &args[0], 1);
  		f->eax = wait(args[0]);
  		break;

  }
  /* ---------------------------------------------------------- */

}

static void
exit (int status)
{
    struct thread *cur = thread_current ();
    printf("%s: exit(%d)\n", cur->name, status);
    struct thread* par = find_thread(cur->parent);
    par->child_status = status;
    // list_remove (&cur->childelem);
    // list_remove (&cur->waitelem);
    thread_exit ();
}

static tid_t
exec (const char *cmd_line)
{
	/* run the executable given in cmd_line, pass any given arguments
   * return the new process's program id (or thread id?)
	 * -1 if the program cannot load or run for any reason
	 * thus, parent cannot return from exec until it knows whether child
	 * successfully loaded its executable
	 */

	return process_execute(cmd_line, thread_current ());

}

static int
write (/*struct file *f,*/ int fd, const void *buffer, unsigned size)
{
	// write to console
	if (fd == STDOUT_FILENO)
	{
    /* putbuf () resides in lib/kernel/console.c
       Writes the SIZE characters in BUFFER to the console.
     */
		putbuf (buffer, size);
		return size;
	}

	// return file_write (f, buffer, size);
  return -1;
}

static int
wait (tid_t pid)
{
	return process_wait(pid);
}

static void
get_args (struct intr_frame *f, int *args, int n)
{
	int i;
	int* ptr = f->esp;
	
	for (i = 0; i < n; i++)
	{
		ptr++;
		args[i] = *ptr;
	}
}
