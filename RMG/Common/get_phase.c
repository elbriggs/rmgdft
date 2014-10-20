/************************** SVN Revision Information **************************
 **    $Id$    **
******************************************************************************/

/****f* QMD-MGDFT/get_phase.c *****
 * NAME
 *   Ab initio real space code with multigrid acceleration
 *   Quantum molecular dynamics package.
 *   Version: 2.1.5
 * COPYRIGHT
 *   Copyright (C) 1995  Emil Briggs
 *   Copyright (C) 1998  Emil Briggs, Charles Brabec, Mark Wensell, 
 *                       Dan Sullivan, Chris Rapcewicz, Jerzy Bernholc
 *   Copyright (C) 2001  Emil Briggs, Wenchang Lu,
 *                       Marco Buongiorno Nardelli,Charles Brabec, 
 *                       Mark Wensell,Dan Sullivan, Chris Rapcewicz,
 *                       Jerzy Bernholc
 * FUNCTION
 *   void get_phase(ION *iptr, double *rtptr, int ip, int icount, int *dvec)
 *   Generates the phase factors for the k-points 
 * INPUTS
 *   iptr: point to structure ION (see main.h )
 *   ip:   momentum l
 *   icount: number of grid point in the sphere
 *   dvec:  true indicates the point is in the sphere
 *          fault indicate not
 * OUTPUT
 *   rtptr: pct.phaseptr (see main.h )
 * PARENTS
 *   get_nloc_smp.c
 * CHILDREN
 *   to_cartesian.c
 * SOURCE
 */

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "common_prototypes.h"

void get_phase (ION * iptr, double * rtptr, int icount, int *dvec)
{

    int kpt, idx, ix, iy, iz, docount;
    int dimx, dimy, dimz, pbasis;
    double ax[3], bx[3], xc, yc, zc;
    double kdr;
    double hxgrid, hygrid, hzgrid;
    SPECIES *sp;

    hxgrid = get_hxgrid();
    hygrid = get_hygrid();
    hzgrid = get_hzgrid();

    if (rtptr == NULL)
        return;

    /* Get species type */
    sp = &ct.sp[iptr->species];

    dimx = sp->nldim;
    dimy = sp->nldim;
    dimz = sp->nldim;
    pbasis = dimx * dimy * dimz;


    for (kpt = 0; kpt < ct.num_kpts; kpt++)
    {

        idx = 0;
        for (ix = 0; ix < dimx; ix++)
        {
    
           xc = iptr->nlxcstart + ix * hxgrid;

            for (iy = 0; iy < dimy; iy++)
            {
                yc = iptr->nlycstart + iy * hygrid;

                for (iz = 0; iz < dimz; iz++)
                {
                   zc = iptr->nlzcstart + iz * hzgrid;

                   // ax[0] = xc - iptr->xtal[0];
                   // ax[1] = yc - iptr->xtal[1];
                   // ax[2] = zc - iptr->xtal[2];
                    ax[0] = xc ;
                    ax[1] = yc ;
                    ax[2] = zc ;
                    to_cartesian (ax, bx);
                    kdr = ct.kp[kpt].kvec[0] * bx[0] +
                          ct.kp[kpt].kvec[1] * bx[1] + ct.kp[kpt].kvec[2] * bx[2];

                    idx = ix * dimy * dimz + iy * dimz + iz;

                    rtptr[idx + 2 * kpt * pbasis] = cos (kdr);
                    rtptr[idx + 2 * kpt * pbasis + pbasis] = sin (kdr);


                }               /* end for */

            }                   /* end for */

        }                       /* end for */

    }                           /* end for */


}                               /* end get_phase */

/******/
