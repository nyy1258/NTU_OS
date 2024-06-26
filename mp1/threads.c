#include "kernel/types.h"
#include "user/setjmp.h"
#include "user/threads.h"
#include "user/user.h"
#define NULL 0


static struct thread* current_thread = NULL;
static int id = 1;
static jmp_buf env_st;
static jmp_buf env_tmp;

struct thread *thread_create(void (*f)(void *), void *arg){
    struct thread *t = (struct thread*) malloc(sizeof(struct thread));
    unsigned long new_stack_p;
    unsigned long new_stack;
    new_stack = (unsigned long) malloc(sizeof(unsigned long)*0x100);
    new_stack_p = new_stack +0x100*8-0x2*8;
    t->fp = f;
    t->arg = arg;
    t->ID  = id;
    t->buf_set = 0;
    t->stack = (void*) new_stack;
    t->stack_p = (void*) new_stack_p;
    id++;

    // part 2
    t->sig_handler[0] = NULL_FUNC;
    t->sig_handler[1] = NULL_FUNC;
    t->signo = -1;
    t->handler_buf_set = 0;
    return t;
}

void thread_add_runqueue(struct thread *t){

    if(current_thread == NULL)
    {
        // TODO
        current_thread = t;
        t->next = t;
        t->previous = NULL;
    }
    else{
        // TODO
        if (current_thread->previous == NULL)
        {
            current_thread->previous = t;
            current_thread->next = t;
            t-> previous = current_thread;
            t-> next = current_thread;
        }
        else {
            struct thread *temp = current_thread;
            
            while(temp->next != current_thread)
            {
                temp = temp->next;
            }

            current_thread->previous = t;
            temp->next = t;
            t->previous = temp;
            t->next = current_thread;     
        }

        // inherit the signal handlers from current thread
        t->sig_handler[0] = current_thread -> sig_handler[0];
        t->sig_handler[1] = current_thread -> sig_handler[1];
    }

    return;
}

void thread_yield(void){
    // TODO

    if(current_thread->signo!=-1)
    {
        if(current_thread->handler_buf_set==0)
        {
            current_thread->handler_buf_set = 1;    
        }

        if(!setjmp(current_thread->handler_env))
        {
            schedule();
            dispatch();
        }
    }
    else{
        if(current_thread->buf_set==0)
        {
            current_thread->buf_set = 1;    
        }

        if(!setjmp(current_thread->env))
        {
            schedule();
            dispatch();
        }
    }
    
    return;
}

void dispatch(void){
    // TODO
    if(current_thread->buf_set == 1)
    {
        // check handler signo
        if(current_thread->signo!=-1)
        {
            if(current_thread->handler_buf_set == 0)
            {
                int no = current_thread->signo;
                if(no == -2)
                {
                    thread_exit();
                }
                else
                {
                    int flag = setjmp(env_tmp);
                    env_tmp->sp = (unsigned long)current_thread->env->sp;

                    if(flag == 0){
                        longjmp(env_tmp, 1);
                    }

                    current_thread->sig_handler[no](no);
                    current_thread->signo = -1;

                }
                
            }
            else{
                longjmp(current_thread->handler_env,1);
            }
        }
        longjmp(current_thread->env, 1);

    }
    else {

        if(!setjmp(env_tmp))
        {
            env_tmp->sp = (unsigned long)current_thread->stack_p;
            longjmp(env_tmp, 1);
        }

        // check handler signo
        if(current_thread->signo!=-1)
        {
            if(current_thread->handler_buf_set==0)
            {
                int no = current_thread->signo;
                if(no == -2)
                {
                    thread_exit();
                }
                else{
                    current_thread->sig_handler[no](no);
                    current_thread->signo = -1;
                }
            }
            else{
                longjmp(current_thread->handler_env, 1);
            }
        }

        current_thread->fp(current_thread->arg);
        thread_exit();
    }
    
}

void schedule(void){
    // TODO
    if(current_thread != NULL) current_thread = current_thread->next;
}

void thread_exit(void){

    if(current_thread->next != current_thread)
    {
        // TODO
        struct thread *temp = current_thread;
        schedule();

        current_thread->previous = temp->previous;
        temp->previous->next = current_thread;
        free(temp->stack);
        free(temp);
        dispatch();
    }
    else{
        // TODO
        // Hint: No more thread to execute
        free(current_thread->stack);
        free(current_thread);
        current_thread = NULL;
        longjmp(env_st, 2);
    }

    return;
}


void thread_start_threading(void){
    // TODO

    if (!setjmp(env_st))
    {
        schedule();
        dispatch();
    }
    
    return;
}

// part 2
void thread_register_handler(int signo, void (*handler)(int)){
    // TODO
    current_thread->sig_handler[signo] = handler;

}
void thread_kill(struct thread *t, int signo){
    // TODO
    if(t->sig_handler[signo] == NULL_FUNC)
    {
        t->signo = -2;
    }else{
        t->signo = signo;
    }
}