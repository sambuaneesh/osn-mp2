#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

extern int time_slice[MLFQ_QUEUES];
extern struct proc *proc_queues[4][NPROC];
extern int count_queues[MLFQ_QUEUES];

struct spinlock tickslock;
uint ticks;

extern char trampoline[], uservec[], userret[];

// in kernelvec.S, calls kerneltrap().
void kernelvec();

extern int devintr();

// writing a function to change the tick count of the queue based on the queue number
// queue number 0 = 1 timer tick
// queue number 1 = 3 timer ticks
// queue number 2 = 9 timer ticks
// queue number 3 = 15 timer ticks
// int get_ticks(int queue_number)
// {
//   if (queue_number == 0)
//     return 1;
//   else if (queue_number == 1)
//     return 3;
//   else if (queue_number == 2)
//     return 9;
//   else if (queue_number == 3)
//     return 15;
//   else
//     return 0;
// }

void trapinit(void)
{
  initlock(&tickslock, "time");
}

// set up to take exceptions and traps while in the kernel.
void trapinithart(void)
{
  w_stvec((uint64)kernelvec);
}

//
// handle an interrupt, exception, or system call from user space.
// called from trampoline.S
//
void usertrap(void)
{
  int which_dev = 0;

  if ((r_sstatus() & SSTATUS_SPP) != 0)
    panic("usertrap: not from user mode");

  // send interrupts and exceptions to kerneltrap(),
  // since we're now in the kernel.
  w_stvec((uint64)kernelvec);

  struct proc *p = myproc();

  // save user program counter.
  p->trapframe->epc = r_sepc();

  if (r_scause() == 8)
  {
    // system call

    if (killed(p))
      exit(-1);

    // sepc points to the ecall instruction,
    // but we want to return to the next instruction.
    p->trapframe->epc += 4;

    // an interrupt will change sepc, scause, and sstatus,
    // so enable only now that we're done with those registers.
    intr_on();

    syscall();
  }
  else if ((which_dev = devintr()) != 0)
  {
    // ok
  }
  else
  {
    printf("usertrap(): unexpected scause %p pid=%d\n", r_scause(), p->pid);
    printf("            sepc=%p stval=%p\n", r_sepc(), r_stval());
    setkilled(p);
  }

  if (killed(p))
    exit(-1);

  if (which_dev == 2 && p->alarm_on == 1 && p->handlerpermission == 1)
  {

    struct trapframe *tf = kalloc();
    memmove(tf, p->trapframe, PGSIZE);
    p->alarm_tf = tf;

    p->cur_ticks = p->cur_ticks + 1;
    if (p->cur_ticks >= p->ticks)
    {
      p->trapframe->epc = p->handler;
      p->handlerpermission = 0;
    }
  }

// give up the CPU if this is a timer interrupt.
#ifdef RR_SCHED
  if (which_dev == 2)
    yield();
#endif

  if (which_dev == 2)
  {
#ifdef MLFQ_SCHED
    if (p != 0 && p->state == RUNNING)
    {
      if (p->timeRunning_q >= time_slice[p->previous_q])
      {
        if (p->previous_q != MLFQ_QUEUES-1)
        {
          push_MLFQ(p, (p->previous_q + 1));
        }
        else
        {
          push_MLFQ(p, 3);
        }
        yield();
      }
    }

    if (p->state == RUNNING)
    {
      int INIT(i);
      while (i < p->previous_q)
      {
        if (count_queues[i] != 0)
        {
          if (proc_queues[i][count_queues[i] - 1]->state == RUNNABLE)
          {
            push_MLFQ(p, p->previous_q);
            yield();
          }
        }
        i = i+1;
      }
    }
#endif
  }

  usertrapret();
}

//
// return to user space
//
void usertrapret(void)
{
  struct proc *p = myproc();

  // we're about to switch the destination of traps from
  // kerneltrap() to usertrap(), so turn off interrupts until
  // we're back in user space, where usertrap() is correct.
  intr_off();

  // send syscalls, interrupts, and exceptions to uservec in trampoline.S
  uint64 trampoline_uservec = TRAMPOLINE + (uservec - trampoline);
  w_stvec(trampoline_uservec);

  // set up trapframe values that uservec will need when
  // the process next traps into the kernel.
  p->trapframe->kernel_satp = r_satp();         // kernel page table
  p->trapframe->kernel_sp = p->kstack + PGSIZE; // process's kernel stack
  p->trapframe->kernel_trap = (uint64)usertrap;
  p->trapframe->kernel_hartid = r_tp(); // hartid for cpuid()

  // set up the registers that trampoline.S's sret will use
  // to get to user space.

  // set S Previous Privilege mode to User.
  unsigned long x = r_sstatus();
  x &= ~SSTATUS_SPP; // clear SPP to 0 for user mode
  x |= SSTATUS_SPIE; // enable interrupts in user mode
  w_sstatus(x);

  // set S Exception Program Counter to the saved user pc.
  w_sepc(p->trapframe->epc);

  // tell trampoline.S the user page table to switch to.
  uint64 satp = MAKE_SATP(p->pagetable);

  // jump to userret in trampoline.S at the top of memory, which
  // switches to the user page table, restores user registers,
  // and switches to user mode with sret.
  uint64 trampoline_userret = TRAMPOLINE + (userret - trampoline);
  ((void (*)(uint64))trampoline_userret)(satp);
}

// interrupts and exceptions from kernel code go here via kernelvec,
// on whatever the current kernel stack is.
void kerneltrap()
{
  int which_dev = 0;
  uint64 sepc = r_sepc();
  uint64 sstatus = r_sstatus();
  uint64 scause = r_scause();

  if ((sstatus & SSTATUS_SPP) == 0)
    panic("kerneltrap: not from supervisor mode");
  if (intr_get() != 0)
    panic("kerneltrap: interrupts enabled");

  if ((which_dev = devintr()) == 0)
  {
    printf("scause %p\n", scause);
    printf("sepc=%p stval=%p\n", r_sepc(), r_stval());
    panic("kerneltrap");
  }

// give up the CPU if this is a timer interrupt.
#ifdef RR_SCHED
  if (which_dev == 2)
  {
    struct proc *current_process = myproc();
    if (current_process != 0 && current_process->state == RUNNING)
    {
      yield();
    }
  }
#endif

  // Restoring trap registers after potential traps caused by yield().
  if (which_dev == 2 && myproc() != 0 && myproc()->state == RUNNING)
  {
#ifdef MLFQ_SCHED
    struct proc *p = myproc();

    if (p->timeRunning_q >= time_slice[p->previous_q])
    {
      if (p->previous_q != MLFQ_QUEUES-1) // if not the last queue
      {
        push_MLFQ(p, p->previous_q + 1);
      }
      else
      {
        push_MLFQ(p, 3);
      }
      yield();
    }

    if (p->state == RUNNING)
    {
      int INIT(i);
      while (i < p->previous_q)
      {
        if (count_queues[i] != 0)
        {
          if (proc_queues[i][count_queues[i] - 1]->state == RUNNABLE)
          {
            push_MLFQ(p, p->previous_q);
            yield();
          }
        }
        i = i+1;
      }
    }
#endif
  }

  w_sepc(sepc);
  w_sstatus(sstatus);
}

void clockintr()
{
  acquire(&tickslock);
  ticks++;
  update_time();
  // for (struct proc *p = proc; p < &proc[NPROC]; p++)
  // {
  //   acquire(&p->lock);
  //   if (p->state == RUNNING)
  //   {
  //     printf("here");
  //     p->rtime++;
  //   }
  //   // if (p->state == SLEEPING)
  //   // {
  //   //   p->wtime++;
  //   // }
  //   release(&p->lock);
  // }
  wakeup(&ticks);
  release(&tickslock);

  // #ifdef FCFS_SCHED
  //   // Disable preemption for the current process only if FCFS scheduling is active
  //   myproc()->disable_preemption = 1;
  // #endif
}

// check if it's an external interrupt or software interrupt,
// and handle it.
// returns 2 if timer interrupt,
// 1 if other device,
// 0 if not recognized.
int devintr()
{
  uint64 scause = r_scause();

  if ((scause & 0x8000000000000000L) &&
      (scause & 0xff) == 9)
  {
    // this is a supervisor external interrupt, via PLIC.

    // irq indicates which device interrupted.
    int irq = plic_claim();

    if (irq == UART0_IRQ)
    {
      uartintr();
    }
    else if (irq == VIRTIO0_IRQ)
    {
      virtio_disk_intr();
    }
    else if (irq)
    {
      printf("unexpected interrupt irq=%d\n", irq);
    }

    // the PLIC allows each device to raise at most one
    // interrupt at a time; tell the PLIC the device is
    // now allowed to interrupt again.
    if (irq)
      plic_complete(irq);

    return 1;
  }
  else if (scause == 0x8000000000000001L)
  {
    // software interrupt from a machine-mode timer interrupt,
    // forwarded by timervec in kernelvec.S.

    if (cpuid() == 0)
    {
      clockintr();
    }

    // acknowledge the software interrupt by clearing
    // the SSIP bit in sip.
    w_sip(r_sip() & ~2);

    return 2;
  }
  else
  {
    return 0;
  }
}
