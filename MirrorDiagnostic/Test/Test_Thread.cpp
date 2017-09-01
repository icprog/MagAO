#include <iostream>
#include <errno.h>
#include <pthread.h>
#include "Timing.h"
#include <sys/time.h>
#include <sys/resource.h>  // set/getpriority

pthread_cond_t cond  =  PTHREAD_COND_INITIALIZER;
pthread_mutex_t mut  =  PTHREAD_MUTEX_INITIALIZER;
int  verbose=0;
int  glob_ppp = 0;

#include "time.h"
#include <sstream>
#include <iomanip>
struct timespec tp;
std::string log_time()
{ 
    std::ostringstream oss;
    clock_gettime(CLOCK_REALTIME,&tp);
    oss << std::setw(7) << tp.tv_nsec/1e6 << " - " ;
    return oss.str();
}

class A
{
    public:
        A(std::string name, bool tredda=false, int row=672, int column=672, int nice=0) :
            _name(name), 
            _n_row(row),
            _n_column(column),
            _tredda(tredda),
            _nice(nice)
        {
            _dato=0;
            _ctr=0;
            _tot_time=0;
            _tg_ave=0.0;
            _tg_max=0.0;
            _tg_min=10000000.0;
            _r    = (float*)malloc(sizeof(*_r) * _n_row );
            _matr = (float*)malloc(sizeof(*_matr) * _n_column * _n_row  );
            _vect = (float*)malloc(sizeof(*_vect) * _n_column);
            for (int i=0; i < _n_column * _n_row; i++)
                _matr[i] = i/1000.0;
            for (int i=0; i < _n_column; i++)
                _vect[i] = i/1000.0;
        }
        

        void spawn()
        {
            int ret;
            pthread_attr_t attribute;
            pthread_t thread_id;
            pthread_attr_init(&attribute);      // Prepare thread attribute

            // Declare thread "detached" so that it dies at end
            pthread_attr_setdetachstate( &attribute, PTHREAD_CREATE_DETACHED);
            if ((ret = pthread_create( &thread_id, &attribute, A::UpdateThread, this)) != 0)
                perror("Error spawning thread"); //TODO throw exception
        }
        long Ctr() {return _ctr;}
        void ComputeMatrix()
        {
            memset(_r,0,_n_row*sizeof(*_r));
            //for (int i=0; i<_n_row; i++)
            //    for (int j=0; j<_n_column; j++)
            //        _r[i] += _matr[i*_n_column+j] * _vect[j];

            float *a=_matr;
            float *b=_vect;
            float *b_end = b + _n_column; 
            float *c=_r;
            float *c_end = c + _n_row; 
            for ( ; c < c_end; c++){
                float *b = _vect;
                for ( ; b < b_end; )
                    *c += *a++ * *b++;
            }
            //std::cout << _name << " computed " << _r[0] << std::endl;
        }

    private:
        static void * UpdateThread(void* pthis)
        {
            int ret;
            A *pt = (A*)pthis;
            pt->WhoAmI();

            if ((ret=setpriority(PRIO_PROCESS, 0, pt->_nice)) != 0){
                std::cerr << pt->_name << " Error setting priority "<< pt->_nice << " error " << ret << std::endl;
            }

            std::cerr << pt->_name << " getpolicy " << sched_getscheduler(0) << " getpriority " << getpriority(PRIO_PROCESS, 0) << std::endl;
            
            while(1){
                pthread_mutex_lock(&mut);
                if (verbose > 1) std::cout << log_time() << pt->_name << ": cond_wait"<< std::endl;
                pthread_cond_wait(&cond, &mut);
                pt->copia(glob_ppp);
                if (verbose > 1) std::cout << log_time() << pt->_name << ": mutex_unlock"<< std::endl;
                pthread_mutex_unlock(&mut);
                pt->doit();
            }
            return 0;
        }

     public:
         void doit()
         {
            double dmsec=0;
            if (_ctr > 0) {
                _tg.stop();
                dmsec = _tg.msec();
                _tg_ave += dmsec;
                if ( dmsec < _tg_min) _tg_min = dmsec;
                if ( dmsec > _tg_max) _tg_max = dmsec;
            }
            _tg.start();
            _t.start();
            ComputeMatrix();
            _t.stop();
            _tot_time += _t.sec();
            _ctr++;
            if (verbose) std::cout << log_time() << _name << " done " << _ctr << " in " << _t.msec() << 
                " looped in " << dmsec << " dato " << _dato << std::endl;
        }
        void copia(int dato)
        {
            _dato = dato;
        }
    private:

        #include <sys/types.h>
        #include <sys/syscall.h>
        #include <linux/unistd.h>
		////_syscall0(pid_t,gettid);
        #include <errno.h>
        void WhoAmI()
        {
            //_t_id=gettid();
			_t_id = syscall( __NR_gettid);
            std::cout << _name << " tid " << _t_id << std::endl;
        }

    public:
        std::string   _name;
        double        _tot_time;
        int           _n_row;
        int           _n_column;
        double        _tg_ave, _tg_min, _tg_max;
    private:
        bool          _tredda;
        long          _ctr;
        Timing        _t;
        Timing        _tg;
        float        *_r;
        float        *_matr;
        float        *_vect;
        int           _dato;
        pthread_t     _thread_id;
        pid_t         _t_id;
        int           _nice;
};

int main(int argc, char** argv)
{
    long cycles=1000;
    Timing t;
    int prioz=0, prioa=0, priob=0;
    int ret;

    for (int i=1; i<argc; i++){
        if (strcmp(argv[i], "-c") == 0) {cycles=atoi(argv[++i]); continue;}
        if (strcmp(argv[i], "-p") == 0) {prioz=atoi(argv[++i]); prioa=atoi(argv[++i]); priob=atoi(argv[++i]); continue;}
        if (strcmp(argv[i], "-v") == 0) {verbose=atoi(argv[++i]); continue;}
    }

    A z("********************",true, 1000, 1000);
    A a("***********",true, 1000, 1000,prioa);
    A b("***",true,1000,1000,priob);
    
    a.spawn();
    b.spawn();
    
    sleep(1);

    struct sched_param param;
    param.sched_priority=1;
    if ((ret=sched_setscheduler(0, SCHED_RR, &param)) != 0){ 
        std::cerr << "thMAIN: Error setting policy - error " << ret   << std::endl;
    }
    //if ((ret=setpriority(PRIO_PROCESS, 0, prioz)) != 0){
    //    std::cerr << "thMAIN: Error setting priority "<< prioz << " error " << ret << std::endl;
    //}
    std::cerr << "thMAIN:"<< " getpolicy " << sched_getscheduler(0) << " getpriority " << getpriority(PRIO_PROCESS, 0) << std::endl;

    t.start();
    struct timespec pse;
    pse.tv_sec=0; pse.tv_nsec=(long)1e6;
    while(a.Ctr()<cycles && b.Ctr()<cycles && z.Ctr()<cycles){
        pthread_mutex_lock(&mut);
        //sched_yield();
        glob_ppp++;
        if (verbose > 2) std::cout << log_time() << "thMAIN:  cond_broadcast " << glob_ppp << std::endl;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mut);
        z.doit();
        nanosleep(&pse, NULL);
    }
    t.stop();
    std::cout << "total time "<<t.sec() << std::endl;
    std::cout << z._name << ": " << z.Ctr() << " cicli in "<< z._tot_time<< "s ["<< z._tot_time*1e3/z.Ctr() << " ms/ciclo] ";
    std::cout << " min/ave/max: " << z._tg_min <<" "<< z._tg_ave/z.Ctr() <<" "<< z._tg_max<< " ms" <<std::endl;
    std::cout << a._name << ": " << a.Ctr() << " cicli in "<< a._tot_time<< "s ["<< a._tot_time*1e3/a.Ctr() << " ms/ciclo] ";
    std::cout << " min/ave/max: " << a._tg_min <<" "<< a._tg_ave/a.Ctr() <<" "<< a._tg_max<< " ms" <<std::endl;
    std::cout << b._name << ": " << b.Ctr() << " cicli in "<< b._tot_time<< "s ["<< b._tot_time*1e3/b.Ctr() << " ms/ciclo] ";
    std::cout << " min/ave/max: " << b._tg_min <<" "<< b._tg_ave/b.Ctr() <<" "<< b._tg_max<< " ms" <<std::endl;
    return 0;
}
