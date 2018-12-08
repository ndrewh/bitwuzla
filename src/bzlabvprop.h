/*  Boolector: Satisfiability Modulo Theories (SMT) solver.
 *
 *  Copyright (C) 2018 Mathias Preiner.
 *  Copyright (C) 2018 Aina Niemetz.
 *
 *  This file is part of Boolector.
 *  See COPYING for more information on using this software.
 */

#ifndef BZLABVPROP_H_INCLUDED
#define BZLABVPROP_H_INCLUDED

#include "bzlabv.h"

struct BzlaBvDomain
{
  BzlaBitVector *lo;
  BzlaBitVector *hi;
};

typedef struct BzlaBvDomain BzlaBvDomain;

/* Create new bit-vector domain of width 'width' with low 0 and high ~0. */
BzlaBvDomain *bzla_bvprop_new_init(BzlaMemMgr *mm, uint32_t width);

/* Create new bit-vector domain with low 'lo' and high 'hi'.
 * Creates copies of lo and hi. */
BzlaBvDomain *bzla_bvprop_new(BzlaMemMgr *mm,
                              const BzlaBitVector *lo,
                              const BzlaBitVector *hi);

/* Delete bit-vector domain. */
void bzla_bvprop_free(BzlaMemMgr *mm, BzlaBvDomain *d);

/* Check whether bit-vector domain is valid, i.e., ~lo | hi == ones. */
bool bzla_bvprop_is_valid(BzlaMemMgr *mm, const BzlaBvDomain *d);

/* Check whether bit-vector domain is fixed, i.e., lo == hi */
bool bzla_bvprop_is_fixed(BzlaMemMgr *mm, const BzlaBvDomain *d);

/* Propagate domains 'd_x', 'd_y', and 'd_z' of z = (x = y).  */
bool bzla_bvprop_eq(BzlaMemMgr *mm,
                    BzlaBvDomain *d_x,
                    BzlaBvDomain *d_y,
                    BzlaBvDomain *d_z,
                    BzlaBvDomain **res_d_x,
                    BzlaBvDomain **res_d_y,
                    BzlaBvDomain **res_d_z);

/* Propagate domains 'd_x' and 'd_z' of z = ~x. */
bool bzla_bvprop_not(BzlaMemMgr *mm,
                     BzlaBvDomain *d_x,
                     BzlaBvDomain *d_z,
                     BzlaBvDomain **res_d_x,
                     BzlaBvDomain **res_d_z);

/* Propagate domains 'd_x' and 'd_z' of z = x << n where n is const. */
bool bzla_bvprop_sll_const(BzlaMemMgr *mm,
                           BzlaBvDomain *d_x,
                           BzlaBvDomain *d_z,
                           BzlaBitVector *n,
                           BzlaBvDomain **res_d_x,
                           BzlaBvDomain **res_d_z);

/* Propagate domains 'd_x' and 'd_z' of z = x >> n where n is const. */
bool bzla_bvprop_srl_const(BzlaMemMgr *mm,
                           BzlaBvDomain *d_x,
                           BzlaBvDomain *d_z,
                           BzlaBitVector *n,
                           BzlaBvDomain **res_d_x,
                           BzlaBvDomain **res_d_z);

/* Propagate domains 'd_x', 'd_y' and 'd_z' of z = x & y. */
bool bzla_bvprop_and(BzlaMemMgr *mm,
                     BzlaBvDomain *d_x,
                     BzlaBvDomain *d_y,
                     BzlaBvDomain *d_z,
                     BzlaBvDomain **res_d_x,
                     BzlaBvDomain **res_d_y,
                     BzlaBvDomain **res_d_z);

/* Propagate domains 'd_x' and 'd_z' of z = x << y where y is not const.
 * Note: bw(y) = log_2 bw(y). */
bool bzla_bvprop_sll(BzlaMemMgr *mm,
                     BzlaBvDomain *d_x,
                     BzlaBvDomain *d_y,
                     BzlaBvDomain *d_z,
                     BzlaBvDomain **res_d_x,
                     BzlaBvDomain **res_d_y,
                     BzlaBvDomain **res_d_z);

/* Propagate domains 'd_x' and 'd_z' of z = x >> y where y is not const.
 * Note: bw(y) = log_2 bw(y). */
bool bzla_bvprop_srl(BzlaMemMgr *mm,
                     BzlaBvDomain *d_x,
                     BzlaBvDomain *d_y,
                     BzlaBvDomain *d_z,
                     BzlaBvDomain **res_d_x,
                     BzlaBvDomain **res_d_y,
                     BzlaBvDomain **res_d_z);

/* Propagate domains 'd_x', 'd_y' and 'd_z' of z = x | y. */
bool bzla_bvprop_or(BzlaMemMgr *mm,
                    BzlaBvDomain *d_x,
                    BzlaBvDomain *d_y,
                    BzlaBvDomain *d_z,
                    BzlaBvDomain **res_d_x,
                    BzlaBvDomain **res_d_y,
                    BzlaBvDomain **res_d_z);

/* Propagate domains 'd_x', 'd_y' and 'd_z' of z = x | y. */
bool bzla_bvprop_xor(BzlaMemMgr *mm,
                     BzlaBvDomain *d_x,
                     BzlaBvDomain *d_y,
                     BzlaBvDomain *d_z,
                     BzlaBvDomain **res_d_x,
                     BzlaBvDomain **res_d_y,
                     BzlaBvDomain **res_d_z);

/* Propagate domains 'd_x' and 'd_z' of z = x[upper:lower]. */
bool bzla_bvprop_slice(BzlaMemMgr *mm,
                       BzlaBvDomain *d_x,
                       BzlaBvDomain *d_z,
                       uint32_t upper,
                       uint32_t lower,
                       BzlaBvDomain **res_d_x,
                       BzlaBvDomain **res_d_z);

/* Propagate domains 'd_x', 'd_y' and 'd_z' of z = x o y. */
bool bzla_bvprop_concat(BzlaMemMgr *mm,
                        BzlaBvDomain *d_x,
                        BzlaBvDomain *d_y,
                        BzlaBvDomain *d_z,
                        BzlaBvDomain **res_d_y,
                        BzlaBvDomain **res_d_x,
                        BzlaBvDomain **res_d_z);

/* Propagate domains 'd_x' and 'd_z' of z = sext(x, n). */
bool bzla_bvprop_sext(BzlaMemMgr *mm,
                      BzlaBvDomain *d_x,
                      BzlaBvDomain *d_z,
                      BzlaBvDomain **res_d_x,
                      BzlaBvDomain **res_d_z);

/* Propagate domains 'd_c', 'd_x', 'd_y' and 'd_z' of z = ite(c, x, y). */
bool bzla_bvprop_ite(BzlaMemMgr *mm,
                     BzlaBvDomain *d_c,
                     BzlaBvDomain *d_x,
                     BzlaBvDomain *d_y,
                     BzlaBvDomain *d_z,
                     BzlaBvDomain **res_d_c,
                     BzlaBvDomain **res_d_x,
                     BzlaBvDomain **res_d_y,
                     BzlaBvDomain **res_d_z);

/* Propagate domains 'd_x', 'd_y' and 'd_z' of z = x + y. */
bool bzla_bvprop_add(BzlaMemMgr *mm,
                     BzlaBvDomain *d_x,
                     BzlaBvDomain *d_y,
                     BzlaBvDomain *d_z,
                     BzlaBvDomain **res_d_y,
                     BzlaBvDomain **res_d_x,
                     BzlaBvDomain **res_d_z);

/**
 * Propagate domains 'd_x', 'd_y' and 'd_z' of z = x + y where + does not
 * overflow.
 */
bool bzla_bvprop_add_aux(BzlaMemMgr *mm,
                         BzlaBvDomain *d_x,
                         BzlaBvDomain *d_y,
                         BzlaBvDomain *d_z,
                         BzlaBvDomain **res_d_x,
                         BzlaBvDomain **res_d_y,
                         BzlaBvDomain **res_d_z,
                         bool no_overflows);

/* Propagate domains 'd_x', 'd_y' and 'd_z' of z = x * y. */
bool bzla_bvprop_mul(BzlaMemMgr *mm,
                     BzlaBvDomain *d_x,
                     BzlaBvDomain *d_y,
                     BzlaBvDomain *d_z,
                     BzlaBvDomain **res_d_x,
                     BzlaBvDomain **res_d_y,
                     BzlaBvDomain **res_d_z);

/* Propagate domains 'd_x', 'd_y' and 'd_z' of z = x * y where * does not
 * overflow. */
bool bzla_bvprop_mul_aux(BzlaMemMgr *mm,
                         BzlaBvDomain *d_x,
                         BzlaBvDomain *d_y,
                         BzlaBvDomain *d_z,
                         BzlaBvDomain **res_d_x,
                         BzlaBvDomain **res_d_y,
                         BzlaBvDomain **res_d_z,
                         bool no_overflows);

/* Propagate domains 'd_x', 'd_y' and 'd_z' of z = x < y (unsigned lt). */
bool bzla_bvprop_ult(BzlaMemMgr *mm,
                     BzlaBvDomain *d_x,
                     BzlaBvDomain *d_y,
                     BzlaBvDomain *d_z,
                     BzlaBvDomain **res_d_x,
                     BzlaBvDomain **res_d_y,
                     BzlaBvDomain **res_d_z);

// TODO:
// propagators:
//
// z = x udiv y
// z = x urem y
#endif
