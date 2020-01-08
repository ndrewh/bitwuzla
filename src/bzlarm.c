/*  Boolector: Satisfiability Modulo Theories (SMT) solver.
 *
 *  Copyright (C) 2019 Aina Niemetz.
 *
 *  This file is part of Boolector.
 *  See COPYING for more information on using this software.
 */

#include "bzlarm.h"

#include "assert.h"

uint32_t
bzla_rm_hash(const BzlaRoundingMode rm)
{
  assert(rm == BZLA_RM_RNA || rm == BZLA_RM_RNE || rm == BZLA_RM_RTN
         || rm == BZLA_RM_RTP || rm == BZLA_RM_RTZ);
  return rm;
}

bool
bzla_rm_is_valid(uint32_t rm)
{
  return rm < BZLA_RM_MAX;
}