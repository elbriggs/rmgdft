
#include "Mgrid.h"
#include "FiniteDiff.h"

using namespace std;

template <typename RmgType>
void Mgrid::mg_restrict (RmgType * full, RmgType * half, int dimx, int dimy, int dimz, int dx2, int dy2, int dz2, int xoffset, int yoffset, int zoffset)
{

    int ix, iy, iz, ibrav;
    int incz, incy, incx, incz2, incy2, incx2;
    int x0, xp, xm, y0, yp, ym, z0, zp, zm;
    rmg_double_t scale, face, corner, edge;
    BaseGrid G;

    ibrav = G.get_ibrav_type();

    incz = 1;
    incy = dimz + 2;
    incx = (dimz + 2) * (dimy + 2);

    incz2 = 1;
    incy2 = dz2 + 2;
    incx2 = (dz2 + 2) * (dy2 + 2);


    switch (ibrav)
    {

        case CUBIC_PRIMITIVE:
        case CUBIC_FC:
        case ORTHORHOMBIC_PRIMITIVE:
        case HEXAGONAL:

            scale = ONE / 64.0;
            for (ix = 1; ix <= dx2; ix++)
            {

                x0 = 2 * ix - 1 + xoffset;
                xp = x0 + 1;
                xm = x0 - 1;

                for (iy = 1; iy <= dy2; iy++)
                {

                    y0 = 2 * iy - 1 + yoffset;
                    yp = y0 + 1;
                    ym = y0 - 1;

                    for (iz = 1; iz <= dz2; iz++)
                    {

                        z0 = 2 * iz - 1 + zoffset;
                        zp = z0 + 1;
                        zm = z0 - 1;

                        face = full[xm * incx + y0 * incy + z0] +
                            full[xp * incx + y0 * incy + z0] +
                            full[x0 * incx + ym * incy + z0] +
                            full[x0 * incx + yp * incy + z0] +
                            full[x0 * incx + y0 * incy + zm] + full[x0 * incx + y0 * incy + zp];

                        corner =
                            full[xm * incx + ym * incy + zm] +
                            full[xm * incx + ym * incy + zp] +
                            full[xm * incx + yp * incy + zm] +
                            full[xm * incx + yp * incy + zp] +
                            full[xp * incx + ym * incy + zm] +
                            full[xp * incx + ym * incy + zp] +
                            full[xp * incx + yp * incy + zm] + full[xp * incx + yp * incy + zp];

                        edge = full[xm * incx + y0 * incy + zm] +
                            full[xm * incx + ym * incy + z0] +
                            full[xm * incx + yp * incy + z0] +
                            full[xm * incx + y0 * incy + zp] +
                            full[x0 * incx + ym * incy + zm] +
                            full[x0 * incx + yp * incy + zm] +
                            full[x0 * incx + ym * incy + zp] +
                            full[x0 * incx + yp * incy + zp] +
                            full[xp * incx + y0 * incy + zm] +
                            full[xp * incx + ym * incy + z0] +
                            full[xp * incx + yp * incy + z0] + full[xp * incx + y0 * incy + zp];


                        half[ix * incx2 + iy * incy2 + iz] =
                            scale * (8.0 * full[x0 * incx + y0 * incy + z0] + 4.0 * face + 2.0 * edge +
                                     corner);


                    }               /* end for */

                }                   /* end for */

            }                       /* end for */

            break;


        case CUBIC_BC:

            scale = ONE / 52.0;

            for (ix = 1; ix <= dx2; ix++)
            {

                x0 = 2 * ix - 1 + xoffset;
                xp = x0 + 1;
                xm = x0 - 1;

                for (iy = 1; iy <= dy2; iy++)
                {

                    y0 = 2 * iy - 1 + yoffset;
                    yp = y0 + 1;
                    ym = y0 - 1;

                    for (iz = 1; iz <= dz2; iz++)
                    {

                        z0 = 2 * iz - 1 + zoffset;
                        zp = z0 + 1;
                        zm = z0 - 1;

                        face = full[xm * incx + ym * incy + z0] +
                            full[xm * incx + y0 * incy + zm] +
                            full[x0 * incx + ym * incy + zm] +
                            full[x0 * incx + yp * incy + zp] +
                            full[xp * incx + y0 * incy + zp] + full[xp * incx + yp * incy + z0];

                        corner =
                            full[xm * incx + ym * incy + zm] +
                            full[xm * incx + y0 * incy + z0] +
                            full[x0 * incx + ym * incy + z0] +
                            full[x0 * incx + y0 * incy + zm] +
                            full[x0 * incx + y0 * incy + zp] +
                            full[x0 * incx + yp * incy + z0] +
                            full[xp * incx + y0 * incy + z0] + full[xp * incx + yp * incy + zp];


                        half[ix * incx2 + iy * incy2 + iz] =
                            scale * (8.0 * full[x0 * incx + y0 * incy + z0] + 4.0 * corner +
                                     2.0 * face);


                    }               /* end for */

                }                   /* end for */

            }                       /* end for */

            break;

        case 20:

            scale = ONE / 80.0;
            for (ix = 1; ix <= dx2; ix++)
            {

                x0 = 2 * ix - 1 + xoffset;
                xp = x0 + 1;
                xm = x0 - 1;

                for (iy = 1; iy <= dy2; iy++)
                {

                    y0 = 2 * iy - 1 + yoffset;
                    yp = y0 + 1;
                    ym = y0 - 1;

                    for (iz = 1; iz <= dz2; iz++)
                    {

                        z0 = 2 * iz - 1 + zoffset;
                        zp = z0 + 1;
                        zm = z0 - 1;

                        face = full[xm * incx + ym * incy + z0] +
                            full[xm * incx + y0 * incy + zm] +
                            full[x0 * incx + ym * incy + zm] +
                            full[x0 * incx + yp * incy + zp] +
                            full[xp * incx + y0 * incy + zp] + full[xp * incx + yp * incy + z0];

                        edge =
                            full[xm * incx + y0 * incy + z0] +
                            full[xm * incx + y0 * incy + zp] +
                            full[xm * incx + yp * incy + z0] +
                            full[x0 * incx + ym * incy + z0] +
                            full[x0 * incx + ym * incy + zp] +
                            full[x0 * incx + y0 * incy + zm] +
                            full[x0 * incx + y0 * incy + zp] +
                            full[x0 * incx + yp * incy + zm] +
                            full[x0 * incx + yp * incy + z0] +
                            full[xp * incx + ym * incy + z0] +
                            full[xp * incx + y0 * incy + zm] + full[xp * incx + y0 * incy + z0];


                        half[ix * incx2 + iy * incy2 + iz] =
                            scale * (8.0 * full[x0 * incx + y0 * incy + z0] + 5.0 * edge + 2.0 * face);


                    }               /* end for */

                }                   /* end for */

            }                       /* end for */

            break;

        default:
            rmg_error_handler ("Lattice type not programmed");

    }                           /* end switch */


}                               /* end mg_restrict */

template <typename RmgType>
void Mgrid::mg_prolong (RmgType * full, RmgType * half, int dimx, int dimy, int dimz, int dx2, int dy2, int dz2, int xoffset, int yoffset, int zoffset)
{

    int ix, iy, iz;
    int incx, incy, incxr, incyr;

    incy = dimz + 2;
    incx = (dimz + 2) * (dimy + 2);

    incyr = dz2 + 2;
    incxr = (dz2 + 2) * (dy2 + 2);


    /* transfer coarse grid points to fine grid along with the
     * high side image point
     */

    for (ix = 1-xoffset; ix <= dimx/2 + 1; ix++)
    {

        for (iy = 1-yoffset; iy <= dimy/2 + 1; iy++)
        {

            for (iz = 1-zoffset; iz <= dimz/2 + 1; iz++)
            {

                full[(2 * ix - 1+xoffset) * incx + (2 * iy - 1+yoffset) * incy + 2 * iz - 1+zoffset] =
                    half[ix * incxr + iy * incyr + iz];

            }                   /* end for */

        }                       /* end for */

    }                           /* end for */



    /* interior center points
     */
    for (ix = 2-xoffset; ix <= dimx; ix += 2)
    {

        for (iy = 2-yoffset; iy <= dimy; iy += 2)
        {

            for (iz = 2-zoffset; iz <= dimz; iz += 2)
            {

                full[ix * incx + iy * incy + iz] =
                    0.125 * full[(ix - 1) * incx + (iy - 1) * incy + iz - 1] +
                    0.125 * full[(ix - 1) * incx + (iy - 1) * incy + iz + 1] +
                    0.125 * full[(ix - 1) * incx + (iy + 1) * incy + iz - 1] +
                    0.125 * full[(ix - 1) * incx + (iy + 1) * incy + iz + 1] +
                    0.125 * full[(ix + 1) * incx + (iy - 1) * incy + iz - 1] +
                    0.125 * full[(ix + 1) * incx + (iy - 1) * incy + iz + 1] +
                    0.125 * full[(ix + 1) * incx + (iy + 1) * incy + iz - 1] +
                    0.125 * full[(ix + 1) * incx + (iy + 1) * incy + iz + 1];

            }                   /* end for */

        }                       /* end for */

    }                           /* end for */



    for (ix = 1-xoffset; ix <= dimx; ix += 2)
    {

        for (iy = 1-yoffset; iy <= dimy; iy += 2)
        {

            for (iz = 2-zoffset; iz <= dimz; iz += 2)
            {

                full[ix * incx + iy * incy + iz] =
                    0.5 * full[ix * incx + iy * incy + iz - 1] +
                    0.5 * full[ix * incx + iy * incy + iz + 1];

            }                   /* end for */

        }                       /* end for */

    }                           /* end for */



    for (ix = 1-xoffset; ix <= dimx; ix += 2)
    {

        for (iy = 2-yoffset; iy <= dimy; iy += 2)
        {

            for (iz = 1-zoffset; iz <= dimz; iz += 2)
            {

                full[ix * incx + iy * incy + iz] =
                    0.5 * full[ix * incx + (iy - 1) * incy + iz] +
                    0.5 * full[ix * incx + (iy + 1) * incy + iz];

            }                   /* end for */

        }                       /* end for */

    }                           /* end for */



    for (ix = 2-xoffset; ix <= dimx; ix += 2)
    {

        for (iy = 1-yoffset; iy <= dimy; iy += 2)
        {

            for (iz = 1-zoffset; iz <= dimz; iz += 2)
            {

                full[ix * incx + iy * incy + iz] =
                    0.5 * full[(ix - 1) * incx + iy * incy + iz] +
                    0.5 * full[(ix + 1) * incx + iy * incy + iz];

            }                   /* end for */

        }                       /* end for */

    }                           /* end for */



    for (ix = 1-xoffset; ix <= dimx; ix += 2)
    {

        for (iy = 2-yoffset; iy <= dimy; iy += 2)
        {

            for (iz = 2-zoffset; iz <= dimz; iz += 2)
            {

                full[ix * incx + iy * incy + iz] =
                    0.25 * full[ix * incx + (iy - 1) * incy + iz - 1] +
                    0.25 * full[ix * incx + (iy - 1) * incy + iz + 1] +
                    0.25 * full[ix * incx + (iy + 1) * incy + iz - 1] +
                    0.25 * full[ix * incx + (iy + 1) * incy + iz + 1];

            }                   /* end for */

        }                       /* end for */

    }                           /* end for */



    for (ix = 2-xoffset; ix <= dimx; ix += 2)
    {

        for (iy = 1-yoffset; iy <= dimy; iy += 2)
        {

            for (iz = 2-zoffset; iz <= dimz; iz += 2)
            {

                full[ix * incx + iy * incy + iz] =
                    0.25 * full[(ix - 1) * incx + iy * incy + iz - 1] +
                    0.25 * full[(ix - 1) * incx + iy * incy + iz + 1] +
                    0.25 * full[(ix + 1) * incx + iy * incy + iz - 1] +
                    0.25 * full[(ix + 1) * incx + iy * incy + iz + 1];

            }                   /* end for */

        }                       /* end for */

    }                           /* end for */



    for (ix = 2-xoffset; ix <= dimx; ix += 2)
    {

        for (iy = 2-yoffset; iy <= dimy; iy += 2)
        {

            for (iz = 1-zoffset; iz <= dimz; iz += 2)
            {

                full[ix * incx + iy * incy + iz] =
                    0.25 * full[(ix - 1) * incx + (iy - 1) * incy + iz] +
                    0.25 * full[(ix + 1) * incx + (iy - 1) * incy + iz] +
                    0.25 * full[(ix - 1) * incx + (iy + 1) * incy + iz] +
                    0.25 * full[(ix + 1) * incx + (iy + 1) * incy + iz];

            }                   /* end for */

        }                       /* end for */

    }                           /* end for */

}                               /* end mg_prolong */



template <typename RmgType>
void Mgrid::eval_residual (RmgType * mat, RmgType * f_mat, int dimx, int dimy, int dimz,
                    rmg_double_t gridhx, rmg_double_t gridhy, rmg_double_t gridhz, RmgType * res)
{
    int size, idx;

    size = (dimx + 2) * (dimy + 2) * (dimz + 2);
    for (idx = 0; idx < size; idx++)
        res[idx] = 0.0;
    FD_app_del2c (mat, res, dimx, dimy, dimz, gridhx, gridhy, gridhz);

    for (idx = 0; idx < size; idx++)
        res[idx] = f_mat[idx] - res[idx];


}                               /* end eval_residual */



template <typename RmgType>
void Mgrid::solv_pois (RmgType * vmat, RmgType * fmat, RmgType * work,
                int dimx, int dimy, int dimz, rmg_double_t gridhx, rmg_double_t gridhy, rmg_double_t gridhz, rmg_double_t step, rmg_double_t k)
{
    int size, idx;
    rmg_double_t scale;
    rmg_double_t diag;

    size = (dimx + 2) * (dimy + 2) * (dimz + 2);
    for (idx = 0; idx < size; idx++)
        work[idx] = ZERO;
    diag = -FD_app_del2c (vmat, work, dimx, dimy, dimz, gridhx, gridhy, gridhz);

    scale = step / diag;
    
    // Non-zero k effectively means we are solving the Helmholtz rather than Poissons equation
    if(k != 0.0) {

        for (idx = 0; idx < size; idx++)
        {

            vmat[idx] += scale * (work[idx] - k*vmat[idx] - fmat[idx]);

        }                           /* end for */

     }
     else {

        for (idx = 0; idx < size; idx++)
        {

            vmat[idx] += scale * (work[idx] - fmat[idx]);

        }                           /* end for */

     }

}                               /* end solv_pois */



// C wrappers
extern "C" void mg_restrict_f (rmg_float_t * full, rmg_float_t * half, int dimx, int dimy, int dimz, int dx2, int dy2, int dz2, int xoffset, int yoffset, int zoffset)
{
    Mgrid MG;
    MG.mg_restrict<float>(full, half, dimx, dimy, dimz, dx2, dy2, dz2, xoffset, yoffset, zoffset);
}

extern "C" void mg_restrict (rmg_double_t * full, rmg_double_t * half, int dimx, int dimy, int dimz, int dx2, int dy2, int dz2, int xoffset, int yoffset, int zoffset)
{
    Mgrid MG;
    MG.mg_restrict<double>(full, half, dimx, dimy, dimz, dx2, dy2, dz2, xoffset, yoffset, zoffset);
}

extern "C" void mg_prolong_f (rmg_float_t * full, rmg_float_t * half, int dimx, int dimy, int dimz, int dx2, int dy2, int dz2, int xoffset, int yoffset, int zoffset)
{
    Mgrid MG;
    MG.mg_prolong<float>(full, half, dimx, dimy, dimz, dx2, dy2, dz2, xoffset, yoffset, zoffset);
}

extern "C" void mg_prolong (rmg_double_t * full, rmg_double_t * half, int dimx, int dimy, int dimz, int dx2, int dy2, int dz2, int xoffset, int yoffset, int zoffset)
{
    Mgrid MG;
    MG.mg_prolong<double>(full, half, dimx, dimy, dimz, dx2, dy2, dz2, xoffset, yoffset, zoffset);
}

extern "C" void eval_residual_f (rmg_float_t * mat, rmg_float_t * f_mat, int dimx, int dimy, int dimz,
                    rmg_double_t gridhx, rmg_double_t gridhy, rmg_double_t gridhz, rmg_float_t * res)
{
    Mgrid MG;
    MG.eval_residual<float>(mat, f_mat, dimx, dimy, dimz, gridhx, gridhy, gridhz, res);
}

extern "C" void eval_residual (rmg_double_t * mat, rmg_double_t * f_mat, int dimx, int dimy, int dimz,
                    rmg_double_t gridhx, rmg_double_t gridhy, rmg_double_t gridhz, rmg_double_t * res)
{
    Mgrid MG;
    MG.eval_residual<double>(mat, f_mat, dimx, dimy, dimz, gridhx, gridhy, gridhz, res);
}

extern "C" void solv_pois_f (rmg_float_t * vmat, rmg_float_t * fmat, rmg_float_t * work,
                int dimx, int dimy, int dimz, rmg_double_t gridhx, rmg_double_t gridhy, rmg_double_t gridhz, rmg_double_t step, rmg_double_t k)
{
    Mgrid MG;
    MG.solv_pois<float>(vmat, fmat, work, dimx, dimy, dimz, gridhx, gridhy, gridhz, step, k);
}

extern "C" void solv_pois (rmg_double_t * vmat, rmg_double_t * fmat, rmg_double_t * work,
                int dimx, int dimy, int dimz, rmg_double_t gridhx, rmg_double_t gridhy, rmg_double_t gridhz, rmg_double_t step, rmg_double_t k)
{
    Mgrid MG;
    MG.solv_pois<double>(vmat, fmat, work, dimx, dimy, dimz, gridhx, gridhy, gridhz, step, k);
}
