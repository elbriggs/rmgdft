#ifndef RMG_BaseThread_H
#define RMG_BaseThread_H 1


#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <mpi.h>
#if GPU_ENABLED
#include <cuda.h>
#include <cuda_runtime.h>
#endif
#include "params.h"
#include "rmgtypes.h"

class BaseThread {

private:

    // Threads to use on each MPI node
    static int THREADS_PER_NODE;

    /* Thread ID number assigned by us */
    int tid;

    // Used to implement a local barrier for all threads inside of the run_threads function
    static pthread_barrier_t run_barrier;

    // Used to implement a local barrier inside of the scf loops
    static pthread_barrier_t scf_barrier;
    static pthread_mutex_t job_mutex;

    // These are used to synchronize the main process and the worker threads
    static sem_t thread_sem;

    // This is used when running with MPI_THREAD_SERIALIZED to ensure 
    // proper serialization
    static pthread_mutex_t mpi_mutex;

    static pthread_attr_t thread_attrs;
    static pthread_t threads[MAX_SCF_THREADS];
    static volatile int in_threaded_region1;

    // These are used to ensure thread ordering
    static volatile int mpi_thread_order_counter1;
    static pthread_mutex_t thread_order_mutex;

    // Used for accessing thread specific data
    static pthread_key_t scf_thread_control_key;

    // Used for timing information
    static pthread_mutex_t timings_mutex;

    /* Synchronization semaphore for this instance */
    sem_t this_sync;
        

    public:

        void wait_for_threads(int jobs);
        void wake_threads(int jobs);
        void scf_barrier_init(int nthreads);
        void scf_barrier_wait(void);
        void scf_barrier_destroy(void);
        void scf_tsd_init(void);
        void scf_tsd_set_value(void *s);
        void scf_tsd_delete(void);
        int get_thread_basetag(void);
        BaseThread *get_thread_control(void);
        int get_thread_tid(void);
        //cudaStream_t *get_thread_cstream(void);
        void set_cpu_affinity(int tid);
        void enter_threaded_region(void);
        void leave_threaded_region(void);
        void RMG_MPI_lock(void);
        void RMG_MPI_unlock(void);
        void RMG_MPI_thread_order_lock(void);
        void RMG_MPI_thread_order_unlock(void);
        void rmg_timings (int what, rmg_double_t time);



};

#endif
