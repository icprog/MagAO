
#include "logtest.h"
#include "Logger.h"
using namespace Arcetri;

extern "C" {
#include "base/thrdlib.h"
}


#define LINESTODO (500000)
#define DELAY 

logtest::logtest( int argc, char *argv[]) : AOApp(argc, argv) {

    log1 = Logger::get("LOG1", Logger::LOG_LEV_INFO);
    log2 = Logger::get("LOG2", Logger::LOG_LEV_INFO);
    log3 = Logger::get("LOG3", Logger::LOG_LEV_INFO);
    log4 = Logger::get("LOG4", Logger::LOG_LEV_INFO, "EXTRA");
    log5 = Logger::get("LOG5", Logger::LOG_LEV_INFO, "EXTRA");
    log6 = Logger::get("TELEMETRY", Logger::LOG_LEV_INFO, "TELEMETRY");
    log7 = Logger::get("TELEMETRY", Logger::LOG_LEV_INFO, "TELEMETRY_EXTRA");
}

void* logtest::th1( void *arg) {
    logtest *test = (logtest *)arg;
    int counter=0;
    while (counter++ < LINESTODO) {
	test->log1->log( Logger::LOG_LEV_INFO, "Log1 line %d", counter);
	DELAY
	    }      
    return NULL;
}

void* logtest::th2( void *arg) {
    logtest *test = (logtest *)arg;
    int counter=0;
    while (counter++ < LINESTODO) {
	test->log2->log( Logger::LOG_LEV_INFO, "Log2 line %d", counter);
	DELAY
	    }      
    return NULL;
}

void* logtest::th3( void *arg) {
    logtest *test = (logtest *)arg;
    int counter=0;
    while (counter++ < LINESTODO) {
	test->log3->log( Logger::LOG_LEV_INFO, "Log3 line %d", counter);
	DELAY
	    }      
    return NULL;
}

void* logtest::th4( void *arg) {
    logtest *test = (logtest *)arg;
    int counter=0;
    while (counter++ < LINESTODO) {
	test->log4->log( Logger::LOG_LEV_INFO, "Log4 line %d", counter);
	DELAY
	    }      
    return NULL;
}

void* logtest::th5( void *arg) {
    logtest *test = (logtest *)arg;
    int counter=0;
    while (counter++ < LINESTODO) {
	test->log5->log( Logger::LOG_LEV_INFO, "Log5 line %d", counter);
	DELAY
    }      
    return NULL;
}

//
// telemetry data log
void* logtest::th6( void *arg) {
    const int vector_size = 100;

    logtest *test = (logtest *)arg;
    int counter=0;

    double test_array[vector_size];
    vector<double> test_vector(vector_size);

    for (int i = 0; i<vector_size; i++) {
	test_vector[i] = test_array[i] = (11.11 * i);
    }

    // test vector, log on two different files
    while (counter++ < 100) {
	test->log6->log( Logger::LOG_LEV_INFO, "Log6 line %d", counter);
	test->log6->log_telemetry(Logger::LOG_LEV_INFO, test_array, vector_size) ;
	test->log7->log_telemetry(Logger::LOG_LEV_INFO, test_array, vector_size) ;
    }      

    // try vector version
    test->log6->log_telemetry(Logger::LOG_LEV_INFO, test_vector);

    // try to log telemetry on a logger without setting TELEMETRY (default to [parent]_TELEMETRY)
    for (counter = 0; counter < 20; counter++) {
	test->log1->log( Logger::LOG_LEV_INFO, "logging telemetry");
	test->log1->log_telemetry(Logger::LOG_LEV_INFO, test_vector);
    }

    
    

    return NULL;
}



void logtest::Run() {


    //
    // 
    Logger * log = Logger::get();

    log->log(Logger::LOG_LEV_INFO, "Starting 8x threads logging test");

    pthread_attr_init(&attr1);
    pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_JOINABLE); 
    pthread_create(&(thread1),&attr1,&th1, (void *)this);

    pthread_attr_init(&attr2);
    pthread_attr_setdetachstate(&attr2, PTHREAD_CREATE_JOINABLE); 
    pthread_create(&(thread2),&attr2,&th2, (void *)this);

    pthread_attr_init(&attr3);
    pthread_attr_setdetachstate(&attr3, PTHREAD_CREATE_JOINABLE); 
    pthread_create(&(thread3),&attr3,&th3, (void *)this);

    pthread_attr_init(&attr4);
    pthread_attr_setdetachstate(&attr4, PTHREAD_CREATE_JOINABLE); 
    pthread_create(&(thread4),&attr4,&th4, (void *)this);

    pthread_attr_init(&attr5);
    pthread_attr_setdetachstate(&attr5, PTHREAD_CREATE_JOINABLE); 
    pthread_create(&(thread5),&attr5,&th5, (void *)this);

    pthread_attr_init(&attr6);
    pthread_attr_setdetachstate(&attr6, PTHREAD_CREATE_JOINABLE); 
    pthread_create(&(thread6),&attr6,&th6, (void *)this);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    pthread_join(thread4, NULL);
    pthread_join(thread5, NULL);
    pthread_join(thread6, NULL);

    log->log(Logger::LOG_LEV_INFO, "End of threads logging test");

    // test rename
    log4->log(Logger::LOG_LEV_INFO, "Testing rename");
    log4->rename("RENAMED","/tmp");
    log4->log(Logger::LOG_LEV_INFO, "Rename done");
    // log 5 should have changed as well
    log5->log(Logger::LOG_LEV_INFO, "Rename done here too");
   
    // rename back
    log4->rename("RENAMED",Utils::getAdoptLog());
    log4->log(Logger::LOG_LEV_INFO, "Rename done");

    // change file
    log4->change("CHANGED");
    log4->log(Logger::LOG_LEV_INFO, "Change done");
   
    // generate rename error
    string badname = "bad\\/\\name";
    log4->rename(badname,Utils::getAdoptLog());
    log4->log(Logger::LOG_LEV_INFO, "Tried to rename to %s", badname.c_str());


    // loggers status
    log->printPoolStatus();
}


int main( int argc, char *argv[]) {

    logtest *test = NULL;

    try {
	test = new logtest( argc, argv);

	test->Exec();

	delete test;
    }
    catch (LoggerFatalException &e) {
	// In this case the logger can't log!!!
	printf("%s\n", e.what().c_str());
    }
    catch (AOException &e) {
	Logger::get()->log(Logger::LOG_LEV_FATAL, "Error: %s", e.what().c_str());
    }

    return 0;
}
  

