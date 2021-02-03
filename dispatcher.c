/*

Dispatch Algorithm : Shortest Remaining Time

Citations:
I used the linked list API, provided in CIS*2750

*/

#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "dispatcher.h"
#include "LinkedListAPI.h"

#define MAX_LINE_LENGTH 100

int p0 = 0;

List* incoming;
List* ready;
List* cpu;
List* completed;
List* hdd;
List* blocked;

/*
Any required standard libraries and your header files here
*/
#include "defs.h"

/* 
this function was created to determine which node is up next to run
in the ready list
*/
Process* nextNode(List* list){

    ListIterator iter = createIterator(list);
    void *elem;
    Process* next;
    int min = 9000;

    while((elem = nextElement(&iter)) != NULL){

        Process* p = (Process*)elem;

        if((p->burst - p->running) < min){
           next = p;
           min = p->burst - p->running; 
        }
        if((p->burst - p->running) == min && p->ready > next->ready){
           next = p;
        }
    }
    return next;
}

void addTime(List *list, queue type){

    ListIterator iter = createIterator(list);
    void *elem;

    while((elem = nextElement(&iter)) != NULL){

        Process *p = (Process*)elem;

        if(type == READY){
            p->ready += 1;
            p->currentBlocked = 0;
        }
        if(type == INCOMING){
            p->arrival -= 1;
        }
        if(type == CPU){
            p->running += 1;
            p->currentBlocked = 0;
        }
        if(type == HDD){
            p->blocked += 1;
            p->currentBlocked += 1;
        }
        if(type == BLOCKED){
            p->blocked2 += 1;
            p->currentBlocked = 0;
        }
    }
}

/* 
this function will move process around each list into their correct new place
after each time interval
*/
void increment(List *list, queue type, int hd){

    ListIterator iter = createIterator(list);
    void *elem;

    label:
    while((elem = nextElement(&iter)) != NULL){

        Process *p = (Process*)elem;

        if(type == HDD){
            if(p->currentBlocked == hd){
                /* remove process from hdd*/
                Process *temp;
                temp = p;

                deleteDataFromList(hdd, p);

                /* if process is completed, send to completed list*/
                if(p->burst == p->running){
                    insertBack(completed, temp);
                }

                else if(getLength(cpu) == 0){
                    insertBack(cpu, temp);
                }
                else{
                    /* figure out whether to move back to cpu, or to ready*/
                    Process *current = getFromFront(cpu);
                    if( (current->burst - current->running) > (temp->burst - temp->running)){
                        Process *temp2;
                        temp2 = current;
                        deleteDataFromList(cpu, current);
                        insertBack(ready, temp2);
                        insertBack(cpu, temp);
                    }
                    else{
                        insertBack(ready, temp);
                    }
                }
            }
        }

        if(type == CPU){
            
            int i;

            /* check if process is due to enter hdd */
            for(i = 0; i < 49; ++i){
                if(p->exchange[i] == p->running){
                    Process* temp;
                    temp = p;
                    deleteDataFromList(cpu, p);
                    if(getLength(hdd) == 0){
                        temp->exchange[i] = -1;
                        insertBack(hdd, temp);
                        goto label;
                    }
                    else{
                        temp->exchange[i] = -1;
                        insertBack(blocked, temp);
                        goto label;
                    }
                }
            }
            /* if process if complete, move to completed list */
            if(p->burst == p->running){
                Process* temp;
                temp = p;
                deleteDataFromList(cpu, p);
                insertBack(completed, temp);
            }
            
        }

        /* check for new proceses coming in*/
        if(type == INCOMING){
            if(p->arrival <= 0){
                /* once process has arrived, move it to ready or CPU */
                Process* temp;
                temp = p;
                deleteDataFromList(incoming, p);

                if(getLength(cpu) == 0  && getLength(ready) != 0 ){
                    Process* next;
                    next = nextNode(ready);
                    if(next->ready > 0 && (next->burst - next->running <= temp->burst - temp->running)){
                        insertBack(ready, temp);
                    }
                    else{
                        insertBack(cpu, temp);
                    }
                }
                else{
                    insertBack(ready, temp);
                }
            }
        }   
        if(type == BLOCKED){
            /* when hdd is empty, add the next element from the blocked list
            using FIFO technique*/
            if(getLength(hdd) == 0){
                Process* temp;
                temp = p;
                deleteDataFromList(blocked, p);
                insertBack(hdd, temp);
            }
        }       
    }
}

void dispatcher(FILE *fd, int harddrive){

    int length = 0;
    int time = 0;
    char line_buffer[MAX_LINE_LENGTH];
    int i = 0;
    char *token, *string;
    incoming = initializeList(print, delete, compare);
    ready = initializeList(print, delete, compare);
    completed = initializeList(print, delete, compare);
    cpu = initializeList(print, delete, compare);
    hdd = initializeList(print, delete, compare);
    blocked = initializeList(print, delete, compare);
    

    // Process simulation input line by line
    while (fgets(line_buffer, MAX_LINE_LENGTH, fd) != NULL && line_buffer[0] != '\n'){

        i = 0;

        Process* process = malloc(sizeof(Process));

        for(i = 0; i < 50; ++i){
            process->exchange[i] = -1;
        }

        i = 0;

        process->arrival = 0;
        process->burst = 0;
        process->pid = 0;
        process->exchanges = 0;
        process->running = 0;
        process->blocked = 0;
        process->blocked2 = 0;
        process->ready = 0;
        process->currentBlocked = 0;

        token = strtok(line_buffer, " ");
        sscanf(token, "%d",&process->arrival);
       
        token = strtok(NULL, " ");
        sscanf(token, "%d",&process->pid);
        
        token = strtok(NULL, " ");
        sscanf(token, "%d",&process->burst);
        
        process->exchanges = 0;
        token = strtok(NULL, " ");

        while ( token != NULL ){
            process->exchanges += sscanf(token, "%d",&process->exchange[i]);
            token = strtok(NULL, " ");
            i++;
        }
        insertBack(incoming, process);
        length ++;
   
    }

    /* loop forever until all nodes are in the completed queue,
    set to high number instead of true, just incase we get an error and
    there is an infinitive loop*/
    while(1000000){

        if(getLength(completed) == length){
            break;
        }
        
        /* move a process from ready into the cpu when it's free */
        if(getLength(cpu) == 0  && getLength(ready) != 0 ){
            Process* next = nextNode(ready);
            Process* temp;
            temp = next;
            deleteDataFromList(ready, next);
            insertBack(cpu, temp);
        }

        /* check if a process in the ready queue can execute faster than the one currently
        running */
        else if(getLength(cpu)!= 0 && getLength(ready) != 0){
            Process* next = nextNode(ready);
            Process* current = getFromFront(cpu);

            if((next->burst - next->running) < (current->burst - current->running)){
                Process* temp;
                Process* temp2;
                temp = next;
                temp2 = current;

                deleteDataFromList(ready, next);
                deleteDataFromList(cpu, current);

                insertBack(cpu, temp);
                insertBack(ready, temp2);
            }
        }

        addTime(cpu, CPU);

        /* p0 runs when cpu is empty*/
        if(getLength(cpu) == 0){
            p0 += 1;
        }

        /* adjust time to each list*/
        addTime(incoming, INCOMING);
        addTime(ready, READY);
        addTime(hdd, HDD);
        addTime(blocked, BLOCKED);

        /* adjust processes from each list*/
        increment(cpu, CPU, harddrive);
        increment(incoming, INCOMING, harddrive);
        increment(ready, READY, harddrive);
        increment(hdd, HDD, harddrive);
        increment(blocked, BLOCKED, harddrive);
        
        time += 1;
        
    }
    printf("0 %d\n", p0);
    string = toString(completed);
    printf("%s", string );

    free(string);
    freeList(incoming);
    freeList(completed);
    freeList(ready);
    freeList(hdd);
    freeList(cpu);
    freeList(blocked);
}

void delete(void *data){

    Process* new;

    new = (Process*)data;

    if(new){
        free(new);
    }
}

char* print(void *data){
    char* string = malloc(1000);
    Process* new;

    new = (Process*)data;

    if(new){
        sprintf(string, "%d %d %d %d\n", new->pid, new->running, new->ready, new->blocked + new->blocked2);
        return string;
    }

    strcpy(string, "");

    return string;
}

int compare(const void *first, const void *second){

    Process* one;
    Process* two;

    if(first == NULL || second == NULL){
        return 1;
    }

    one = (Process*)first;
    two = (Process*)second;

    if(one->pid == two->pid){
        return 0;
    }

    return 1;
}
