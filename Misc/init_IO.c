/************************** SVN Revision Information **************************
 **    $Id$    **
 ******************************************************************************/

/***** RMG: Common/init_IO.c *****
 * NAME
 *   Ab initio real space multigrid acceleration
 *   Quantum molecular dynamics package.
 * COPYRIGHT
 *   Copyright (C) 2009  Frisco Rose, Jerzy Bernholc
 * FUNCTION
 *   void init_IO( int argc, char **argv )
 *   Initializes settings and creates directory structures for ouput logging
 *   Make each run image manage its own directory of input/output
 * INPUTS
 *   argc and argv from main
 * OUTPUT
 *   none
 * PARENTS
 *   main.c
 * CHILDREN
 *   init_pe.c read_pseudo.c
 * SOURCE
 */



#include "grid.h"
#include "const.h"
#include "params.h"
#include "rmgtypes.h"
#include "rmg_alloc.h"
#include "rmgtypedefs.h"
#include "typedefs.h"
#include "common_prototypes.h"
#include "macros.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include "hybrid.h"

void init_IO (int argc, char **argv)
{

    int i, npes, worldpe, image, status, lognum = 0, provided=0, retval;
    char workdir[MAX_PATH], logname[MAX_PATH], basename[MAX_PATH], *quantity, *extension, *endptr;
    struct stat buffer;
    time_t timer;

    /* Set start of program time */
    timer = time (NULL);
    MPI_Init_thread(&argc, &argv, ct.mpi_threadlevel, &provided);

    /* get this cores mpi rank */
    MPI_Comm_rank (MPI_COMM_WORLD, &worldpe);
    pct.worldrank = worldpe;

    /* get total mpi core count */
    MPI_Comm_size (MPI_COMM_WORLD, &npes);
    pct.total_npes = npes;

    if(argc == 2)
    {
        read_init(argv[1]);
    }
    else
        read_init("ctrl_init.dat");


    /* Define a default output stream, gets redefined to log file later */
    ct.logfile = stdout;

    // assign pct.thisimg:  which image on this processor
    int pe1, pe2, j, k;
    pe2 = 0;
    for(i = 0; i < pct.images; i+= ct.images_per_node)
    {
        pe1 = 0;
        for(j = 0; j < ct.images_per_node; j++)
        {
            pe1 += pct.image_npes[i+j];
            for(k = 0; k < pct.image_npes[i + j]; k++)
            {
                if(pct.worldrank == (pe2 + k * ct.images_per_node + j) ) pct.thisimg = i;
            }
        }
        pe2 += pe1;
    } 

    snprintf (ct.cfile, MAX_PATH, "%s%s", pct.image_path[pct.thisimg], pct.image_input[pct.thisimg]);
    snprintf (ct.basename, MAX_PATH, "%s%s", pct.image_path[pct.thisimg], pct.image_input[pct.thisimg]);


    /* Setup image number that this core belongs to */
    image = npes / pct.images;
    NPES = pct.image_npes[pct.thisimg];   // NPES is the per image number of PES


    //error_handler("message a"); 
    /* PE(MPI) initialization, need mpi groups defined before logfile is initialized */
    init_pe ( image );


    /* if logname exists, increment until unique filename found */
    if (pct.imgpe == 0)
    {
        snprintf (logname, MAX_PATH, "%s.log", ct.basename);

        int name_incr;
        name_incr = filename_increment(logname);
        snprintf (ct.logname, MAX_PATH, "%s.%02d", logname, name_incr);

        /* open and save logfile handle, printf is stdout before here */
        ct.logfile = fopen(ct.logname, "w");
    }

    MPI_Comm_size (pct.img_comm, &status);
    printf ("\nRMG run started at GMT %s", asctime (gmtime (&timer)));
    printf ("\nRMG running with %d images and %d images per node.\n", pct.images, ct.images_per_node);
    printf ("\nRMG running in message passing mode with %d procs for this image.", status);

    /* Read in our pseudopotential information */
    read_pseudo ();


#if GPU_ENABLED
    cudaDeviceReset();
    if( CUDA_SUCCESS != cuInit( 0 ) ) {
        fprintf(stderr, "CUDA: Not initialized\n" ); exit(-1);
    }
    if( CUDA_SUCCESS != cuDeviceGet( &ct.cu_dev, 0 ) ) {
        fprintf(stderr, "CUDA: Cannot get the device\n"); exit(-1);
    }
    cudaSetDevice(ct.cu_dev);
    //  if( CUDA_SUCCESS != cuCtxCreate( &ct.cu_context, CU_CTX_SCHED_YIELD, ct.cu_dev ) ) {
    //      fprintf(stderr, "CUDA: Cannot create the context\n"); exit(-1);
    //  }
    cudaSetDevice(ct.cu_dev);
    if( CUBLAS_STATUS_SUCCESS != cublasInit( ) ) {
        fprintf(stderr, "CUBLAS: Not initialized\n"); exit(-1);
    }

#endif

    if(provided < ct.mpi_threadlevel) {

        printf("Thread support requested = %d but only %d provided. Terminating.\n", ct.mpi_threadlevel, provided);
        MPI_Finalize();
        exit(0);

    }
    printf("Running with thread level = %d\n", provided);
    fflush(NULL);
    init_HYBRID_MODEL(ct.THREADS_PER_NODE);

    // Allocate storage for trade_images and global sums routines
    init_TradeImages();
    set_MPI_comm(pct.grid_comm);

    init_global_sums();

}
