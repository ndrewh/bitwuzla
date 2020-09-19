/*  Boolector: Satisfiability Modulo Theories (SMT) solver.
 *
 *  Copyright (C) 2007-2009 Robert Daniel Brummayer.
 *  Copyright (C) 2007-2014 Armin Biere.
 *  Copyright (C) 2012-2018 Aina Niemetz.
 *  Copyright (C) 2012-2020 Mathias Preiner.
 *
 *  This file is part of Boolector.
 *  See COPYING for more information on using this software.
 */

#include "bzlaparse.h"

#include <ctype.h>

#include "boolector.h"
#include "bzlacore.h"
#include "bzlaopt.h"
#include "parser/bzlabtor.h"
#include "parser/bzlabtor2.h"
#include "parser/bzlasmt.h"
#include "parser/bzlasmt2.h"
#include "utils/bzlamem.h"
#include "utils/bzlastack.h"

static bool
has_compressed_suffix(const char *str, const char *suffix)
{
  int32_t l = strlen(str), k = strlen(suffix), d = l - k;
  if (d < 0) return 0;
  if (!strcmp(str + d, suffix)) return 1;
  if (d - 3 >= 0 && !strcmp(str + l - 3, ".gz") && !strcmp(str + l - 3, ".7z")
      && !strncmp(str + d - 3, suffix, k))
    return 1;
  return 0;
}

/* return BITWUZLA_(SAT|UNSAT|UNKNOWN|PARSE_ERROR) */
static int32_t
parse_aux(Bzla *bzla,
          FILE *infile,
          BzlaCharStack *prefix,
          const char *infile_name,
          FILE *outfile,
          const BzlaParserAPI *parser_api,
          char **error_msg,
          int32_t *status,
          char *msg)
{
  assert(bzla);
  assert(infile);
  assert(infile_name);
  assert(outfile);
  assert(parser_api);
  assert(error_msg);
  assert(status);

  BzlaParser *parser;
  BzlaParseResult parse_res;
  int32_t res;
  char *emsg;

  res        = BOOLECTOR_UNKNOWN;
  *error_msg = 0;

  BZLA_MSG(bzla->msg, 1, "%s", msg);
  parser = parser_api->init(bzla);

  if ((emsg = parser_api->parse(
           parser, prefix, infile, infile_name, outfile, &parse_res)))
  {
    res                   = BOOLECTOR_PARSE_ERROR;
    bzla->parse_error_msg = bzla_mem_strdup(bzla->mm, emsg);
    *error_msg            = bzla->parse_error_msg;
  }
  else
  {
    res = parse_res.nsatcalls ? parse_res.result : BOOLECTOR_PARSE_UNKNOWN;

    if (parse_res.logic == BZLA_LOGIC_QF_BV)
      BZLA_MSG(bzla->msg, 1, "logic QF_BV");
    else if (parse_res.logic == BZLA_LOGIC_BV)
      BZLA_MSG(bzla->msg, 1, "logic BV");
    else if (parse_res.logic == BZLA_LOGIC_QF_UFBV)
      BZLA_MSG(bzla->msg, 1, "logic QF_UFBV");
    else if (parse_res.logic == BZLA_LOGIC_QF_ABV)
      BZLA_MSG(bzla->msg, 1, "logic QF_ABV");
    else
    {
      assert(parse_res.logic == BZLA_LOGIC_QF_AUFBV);
      BZLA_MSG(bzla->msg, 1, "logic QF_AUFBV");
    }

    if (parse_res.status == BOOLECTOR_SAT)
      BZLA_MSG(bzla->msg, 1, "status sat");
    else if (parse_res.status == BOOLECTOR_UNSAT)
      BZLA_MSG(bzla->msg, 1, "status unsat");
    else
    {
      assert(parse_res.status == BOOLECTOR_UNKNOWN);
      BZLA_MSG(bzla->msg, 1, "status unknown");
    }
  }

  if (status) *status = parse_res.status;

  /* cleanup */
  parser_api->reset(parser);

  return res;
}

int32_t
bzla_parse(Bzla *bzla,
           FILE *infile,
           const char *infile_name,
           FILE *outfile,
           char **error_msg,
           int32_t *status,
           bool *parsed_smt2)
{
  assert(bzla);
  assert(infile);
  assert(infile_name);
  assert(outfile);
  assert(error_msg);
  assert(status);
  assert(parsed_smt2);

  const BzlaParserAPI *parser_api;
  int32_t idx, first, second, res;
  uint32_t len;
  char ch, *msg;
  BzlaCharStack prefix;
  BzlaMemMgr *mem;

  idx = 0;
  len = 40 + strlen(infile_name);
  BZLA_NEWN(bzla->mm, msg, len);
  mem = bzla_mem_mgr_new();
  BZLA_INIT_STACK(mem, prefix);
  *parsed_smt2 = false;

  if (has_compressed_suffix(infile_name, ".btor"))
  {
    parser_api = bzla_parsebzla_parser_api();
    sprintf(msg, "parsing '%s'", infile_name);
  }
  if (has_compressed_suffix(infile_name, ".btor2"))
  {
    parser_api = bzla_parsebtor2_parser_api();
    sprintf(msg, "parsing '%s'", infile_name);
  }
  else if (has_compressed_suffix(infile_name, ".smt2"))
  {
    parser_api = bzla_parsesmt2_parser_api();
    sprintf(msg, "parsing '%s'", infile_name);
    *parsed_smt2 = true;
  }
  else
  {
    first = second = 0;
    parser_api     = bzla_parsebzla_parser_api();
    sprintf(msg, "assuming BTOR input, parsing '%s'", infile_name);
    for (;;)
    {
      ch = getc(infile);
      BZLA_PUSH_STACK(prefix, ch);
      if (!ch || ch == EOF) break;
      if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
      {
        BZLA_PUSH_STACK(prefix, ch);
      }
      else if (ch == ';')
      {
        BZLA_PUSH_STACK(prefix, ';');
        do
        {
          ch = getc(infile);
          if (ch == EOF) break;
          BZLA_PUSH_STACK(prefix, ch);
        } while (ch != '\n');
        if (ch == EOF) break;
      }
      else if (!first)
      {
        first = ch;
        idx   = BZLA_COUNT_STACK(prefix) - 1;
      }
      else
      {
        second = ch;
        break;
      }
    }

    if (ch != EOF && ch)
    {
      assert(first && second);
      if (first == '(')
      {
        if (second == 'b')
        {
          parser_api = bzla_parsesmt_parser_api();
          sprintf(msg, "assuming SMT-LIB v1 input,  parsing '%s'", infile_name);
        }
        else
        {
          parser_api   = bzla_parsesmt2_parser_api();
          *parsed_smt2 = true;
          sprintf(msg, "assuming SMT-LIB v2 input,  parsing '%s'", infile_name);
        }
      }
      else
      {
        do
        {
          ch = getc(infile);
          if (ch == EOF) break;
          BZLA_PUSH_STACK(prefix, ch);
        } while (ch != '\n');
        BZLA_PUSH_STACK(prefix, 0);
        if (strstr(prefix.start + idx, " sort ") != NULL)
        {
          parser_api = bzla_parsebtor2_parser_api();
          sprintf(msg, "assuming BTOR2 input,  parsing '%s'", infile_name);
        }
        (void) BZLA_POP_STACK(prefix);
      }
    }
  }

  res = parse_aux(bzla,
                  infile,
                  &prefix,
                  infile_name,
                  outfile,
                  parser_api,
                  error_msg,
                  status,
                  msg);

  /* cleanup */
  BZLA_RELEASE_STACK(prefix);
  bzla_mem_mgr_delete(mem);
  BZLA_DELETEN(bzla->mm, msg, len);

  return res;
}

int32_t
bzla_parse_btor(Bzla *bzla,
                FILE *infile,
                const char *infile_name,
                FILE *outfile,
                char **error_msg,
                int32_t *status)
{
  assert(bzla);
  assert(infile);
  assert(infile_name);
  assert(outfile);
  assert(error_msg);
  assert(status);

  const BzlaParserAPI *parser_api;
  parser_api = bzla_parsebzla_parser_api();
  return parse_aux(
      bzla, infile, 0, infile_name, outfile, parser_api, error_msg, status, 0);
}

int32_t
bzla_parse_btor2(Bzla *bzla,
                 FILE *infile,
                 const char *infile_name,
                 FILE *outfile,
                 char **error_msg,
                 int32_t *status)
{
  assert(bzla);
  assert(infile);
  assert(infile_name);
  assert(outfile);
  assert(error_msg);
  assert(status);

  const BzlaParserAPI *parser_api;
  parser_api = bzla_parsebtor2_parser_api();
  return parse_aux(
      bzla, infile, 0, infile_name, outfile, parser_api, error_msg, status, 0);
}

int32_t
bzla_parse_smt1(Bzla *bzla,
                FILE *infile,
                const char *infile_name,
                FILE *outfile,
                char **error_msg,
                int32_t *status)
{
  assert(bzla);
  assert(infile);
  assert(infile_name);
  assert(outfile);
  assert(error_msg);
  assert(status);

  const BzlaParserAPI *parser_api;
  parser_api = bzla_parsesmt_parser_api();
  return parse_aux(
      bzla, infile, 0, infile_name, outfile, parser_api, error_msg, status, 0);
}

int32_t
bzla_parse_smt2(Bzla *bzla,
                FILE *infile,
                const char *infile_name,
                FILE *outfile,
                char **error_msg,
                int32_t *status)
{
  assert(bzla);
  assert(infile);
  assert(infile_name);
  assert(outfile);
  assert(error_msg);
  assert(status);

  const BzlaParserAPI *parser_api;
  parser_api = bzla_parsesmt2_parser_api();
  return parse_aux(
      bzla, infile, 0, infile_name, outfile, parser_api, error_msg, status, 0);
}
