#ifndef RMG_FiniteDiff_H
#define RMG_FiniteDiff_H 1

/* Order of finite differencing for driver routines */
#define APP_CI_FOURTH 4
#define APP_CI_SIXTH 6

#ifdef __cplusplus

#include "rmg_error.h"
class FiniteDiff : public RmgError {

private:

public:
    template <typename RmgType>
    rmg_double_t app_cil_sixth_standard (RmgType *rptr, RmgType *b, int dimx, int dimy, int dimz, rmg_double_t gridhx, rmg_double_t gridhy, rmg_double_t gridhz);

    template <typename RmgType>
    rmg_double_t app_cil_sixth_global (RmgType * rptr, RmgType * b, rmg_double_t gridhx, rmg_double_t gridhy, rmg_double_t gridhz);

    template <typename RmgType>
    void app_cir_sixth_standard (RmgType * rptr, RmgType * b, int dimx, int dimy, int dimz);

    template <typename RmgType>
    void app_cir_sixth_global (RmgType * rptr, RmgType * b);

    template <typename RmgType>
    rmg_double_t app_del2c (RmgType * a, RmgType * b, int dimx, int dimy, int dimz, rmg_double_t gridhx, rmg_double_t gridhy, rmg_double_t gridhz);

    template <typename RmgType>
    rmg_double_t app6_del2 (RmgType * a, RmgType * b, int dimx, int dimy, int dimz, rmg_double_t gridhx, rmg_double_t gridhy, rmg_double_t gridhz);

    template <typename RmgType>
    rmg_double_t app_cil_fourth_global (RmgType * rptr, RmgType * b, rmg_double_t gridhx, rmg_double_t gridhy, rmg_double_t gridhz);

    template <typename RmgType>
    rmg_double_t app_cil_fourth_standard (RmgType * rptr, RmgType * b, int dimx, int dimy, int dimz, rmg_double_t gridhx, rmg_double_t gridhy, rmg_double_t gridhz);

    template <typename RmgType>
    void app_cir_fourth_global (RmgType * rptr, RmgType * b);

    template <typename RmgType>
    void app_cir_fourth_standard (RmgType * rptr, RmgType * b, int dimx, int dimy, int dimz);
};
#endif

#endif
