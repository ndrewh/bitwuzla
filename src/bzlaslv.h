/*  Boolector: Satisfiability Modulo Theories (SMT) solver.
 *
 *  Copyright (C) 2015-2016 Aina Niemetz.
 *  Copyright (C) 2015 Mathias Preiner.
 *
 *  This file is part of Boolector.
 *  See COPYING for more information on using this software.
 */

#ifndef BZLASLV_H_INCLUDED
#define BZLASLV_H_INCLUDED

#include <stdbool.h>
#include <stdio.h>

#include "bzlatypes.h"
#include "utils/bzlamem.h"
#include "utils/bzlanodemap.h"

enum BzlaSolverKind
{
  BZLA_FUN_SOLVER_KIND,
  BZLA_SLS_SOLVER_KIND,
  BZLA_PROP_SOLVER_KIND,
  BZLA_AIGPROP_SOLVER_KIND,
  BZLA_QUANT_SOLVER_KIND,
};
typedef enum BzlaSolverKind BzlaSolverKind;

typedef struct BzlaSolver *(*BzlaSolverClone)(Bzla *,
                                              struct BzlaSolver *,
                                              BzlaNodeMap *);
typedef void (*BzlaSolverDelete)(struct BzlaSolver *);
typedef BzlaSolverResult (*BzlaSolverSat)(struct BzlaSolver *);
typedef void (*BzlaSolverGenerateModel)(struct BzlaSolver *, bool, bool);
typedef void (*BzlaSolverPrintStats)(struct BzlaSolver *);
typedef void (*BzlaSolverPrintTimeStats)(struct BzlaSolver *);
typedef void (*BzlaSolverPrintModel)(struct BzlaSolver *,
                                     const char *format,
                                     FILE *file);

#define BZLA_SOLVER_STRUCT                       \
  struct                                         \
  {                                              \
    BzlaSolverKind kind;                         \
    Bzla *bzla;                                  \
    struct                                       \
    {                                            \
      BzlaSolverClone clone;                     \
      BzlaSolverDelete delet;                    \
      BzlaSolverSat sat;                         \
      BzlaSolverGenerateModel generate_model;    \
      BzlaSolverPrintStats print_stats;          \
      BzlaSolverPrintTimeStats print_time_stats; \
      BzlaSolverPrintModel print_model;          \
    } api;                                       \
  }

struct BzlaSolver
{
  BZLA_SOLVER_STRUCT;
};
typedef struct BzlaSolver BzlaSolver;

#endif
