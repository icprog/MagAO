#include "DiagnSet.h"
#include "DiagnVar.h"
#include <stdio.h>
#include <iostream>
#include <time.h>
#include "Timing.h"


//@Function: save_filtervector
//
//@
#include <fstream>
#include <boost/archive/binary_oarchive.hpp> //serialize
void save_filtervector(const DiagnSet &s, const char * filename){
    // make an archive
    std::ofstream ofs(filename);
    boost::archive::binary_oarchive oa(ofs);
    oa << s;
}

//@Function: randu
//
//Return an uniform variate in the range [min,max]
//@
static double randu(double min, double max){
    return ((double)rand()/RAND_MAX) * (max-min) + min;
}

//@Function: log_time
//
//@
#include "time.h"
#include <sstream>
#include <iomanip>
struct timespec tp;
std::string log_time() // link -lrt
{ 
    std::ostringstream oss;
    clock_gettime(CLOCK_REALTIME,&tp);
    oss << tp.tv_sec % 100 << "." << std::setw(6) << tp.tv_nsec/1e3 << " - " ;
    return oss.str();
}


///////////////////////////////////////////////////////////////////////


#include <sys/resource.h>  // set/getpriority

pthread_cond_t cond  =  PTHREAD_COND_INITIALIZER;
pthread_mutex_t mut  =  PTHREAD_MUTEX_INITIALIZER;
bool raw_ready = false;
int  verbose=0;
double               valueA=0.0;
double               valueB=0.0;


DiagnSetPtr          slowvars;

class FunctPippo : public Funct {
    public:
        FunctPippo(double *x) : x_(x)
        {
            if(DEBUG) std::cout << "--- FunctPippo constructor(float32*)" << std::endl; 
        }
        FunctPippo(const FunctPippo&):Funct()
        {
            if(DEBUG) std::cout << "--- FunctPippo copy constructor" << std::endl; 
        }
        virtual void doit(double *v) 
        {
           // std::cout << "FunctPippo address: " <<  x_ << std::endl;
            for (int i=0; i< (*x_); i++) *v = *v + i; 
            *v = (double)(*x_)*2;
        }
        virtual void doit(const std::string& , double ) {}; 
    private:
        double  *x_;
};

void * UpdateThread(void*)
{
    int ret;
    int nice=20;
    double timestamp = 1.23;
    if ((ret=setpriority(PRIO_PROCESS, 0, nice)) != 0){
        std::cerr << "Error setting priority "<< nice << " error " << ret << std::endl;
    }

    std::cerr <<"getpolicy " << sched_getscheduler(0) << " getpriority " << getpriority(PRIO_PROCESS, 0) << std::endl;
        
    while(1){
        pthread_mutex_lock(&mut);
        while (raw_ready != true){
            if (verbose > 1) std::cout << log_time() << "cond_wait"<< std::endl;
            pthread_cond_wait(&cond, &mut);
        }
        //copia
        if (verbose > 1) std::cout << log_time() << "mutex_unlock"<< std::endl;
        pthread_mutex_unlock(&mut);
       
        // update
        for (DiagnSet::iterator it = slowvars->begin(); it != slowvars->end();  it++)
        {
            DiagnVar& v = **it;
            v.Update(timestamp);
            if (verbose > 0 && it == slowvars->begin() &&  (v.GetCounter() % 10 == 0) ) std::cout << log_time() << v << std::endl; 
        }
    }
    return 0;
}

int DiagnSet::_ctr=0;
//int DiagnVect::_ctr=0;

int main(int argc, char** argv)
{
    std::string          groupname;
    FunctPtr             hAlarm   (new FunctAlarm());                   
    FunctPtr             hWarning (new FunctWarning());
    int                  cycles=100;
    Timing               ta;
    DiagnSet             vars;


    for (int i=1; i<argc; i++){
        if (strcmp(argv[i], "-c") == 0) {cycles=atoi(argv[++i]); continue;}
        if (strcmp(argv[i], "-v") == 0) {verbose=atoi(argv[++i]); continue;}
    }


    DiagnParam diagnparam = DiagnParam();
    diagnparam.SetEnabled(true);
    diagnparam.SetWarningMax(1e10);
    diagnparam.SetAlarmMax(1e10);
    /////////////// Group1 ///////////////////
    groupname = "A";
    for (int i=0; i<10; i++)
    {
        std::cout << "Creating Var " << groupname << "-" << i << std::endl;
        FunctPtr fConv (new FunctPippo(&valueA));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        diagnparam.SetMeanPeriod(i);
        theVar->ImportParams(diagnparam);
        //if (i%2==1) theVar->SetEnabled(false); TODO Use ImportParams
        vars.Add(theVar);
    } 

    /////////////// Group2 popolato in ordine inverso ///////////////////
    groupname = "B";
    for (int i=9; i>=0; i--)
    {
        std::cout << "Creating Var " << groupname << "-" << i << std::endl;
        FunctPtr fConv (new FunctPippo(&valueB));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        diagnparam.SetMeanPeriod(i);
        theVar->ImportParams(diagnparam);
        vars.Add(theVar);
    } 

    
    // Test della funzione Filter
    DiagnSetPtr selez = vars.Filter(DiagnWhich());
    std::sort(selez->begin(), selez->end(), less<DiagnVarPtr>());
    std::cout << "Tutte Variabili: " << std::endl;
    for (DiagnSet::iterator it = selez->begin(); it != selez->end();  it++)
    {
        DiagnVar& a = **it;
        std::cout << a << " " << std::endl;
    }
    std::cout << std::endl;

    selez = vars.Filter(DiagnWhich("A"));
    std::cout << "Tutte Variabili A: " << std::endl;
    for (DiagnSet::iterator it = selez->begin(); it != selez->end();  it++)
    {
        DiagnVar& a = **it;
        std::cout << a << " " << std::endl;
    }
    std::cout << std::endl;
    DiagnSetPtr filtroA = selez;

    selez = vars.Filter(DiagnWhich("*",3,5));
    std::cout << "Tutte Variabili da 3 a 5: " << std::endl;
    for (DiagnSet::iterator it = selez->begin(); it != selez->end();  it++)
    {
        DiagnVar& a = **it;
        std::cout << a << " " << std::endl;
    }
    std::cout << std::endl;
    DiagnSetPtr filtro35 = selez;

    DiagnSetPtr filtroA35 = filtroA & filtro35;
    std::cout << "Variabili A & Variabili da 3 a 5: " << std::endl;
    for (DiagnSet::iterator it = filtroA35->begin(); it != filtroA35->end();  it++)
    {
        DiagnVar& a = **it;
        std::cout << a << " " << std::endl;
    }
    std::cout << std::endl;

    DiagnSetPtr filtroAo35 = filtroA | filtro35;
    std::cout << "Variabili A | Variabili da 3 a 5: " << std::endl;
    for (DiagnSet::iterator it = filtroAo35->begin(); it != filtroAo35->end();  it++)
    {
        DiagnVar& a = **it;
        std::cout << a << " " << std::endl;
    }
    std::cout << std::endl;

    DiagnSetPtr filtroB = vars.Filter(DiagnWhich("B",DiagnWhich::All,DiagnWhich::All));
    DiagnSetPtr filtroAe35oB = (filtroA & filtro35) | filtroB;
    std::cout << "(Variabili A & Variabili da 3 a 5) | Variabili B : " << std::endl;
    for (DiagnSet::iterator it = filtroAe35oB->begin(); it != filtroAe35oB->end();  it++)
    {
        DiagnVar& a = **it;
        std::cout << a << " " << std::endl;
    }
    std::cout << std::endl;

    DiagnSetPtr filtroAeC = filtroA & vars.Filter(DiagnWhich("C")); ;
    std::cout << "Variabili A & Variabili C (nullo) : " << std::endl;
    for (DiagnSet::iterator it = filtroAeC->begin(); it != filtroAeC->end();  it++)
    {
        DiagnVar& a = **it;
        std::cout << a << " " << std::endl;
    }
    std::cout << std::endl;

    selez = vars.Filter(DiagnWhich("A",DiagnWhich::All,DiagnWhich::All,DiagnWhich::Slow,DiagnWhich::Enabled));
    std::cout << "Tutte Variabili A, Ena e Slow: " << std::endl;
    for (DiagnSet::iterator it = selez->begin(); it != selez->end();  it++)
    {
        DiagnVar& a = **it;
        std::cout << a << " " << std::endl;
    }
    std::cout << std::endl;

    // Crea array slowvars
    slowvars    = vars.Filter(DiagnWhich("*",DiagnWhich::All,DiagnWhich::All,DiagnWhich::Slow,DiagnWhich::Enabled));
    std::cout << "Tutte Variabili Ena Slow: " << std::endl;
    for (DiagnSet::iterator it = slowvars->begin(); it != slowvars->end();  it++)
    {
        DiagnVar& a = **it;
        std::cout << a << " " << std::endl;
    }
    std::cout << std::endl;

    // Array fastvars
    DiagnSetPtr fastvars = vars.Filter(DiagnWhich("*",DiagnWhich::All,DiagnWhich::All,DiagnWhich::Fast,DiagnWhich::Enabled));
    std::cout << "Tutte Variabili Ena Fast: " << std::endl;
    for (DiagnSet::iterator it = fastvars->begin(); it != fastvars->end();  it++)
    {
        DiagnVar& a = **it;
        std::cout << a << " " << std::endl;
    }
    std::cout << std::endl;


    // Thredda
    int ret;
    pthread_attr_t attribute;
    pthread_t thread_id;
    pthread_attr_init(&attribute);      // Prepare thread attribute
    pthread_attr_setdetachstate( &attribute, PTHREAD_CREATE_DETACHED);
    if ((ret = pthread_create( &thread_id, &attribute, UpdateThread, NULL)) != 0)
        perror("Error spawning thread"); //TODO throw exception

    
    // Set priority    
    struct sched_param param;
    param.sched_priority=1;
    if ((ret=sched_setscheduler(0, SCHED_RR, &param)) != 0){ 
        std::cerr << "thMAIN: Error setting policy - error " << ret   << std::endl;
    }
    std::cerr << "thMAIN:"<< " getpolicy " << sched_getscheduler(0) << " getpriority " << getpriority(PRIO_PROCESS, 0) << std::endl;


    ta.start();
    double timestamp=1.23;
    for (int i=0; i<cycles; i++)
    {
        pthread_mutex_lock(&mut);
        raw_ready=false;
        valueA=randu(1e4, 1.01e4);
        valueB=valueA*1e2;
        raw_ready=true;
        if (verbose > 2) std::cout << log_time() << "thMAIN:  cond_broadcast " << valueA << std::endl;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mut);
        
        for (DiagnSet::iterator it = fastvars->begin(); it != fastvars->end();  it++)
        {
            DiagnVar& v = **it;
            v.Update(timestamp);
            if (verbose > 0 && it == fastvars->begin() &&  (v.GetCounter() % 100 == 0) ) std::cout << log_time() << v << std::endl; 
        }
        usleep(1000); // yield processor: we're RealTime and we need other thread to be scheduled sometimes.
    }
    ta.stop();
    std::cout << "total time "<<ta.sec() << std::endl;
    
    try{
    DiagnVarPtr a0 = vars.Find(MatchDiagnVar("A", 0)); 
    std::cout << *a0 << std::endl;
    DiagnVarPtr b0 = vars.Find(MatchDiagnVar("B", 0)); 
    std::cout << *b0 << std::endl;
    DiagnVarPtr b100 = vars.Find(MatchDiagnVar("B", 100)); 
    std::cout << *b100 << std::endl;
    } catch (...) {}

    
    save_filtervector(vars, "Test_DiagnSet.txt");
    return 0;
}
