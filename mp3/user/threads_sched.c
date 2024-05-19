#include "kernel/types.h"
#include "user/user.h"
#include "user/list.h"
#include "user/threads.h"
#include "user/threads_sched.h"

#define NULL 0

/* default scheduling algorithm */
struct threads_sched_result schedule_default(struct threads_sched_args args)
{
    struct thread *thread_with_smallest_id = NULL;
    struct thread *th = NULL;
    list_for_each_entry(th, args.run_queue, thread_list) {
        if (thread_with_smallest_id == NULL || th->ID < thread_with_smallest_id->ID) {
            thread_with_smallest_id = th;
        }
    }

    struct threads_sched_result r;
    if (thread_with_smallest_id != NULL) {
        r.scheduled_thread_list_member = &thread_with_smallest_id->thread_list;
        r.allocated_time = thread_with_smallest_id->remaining_time;
    } else {
        r.scheduled_thread_list_member = args.run_queue;
        r.allocated_time = 1;
    }

    return r;
}

/* Earliest-Deadline-First scheduling */
struct threads_sched_result schedule_edf(struct threads_sched_args args)
{
    struct thread *EDF_th = NULL;
    struct thread *th = NULL;
    struct release_queue_entry *rth = NULL;
    int allocate = 0;

    //printf("current time: %d\n", args.current_time);

    list_for_each_entry(th, args.run_queue, thread_list) {
        // printf("thread id: %d\n", th->ID);
        // printf("thread current deadline: %d\n", th->current_deadline);
        // printf("thread remaining time: %d\n", th->remaining_time);
        
        if(EDF_th == NULL || th->current_deadline <= EDF_th->current_deadline)
        {
            if(th->current_deadline == EDF_th->current_deadline)
            {
                if(th->ID < EDF_th->ID)
                {
                    EDF_th = th;
                }
            }else{
               EDF_th = th;
            }
            
        }
    }

    // find allocation time
    int upcoming = 10000;
    struct release_queue_entry *uth = NULL;

    list_for_each_entry(rth, args.release_queue, thread_list) {
        // printf("wait thread id: %d\n", rth->thrd->ID);
        // printf("current deadline: %d\n", rth->thrd->current_deadline);
        // printf("period: %d\n", rth->thrd->period);
        
        int next_deadline = rth->thrd->current_deadline + rth->thrd->period;
        //printf("next deadline: %d\n", next_deadline);

        if(next_deadline < upcoming)
        {
            upcoming = next_deadline;
            uth = rth;
        }

    }

    //printf("upcoming: %d\n", upcoming);


    if(uth != NULL && upcoming < EDF_th->current_deadline)
    {
        allocate = uth->thrd->current_deadline - args.current_time;
    }else{
        allocate = EDF_th->remaining_time;
    }

    //printf("allocate: %d\n", allocate);

    if(EDF_th->remaining_time > EDF_th->current_deadline - args.current_time)
    {
        //allocate = EDF_th->current_deadline - args.current_time;
        //printf("miss allocate: %d\n", allocate);

                // check missing deadline with smalliest id
        list_for_each_entry(th, args.run_queue, thread_list) 
        {
            // printf("thread id: %d\n", th->ID);
            // printf("thread current deadline: %d\n", th->current_deadline);
            // printf("thread remaining time: %d\n", th->remaining_time);
            
            if(th->remaining_time > th->current_deadline - args.current_time)
            {
                if(th->ID <  EDF_th->ID)
                {
                    EDF_th = th;
                }

                
            }
        }

        allocate = EDF_th->current_deadline - args.current_time;
        //printf("miss allocate: %d\n", allocate);
        if(allocate < 0)
        {
            allocate = 0;
            //printf("miss deadline: %d\n", allocate);
        }
    }


    int p = 10000;
    if(EDF_th == NULL)
    {
        // printf("running queue empty\n");
        // printf("current time: %d\n", args.current_time);
        list_for_each_entry(rth, args.release_queue, thread_list) {
        // printf("wait thread id: %d\n", rth->thrd->ID);
        // printf("current deadline: %d\n", rth->thrd->current_deadline);
        // printf("period: %d\n", rth->thrd->period);
        
        int next_period = rth->thrd->current_deadline;
        
        if(next_period < p)
        {
            p = next_period;
        }

        }
        // printf("miss allocate: %d\n", p);
        // printf("allocate: %d\n",p-args.current_time);
    }


    struct threads_sched_result r;
    if (EDF_th != NULL) {
        r.scheduled_thread_list_member = &EDF_th->thread_list;
        r.allocated_time = allocate;
    } else {
        // run queue is empty
        r.scheduled_thread_list_member = args.run_queue;
        r.allocated_time = p-args.current_time;
    }

    return r;

    //return schedule_default(args);
}

/* Rate-Monotonic Scheduling */
struct threads_sched_result schedule_rm(struct threads_sched_args args)
{
    struct thread *rate_th = NULL;
    struct thread *th = NULL;
    struct release_queue_entry *rth = NULL;

    //printf("current time: %d\n", args.current_time);

    list_for_each_entry(th, args.run_queue, thread_list) {
        // printf("thread id: %d\n", th->ID);
        // printf("thread current deadline: %d\n", th->current_deadline);
        // printf("thread remaining time: %d\n", th->remaining_time);
        
        if(rate_th == NULL || th->period <= rate_th->period)
        {
            if(th->period == rate_th->period)
            {
                if(th->ID < rate_th->ID)
                {
                    rate_th = th;
                }
            }else{
               rate_th = th;
            }
            
        }
    }


    // find allocation time
    int next_deadline = 10000;
    int need_check = args.current_time + rate_th->remaining_time;
    int allocate = 0;
    struct release_queue_entry *uth = NULL;

    //printf("need_check: %d\n", need_check);

    list_for_each_entry(rth, args.release_queue, thread_list) {
        // printf("wait thread id: %d\n", rth->thrd->ID);
        // printf("current deadline: %d\n", rth->thrd->current_deadline);
        // printf("period: %d\n", rth->thrd->period);
        
        if(rth->thrd->current_deadline > need_check) continue;

        if(rth->thrd->period < rate_th->period)
        {
            if(rth->thrd->current_deadline < next_deadline)
            {
                next_deadline = rth->thrd->current_deadline;
                uth = rth;
            }
        }else if(rth->thrd->period == rate_th->period  && rth->thrd->ID < rate_th->ID)
        {
            if(rth->thrd->current_deadline < next_deadline)
            {
                next_deadline = rth->thrd->current_deadline;
                uth = rth;
            }

        }
        //printf("next deadline: %d\n", next_deadline);

    }
    
    if(uth != NULL)
    {
        allocate = next_deadline - args.current_time;
    }else{
        allocate = rate_th->remaining_time;
    }

    //printf("check allocate:%d\n", allocate);


    if(rate_th->remaining_time > rate_th->current_deadline - args.current_time)
    {

        // check missing deadline with smalliest id
        list_for_each_entry(th, args.run_queue, thread_list) 
        {
            // printf("thread id: %d\n", th->ID);
            // printf("thread current deadline: %d\n", th->current_deadline);
            // printf("thread remaining time: %d\n", th->remaining_time);
            
            if(th->remaining_time > th->current_deadline - args.current_time)
            {
                if(th->ID <  rate_th->ID)
                {
                    rate_th = th;
                }

                
            }
        }

        allocate = rate_th->current_deadline - args.current_time;
        //printf("miss allocate: %d\n", allocate);
        if(allocate < 0)
        {
            allocate = 0;
            //printf("miss deadline: %d\n", allocate);
        }
    }




    // no running queue thread
    int p = 10000;
    if(rate_th == NULL)
    {
        // printf("running queue empty\n");
        // printf("current time: %d\n", args.current_time);
        list_for_each_entry(rth, args.release_queue, thread_list) {
        // printf("wait thread id: %d\n", rth->thrd->ID);
        // printf("current deadline: %d\n", rth->thrd->current_deadline);
        // printf("period: %d\n", rth->thrd->period);
        
        int next_period = rth->thrd->current_deadline;
        
        if(next_period < p)
        {
            p = next_period;
        }

        }
        // printf("miss allocate: %d\n", p);
        // printf("allocate: %d\n",p-args.current_time);
    }


    struct threads_sched_result r;
    if (rate_th != NULL) {
        r.scheduled_thread_list_member = &rate_th->thread_list;
        r.allocated_time = allocate;
    } else {
        // run queue is empty
        r.scheduled_thread_list_member = args.run_queue;
        r.allocated_time = p-args.current_time;
    }

    return r;
    //return schedule_default(args);
}
