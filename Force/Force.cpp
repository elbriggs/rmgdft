/************************** SVN Revision Information **************************
 **    $Id$    **
******************************************************************************/

/****f* QMD-MGDFT/force.c *****
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
 *   void force(double *rho, double *rhoc, double *vh, double *vxc, STATE *states)
 *   Driver routine to calculate ionic forces.
 * INPUTS
 *   rho: total charge density
 *   rhoc: compensating charge density
 *   vh:   Hartree potential
 *   vxc:  exchange-correlation potential
 *   state: points to orbital structure which include eigenvalues, 
 *          wave functions and so on. (see main.h)
 * OUTPUT
 *   Resulted forces are stored in structure ct.xxxxx 
 * PARENTS
 *   cdfastrlx.c fastrlx.c moldyn.c quench.c
 * CHILDREN
 *   iiforce.c nlforce.c lforce.c nlccforce.c
 * SOURCE
 */


#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "transition.h"
#include "const.h"
#include "RmgTimer.h"
#include "rmgtypedefs.h"
#include "params.h"
#include "typedefs.h"
#include "common_prototypes.h"
#include "common_prototypes1.h"
#include "rmg_error.h"
#include "Kpoint.h"


/*Set this to 1 to have forces written out part by part*/
/* If you want this , you should also make sure that VERBOSE flag is enabled in
 * nlforce.c*/
#define VERBOSE 1


template void Force<double> (double * rho, double * rho_oppo, double * rhoc, double * vh, double*vh_in,
        double * vxc, double *vxc_in, double * vnuc, Kpoint<double> **Kptr);
template void Force<std::complex<double> > (double * rho, double * rho_oppo, double * rhoc, double * vh, double *vh_in, 
        double * vxc, double *vxc_in, double * vnuc, Kpoint<std::complex<double>> **Kptr);


template <typename OrbitalType> void Force (double * rho, double * rho_oppo, double * rhoc, double * vh, double *vh_in,
        double * vxc, double *vxc_in, double * vnuc, Kpoint<OrbitalType> **Kptr)
{
    RmgTimer RT0("2-Force");
    RmgTimer RTt("1-TOTAL: run: Force");
    int ion, idx, size1;
    double *vtott, *rho_tot;
    int Zi;

    double *force_tmp, *force_sum;
    double fac_spin = 1.0/(1.0 + ct.spin_flag);

    force_tmp = new double[3 *ct.num_ions];
    force_sum = new double[3 *ct.num_ions];

    for(int i = 0; i < ct.num_ions * 3; i++) force_sum[i] = 0.0;

    vtott = new double[get_FP0_BASIS()];

    for (idx = 0; idx < get_FP0_BASIS(); idx++)
        vtott[idx] = vxc[idx] + vh[idx] + vnuc[idx];


    for (int ion = pct.gridpe; ion < ct.num_ions; ion+=pct.grid_npes)
    {

        Zi = ct.sp[ct.ions[ion].species].zvalence;


        force_sum[ion *3 + 0] = ct.e_field * ct.x_field_0 * Zi * fac_spin;
        force_sum[ion *3 + 1] = ct.e_field * ct.y_field_0 * Zi * fac_spin;
        force_sum[ion *3 + 2] = ct.e_field * ct.z_field_0 * Zi * fac_spin;

    }


    /* Get the ion-ion component and store. */
    RmgTimer *RT1 = new RmgTimer("2-Force: ion-ion");
    for(int i = 0; i < ct.num_ions * 3; i++) force_tmp[i] = 0.0;
    iiforce (force_tmp);
    for(int i = 0; i < ct.num_ions * 3; i++) force_sum[i] += force_tmp[i] * fac_spin;
    delete RT1;

#if VERBOSE
    output_force(force_tmp, "Ion-Ion force:");
#endif

    /* Add in the local */
    RmgTimer *RT2 = new RmgTimer("2-Force: local");
    for(int i = 0; i < ct.num_ions * 3; i++) force_tmp[i] = 0.0;
    if (ct.spin_flag)
    {
        rho_tot = new double[get_FP0_BASIS()];
        for (idx = 0; idx < get_FP0_BASIS(); idx++)
            rho_tot[idx] = rho[idx] + rho_oppo[idx];
        Lforce(rho_tot, vh, force_tmp);
        delete[] rho_tot;
    }
    else
        Lforce (rho, vh, force_tmp);

    for(int i = 0; i < ct.num_ions * 3; i++) force_sum[i] += force_tmp[i];
    delete RT2;


#if VERBOSE
    output_force(force_tmp, "Local force:"); 
#endif


    /* Add in the non-local stuff */
    RmgTimer *RT3 = new RmgTimer("2-Force: non-local");
    for(int i = 0; i < ct.num_ions * 3; i++) force_tmp[i] = 0.0;
    Nlforce (vtott, Kptr, force_tmp);
    for(int i = 0; i < ct.num_ions * 3; i++) force_sum[i] += force_tmp[i];
    delete RT3;


#if VERBOSE
    output_force(force_tmp, "Non-Local force:");
#endif



    /* The non-linear core correction part if any */
    RmgTimer *RT4 = new RmgTimer("2-Force: core correction");
    for(int i = 0; i < ct.num_ions * 3; i++) force_tmp[i] = 0.0;
    Nlccforce (rho, vxc, force_tmp);
    for(int i = 0; i < ct.num_ions * 3; i++) force_sum[i] += force_tmp[i] * fac_spin;
    delete RT4;

#if VERBOSE
    output_force(force_tmp, "Non-linear core force:");
#endif

    RmgTimer *RT5 = new RmgTimer("2-Force: corrections");
    for(int i = 0; i < ct.num_ions * 3; i++) force_tmp[i] = 0.0;
    CorrectForces (vh, vh_in, vxc, vxc_in, force_tmp);
    for(int i = 0; i < ct.num_ions * 3; i++) force_sum[i] += force_tmp[i];
    delete RT5;

#if VERBOSE
    output_force(force_tmp, "Correction force:");
#endif

    size1 = 3 * ct.num_ions;
    global_sums (force_sum, &size1, pct.img_comm);

    for (ion = 0; ion < ct.num_ions; ion++)
    {

        ct.ions[ion].force[ct.fpt[0]][0] = force_sum[ion * 3 + 0];
        ct.ions[ion].force[ct.fpt[0]][1] = force_sum[ion * 3 + 1];
        ct.ions[ion].force[ct.fpt[0]][2] = force_sum[ion * 3 + 2];

    }


    if (!ct.is_gamma) {
        RmgTimer *RT5 = new RmgTimer("2-Force: symmetrization");
        symforce ();
        delete RT5;
    }
    delete[] vtott;
    delete[] force_sum;
    delete[] force_tmp;

    /* Impose force constraints, if any */
    if( ct.constrainforces )
        constrain ();



}                               /* end force */

void output_force(double *force_tmp, char *desc)    
{
    int size1 = 3 * ct.num_ions;
    global_sums (force_tmp, &size1, pct.grid_comm);
    if (pct.imgpe == 0)
    {
        printf ("\n\n %s", desc);

        for (int ion = 0; ion < ct.num_ions; ion++)
        {
            printf ("\n Ion %d Force  %10.7f  %10.7f  %10.7f",
                    ion, force_tmp[3 * ion],force_tmp[3 * ion + 1],force_tmp[3 * ion + 2]);
        }
    }
}

/******/
