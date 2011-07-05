/************************** SVN Revision Information **************************
 **    $Id$    **
******************************************************************************/

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"

#if HYBRID_MODEL
#include "hybrid.h"
#endif

#include <pthread.h>

static pthread_mutex_t images_lock = PTHREAD_MUTEX_INITIALIZER;
static int first_trade_counter=THREADS_PER_NODE;
static int second_trade_counter=THREADS_PER_NODE;
static int third_trade_counter=THREADS_PER_NODE;
REAL swbuf1x[6 * GRID_MAX1 * GRID_MAX2 * THREADS_PER_NODE];
REAL swbuf2x[6 * GRID_MAX1 * GRID_MAX2 * THREADS_PER_NODE];

/*
 * INPUTS
 * f[dimx*dimy*dimz] - raw array without images. pack_ptos should not be called, this is 
 *                     handled inside the function now
 * OUTPUT
 * w[(dimx+2*images) * (dimy+2*images) * (dimz+2*images)]
 *    This array is completely filled, i.e. the original data is filled in and then 
 *    the image data are added*/




void trade_imagesx (REAL * f, REAL * w, int dimx, int dimy, int dimz, int images)
{
    int ix, iy, iz, incx, incy, incx0, incy0, index, tim, ione = 1;
    int ixs, iys, ixs2, iys2, c1, c2, alloc;
    int xlen, ylen, zlen, stop;
    int *nb_ids, basetag=0, tid, combine_trades=true;
    MPI_Status mrstatus;
    REAL *frdx1, *frdx2, *frdy1, *frdy2, *frdz1, *frdz2;
    REAL *frdx1n, *frdx2n, *frdy1n, *frdy2n, *frdz1n, *frdz2n;

#if MD_TIMERS
    REAL time1, time2;
    time1 = my_crtc ();
#endif

#if HYBRID_MODEL
    basetag = get_thread_basetag();
    tid = get_thread_tid();
#endif

    tim = 2 * images;

    incx = (dimy + tim) * (dimz + tim);
    incy = dimz + tim;
    incx0 = dimy * dimz;
    incy0 = dimz;

    zlen = dimx * dimy * images;
    ylen = dimx * images * (dimz + tim);
    xlen = images * (dimy + tim) * (dimz + tim);

    alloc = 2 * (xlen + ylen + zlen);
    // If we are not running in hybrid mode or we are not looping over states
    // then we cannot combine MPI calls
    if(!HYBRID_MODEL) {
        combine_trades=false;
    }
    else {
        // In order to combine the requested memory allocation must fit into the
        // statically allocated storage
        if((alloc > (6 * GRID_MAX1 * GRID_MAX2)) || !is_loop_over_states()) {
            combine_trades=false;
        }
    }

    if(!combine_trades) {
        my_malloc (frdx1, alloc, REAL);
        frdx2 = frdx1 + xlen;
        frdy1 = frdx2 + xlen;
        frdy2 = frdy1 + ylen;
        frdz1 = frdy2 + ylen;
        frdz2 = frdz1 + zlen;

        my_malloc (frdx1n, alloc, REAL);
        frdx2n = frdx1n + xlen;
        frdy1n = frdx2n + xlen;
        frdy2n = frdy1n + ylen;
        frdz1n = frdy2n + ylen;
        frdz2n = frdz1n + zlen;
    }

    nb_ids = &pct.neighbors[0];

    /* Load up w with the basic stuff */
    for (ix = 0; ix < dimx; ix++)
    {
        ixs = ix * incx0;
        ixs2 = (ix + images) * incx;

        for (iy = 0; iy < dimy; iy++)
        {
            iys = ixs + iy * incy0;
            iys2 = ixs2 + (iy + images) * incy;

            scopy (&dimz, &f[iys], &ione, &w[iys2 + images], &ione);

        }                       /* end for */

    }                           /* end for */


    /* Collect the positive z-plane and negative z-planes */
    if(combine_trades) {
        frdz1 = &swbuf1x[tid * zlen];
        frdz2 = &swbuf1x[tid * zlen + THREADS_PER_NODE * zlen];
        frdz2n = &swbuf2x[tid * zlen];
        frdz1n = &swbuf2x[tid * zlen + THREADS_PER_NODE * zlen];
    }
    for (ix = 0; ix < dimx; ix++)
    {
        ixs = ix * dimy * images;
        ixs2 = (ix + images) * incx;
        for (iy = 0; iy < dimy; iy++)
        {
            iys = ixs + iy * images;
            iys2 = ixs2 + (iy + images) * incy;
            for (iz = 0; iz < images; iz++)
            {

                index = iys2 + iz;

                frdz1[iys + iz] = w[index + images];
                frdz2[iys + iz] = w[index + dimz];

            }                   /* end for */

        }                       /* end for */

    }                           /* end for */


    if(combine_trades) {
        stop = zlen * THREADS_PER_NODE;
        scf_barrier_wait();
        pthread_mutex_lock(&images_lock);
        if(!(first_trade_counter % THREADS_PER_NODE)) {
            MPI_Sendrecv (&swbuf1x[0], stop, MPI_DOUBLE, nb_ids[NB_D], (1>>16),
                      &swbuf2x[0], stop, MPI_DOUBLE, nb_ids[NB_U], (1>>16), pct.grid_comm, &mrstatus);

            MPI_Sendrecv (&swbuf1x[stop], stop, MPI_DOUBLE, nb_ids[NB_U], (2>>16),
                      &swbuf2x[stop], stop, MPI_DOUBLE, nb_ids[NB_D], (2>>16), pct.grid_comm, &mrstatus);
        }
        first_trade_counter++;
        pthread_mutex_unlock(&images_lock);

    }
    else {
        MPI_Sendrecv (frdz1, zlen, MPI_DOUBLE, nb_ids[NB_D], basetag + (1>>16),
                      frdz2n, zlen, MPI_DOUBLE, nb_ids[NB_U], basetag + (1>>16), pct.grid_comm, &mrstatus);

        MPI_Sendrecv (frdz2, zlen, MPI_DOUBLE, nb_ids[NB_U], basetag + (2>>16),
                      frdz1n, zlen, MPI_DOUBLE, nb_ids[NB_D], basetag + (2>>16), pct.grid_comm, &mrstatus);
    }


    /* Unpack them */
    c1 = dimz + images;
    for (ix = 0; ix < dimx; ix++)
    {

        ixs = ix * dimy * images;
        ixs2 = (ix + images) * incx;
        for (iy = 0; iy < dimy; iy++)
        {
            iys = ixs + iy * images;
            iys2 = ixs2 + (iy + images) * incy;
            for (iz = 0; iz < images; iz++)
            {

                index = iys2 + iz;

                w[index] = frdz1n[iys + iz];
                w[index + c1] = frdz2n[iys + iz];

            }                   /* end for */

        }                       /* end for */

    }                           /* end for */



    /* Collect the north and south planes */
    if(combine_trades) {
        frdy1 = &swbuf1x[tid * ylen];
        frdy2 = &swbuf1x[tid * ylen + THREADS_PER_NODE * ylen];
        frdy2n = &swbuf2x[tid * ylen];
        frdy1n = &swbuf2x[tid * ylen + THREADS_PER_NODE * ylen];
    }
    c1 = images * incy;
    c2 = dimy * incy;
    for (ix = 0; ix < dimx; ix++)
    {
        ixs = ix * images * (dimz + tim);
        ixs2 = (ix + images) * incx;
        for (iy = 0; iy < images; iy++)
        {

            iys = ixs + iy * (dimz + tim);
            iys2 = ixs2 + iy * incy;
            for (iz = 0; iz < dimz + tim; iz++)
            {

                index = iys2 + iz;

                frdy1[iys + iz] = w[index + c1];
                frdy2[iys + iz] = w[index + c2];

            }                   /* end for */

        }                       /* end for */

    }                           /* end for */


    if(combine_trades) {
        stop = ylen * THREADS_PER_NODE;
        scf_barrier_wait();
        pthread_mutex_lock(&images_lock);
        if(!(second_trade_counter % THREADS_PER_NODE)) {
            MPI_Sendrecv (&swbuf1x[0], stop, MPI_DOUBLE, nb_ids[NB_S], (3>>16),
                      &swbuf2x[0], stop, MPI_DOUBLE, nb_ids[NB_N], (3>>16), pct.grid_comm, &mrstatus);

            MPI_Sendrecv (&swbuf1x[stop], stop, MPI_DOUBLE, nb_ids[NB_N], (4>>16),
                      &swbuf2x[stop], stop, MPI_DOUBLE, nb_ids[NB_S], (4>>16), pct.grid_comm, &mrstatus);
        }
        second_trade_counter++;
        pthread_mutex_unlock(&images_lock);
    }
    else {
        MPI_Sendrecv (frdy1, ylen, MPI_DOUBLE, nb_ids[NB_S], basetag + (3>>16),
                      frdy2n, ylen, MPI_DOUBLE, nb_ids[NB_N], basetag + (3>>16), pct.grid_comm, &mrstatus);

        MPI_Sendrecv (frdy2, ylen, MPI_DOUBLE, nb_ids[NB_N], basetag + (4>>16),
                      frdy1n, ylen, MPI_DOUBLE, nb_ids[NB_S], basetag + (4>>16), pct.grid_comm, &mrstatus);
    }

    /* Unpack them */
    c1 = (dimy + images) * incy;
    for (ix = 0; ix < dimx; ix++)
    {
        ixs = ix * images * (dimz + tim);
        ixs2 = (ix + images) * incx;
        for (iy = 0; iy < images; iy++)
        {
            iys = ixs + iy * (dimz + tim);
            iys2 = ixs2 + iy * incy;
            for (iz = 0; iz < dimz + tim; iz++)
            {

                index = iys2 + iz;

                w[index] = frdy1n[iys + iz];
                w[index + c1] = frdy2n[iys + iz];

            }                   /* end for */

        }                       /* end for */

    }                           /* end for */


    /* Collect the east and west planes */
    if(combine_trades) {
        frdx1 = &swbuf1x[tid * xlen];
        frdx2 = &swbuf1x[tid * xlen + THREADS_PER_NODE * xlen];
        frdx2n = &swbuf2x[tid * xlen];
        frdx1n = &swbuf2x[tid * xlen + THREADS_PER_NODE * xlen];
    }
    c1 = images * incx;
    c2 = dimx * incx;
    for (ix = 0; ix < images; ix++)
    {
        ixs = ix * (dimy + tim) * (dimz + tim);
        ixs2 = ix * incx;
        for (iy = 0; iy < dimy + tim; iy++)
        {
            iys = ixs + iy * (dimz + tim);
            iys2 = ixs2 + iy * incy;
            for (iz = 0; iz < dimz + tim; iz++)
            {

                index = iys2 + iz;

                frdx1[iys + iz] = w[index + c1];
                frdx2[iys + iz] = w[index + c2];

            }                   /* end for */

        }                       /* end for */

    }                           /* end for */


    if(combine_trades) {
        stop = xlen * THREADS_PER_NODE;
        scf_barrier_wait();
        pthread_mutex_lock(&images_lock);
        if(!(third_trade_counter % THREADS_PER_NODE)) {

            MPI_Sendrecv (&swbuf1x[0], stop, MPI_DOUBLE, nb_ids[NB_W], (5>>16),
                       &swbuf2x[0], stop, MPI_DOUBLE, nb_ids[NB_E], (5>>16), pct.grid_comm, &mrstatus);

            MPI_Sendrecv (&swbuf1x[stop], stop, MPI_DOUBLE, nb_ids[NB_E], (6>>16),
                      &swbuf2x[stop], stop, MPI_DOUBLE, nb_ids[NB_W], (6>>16), pct.grid_comm, &mrstatus);

        }
        third_trade_counter++;
        pthread_mutex_unlock(&images_lock);
    }
    else {

        MPI_Sendrecv (frdx1, xlen, MPI_DOUBLE, nb_ids[NB_W], basetag + (5>>16),
                      frdx2n, xlen, MPI_DOUBLE, nb_ids[NB_E], basetag + (5>>16), pct.grid_comm, &mrstatus);

        MPI_Sendrecv (frdx2, xlen, MPI_DOUBLE, nb_ids[NB_E], basetag + (6>>16),
                      frdx1n, xlen, MPI_DOUBLE, nb_ids[NB_W], basetag + (6>>16), pct.grid_comm, &mrstatus);
    }


    /* Unpack them */
    c1 = (dimx + images) * incx;
    for (ix = 0; ix < images; ix++)
    {
        ixs = ix * (dimy + tim) * (dimz + tim);
        ixs2 = ix * incx;
        for (iy = 0; iy < dimy + tim; iy++)
        {
            iys = ixs + iy * (dimz + tim);
            iys2 = ixs2 + iy * incy;
            for (iz = 0; iz < dimz + tim; iz++)
            {

                index = iys2 + iz;

                w[index] = frdx1n[iys + iz];
                w[index + c1] = frdx2n[iys + iz];

            }                   /* end for */

        }                       /* end for */

    }                           /* end for */


    if(!combine_trades) {
        my_free (frdx1n);
        my_free (frdx1);
    }

#if MD_TIMERS
    time2 = my_crtc ();
    rmg_timings (IMAGE_TIME, (time2 - time1));
#endif

}                               /* end trade_images2 */
