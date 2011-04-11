/************************** SVN Revision Information **************************
 **    $Id$    **
******************************************************************************/

#include "main.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

/* This function is used to normalize the wavefunctions */
/* Parallelized over ions, so that orthogonalization is more efficient*/
/* This should be used only withing orthogonalization loop, or it has to be followed
 * by betaxpsi (update of newsint) */

void norm_psi1_parallel (STATE * sp, int istate, int kidx)
{

    int idx, ion, nh, i, j, size, incx = 1, inh, sidx;
    REAL sumbetaR, sumpsi, sum, t1;
    REAL *tmp_psiR, *tmp_psiI, *qqq, *sintR, *sintI, *ptr;
    ION *iptr;

    sidx = kidx * ct.num_ions * ct.num_states * ct.max_nl + istate * ct.max_nl;

    size = sp->pbasis;

    tmp_psiR = sp->psiR;
#if !GAMMA_PT
    tmp_psiI = sp->psiI;
#endif

    sumpsi = 0.0;
    sumbetaR = 0.0;

    /*Ion parallelization */
    for (ion = pct.gridpe; ion < ct.num_ions; ion += NPES)
    {
        qqq = pct.qqq[ion];
        nh = pct.prj_per_ion[ion];
        iptr = &ct.ions[ion];


        sintR = &iptr->newsintR[sidx];
#if !GAMMA_PT
        sintI = &iptr->newsintI[sidx];
#endif


        for (i = 0; i < nh; i++)
        {
            inh = i * nh;
            for (j = 0; j < nh; j++)
            {
                if (qqq[inh + j] != 0.0)
                {
#if GAMMA_PT
                    sumbetaR += qqq[inh + j] * sintR[i] * sintR[j];
#else
                    sumbetaR += qqq[inh + j] * (sintR[i] * sintR[j] + sintI[i] * sintI[j]);
#endif

                }
            }
        }
    }


    for (idx = 0; idx < P0_BASIS; idx++)
    {
        sumpsi += tmp_psiR[idx] * tmp_psiR[idx];
#if !GAMMA_PT
        sumpsi += tmp_psiI[idx] * tmp_psiI[idx];
#endif
    }


    /*Sum sumpsi and sumbeta */
    sum = real_sum_all ( (ct.vel * sumpsi + sumbetaR), pct.grid_comm );


    sum = 1.0 / sum;
    if (sum < 0.0)
    {
        printf ("the %dth state is wrong\n", istate);
        error_handler ("<psi|S|psi> cann't be negative");
    }

    t1 = sqrt (sum);
    QMD_sscal (size, t1, tmp_psiR, incx);
#if !GAMMA_PT
    QMD_sscal (size, t1, tmp_psiI, incx);
#endif


    /* update <beta|psi> */
    /* Again, this is parallelized over ions */
    for (ion = pct.gridpe; ion < ct.num_ions; ion += NPES)
    {

        iptr = &ct.ions[ion];

        ptr = &iptr->newsintR[sidx];
        QMD_sscal (ct.max_nl, t1, ptr, incx);
#if !GAMMA_PT
        ptr = &iptr->newsintI[sidx];
        QMD_sscal (ct.max_nl, t1, ptr, incx);
#endif
    }


}                               /* end norm_psi1 */
