/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2016 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <locale.h>

#ifdef WIN32
#include <process.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(String) gettext(String)
#ifdef gettext_noop
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif
#else /* NLS is disabled */
#define _(String) (String)
#define N_(String) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(Domain,String) (String)
#define dcgettext(Domain,String,Type) (String)
#define bindtextdomain(Domain,Directory) (Domain)
#define bind_textdomain_codeset(Domain,Codeset) (Codeset)
#endif /* ENABLE_NLS */

#include "round.h"

const char *round_to_str(int round)
{
    static char buf[64];

    if (round == 0)
	return "";

    switch (round & ROUND_TYPE_MASK) {
    case 0:
	snprintf(buf, sizeof(buf), "%s %d", _("Round"), round & ROUND_MASK);
	return buf;
    case ROUND_ROBIN:
	return _("Round Robin");
    case ROUND_REPECHAGE:
	return _("Repechage");
    case ROUND_SEMIFINAL:
	if ((round & ROUND_UP_DOWN_MASK) == ROUND_UPPER)
            snprintf(buf, sizeof(buf), "%s 1", _("Semifinal"));
	else if ((round & ROUND_UP_DOWN_MASK) == ROUND_LOWER)
            snprintf(buf, sizeof(buf), "%s 2", _("Semifinal"));
	else
            snprintf(buf, sizeof(buf), "%s", _("Semifinal"));
	return buf;
    case ROUND_BRONZE:
	if ((round & ROUND_UP_DOWN_MASK) == ROUND_UPPER)
            snprintf(buf, sizeof(buf), "%s 1", _("Bronze"));
	else if ((round & ROUND_UP_DOWN_MASK) == ROUND_LOWER)
            snprintf(buf, sizeof(buf), "%s 2", _("Bronze"));
	else
            snprintf(buf, sizeof(buf), "%s", _("Bronze"));
	return buf;
    case ROUND_SILVER:
	return _("Silver medal match");
    case ROUND_FINAL:
	return _("Final");
    }
    return "";
}
