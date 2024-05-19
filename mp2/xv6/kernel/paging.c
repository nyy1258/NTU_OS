#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"
#include "proc.h"

/* NTU OS 2022 */
/* Page fault handler */
int handle_pgfault(uint64 va) {
  /* Find the address that caused the fault */
  // reference: https://hackmd.io/@Chang-Chia-Chi/MIT6S081/https%3A%2F%2Fhackmd.io%2F%40Chang-Chia-Chi%2FSyVVrPBRt
  
  struct proc *p = myproc();

  if (va >= p->sz || va < p->trapframe->sp) 
  {
    p->killed = 1;
  }else{
    char *mem = kalloc();

    if (mem == 0)
    {
      p->killed = 1;
    }else
    {
      memset(mem, 0, PGSIZE);
      if(mappages(p->pagetable, PGROUNDDOWN(va), PGSIZE, (uint64)mem, PTE_U|PTE_R|PTE_W|PTE_X) != 0) {
        kfree(mem);
        p->killed = 1;
      }
    }
  }

  return 0;
}
