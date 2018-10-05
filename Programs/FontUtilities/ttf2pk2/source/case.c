/*
 *   case.c
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>         /* for definition of NULL */

#include "case.h"


/* This is a table of uppercase/lowercase pairs which can't be deduced */
/* by lowercasing the uppercase Adobe glyph name.                      */
/* It has been extracted from the ucs2ps.um database compiled by       */
/* Bjorn Brox <brox@corena.no>.                                        */

Case casetable[] =
{
  {"afii10017",    "afii10065"},
  {"afii10018",    "afii10066"},
  {"afii10019",    "afii10067"},
  {"afii10020",    "afii10068"},
  {"afii10021",    "afii10069"},
  {"afii10022",    "afii10070"},
  {"afii10023",    "afii10071"},
  {"afii10024",    "afii10072"},
  {"afii10025",    "afii10073"},
  {"afii10026",    "afii10074"},
  {"afii10027",    "afii10075"},
  {"afii10028",    "afii10076"},
  {"afii10029",    "afii10077"},
  {"afii10030",    "afii10078"},
  {"afii10031",    "afii10079"},
  {"afii10032",    "afii10080"},
  {"afii10033",    "afii10081"},
  {"afii10034",    "afii10082"},
  {"afii10035",    "afii10083"},
  {"afii10036",    "afii10084"},
  {"afii10037",    "afii10085"},
  {"afii10038",    "afii10086"},
  {"afii10039",    "afii10087"},
  {"afii10040",    "afii10088"},
  {"afii10041",    "afii10089"},
  {"afii10042",    "afii10090"},
  {"afii10043",    "afii10091"},
  {"afii10044",    "afii10092"},
  {"afii10045",    "afii10093"},
  {"afii10046",    "afii10094"},
  {"afii10047",    "afii10095"},
  {"afii10048",    "afii10096"},
  {"afii10049",    "afii10097"},
  {"afii10050",    "afii10098"},
  {"afii10051",    "afii10099"},
  {"afii10052",    "afii10100"},
  {"afii10053",    "afii10101"},
  {"afii10054",    "afii10102"},
  {"afii10055",    "afii10103"},
  {"afii10056",    "afii10104"},
  {"afii10057",    "afii10105"},
  {"afii10058",    "afii10106"},
  {"afii10059",    "afii10107"},
  {"afii10060",    "afii10108"},
  {"afii10061",    "afii10109"},
  {"afii10062",    "afii10110"},
  {"afii10145",    "afii10193"},
  {"afii10146",    "afii10194"},
  {"afii10147",    "afii10195"},
  {"afii10148",    "afii10196"},
  {"afii10149",    "afii10197"},
  {"afii10152",    "afii10200"},
  {"afii10202",    "afii10154"},
  {"afii10155",    "afii10203"},
  {"afii10156",    "afii10204"},
  {"afii10157",    "afii10205"},
  {"afii10158",    "afii10206"},
  {"afii10160",    "afii10208"},
  {"afii10161",    "afii10209"},
  {"afii10162",    "afii10210"},
  {"afii10163",    "afii10211"},
  {"afii10164",    "afii10212"},
  {"afii10166",    "afii10214"},
  {"afii10167",    "afii10215"},
  {"afii10168",    "afii10216"},
  {"afii10170",    "afii10218"},
  {"afii10171",    "afii10219"},
  {"afii10172",    "afii10220"},
  {"afii10173",    "afii10221"},
  {"afii10174",    "afii10222"},
  {"afii10176",    "afii10224"},
  {"afii10178",    "afii10226"},
  {"afii10179",    "afii10227"},
  {"afii10181",    "afii10229"},
  {"afii10182",    "afii10230"},
  {"afii10184",    "afii10232"},
  {"afii10185",    "afii10233"},
  {"afii10187",    "afii10235"},
  {"afii10188",    "afii10236"},
  {"afii10190",    "afii10238"},
  {"afii10785",    "afii10833"},
  {"afii10786",    "afii10834"},
  {"afii10787",    "afii10835"},
  {"afii10795",    "afii10843"},
  {"afii10798",    "afii10846"},
  {"afii10799",    "afii10847"},
  {"afii10800",    "afii10848"},
  {"afii10801",    "afii10849"},
  {"afii64308",    "afii64436"},
  {"afii10803",    "afii10851"},
  {"afii10808",    "afii10856"},
  {"afii10809",    "afii10857"},
  {"afii10810",    "afii10858"},
  {"afii10811",    "afii10859"},
  {"afii10817",    "afii10865"},
  {"afii10818",    "afii10866"},
  {"afii10819",    "afii10867"},
  {"afii10822",    "afii10870"},
  {"afii10827",    "afii10875"},
  {"afii10914",    "afii10962"},
  {"afii10920",    "afii10968"},
  {"afii10924",    "afii10972"},
  {"afii10927",    "afii10975"},
  {"afii10929",    "afii10977"},
  {"afii10930",    "afii10978"},
  {"afii10931",    "afii10979"},
  {"afii10932",    "afii10980"},
  {"afii10934",    "afii10982"},
  {"afii10943",    "afii10991"},
  {"afii10944",    "afii10992"},
  {"afii10951",    "afii10967"},
/*{?               "ash"}          */
  {"Beta",         "beta1"},
  {"Bhook",        "bhooktop"},
  {"Chook",        "chooktop"},
/*{"Oopen"         "cturn" ?}      */
  {"Dbar1",        "dbar"},
  {"Dhook",        "dhooktop"},
  {"Dmacron",      "dmacron3"},
  {"Dslash",       "dmacron"},
  {"I",            "dotlessi"},
  {"J",            "dotlessj"},
/*{"Dbar",         "drighttail" ?} */
/*{"Dbar"          "drthook" ?}    */
/*{"Dslash",       ?}              */
  {"Fhook",        "fscript"},
  {"Ghook",        "ghooktop"},
  {"Ibar",         "ibarred"},
  {"I",            "iundotted"},
  {"Kappa",        "kappa1"},
  {"Khook",        "khooktop"},
  {"S",            "longs"},
  {"mcapturn",     "mturn"},
  {"mcapturn",     "mturned"},
  {"Mu",           "mu1"},
  {"Nhook",        "nlefthookatleft"},
  {"Nhook",        "nlftlfthook"},
  {"Obar",         "obarred"},
  {"Pi",           "omega1"},
  {"Phi",          "phi1"},
  {"Phi",          "philatin"},
  {"Pi",           "pi1"},
  {"Rho",          "rho1"},
  {"Sigma",        "sigma1"},
  {"Sigma",        "sigmafinal"},
  {"Sigma",        "sigmalunate"},
  {"S",            "slong"},
  {"Theta",        "theta1"},
  {"Thook",        "thooktop"},
  {"Trthook",      "trighttail"},
  {"Upsilon2",     "upsilon"},
  {"Vcursive",     "vscript"},
  {NULL,           NULL}
};


/* end */
