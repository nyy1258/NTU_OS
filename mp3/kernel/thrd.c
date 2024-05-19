#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

// for mp3
uint64
sys_thrdstop(void)
{
  int delay;
  uint64 context_id_ptr;
  uint64 handler, handler_arg;
  if (argint(0, &delay) < 0)
    return -1;
  if (argaddr(1, &context_id_ptr) < 0)
    return -1;
  if (argaddr(2, &handler) < 0)
    return -1;
  if (argaddr(3, &handler_arg) < 0)
    return -1;

  //struct proc *proc = myproc();

  //TODO: mp3
  struct proc *p = myproc();
  int id = 0;

  if (copyin(p->pagetable, (char *)&id, context_id_ptr, sizeof(id)) == -1)
  {
      return -1;
  };
  
  //printf("check id: %d\n", id);

  if(id == -1)
  {
    int i = 0;
    for(; i < MAX_THRD_NUM; i++)
    {
        if (p->context_used[i] == 0)
        {
            p->context_used[i] = 1;
            id = i;
            break;
        }
    }

    if(i == MAX_THRD_NUM) return -1;

    if (copyout(p->pagetable, context_id_ptr, (char *)&id, sizeof(id)) == -1)
    {
        return -1;
    };
  }

  p -> totalticks = 0;
  p -> delay = delay;
  p -> context_id = id;
  p -> thrd_handler = handler;
  p -> handler_arg = handler_arg; 


  return 0;
}

// for mp3
uint64
sys_cancelthrdstop(void)
{
  int context_id, is_exit;
  if (argint(0, &context_id) < 0)
    return -1;
  if (argint(1, &is_exit) < 0)
    return -1;

  if (context_id < 0 || context_id >= MAX_THRD_NUM) {
    return -1;
  }

  //struct proc *proc = myproc();

  //TODO: mp3
  struct proc *p = myproc();
  p->context_id = context_id;
  p->isexit = is_exit;

  return p->totalticks;
}

// for mp3
uint64
sys_thrdresume(void)
{
  int context_id;
  if (argint(0, &context_id) < 0)
    return -1;

  //struct proc *proc = myproc();

  //TODO: mp3
  struct proc *p = myproc();
  p->context_id = context_id;

  return 0;
}
