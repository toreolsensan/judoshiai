%option noyywrap
%{

/*----------------------------------------------------------------------*/
/*  MOLE BASIC                                                          */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*        file: basic.l                                                 */
/*    language: flex & ANSI C                                           */ 
/*     program: MOLE BASIC                                              */
/*                                                                      */
/* --- Description -----------------------------------------------------*/
/* This is the lex part of the basic interperter.                       */
/*                                                                      */
/* --- Version information ---------------------------------------------*/
/* 0.1  1999-02-12  first version, no boundary checks and still buggy   */
/* 0.3  1999-02-20  Finaly a working version, still beta though.        */
/* 0.5  1999-02-26  First publicly released beta version.               */
/* 0.7  1999-02-29  Jumping changed drasticly, some old ignoring stuff  */
/*                  removed or changed. DO..UNTIL added.                */
/*                                                                      */
/* --- Note ------------------------------------------------------------*/
/*                                                                      */
/* Althought it isn't required, let me know if you are using this       */
/* in a commercial product or altered the source code. Don't            */
/* hestitate to send extensions, changes and bug(-fixes).               */
/* If it's usable and well tested, I'll take it into the the next       */
/* release.                                                             */
/*                                                                      */
/* Remco Schellekens merty@xs4all.nl                                    */
/* http://www.xs4all.nl/~merty/mole/index.html                          */
/* --- Credits ---------------------------------------------------------*/
/*                                                                      */
/* Thanks to the critics of the 1st beta: pasQuale Krul and all         */
/* co-workers of the P44 development team of IBM Global Services.       */
/*                                                                      */
/* --- Copyright -------------------------------------------------------*/
/*                                                                      */
/* Copyright (C) 1999  Remco Schellekens (merty@xs4all.nl)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation; either version 2 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU General Public License for more details.                         */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with this program; if not, write to the Free Software          */
/* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/* 2011-05-27 Modified for JudoShiai purposes by Hannu Jokinen.         */
/*                                                                      */
/*----------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "basic_defs.h"
#include "basic.tab.h"

#ifdef  DEBUG_ALL
#define STATE_DEBUG
#define RULE_DEBUG
#define YACC_DEBUG
#endif

#define NOTHING            0
#define LNUM_STATE         1
#define COMMAND_STATE      2
#define EXPRESSION_STATE   4
#define JUMP_LNUM_STATE    8

/* some flags */
int ignore_end=0;
int thenstart=0;
int cmdcnt=0;
int did_jump=0;

int state=LNUM_STATE;
int result;

/* just change state, this function is called to provida a debug hook */

void chstate(int newstate) 
{
    state=newstate;
#ifdef STATE_DEBUG    
    switch(state) {
        case  1: {fprintf(stderr,"state changed to LNUM_STATE\n");break;}
        case  2: {fprintf(stderr,"state changed to COMMAND_STATE\n");break;}
        case  4: {fprintf(stderr,"state changed to EXPRESSION_STATE\n");break;}
        case  8: {fprintf(stderr,"state changed to JUMP_LNUM_STATE\n");break;}
    }
#endif
}

int statef(int flags) 
{
    return (state&flags)?1:0;
}

/* this is wat most commands needs: result code to yacc, state it has to be */
/* and state it will be after change                                        */

int do_cmd(int command,int from_state,int to_state)
{
#ifdef RULE_DEBUG
    fprintf(stderr,"checking rule %d SP\n",command);
#endif
    if (statef(from_state)) {
        chstate(to_state);
#ifdef YACC_DEBUG
        fprintf(stderr,"sending yacc %d\n",command);
#endif
        return command;
    } 
    return -1;
}

%}

LNUMBERX Xyts543
SP [ ]*
LNUMBER [1-9][0-9]*
NUMBER ([0-9]+|([0-9]*\.[0-9]+)([eE][-+]?[0-9]+)?)
ALPHA [a-zA-Z]
VARIABLE [a-zA-Z][a-zA-Z0-9]*
STRING [a-zA-Z][a-zA-Z0-9]*\$
NL \n
SEMICOLON \;
DQUOTE \"
COLON \:
HEXNUMBER \&[Hh][0-9a-zA-Z]+
OCTNUMBER \&[Oo][0-9a-zA-Z]+

AS        [Aa][Ss]
ABS       [Aa][Bb][Ss]\(
ACOS      [Aa][Cc][Oo][Ss]\(
AND       [Aa][Nn][Dd]
APPEND    [Aa][Pp][Pp][Ee][Nn][Dd]
ASIN      [Aa][Ss][Ii][Nn]
ASC       [Aa][Ss][Cc]\(
ATN       [Aa][Tt][Nn]\(
BEEP      [Bb][Ee][Ee][Pp]
CHR       [Cc][Hh][Rr]
CHDIR     [Cc][Hh][Dd][Ii][Rr]
CINT      [Cc][Ii][Nn][Tt]\(
CLS       [Cc][Ll][Ss]
CLOSE     [Cc][Ll][Oo][Ss][Ee]
COLOR     [Cc][Oo][Ll][Oo][Rr]
COS       [Cc][Oo][Ss]\(
DATE      [Dd][Aa][Tt][Ee]
DATA      [Dd][Aa][Tt][Aa]
DIM       [Dd][Ii][Mm]
DO        [Dd][Oo]
ELSE      [Ee][Ll][Ss][Ee]
END       [Ee][Nn][Dd]
ENVIRON   [Ee][Nn][Vv][Ii][Rr][Oo][Nn]
EOF       [Ee][Oo][F]
EXP       [Ee][Xx][Pp]\(
FOR       [Ff][Oo][Rr]
FREEFILE  [Ff][Rr][Ee][Ee][Ff][Ii][Ll][Ee]
GET       [Gg][Ee][Tt]
GOSUB     [Gg][Oo][Ss][Uu][Bb]
GOTO      [Gg][Oo][Tt][Oo]
HEX       [Hh][Ee][Xx]
IF        [Ii][Ff]
INKEY     [Ii][Nn][Kk][Ee][Yy]
INPUT     [Ii][Nn][Pp][Uu][Tt]
INSTR     [Ii][Nn][Ss][Tt][Rr]\(
INT       [Ii][Nn][Tt]
KILL      [Kk][Ii][Ll][Ll]
LET       [Ll][Ee][Tt]
LEFT      [Ll][Ee][Ff][Tt]
LEN       [Ll][Ee][Nn]\(
LOC       [Ll][Oo][Cc]
LOCATE    [Ll][Oo][Cc][Aa][Tt][Ee]
LOF       [Ll][Oo][Ff]
LOG       [Ll][Oo][Gg]\(
LOWER     [Ll][Oo][Ww][Ee][Rr]
LSET      [Ll][Ss][Ee][Tt]
LTRIM     [Ll][Tt][Rr][Ii][Mm]
MID       [Mm][Ii][Dd]
MKDIR     [Mm][Kk][Dd][Ii][Rr]
MOD       [Mm][Oo][Dd]
NAME      [Nn][Aa][Mm][Ee]
NEXT      [Nn][Ee][Xx][Tt]
NOT       [Nn][Oo][Tt]
OCT       [Oo][Cc][Tt]
OPEN      [Oo][Pp][Ee][Nn]
OR        [Oo][Rr]
OUTPUT    [Oo][Uu][Tt][Pp][Uu][Tt]
ON        [Oo][Nn]
PUT       [Pp][Uu][Tt]
PRINT     [Pp][Rr][Ii][Nn][Tt]
RANDOMIZE [Rr][Aa][Nn][Dd][Oo][Mm][Ii][Zz][Ee]
READ      [Rr][Ee][Aa][Dd]
REM       [Rr][Ee][Mm]
RESTORE   [Rr][Ee][Ss][Tt][Oo][Rr][Ee]
RETURN    [Rr][Ee][Tt][Uu][Rr][Nn]
RIGHT     [Rr][Ii][Gg][Hh][Tt]
RND       [Rr][Nn][Dd]\(
RMDIR     [Rr][Mm][Dd][Ii][Rr]
RSET      [Rr][Ss][Ee][Tt]
RTRIM     [Rr][Tt][Rr][Ii][Mm]
SEEK      [Ss][Ee][Ee][Kk]
SPACE     [Ss][Pp][Aa][Cc][Ee]
STEP      [Ss][Tt][Ee][Pp]
SWAP      [Ss][Ww][Aa][Pp]
SGN       [Ss][Gg][Nn]\(
SIN       [Ss][Ii][Nn]\(
SQL       [Ss][Qq][Ll]
SQR       [Ss][Qq][Rr]\(
STR       [Ss][Tt][Rr]
STRINGSTR [Ss][Tt][Rr][Ii][Nn][Gg]\$
SYSTEM    [Ss][Yy][Ss][Tt][Ee][Mm]\(
TAN       [Tt][Aa][Nn]\(
THEN      [Tt][Hh][Ee][Nn]
TIMER     [Tt][Ii][Mm][Ee][Rr]
TIMESTR   [Tt][Ii][Mm][Ee]\$
TO        [Tt][Oo]
UPPER     [Uu][Pp][Pp][Ee][Rr]
UNTIL     [Uu][Nn][Tt][Ii][Ll]
VAL       [Vv][Aa][Ll]\(
WEND      [Ww][Ee][Nn][Dd]
WHILE     [Ww][Hh][Ii][Ll][Ee]
XOR       [Xx][Oo][Rr]

%%

^"#".*{NL} { cmdcnt++; return NL; }

{NL} {
#ifdef RULE_DEBUG
    fprintf(stderr,"checking rule NL\n");
#endif
    chstate(LNUM_STATE);
#ifdef YACC_DEBUG
    fprintf(stderr,"sending yacc NL\n");
#endif
    if (ignore_end) yyterminate(); /* set by yacc to notify we are done */

    thenstart=0;

    cmdcnt++;

    return NL;
}

^{LNUMBERX}.*{NL} {
#ifdef RULE_DEBUG
    fprintf(stderr,"checking rule LNUMBER SP .* NL\n");
#endif
    if (ignore_end) yyterminate(); /* set by yacc to notify we are done */
	if (!(statef(LNUM_STATE|JUMP_LNUM_STATE))) REJECT;
	if (did_jump)
		did_jump=0;
	else 
		cmdcnt++;
    REJECT;
}

{LNUMBER} {
    /* linenumber, as begin of line or as option of GOSUB or GOTO */
    /* not ON .. GOTO/GOSUB though, 'cause yacc could get a bit   */
    /* upset about it.                                            */

#ifdef RULE_DEBUG
    fprintf(stderr,"checking rule LNUMBER SP\n");
#endif
    if (statef(LNUM_STATE|JUMP_LNUM_STATE)) {
        if (statef(LNUM_STATE)) 
            chstate(COMMAND_STATE);
        else 
            chstate(LNUM_STATE);
        yylval.d_val=atof(yytext);
#ifdef YACC_DEBUG
        fprintf(stderr,"sending yacc LINENUMBER\n");
#endif
        return LINENUMBER; 
    } else {
        REJECT;
    };
}

{END} {
    /* DON't just stop, yacc could be upset about it */

    if ((result=do_cmd(END,LNUM_STATE|COMMAND_STATE,NOTHING))<0) {
        yyerror("END command on wrong place");
        yyterminate();
    }
    return result;
}

{REM}.* {
    if ((result=do_cmd(REM,LNUM_STATE|COMMAND_STATE,NOTHING))<0) {
        yyerror("REM command on wrong place");
        yyterminate();
    }
    return result;
}

{DATA}.* {
    /* DATA arguments are read in pre-parse fase, so just ignore those */

    if ((result=do_cmd(DATA,LNUM_STATE|COMMAND_STATE,NOTHING))<0) {
        yyerror("DATA command on wrong place");
        yyterminate();
    }
    return result;
}

{INPUT} { 
    /* input can be used as command or as keyword in OPEN ... */
    if ((result=do_cmd(INPUT,LNUM_STATE|COMMAND_STATE|EXPRESSION_STATE,
        EXPRESSION_STATE))<0) {
        if (statef(LNUM_STATE|COMMAND_STATE))
            yyerror("INPUT command on wrong place");
        else
            yyerror("INPUT keyword on wrong place");
        yyterminate();
    } 
    return result;
}

{SEEK} { 
    /* most implementations of BASIC don't have seek, don't know */
    /* why 'cause its realy handy sometimes                      */
    if ((result=do_cmd(SEEK,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("SEEK command on wrong place");
        yyterminate();
    } 
    return result;
}

{DO} { 
    if ((result=do_cmd(DO,LNUM_STATE|COMMAND_STATE,NOTHING))<0) {
        yyerror("DO command on wrong place");
        yyterminate();
    } 
    return result;
}

{UNTIL} { 
    if ((result=do_cmd(UNTIL,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("UNTIL command on wrong place");
        yyterminate();
    } 
    return result;
}


{NAME} { 
    if ((result=do_cmd(NAME,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("NAME command on wrong place");
        yyterminate();
    } 
    return result;
}

{OUTPUT} { 
    if ((result=do_cmd(OUTPUT,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("OUTPUT keyword on wrong place");
        yyterminate();
    } 
    return result;
}

{APPEND} { 
    if ((result=do_cmd(APPEND,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("APPEND keyword on wrong place");
        yyterminate();
    } 
    return result;
}

{WHILE} { 
    if ((result=do_cmd(WHILE,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("WHILE command on wrong place");
        yyterminate();
    } 
    return result;
}

{WEND} { 
    if ((result=do_cmd(WEND,LNUM_STATE|COMMAND_STATE,LNUM_STATE))<0) {
        yyerror("WEND command on wrong place");
        yyterminate();
    } 
    return result;
}

{PRINT} {
    if ((result=do_cmd(PRINT,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("PRINT command on wrong place");
        yyterminate();
    } 
    return result;
}

{READ} {
    if ((result=do_cmd(READ,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("READ command on wrong place");
        yyterminate();
    } 
    return result;
}

{OPEN} {
    if ((result=do_cmd(OPEN,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("OPEN command on wrong place");
        yyterminate();
    } 
    return result;
}

{AS} {
    if ((result=do_cmd(AS,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("AS keyword on wrong place");
        yyterminate();
    } 
    return result;
}

{STR}\$\( {
    if ((result=do_cmd(STR,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("STR$ expression on wrong place");
        yyterminate();
    } 
    return result;
}

{RANDOMIZE} {
    if ((result=do_cmd(RANDOMIZE,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("RANDOMIZE command on wrong place");
        yyterminate();
    } 
    return result;
}

{SWAP} {
    if ((result=do_cmd(SWAP,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("SWAP command on wrong place");
        yyterminate();
    } 
    return result;
}

{ON} {
    if ((result=do_cmd(ON,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("ON command on wrong place");
        yyterminate();
    } 
    return result;
}

{ENVIRON} {
    if ((result=do_cmd(ENVIRON,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("ENVIRON command on wrong place");
        yyterminate();
    } 
    return result;
}

{LOCATE} {
    if ((result=do_cmd(LOCATE,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("LOCATE command on wrong place");
        yyterminate();
    } 
    return result;
}

{KILL} {
    if ((result=do_cmd(KILL,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("KILL command on wrong place");
        yyterminate();
    } 
    return result;
}

{GET} {
    if ((result=do_cmd(GET,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("GET command on wrong place");
        yyterminate();
    } 
    return result;
}

{PUT} {
    if ((result=do_cmd(PUT,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("PUT command on wrong place");
        yyterminate();
    } 
    return result;
}

{RSET} {
    if ((result=do_cmd(RSET,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("RSET command on wrong place");
        yyterminate();
    } 
    return result;
}

{FREEFILE} {
    if ((result=do_cmd(FREEFILE,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("FREEFILE expression on wrong place");
        yyterminate();
    } 
    return result;
}

{LSET} {
    if ((result=do_cmd(LSET,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("LSET command on wrong place");
        yyterminate();
    } 
    return result;
}


{LET} {
    /* actually, this command could ignored, it is just for syntax checking */
    /* send to yacc.                                                        */
    if ((result=do_cmd(LET,LNUM_STATE|COMMAND_STATE,COMMAND_STATE))<0) {
        yyerror("LET command on wrong place");
        yyterminate();
    } 
    return result;
}

{FOR} {
    /* again, FOR can be a keyword in OPEN command also           */
    if ((result=do_cmd(FOR,LNUM_STATE|COMMAND_STATE|EXPRESSION_STATE,
        EXPRESSION_STATE))<0) {
        if (statef(LNUM_STATE|COMMAND_STATE))
            yyerror("FOR command on wrong place");
        else
            yyerror("FOR keyword on wrong place");
        yyterminate();
    } 
    return result;
}

{TO} {
    if ((result=do_cmd(TO,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("TO command on wrong place");
        yyterminate();
    } 
    return result;
}

{AND} {
    if ((result=do_cmd(AND,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("AND expression on wrong place");
        yyterminate();
    } 
    return result;
}

{MOD} {
    if ((result=do_cmd(MOD,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("MOD expression on wrong place");
        yyterminate();
    } 
    return result;
}

{OR} {
    if ((result=do_cmd(OR,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("OR expression on wrong place");
        yyterminate();
    } 
    return result;
}

{NOT} {
    if ((result=do_cmd(NOT,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("NOT expression on wrong place");
        yyterminate();
    } 
    return result;
}

{XOR} {
    if ((result=do_cmd(XOR,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("XOR expression on wrong place");
        yyterminate();
    } 
    return result;
}

{VAL} {
    if ((result=do_cmd(VAL,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("VAL expression on wrong place");
        yyterminate();
    } 
    return result;
}

{COLOR} {
    if ((result=do_cmd(COLOR,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("COLOR command on wrong place");
        yyterminate();
    } 
    return result;
}


{COS} {
    if ((result=do_cmd(COS,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("COS expression on wrong place");
        yyterminate();
    } 
    return result;
}

{CLOSE} {
    if ((result=do_cmd(CLOSE,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("CLOSE command on wrong place");
        yyterminate();
    } 
    return result;
}

{ACOS} {
    if ((result=do_cmd(ACOS,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("ACOS expression on wrong place");
        yyterminate();
    } 
    return result;
}

{ASIN} {
    if ((result=do_cmd(ASIN,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("ASIN expression on wrong place");
        yyterminate();
    } 
    return result;
}

{EXP} {
    if ((result=do_cmd(EXP,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("EXP expression on wrong place");
        yyterminate();
    } 
    return result;
}

{SIN} {
    if ((result=do_cmd(SIN,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("SIN expression on wrong place");
        yyterminate();
    } 
    return result;
}

{TAN} {
    if ((result=do_cmd(TAN,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("TAN expression on wrong place");
        yyterminate();
    } 
    return result;
}

{SQL} {
    if ((result=do_cmd(SQL,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("SQL command on wrong place");
        yyterminate();
    } 
    return result;
}

{SQR} {
    if ((result=do_cmd(SQR,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("SQR expression on wrong place");
        yyterminate();
    } 
    return result;
}

{EOF} {
    /* I use "BEOF" as token 'cause sometimes EOF is already defined */
    /* somewhere                                                     */
    if ((result=do_cmd(BEOF,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("EOF expression on wrong place");
        yyterminate();
    } 
    return result;
}        

{LOC} {
    if ((result=do_cmd(LOC,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("LOC expression on wrong place");
        yyterminate();
    } 
    return result;
}

{LOF} {
    if ((result=do_cmd(LOF,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("LOF expression on wrong place");
        yyterminate();
    } 
    return result;
}

{LOG} {
    if ((result=do_cmd(LOG,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("LOG expression on wrong place");
        yyterminate();
    } 
    return result;
}

{STEP} {
    if ((result=do_cmd(STEP,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("STEP command on wrong place");
        yyterminate();
    } 
    return result;
}

{NEXT} {
    if ((result=do_cmd(NEXT,LNUM_STATE|COMMAND_STATE,LNUM_STATE))<0) {
        yyerror("NEXT command on wrong place");
        yyterminate();
    } 
    return result;
}

{GOTO} {
    if ((result=do_cmd(GOTO,LNUM_STATE|COMMAND_STATE,JUMP_LNUM_STATE))>0) return result;
    if ((result=do_cmd(GOTO,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("GOTO command on wrong place");
        yyterminate();
    } 
    return result;
}

{GOSUB} {
    if ((result=do_cmd(GOSUB,LNUM_STATE|COMMAND_STATE,JUMP_LNUM_STATE))>0) return result;
    if ((result=do_cmd(GOSUB,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("GOSUB command on wrong place");
        yyterminate();
    } 

    return result;
}

{RESTORE} {
    if ((result=do_cmd(RESTORE,LNUM_STATE|COMMAND_STATE,NOTHING))<0) {
        yyerror("RESTORE command on wrong place");
        yyterminate();
    } 
    return result;
}

{RETURN} {
    if ((result=do_cmd(RETURN,LNUM_STATE|COMMAND_STATE,LNUM_STATE))<0) {
        yyerror("RETURN command on wrong place");
        yyterminate();
    } 
    return result;
}

{ELSE} {
#ifdef RULE_DEBUG
    fprintf(stderr,"checking rule ELSE SP\n");
#endif
    if (thenstart) {
        chstate(COMMAND_STATE);
#ifdef YACC_DEBUG
        fprintf(stderr,"sending yacc ELSE\n");
#endif
        return ELSE;
    }
    yyerror("ELSE command on wrong place");
    yyterminate();
}

{IF} {
    if ((result=do_cmd(IF,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("IF command on wrong place");
        yyterminate();
    } 
    return result;
}

{THEN} {
    if ((result=do_cmd(THEN,EXPRESSION_STATE,COMMAND_STATE))<0) {
        yyerror("THEN command on wrong place");
        yyterminate();
    } 
    thenstart=1;
    return result;
}

{SYSTEM} {
    if ((result=do_cmd(SYSTEM,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("SYSTEM command on wrong place");
        yyterminate();
    } 
    return result;
}


{CLS} {
    /* my first BASIC command, Y2K compliant and bug-free forever */
    if ((result=do_cmd(CLS,LNUM_STATE|COMMAND_STATE,NOTHING))<0) {
        yyerror("CLS command on wrong place");
        yyterminate();
    } 
    return result;
}

{BEEP} {
    /* we can't have BASIC without one, do we ? */
    if ((result=do_cmd(BEEP,LNUM_STATE|COMMAND_STATE,NOTHING))<0) {
        yyerror("BEEP command on wrong place");
        yyterminate();
    } 
    return result;
}

{ASC} {
    if ((result=do_cmd(ASC,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("ASC expression found on wrong location");
        yyterminate();
    } 
    return result;
}

{ATN} {
    if ((result=do_cmd(ATN,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("ATN expression found on wrong location");
        yyterminate();
    } 
    return result;
}

{CINT} {
    /* Each BASIC has it's own interpretation of CINT and INT */
    /* maybe I let people vote wich interpretation we want    */
    if ((result=do_cmd(CINT,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("CINT expression found on wrong location");
        yyterminate();
    } 
    return result;
}

{INT} {
    if ((result=do_cmd(INT,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("INT expression found on wrong location");
        yyterminate();
    } 
    return result;
}

{SGN} {
    if ((result=do_cmd(SGN,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("SGN expression found on wrong location");
        yyterminate();
    } 
    return result;
}

{RND} {
    if ((result=do_cmd(RND,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("RND expression found on wrong location");
        yyterminate();
    } 
    return result;
}

{LEN} {
    if ((result=do_cmd(LEN,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("LEN expression found on wrong location");
        yyterminate();
    } 
    return result;
}

{INKEY}\$ {
    if ((result=do_cmd(INKEY,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("INKEY expression found on wrong location");
        yyterminate();
    } 
    return result;
}

{INSTR} {
    if ((result=do_cmd(INSTR,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("INSTR expression found on wrong location");
        yyterminate();
    } 
    return result;
}

{ABS} {
    if ((result=do_cmd(ABS,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("ABS expression found on wrong location");
        yyterminate();
    } 
    return result;
}

{CHR}\$\( {
    if ((result=do_cmd(CHR,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("CHR$ expression found on wrong location");
        yyterminate();
    } 
    return result;
}

{RMDIR} {
    if ((result=do_cmd(RMDIR,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("RMDIR command found on wrong location");
        yyterminate();
    } 
    return result;
}

{CHDIR} {
    if ((result=do_cmd(CHDIR,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("CHDIR command found on wrong location");
        yyterminate();
    } 
    return result;
}

{DIM} {
    if ((result=do_cmd(DIM,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("DIM command found on wrong location");
        yyterminate();
    } 
    return result;
}

{MKDIR} {
    if ((result=do_cmd(MKDIR,LNUM_STATE|COMMAND_STATE,EXPRESSION_STATE))<0) {
        yyerror("MKDIR command found on wrong location");
        yyterminate();
    } 
    return result;
}

{STRINGSTR}\( {
    /* I used STRINGSTR to make it different than STRING */
    if ((result=do_cmd(STRINGSTR,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("STRING$ expression found on wrong location");
        yyterminate();
    } 
    return result;
}

{SPACE}\$\( {
    /* Some BASIC's has SPC to or both of them, never know why */
    if ((result=do_cmd(SPACE,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("SPACE$ expression found on wrong location");
        yyterminate();
    } 
    return result;
}

{ENVIRON}\$\( {
    if ((result=do_cmd(ENVIRONSTR,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("ENIVORN$ expression found on wrong location");
        yyterminate();
    } 
    return result;
}

{OCT}\$\( {
    if ((result=do_cmd(OCT,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("OCT$ expression found on wrong location");
        yyterminate();
    } 
    return result;
}


{LEFT}\$\( {
    if ((result=do_cmd(LEFT,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("LEFT$ expression found on wrong location");
        yyterminate();
    } 
    return result;
}

{RIGHT}\$\( {
    if ((result=do_cmd(RIGHT,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("RIGHT$ expression found found on wrong location");
        yyterminate();
    } 
    return result;
}

{LOWER}\$\( {
    if ((result=do_cmd(LOWER,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("LOWER$ expression found found on wrong location");
        yyterminate();
    } 
    return result;
}

{UPPER}\$\( {
    if ((result=do_cmd(UPPER,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("UPPER$ expression found found on wrong location");
        yyterminate();
    } 
    return result;
}

{RTRIM}\$\( {
    if ((result=do_cmd(RTRIM,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("RTRIM$ expression found found on wrong location");
        yyterminate();
    } 
    return result;
}

{LTRIM}\$\( {
    if ((result=do_cmd(LTRIM,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("LTRIM$ expression found found on wrong location");
        yyterminate();
    } 
    return result;
}

{MID}\$\( {
    if ((result=do_cmd(MID,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("MID$ expression found on wrong location");
        yyterminate();
    } 
    return result;
}


{HEX}\$\( {
    if ((result=do_cmd(HEX,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("HEX$ expression found on wrong location");
        yyterminate();
    } 
    return result;
}

{DATE}\$ {
    /* date will be proccessed by lex and send as passedtext to yacc */
    /* Date can't be set with this, or you want to run basic as root */
    /* wich I don't think is a good idea...                          */
    struct tm *date;
    time_t ttime;
#ifdef RULE_DEBUG
    fprintf(stderr,"checking rule DATE$\n");
#endif
    if (statef(EXPRESSION_STATE)) {
        ttime=time(NULL);
        date=localtime(&ttime);
        sprintf(
            yylval.str,
            "%d-%02d-%02d",
            date->tm_year+1900,date->tm_mon+1,date->tm_mday
        );
#ifdef YACC_DEBUG
        fprintf(stderr,"sending yacc PASSEDTEXT\n");
#endif
        return PASSEDTEXT;
    } 
    yyerror("DATE$ expression on wrong location");
    yyterminate();
}

{TIMESTR} {
    struct tm *date;
    time_t ttime;
#ifdef RULE_DEBUG
    fprintf(stderr,"checking rule TIME $\n");
#endif
    if (statef(EXPRESSION_STATE)) {
        ttime=time(NULL);
        date=localtime(&ttime);
        sprintf(
            yylval.str,
            "%0d-%02d-%02d",
            date->tm_hour,date->tm_min,date->tm_sec
        );
#ifdef YACC_DEBUG
        fprintf(stderr,"sending yacc PASSEDTEXT\n");
#endif
        return PASSEDTEXT;
    } 
    yyerror("TIME$ expression on wrong location");
    yyterminate();
}

{TIMER} {
    /* "Seconds from midnight 12 O'clock" mostly used by randomize */
    struct tm *date;
    time_t ttime;
#ifdef RULE_DEBUG
    fprintf(stderr,"checking rule TIMER\n");
#endif
    if (statef(EXPRESSION_STATE)) {
        ttime=time(NULL);
        date=localtime(&ttime);
        yylval.d_val=date->tm_hour*3600+date->tm_min*60+date->tm_sec;
#ifdef YACC_DEBUG
        fprintf(stderr,"sending yacc NUMBER\n");
#endif
        return NUMBER;
    } 
    yyerror("TIMER expression on wrong location");
    yyterminate();
}

{COLON} {
#ifdef RULE_DEBUG
    fprintf(stderr,"checking rule SP COLON SP\n");
#endif
    chstate(COMMAND_STATE);
#ifdef YACC_DEBUG
    fprintf(stderr,"sending yacc ':'\n");
#endif
    thenstart=0;
    return ':';
}

{DQUOTE}[^"]*{DQUOTE} { 
    /* I could used yacc for this, but this was easier in the end to send */
    /* yacc the whole string at once */
    char *p;
#ifdef RULE_DEBUG
    fprintf(stderr,"checking rule DQUOTE . DQUOTE\n");
#endif
    if (statef(EXPRESSION_STATE)) {
        /* yytext[1] to get rid of first double quote */
        strncpy(yylval.str,&yytext[1],sizeof(yylval.str));
        p=&yylval.str[0];
        while(*p!='"') p++;
        /* ....and there goes number 2 of double quotes */
        *p='\0';
#ifdef YACC_DEBUG
        fprintf(stderr,"sending yacc PASSEDTEXT\n");
#endif
        return PASSEDTEXT;
    } 
    yyerror("Misplaced quoted text");
    yyterminate();
}


{SEMICOLON} {
    /* Actually, I have no idea why I declared semicolon seperatly */
    /* It could be left out..or not ?                              */
    if ((result=do_cmd(SEMICOLON,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("Semicolon on wrong place");
        yyterminate();
    } 
    yylval.d_val=';';
    return result;
}

{HEXNUMBER} {
    char convbuf[500];
    /* number, sometimes used as linenumber too */
    if ((result=do_cmd(NUMBER,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("Number found when expecting command");
        yyterminate();
    } 
    memset(convbuf,0,sizeof(convbuf));
    convbuf[0]='0'; 
    convbuf[1]='x';  
    strncpy(&convbuf[2],&yytext[2],STRING_SIZE-2);
    yylval.d_val=strtol(convbuf,NULL,0);
    return result;
}

{OCTNUMBER} {
    char convbuf[500];
    /* number, sometimes used as linenumber too */
    if ((result=do_cmd(NUMBER,EXPRESSION_STATE,EXPRESSION_STATE))<0) {
        yyerror("Number found when expecting command");
        yyterminate();
    } 
    memset(convbuf,0,sizeof(convbuf));
    convbuf[0]='0'; 
    strncpy(&convbuf[1],&yytext[2],STRING_SIZE-2);
    yylval.d_val=strtol(convbuf,NULL,0);
    return result;
}


{NUMBER} {
    /* number, sometimes used as linenumber too */
    if (((result=do_cmd(NUMBER,EXPRESSION_STATE,EXPRESSION_STATE))<0)) {
		if (!((thenstart)&&(statef(COMMAND_STATE)))) {
			yyerror("Number found when expecting command");
			yyterminate();
		} else {
			result=NUMBER;
		}
    } 
    yylval.d_val=atof(yytext);
    return result;
}

{STRING} {
    char *p;
#ifdef RULE_DEBUG
    fprintf(stderr,"checking rule STRING\n");
#endif
    if (statef(EXPRESSION_STATE|LNUM_STATE|COMMAND_STATE)) {
        chstate(EXPRESSION_STATE);
        memset(&yylval.str[0],0,sizeof(STRING_SIZE));
        strncpy(&yylval.str[0],yytext,STRING_SIZE-1);
        /* BASIC is case insensitive, saves a lot of typos */
        p=yylval.str;
        while(*p) {
            *p=toupper(*p);
            p++;
        }
#ifdef YACC_DEBUG
        fprintf(stderr,"sending yacc STRING\n");
#endif
        return STRING;
    } 
    yyerror("String on wrong location");
    yyterminate();
}

{VARIABLE} {
    char *p;
#ifdef RULE_DEBUG
    fprintf(stderr,"checking rule VARIABLE\n");
#endif
    if (statef(EXPRESSION_STATE|LNUM_STATE|COMMAND_STATE)) {
        chstate(EXPRESSION_STATE);
        memset(yylval.str,0,sizeof(yylval.str));
        strncpy(yylval.str,yytext,sizeof(yylval.str)-1);
        p=yylval.str;
        while(*p) {
            *p=toupper(*p);
            p++;
        }
#ifdef YACC_DEBUG
        fprintf(stderr,"sending yacc VAR\n");
#endif
        return VAR;
    } 
    yyerror("Variable on wrong location");
    yyterminate();
}

{VARIABLE}\( {
    /* DIM-ed variable, threated different */
    char *p;
#ifdef RULE_DEBUG
    fprintf(stderr,"checking rule dim VARIABLE\n");
#endif
    if (statef(EXPRESSION_STATE|LNUM_STATE|COMMAND_STATE)) {
        chstate(EXPRESSION_STATE);
        memset(yylval.str,0,sizeof(yylval.str));
        strncpy(yylval.str,yytext,sizeof(yylval.str)-1);
        p=yylval.str;
        while(*p) {
            *p=toupper(*p);
            p++;
        }
        *(p-1)='\0';
#ifdef YACC_DEBUG
        fprintf(stderr,"sending yacc DIMVAR\n");
#endif
        return DIMVAR;
    } 
    yyerror("String on wrong location");
    yyterminate();
}

{VARIABLE}\$\( {
    char *p;
#ifdef RULE_DEBUG
    fprintf(stderr,"checking rule dim VARIABLE$\n");
#endif
    if (statef(EXPRESSION_STATE|LNUM_STATE|COMMAND_STATE)) {
        chstate(EXPRESSION_STATE);
        memset(yylval.str,0,sizeof(yylval.str));
        strncpy(yylval.str,yytext,sizeof(yylval.str)-1);
        p=yylval.str;
        while(*p) {
            *p=toupper(*p);
            p++;
        }
        *(p-1)='\0';
#ifdef YACC_DEBUG
        fprintf(stderr,"sending yacc DIMSTR\n");
#endif
        return DIMSTR;
    } 
    yyerror("String on wrong location");
    yyterminate();
}

. {
    /* what's left. Mostly expression commands ('+','-' ...) */
#ifdef RULE_DEBUG
    fprintf(stderr,"checking rule '.'\n");
#endif
    if (yytext[0]!=' ') {
        if (statef(EXPRESSION_STATE)) {
#ifdef YACC_DEBUG
            fprintf(stderr,"sending yacc %c\n",yytext[0]);
#endif
            return yytext[0];
        }
        yyerror("Syntax error");
        yyterminate();
    }
}

<<EOF>> {
    /* the end of the ride */
   yyterminate();
}
%%

void pushback(char c) 
{
	unput((int) c);
}

void init_flex(void)
{
    ignore_end=0;
    thenstart=0;
    cmdcnt=0;
    did_jump=0;
    state=LNUM_STATE;
}
