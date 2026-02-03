#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

Process MinProc(Process x, Process y);
void SortProcess(Process* p, int num);
int GetPeriod();

void outprint(int time_x, int time_y, int pid, int arrival_time, int remaining_time);


static int highest_nonempty(LinkedQueue **Q, int queue_num)
{
    // Start from highest priority: Q[queue_num-1]
    for (int i = queue_num - 1; i >= 0; i--) {
        if (!IsEmptyQueue(Q[i])) {
            return i;  // Return the highest-priority non-empty queue
        }
    }
    return -1; // No ready process
}

static int find_proc_index(Process* proc, int proc_num, int pid) {//pid>>process id
    for (int i = 0; i < proc_num; ++i){
        if (proc[i].process_id == pid){
            return i;
        }
    }
    return -1;
}

// Implement by students
void scheduler(Process* proc, LinkedQueue** ProcessQueue, int proc_num, int queue_num, int period){
    printf("Process number: %d\n", proc_num);
    for (int i = 0;i < proc_num; i++)
        printf("%d %d %d\n", proc[i].process_id, proc[i].arrival_time, proc[i].execution_time);

    printf("\nQueue number: %d\n", queue_num);
    printf("Period: %d\n", period);
    for (int i = 0;i < queue_num; i++){
        printf("%d %d %d\n", i, ProcessQueue[i]->time_slice, ProcessQueue[i]->allotment_time);
    }

    /*
       an example of outprint function,
       it will output "Time_slot:1-2, pid:3, arrival-time:4, remaining_time:5" to output.log file.
    */

    int *remaining  = (int *)malloc(proc_num * sizeof(int));  // reamining time exetracted from proc
    int *cur_queue  = (int *)calloc(proc_num, sizeof(int));   // current queue index
    int *used_slice = (int *)calloc(proc_num, sizeof(int));   // slices used in current queue
    int *has_arrived = (int *)calloc(proc_num, sizeof(int));  // boolean whether process has arrived 

    for (int i = 0; i < proc_num; ++i){         //extraction
        remaining[i] = proc[i].execution_time;
        //printf("Process %d remaining time: %d\n", proc[i].process_id, remaining[i]);
    }
    int current_time = 0;
    int finished = 0;
    int S = GetPeriod();
    int Pflag = 0;
    //printf("\nScheduling Start!\n");

    while (finished < proc_num) {
        
        //loop through all processes to check for arrivals
        Process temp_arrivals[128];         
        int arrival_cnt = 0;
        int top_q = queue_num - 1;

        if(Pflag == 0){
        for (int i = 0; i < proc_num; ++i) {    //put in temp array, sort by pid before enqueue
            if (!has_arrived[i] && proc[i].arrival_time <= current_time) {
                has_arrived[i] = 1;
                temp_arrivals[arrival_cnt] = proc[i];
                temp_arrivals[arrival_cnt].execution_time = remaining[i];
                ++arrival_cnt;
            }
        }

        //more than one arrival at the same time
        if (arrival_cnt >= 2) {
            SortProcess(temp_arrivals, arrival_cnt); //sort by then by pid
        }

        
        for (int i = 0; i < arrival_cnt; i++) {
            ProcessQueue[top_q] = EnQueue(ProcessQueue[top_q], temp_arrivals[i]);
            //printf("Time %d: Process %d arrived, enqueued to Q%d\n", current_time, temp_arrivals[i].process_id, top_q);
        }

        }
        else{
            Pflag = 0;
        }

        //check for highest priority non-empty queue
        int qid = highest_nonempty(ProcessQueue, queue_num); //get highest non-empty queue id
        if (qid == -1) {
            ++current_time;
            continue;  // CPU idle: no process in queue >> skip to next while iteration
        }

        Process running = DeQueue(ProcessQueue[qid]); //dequeue process from selected queue
        int proc_idx = find_proc_index(proc, proc_num, running.process_id); //get process index in proc array
        int slice = ProcessQueue[qid]->time_slice;  //get time slice of selected queue


        /*int next_arrival = -1;
        for (int i = 0; i < proc_num; ++i) {
            if (!has_arrived[i] && proc[i].arrival_time > current_time) { //check not yet arrived 
                if (next_arrival == -1 || proc[i].arrival_time < next_arrival) {
                    next_arrival = proc[i].arrival_time;
                }
            }    
        }*/
        int max_run = (remaining[proc_idx] < slice) ? remaining[proc_idx] : slice;
        int planned_end = current_time + max_run;
        int run_time;
        int Sflag = 0;
        //printf("Q%d, Q%d!!\n", qid, top_q); 


        /*if (next_arrival != -1 && next_arrival < current_time + max_run && qid < top_q) { //1. high prio arrival, no interrupt if both at top_q
            run_time = next_arrival - current_time;  //if exist next arrival AND arrived before max_run time, run until arrival
            printf("checked1\n");
        }
        else 
        if(ProcessQueue[qid]->allotment_time > 0 && used_slice[proc_idx] + max_run > ProcessQueue[qid]->allotment_time) {
            run_time = ProcessQueue[qid]->allotment_time - used_slice[proc_idx]; //2. if allotment time exceeded, run until used up
            printf("checked2\n");
        }
        else */
        if(planned_end > S) {//3. period boost
            run_time = S - current_time;  //if max_run exceed period, run until period end
            Sflag = 1;
            S += GetPeriod();               //extend period
            //printf("checked3\n");
        }
        else {//4. normal case
            run_time = max_run;  //run for max_run time
            //printf("checked4\n");
        }

        

        int start_t = current_time;
        int end_t = current_time + run_time;
        
        remaining[proc_idx] -= run_time;  //decrease remaining time
        used_slice[proc_idx] += run_time;  //count alloted time used in current queue
        running.execution_time = remaining[proc_idx];  // Sync

        //printf("Time %d to %d: Process %d running from Q%d for %d units, remaining %d\n", start_t, end_t, running.process_id, qid, run_time , remaining[proc_idx]);

        //output log
        outprint(current_time, current_time + run_time, running.process_id, proc[proc_idx].arrival_time, remaining[proc_idx]);

        //time elapse
        current_time = end_t;  //advance current time

        if (Sflag == 1) { //period boost 
            Process *temp = NULL;
            int count = 0;

            for (int q = 0; q < queue_num - 1; ++q) {
                while (!IsEmptyQueue(ProcessQueue[q])) { //all q: deq until empty
                    Process p = DeQueue(ProcessQueue[q]);
                    int idx = find_proc_index(proc, proc_num, p.process_id); //get index to reset used_slice
                    used_slice[idx] = 0;
                    temp = realloc(temp, (count+1)*sizeof(Process));
                    temp[count++] = p; //store in temp
                }   
            }

            used_slice[proc_idx] = 0;
            running.execution_time = remaining[proc_idx]; //Add the interrupted job at S
            temp = realloc(temp, (count+1)*sizeof(Process));
            temp[count++] = running;

            for (int i = 0; i < proc_num; ++i) {    //put in temp array, sort by pid before enqueue
                if (!has_arrived[i] && proc[i].arrival_time <= current_time) {
                    has_arrived[i] = 1;
                    Process pending = proc[i];
                    temp = realloc(temp, (count+1)*sizeof(Process));
                    temp[count++] = pending;
                    Pflag = 1;
                }
            }

            /*printf("=== BOOST: Before sort (count=%d) ===\n", count);
                for (int i = 0; i < count; i++){
                    printf("  [%d] pid=%d\n", i, temp[i].process_id);
                }*/

            if (count > 1) {
                for (int i = 0; i < count - 1; ++i) {
                    for (int j = 0; j < count - 1; ++j) {
                        if (temp[j].process_id > temp[j + 1].process_id) {
                            printf("%d > %d, swapped\n",temp[j].process_id, temp[j + 1].process_id);
                            Process swap = temp[j];
                            temp[j] = temp[j + 1];
                            temp[j + 1] = swap;
                        }
                    }

                }
            }
            
            /*printf("=== After sort ===\n");
            for (int i = 0; i < count; i++){
            printf("  [%d] pid=%d\n", i, temp[i].process_id);}*/
            
            for (int i = 0; i < count; i++) {  //re-enqueue sorted processes
                ProcessQueue[top_q] = EnQueue(ProcessQueue[top_q], temp[i]); //enqueue to highest priority queue
            }

 
        }
        
        if (remaining[proc_idx] == 0) {
            finished++; //while loop termination condition
            //printf("FINNISHH!\n");
        }
        else if(Sflag == 1){
            Sflag = 0;//reset
        }
        else { //not finished: to see if demotion needed
            int next_q = qid;
            int allot = ProcessQueue[qid]->allotment_time;  //get allotment time of current queue
            if (allot > 0 && used_slice[proc_idx] >= allot) {
                next_q = qid - 1;
                if (next_q < 0) next_q = 0;
                used_slice[proc_idx] = 0;
            }
            running.execution_time = remaining[proc_idx];
            ProcessQueue[next_q] = EnQueue(ProcessQueue[next_q], running);
        }

    }

    free(remaining);
    free(has_arrived);
    free(used_slice);
    free(cur_queue);
}
