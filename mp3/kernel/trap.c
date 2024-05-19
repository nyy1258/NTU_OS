#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

struct spinlock tickslock;
uint ticks;

extern char trampoline[], uservec[], userret[];

// in kernelvec.S, calls kerneltrap().
void kernelvec();

extern int devintr();

void
trapinit(void)
{
  initlock(&tickslock, "time");
}

// set up to take exceptions and traps while in the kernel.
void
trapinithart(void)
{
  w_stvec((uint64)kernelvec);
}

// switch context
void Store_context(int id){
  struct proc *cur = myproc();

  cur->contextdata[id].s_val[0] = cur->trapframe->s0;
  cur->contextdata[id].s_val[1] = cur->trapframe->s1;
  cur->contextdata[id].s_val[2] = cur->trapframe->s2;
  cur->contextdata[id].s_val[3] = cur->trapframe->s3;
  cur->contextdata[id].s_val[4] = cur->trapframe->s4;
  cur->contextdata[id].s_val[5] = cur->trapframe->s5;
  cur->contextdata[id].s_val[6] = cur->trapframe->s6;
  cur->contextdata[id].s_val[7] = cur->trapframe->s7;
  cur->contextdata[id].s_val[8] = cur->trapframe->s8;
  cur->contextdata[id].s_val[9] = cur->trapframe->s9;
  cur->contextdata[id].s_val[10] = cur->trapframe->s10;
  cur->contextdata[id].s_val[11] = cur->trapframe->s11;

  cur->contextdata[id].a_val[0] = cur->trapframe->a0;
  cur->contextdata[id].a_val[1] = cur->trapframe->a1;
  cur->contextdata[id].a_val[2] = cur->trapframe->a2;
  cur->contextdata[id].a_val[3] = cur->trapframe->a3;
  cur->contextdata[id].a_val[4] = cur->trapframe->a4;
  cur->contextdata[id].a_val[5] = cur->trapframe->a5;
  cur->contextdata[id].a_val[6] = cur->trapframe->a6;
  cur->contextdata[id].a_val[7] = cur->trapframe->a7;

  cur->contextdata[id].t_val[0] = cur->trapframe->t0;
  cur->contextdata[id].t_val[1] = cur->trapframe->t1;
  cur->contextdata[id].t_val[2] = cur->trapframe->t2;
  cur->contextdata[id].t_val[3] = cur->trapframe->t3;
  cur->contextdata[id].t_val[4] = cur->trapframe->t4;
  cur->contextdata[id].t_val[5] = cur->trapframe->t5;
  cur->contextdata[id].t_val[6] = cur->trapframe->t6;

  cur->contextdata[id].epc = cur->trapframe->epc;
  cur->contextdata[id].tp = cur->trapframe->tp;
  cur->contextdata[id].gp = cur->trapframe->gp;
  cur->contextdata[id].ra = cur->trapframe->ra;
  cur->contextdata[id].sp = cur->trapframe->sp;

  cur->contextdata[id].kernel_satp = cur->trapframe->kernel_satp;
  cur->contextdata[id].kernel_sp = cur->trapframe->kernel_sp;
  cur->contextdata[id].kernel_trap = cur->trapframe->kernel_trap;
  cur->contextdata[id].kernel_hartid = cur->trapframe->kernel_hartid;
}

void Restore_context(int id){
  struct proc *cur = myproc();

  cur->trapframe->s0 = cur->contextdata[id].s_val[0];
  cur->trapframe->s1 = cur->contextdata[id].s_val[1];
  cur->trapframe->s2 = cur->contextdata[id].s_val[2];
  cur->trapframe->s3 = cur->contextdata[id].s_val[3];
  cur->trapframe->s4 = cur->contextdata[id].s_val[4];
  cur->trapframe->s5 = cur->contextdata[id].s_val[5];
  cur->trapframe->s6 = cur->contextdata[id].s_val[6];
  cur->trapframe->s7 = cur->contextdata[id].s_val[7];
  cur->trapframe->s8 = cur->contextdata[id].s_val[8];
  cur->trapframe->s9 = cur->contextdata[id].s_val[9];
  cur->trapframe->s10 = cur->contextdata[id].s_val[10];
  cur->trapframe->s11 = cur->contextdata[id].s_val[11];

  cur->trapframe->a0 = cur->contextdata[id].a_val[0];
  cur->trapframe->a1 = cur->contextdata[id].a_val[1];
  cur->trapframe->a2 = cur->contextdata[id].a_val[2];
  cur->trapframe->a3 = cur->contextdata[id].a_val[3];
  cur->trapframe->a4 = cur->contextdata[id].a_val[4];
  cur->trapframe->a5 = cur->contextdata[id].a_val[5];
  cur->trapframe->a6 = cur->contextdata[id].a_val[6];
  cur->trapframe->a7 = cur->contextdata[id].a_val[7];

  cur->trapframe->t0 = cur->contextdata[id].t_val[0];
  cur->trapframe->t1 = cur->contextdata[id].t_val[1];
  cur->trapframe->t2 = cur->contextdata[id].t_val[2];
  cur->trapframe->t3 = cur->contextdata[id].t_val[3];
  cur->trapframe->t4 = cur->contextdata[id].t_val[4];
  cur->trapframe->t5 = cur->contextdata[id].t_val[5];
  cur->trapframe->t6 = cur->contextdata[id].t_val[6];

  cur->trapframe->epc = cur->contextdata[id].epc;
  cur->trapframe->tp = cur->contextdata[id].tp;
  cur->trapframe->gp = cur->contextdata[id].gp;
  cur->trapframe->ra = cur->contextdata[id].ra;
  cur->trapframe->sp = cur->contextdata[id].sp;

  cur->trapframe->kernel_satp = cur->contextdata[id].kernel_satp;
  cur->trapframe->kernel_sp = cur->contextdata[id].kernel_sp;
  cur->trapframe->kernel_trap = cur->contextdata[id].kernel_trap;
  cur->trapframe->kernel_hartid = cur->contextdata[id].kernel_hartid;
}

//
// handle an interrupt, exception, or system call from user space.
// called from trampoline.S
//
void
usertrap(void)
{
  int which_dev = 0;

  if((r_sstatus() & SSTATUS_SPP) != 0)
    panic("usertrap: not from user mode");

  // send interrupts and exceptions to kerneltrap(),
  // since we're now in the kernel.
  w_stvec((uint64)kernelvec);

  struct proc *p = myproc();
  
  // save user program counter.
  p->trapframe->epc = r_sepc();
  
  if(r_scause() == 8){
    // system call

    if(p->killed)
      exit(-1);

    // sepc points to the ecall instruction,
    // but we want to return to the next instruction.
    p->trapframe->epc += 4;

    // an interrupt will change sstatus &c registers,
    // so don't enable until done with those registers.
    intr_on();

    syscall();
  } else if((which_dev = devintr()) != 0){
    // ok
  } else {
    printf("usertrap(): unexpected scause %p pid=%d\n", r_scause(), p->pid);
    printf("            sepc=%p stval=%p\n", r_sepc(), r_stval());
    p->killed = 1;
  }

  if(p->killed)
    exit(-1);

  // give up the CPU if this is a timer interrupt.
  if(which_dev == 2){
    //TODO: mp3
    if (p->delay > 0)
    {
      p->totalticks += 1;
    }

    yield();
  }
  usertrapret();
}

//
// return to user space
//
void
usertrapret(void)
{
  struct proc *p = myproc();

  // we're about to switch the destination of traps from
  // kerneltrap() to usertrap(), so turn off interrupts until
  // we're back in user space, where usertrap() is correct.
  intr_off();

  // send syscalls, interrupts, and exceptions to trampoline.S
  w_stvec(TRAMPOLINE + (uservec - trampoline));

  // set up trapframe values that uservec will need when
  // the process next re-enters the kernel.
  p->trapframe->kernel_satp = r_satp();         // kernel page table
  p->trapframe->kernel_sp = p->kstack + PGSIZE; // process's kernel stack
  p->trapframe->kernel_trap = (uint64)usertrap;
  p->trapframe->kernel_hartid = r_tp();         // hartid for cpuid()


  //  wy+++>>
  if (p->totalticks == p-> delay){
    Store_context(p->context_id);
    p->trapframe->epc = p->thrd_handler;
    p->trapframe->a0 = p-> handler_arg;
    p->delay = -1;
  }
  else if (p->trapframe->a7 == 23){  // thrdresume
    Restore_context(p->context_id);
  }
  else if (p->trapframe->a7 == 24){  // cancelthrdstop

    if(p->context_id != -1)
    {
      if(p->isexit == 0)
      {
        Store_context(p->context_id);
      }else{
        int temp = p->context_id;
        p->context_used[temp] = 0;
        p->delay = -1;
      }
    }
  }
  // wy+++<<

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
  // jump to trampoline.S at the top of memory, which 
  // switches to the user page table, restores user registers,
  // and switches to user mode with sret.
  uint64 fn = TRAMPOLINE + (userret - trampoline);
  
  ((void (*)(uint64,uint64))fn)(TRAPFRAME, satp);
}

// interrupts and exceptions from kernel code go here via kernelvec,
// on whatever the current kernel stack is.
void 
kerneltrap()
{
  int which_dev = 0;
  uint64 sepc = r_sepc();
  uint64 sstatus = r_sstatus();
  uint64 scause = r_scause();
  
  if((sstatus & SSTATUS_SPP) == 0)
    panic("kerneltrap: not from supervisor mode");
  if(intr_get() != 0)
    panic("kerneltrap: interrupts enabled");

  if((which_dev = devintr()) == 0){
    printf("scause %p\n", scause);
    printf("sepc=%p stval=%p\n", r_sepc(), r_stval());
    panic("kerneltrap");
  }

  // give up the CPU if this is a timer interrupt.
  if(which_dev == 2 && myproc() != 0 && myproc()->state == RUNNING){
    //TODO: mp3
    struct proc *p = myproc();
    if (p->delay > 0)
    {
      p->totalticks += 1;
    }
    yield();
  }

  // the yield() may have caused some traps to occur,
  // so restore trap registers for use by kernelvec.S's sepc instruction.
  w_sepc(sepc);
  w_sstatus(sstatus);

}

void
clockintr()
{
  acquire(&tickslock);
  ticks++;
  wakeup(&ticks);
  release(&tickslock);
}

// check if it's an external interrupt or software interrupt,
// and handle it.
// returns 2 if timer interrupt,
// 1 if other device,
// 0 if not recognized.
int
devintr()
{
  uint64 scause = r_scause();

  if((scause & 0x8000000000000000L) &&
     (scause & 0xff) == 9){
    // this is a supervisor external interrupt, via PLIC.

    // irq indicates which device interrupted.
    int irq = plic_claim();

    if(irq == UART0_IRQ){
      uartintr();
    } else if(irq == VIRTIO0_IRQ){
      virtio_disk_intr();
    } else if(irq){
      printf("unexpected interrupt irq=%d\n", irq);
    }

    // the PLIC allows each device to raise at most one
    // interrupt at a time; tell the PLIC the device is
    // now allowed to interrupt again.
    if(irq)
      plic_complete(irq);

    return 1;
  } else if(scause == 0x8000000000000001L){
    // software interrupt from a machine-mode timer interrupt,
    // forwarded by timervec in kernelvec.S.

    if(cpuid() == 0){
      clockintr();
    }
    
    // acknowledge the software interrupt by clearing
    // the SSIP bit in sip.
    w_sip(r_sip() & ~2);

    return 2;
  } else {
    return 0;
  }
}

