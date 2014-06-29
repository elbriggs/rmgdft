/*
 *
 * Copyright (c) 2014, Emil Briggs
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
*/


#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex>
#include "transition.h"
#include "const.h"
#include "RmgTimer.h"
#include "rmgtypedefs.h"
#include "params.h"
#include "typedefs.h"
#include "common_prototypes.h"
#include "common_prototypes1.h"
#include "rmg_error.h"
#include "State.h"
#include "Kpoint.h"
#include "RmgSumAll.h"
#include <complex>
#include "../Headers/prototypes.h"


template class State<double>;
template class State<std::complex <double> >;
template State<double>::State(void);
template State<std::complex<double> >::State(void);

template void State<double>::normalize(double *tpsi, int istate);
template void State<std::complex <double> >::normalize(std::complex <double> *tpsi, int istate);
template void State<double>::set_storage(double *storage);
template void State<std::complex <double> >::set_storage(std::complex <double> *tpsi);

template <class StateType> State<StateType>::State(void)
{


}

template <class StateType> void State<StateType>::set_storage(StateType *storage)
{
    this->psi = storage;
}

template <class StateType> void State<StateType>::normalize(double *tpsi, int istate)
{
    double vel = (double) (Rmg_G->get_NX_GRID(1) * Rmg_G->get_NY_GRID(1) * Rmg_G->get_NZ_GRID(1));
    vel = Rmg_L.get_omega() / vel;

    if(ct.norm_conserving_pp) {

        double sum1 = 0.0, sum2;
        for(int idx = 0;idx < this->Kptr->pbasis;idx++) {
            sum1 = sum1 + tpsi[idx] * tpsi[idx];
        }

        sum2 = vel * RmgSumAll<double>(sum1, this->Kptr->comm);
        sum2 = sqrt(1.0 / sum2);

        for(int idx = 0;idx < this->Kptr->pbasis;idx++) {
            tpsi[idx] = tpsi[idx] * sum2;
        }

    }
    else {

        int idx, ion, nh, i, j, size, incx = 1, inh, sidx_local, nidx, oion;
        double sumbeta, sumpsi, sum, t1;
        double *tmp_psiR, *qqq, *sintR, *ptr;
        ION *iptr;

        sidx_local = this->Kptr->kidx * pct.num_nonloc_ions * ct.num_states * ct.max_nl + istate * ct.max_nl;

        size = this->Kptr->pbasis;

        tmp_psiR = tpsi;

        sumpsi = 0.0;
        sumbeta = 0.0;

        nidx = -1;
        for (ion = 0; ion < pct.num_owned_ions; ion++)
        {

            oion = pct.owned_ions_list[ion];
            
            iptr = &ct.ions[oion];
           
            nh = ct.sp[iptr->species].nh;
            
            /* Figure out index of owned ion in nonloc_ions_list array*/
            do {
                
                nidx++;
                if (nidx >= pct.num_nonloc_ions)
                    //error_handler("Could not find matching entry in pct.nonloc_ions_list for owned ion %d", oion);
                    rmg_error_handler(__FILE__, __LINE__, "Could not find matching entry in pct.nonloc_ions_list");
            
            } while (pct.nonloc_ions_list[nidx] != oion);

            qqq = pct.qqq[oion];


            /*nidx adds offset due to current ion*/
            sintR = &pct.newsintR_local[sidx_local + nidx * ct.num_states * ct.max_nl];


            for (i = 0; i < nh; i++)
            {
                inh = i * nh;
                for (j = 0; j < nh; j++)
                {
                    if (qqq[inh + j] != 0.0)
                    {
                        sumbeta += qqq[inh + j] * sintR[i] * sintR[j];
                    }
                }
            }
        }


        for (idx = 0; idx < get_P0_BASIS(); idx++)
        {
            sumpsi += tmp_psiR[idx] * tmp_psiR[idx];
        }

        sum = real_sum_all (vel * sumpsi + sumbeta, pct.grid_comm);
        sum = 1.0 / sum;
        if (sum < 0.0)
        {
            rmg_printf ("the %dth state is wrong\n", istate);
            rmg_error_handler (__FILE__, __LINE__, "<psi|S|psi> cann't be negative");
        }

        t1 = sqrt (sum);
        QMD_dscal (size, t1, tmp_psiR, incx);

        /* update <beta|psi> - Local version*/
        
        for (ion = 0; ion < pct.num_nonloc_ions; ion++)
        {

            ptr = &pct.newsintR_local[ion * ct.num_states * ct.max_nl];
            ptr += sidx_local;
            
            QMD_dscal (ct.max_nl, t1, ptr, incx);
        }


    }
}

template <class StateType> void State<StateType>::normalize(std::complex<double> *tpsi, int istate)
{

    double vel = (double) (Rmg_G->get_NX_GRID(1) * Rmg_G->get_NY_GRID(1) * Rmg_G->get_NZ_GRID(1));
    vel = Rmg_L.get_omega() / vel;

    if(ct.norm_conserving_pp) {

        double sum1 = 0.0, sum2;
        for(int idx = 0;idx < this->Kptr->pbasis;idx++) {
            sum1 = sum1 + std::real(tpsi[idx] * std::conj(tpsi[idx]));
        }

        sum2 = vel * RmgSumAll<double>(sum1, this->Kptr->comm);
        sum2 = sqrt(1.0 / sum2);

        for(int idx = 0;idx < this->Kptr->pbasis;idx++) {
            tpsi[idx] = tpsi[idx] * sum2;
        }

    }
}
