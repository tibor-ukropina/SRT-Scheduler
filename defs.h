typedef enum COMP{
    INCOMING, READY, BLOCKED, CPU, HDD
} queue;

/* process struct*/
typedef struct{
    int arrival;
    int burst;
    int pid;
    int exchanges;
    int exchange[50];
    int running;
    int blocked;
    int blocked2;
    int ready;
    int currentBlocked;

}Process;

/* helper functions */
void delete(void *data);
char* print(void *data);
int compare(const void *first, const void *second);
Process* initialize(Process* p);