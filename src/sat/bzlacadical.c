/***
 * Bitwuzla: Satisfiability Modulo Theories (SMT) solver.
 *
 * This file is part of Bitwuzla.
 *
 * Copyright (C) 2007-2022 by the authors listed in the AUTHORS file.
 *
 * See COPYING for more information on using this software.
 */

#include "sat/bzlacadical.h"

#include "utils/bzlaabort.h"

/*------------------------------------------------------------------------*/
#ifdef BZLA_USE_CADICAL
/*------------------------------------------------------------------------*/

#include "bzlacore.h"
#include "ccadical.h"

static void *
init(BzlaSATMgr *smgr)
{
  (void) smgr;
  CCaDiCaL *slv = ccadical_init();
  if (smgr->inc_required
      && bzla_opt_get(smgr->bzla, BZLA_OPT_SAT_ENGINE_CADICAL_FREEZE))
  {
    ccadical_set_option(slv, "checkfrozen", 1);
  }
  ccadical_set_option(slv, "shrink", 0);

  if (bzla_opt_get(smgr->bzla, BZLA_OPT_SAT_ENGINE_CADICAL_DEBUG)) {
    ccadical_set_option(slv, "binary", 0);
    FILE *pf = fopen("tmp_proof", "w");
    if (ccadical_trace_proof(slv, pf, "tmp_proof")) {
      fprintf(stderr, "Proof tracing start.\n");
    }
  }

  if (bzla_opt_get(smgr->bzla, BZLA_OPT_SAT_ENGINE_CADICAL_SHUFFLE)) {
    ccadical_set_option(slv, "shuffle", 1);
    ccadical_set_option(slv, "shuffleinit", 1);
    ccadical_set_option(slv, "shufflerandom", 1);
  }

  ccadical_set_option(slv, "seed", bzla_rng_rand(smgr->bzla->rng));
  if (bzla_opt_get(smgr->bzla, BZLA_OPT_SAT_ENGINE_DECISION_WEIGHTING)) {
    ccadical_set_option(slv, "nodecidenobump", 1);
    // ccadical_set_option(slv, "decisiongroupshuffle", 1);
    /* ccadical_set_option(slv, "decompose", 0); */
    /* ccadical_set_option(slv, "shuffle", 1); */
  }

  if (bzla_opt_get(smgr->bzla, BZLA_OPT_SAT_ENGINE_POLARITY_INITIALIZATION)) {
    ccadical_set_option(slv, "lucky", 0);
  }

  if (getenv("BZLA_CADICAL_TEST") && !strcmp(getenv("BZLA_CADICAL_TEST"), "1")){
    // ccadical_set_option(slv, "hintsremoveonconflict", 1);
    // ccadical_set_option(slv, "reweightonconflict", 1);
    // ccadical_set_option(slv, "bumpdecision", 1);
    // ccadical_set_option(slv, "reweightonconflict", 1);

    ccadical_set_option(slv, "nobumpnondecisions", 1);
    // ccadical_set_option(slv, "autodecisiongroups", 1);
  }
  // ccadical_set_option(slv, "compact", 0);
  // ccadical_set_option(slv, "probeonlydecidable", 1);

  /* ccadical_set_option(slv, "rephaseint", 1e4); */

  if (getenv("BZLA_CADICAL_VERBOSE") && !strcmp(getenv("BZLA_CADICAL_VERBOSE"), "1")) {
    fprintf(stderr, "Cadical will run in verbose mode.\n");
    ccadical_set_option(slv, "verbose", 1);
  }

  // ccadical_set_option(slv, "chrono", 0);
  // ccadical_set_option(slv, "chronoreusetrail", 0);
  // ccadical_set_option(slv, "log", 1);


  return slv;
}

static void
add(BzlaSATMgr *smgr, int32_t lit)
{
  ccadical_add(smgr->solver, lit);
}

static void
assume(BzlaSATMgr *smgr, int32_t lit)
{
  ccadical_assume(smgr->solver, lit);
}

static int32_t
deref(BzlaSATMgr *smgr, int32_t lit)
{
  int32_t val;
  val = ccadical_deref(smgr->solver, lit);
  if (val > 0) return 1;
  if (val < 0) return -1;
  return 0;
}

static void
enable_verbosity(BzlaSATMgr *smgr, int32_t level)
{
  // if (level <= 1)
  //   ccadical_set_option(smgr->solver, "quiet", 1);
  // else if (level >= 2)
  //   ccadical_set_option(smgr->solver, "verbose", level - 2);
}

static int32_t
failed(BzlaSATMgr *smgr, int32_t lit)
{
  return ccadical_failed(smgr->solver, lit);
}

static void
reset(BzlaSATMgr *smgr)
{
  ccadical_reset(smgr->solver);
  smgr->solver = 0;
}

static int32_t
sat(BzlaSATMgr *smgr, int32_t limit)
{
  (void) limit;

  // For every SAT call, dump the source tracking info to source_tracking.txt
#ifdef BZLA_SOURCE_TRACKING
  if (getenv("BZLA_DUMP_SOURCE") && !strcmp(getenv("BZLA_DUMP_SOURCE"), "1")) {
    FILE *f = fopen("source_tracking.txt", "w");
    uint64_t *arr = smgr->bzla->avmgr->amgr->cnfid2source.start;
    for (int i=1; i < smgr->maxvar; i++) {
      fprintf(f, "%d %lx\n", i, arr[i]);
    }
    fclose(f);
  }
#endif
  return ccadical_sat(smgr->solver);
}

static void
setterm(BzlaSATMgr *smgr)
{
  /* for CaDiCaL, state is the first argument (unlike, e.g., Lingeling) */
  ccadical_set_terminate(smgr->solver, smgr->term.state, smgr->term.fun);
}

/*------------------------------------------------------------------------*/
/* incremental API                                                        */
/*------------------------------------------------------------------------*/

static int32_t
inc_max_var(BzlaSATMgr *smgr)
{
  int32_t var = smgr->maxvar + 1;
  if (smgr->inc_required)
  {
    ccadical_freeze(smgr->solver, var);
  }
  return var;
}

static void
melt(BzlaSATMgr *smgr, int32_t lit)
{
  if (smgr->inc_required) ccadical_melt(smgr->solver, lit);
}

/*------------------------------------------------------------------------*/

bool
bzla_sat_enable_cadical(BzlaSATMgr *smgr)
{
  assert(smgr != NULL);

  BZLA_ABORT(smgr->initialized,
             "'bzla_sat_init' called before 'bzla_sat_enable_cadical'");

  smgr->name = "CaDiCaL";

  BZLA_CLR(&smgr->api);
  smgr->api.add              = add;
  smgr->api.assume           = assume;
  smgr->api.deref            = deref;
  smgr->api.enable_verbosity = enable_verbosity;
  smgr->api.failed           = failed;
  smgr->api.fixed            = 0;
  smgr->api.inc_max_var      = 0;
  smgr->api.init             = init;
  smgr->api.melt             = 0;
  smgr->api.repr             = 0;
  smgr->api.reset            = reset;
  smgr->api.sat              = sat;
  smgr->api.set_output       = 0;
  smgr->api.set_prefix       = 0;
  smgr->api.stats            = 0;
  smgr->api.setterm          = setterm;

  if (bzla_opt_get(smgr->bzla, BZLA_OPT_SAT_ENGINE_CADICAL_FREEZE))
  {
    smgr->api.inc_max_var = inc_max_var;
    smgr->api.melt        = melt;
  }
  else
  {
    smgr->have_restore = true;
  }

  return true;
}

/*------------------------------------------------------------------------*/
#endif
/*------------------------------------------------------------------------*/
