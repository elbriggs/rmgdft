/************************** SVN Revision Information **************************
 **    $Id$    **
******************************************************************************/

/****f* QMD-MGDFT/wvfn_residual.c *****
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
 *   void wvfn_residual(STATE *states)
 *   calculate and output mean occupied Wavefunction residual
 * INPUTS
 *   states: point to orbital structure (see main.h)
 * OUTPUT
 *   no explicit output
 * PARENTS
 *   fastrlx.c
 * CHILDREN
 *
 * SOURCE
 */


#include <stdio.h>
#include <time.h>
#include <math.h>
#include "main.h"




void wvfn_residual (STATE * states)
{
    int is;
    REAL eigmean = 0.0;

    for (is = 0; is < ct.num_states; is++)
        if (states[is].occupation[0] > 0.1)
            eigmean += states[is].res;

    eigmean = eigmean / ((REAL) ct.num_states);
    ct.meanres = eigmean;
    if (pct.gridpe == 0)
        printf ("Mean occupied wavefunction residual = %14.6e\n", eigmean);

}
