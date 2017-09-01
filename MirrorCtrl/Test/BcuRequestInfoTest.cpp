
#include "BcuRequestInfo.h"

struct TestDecreaseArg {
    int id;
    int sleep_t;
    BcuRequestInfo* bcuReqInfo;
};

void* testDecrease(void* arg) {
    // Get arguments
    TestDecreaseArg* myArgs = (TestDecreaseArg*)arg;
    int id = myArgs->id;
    int sleep_t = myArgs->sleep_t;
    BcuRequestInfo* bcuReqInfo = myArgs->bcuReqInfo;
    
    printf("Decreaser thread [%d] started...\n", id);
    
    while(bcuReqInfo->decreaseReplyCounter(id) != 0) {
        printf("[%d] decreasing...\n", id);
        usleep(sleep_t);
    }
    printf("Decreaser thread [%d] terminated\n", id);
    return NULL;
}

void* testWait(void* arg) {
    printf("Thread Waiting started...\n");
    BcuRequestInfo* bcuReqInfo = (BcuRequestInfo*)arg;
    bcuReqInfo->waitReplyReady();
    printf("Reply ready !!!\n");
    return NULL;
}


int main() {

   pthread_t threadWaiting;
   int DECREASER_FROM = 3;
   int DECREASER_TO = 5;
   int REPLY_NUM = 10;

   BcuRequestInfo* bcuReqInfo = new BcuRequestInfo(DECREASER_FROM, DECREASER_TO, REPLY_NUM);
   
   pthread_create(&threadWaiting, NULL, &testWait, (void*)bcuReqInfo);
   
   for(int id=DECREASER_FROM; id<=DECREASER_TO; id++) {
        printf("Starting decreaser thread [%d]...\n", id);
        pthread_t* decreaser = new pthread_t();
        TestDecreaseArg *arg = new TestDecreaseArg();
        arg->id = id;
        arg->sleep_t = 500000*(id+1);
        arg->bcuReqInfo = bcuReqInfo;
        pthread_create(decreaser, NULL, &testDecrease, (void*)(arg));
   }
   
   pthread_join(threadWaiting, NULL);
   
}
