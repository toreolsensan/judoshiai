%{
/*----------------------------------------------------------------------*/
/*  MOLE BASIC                                                          */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*        file: basic.y                                                 */
/*    language: yacc/bison & ANSI C                                     */ 
/*     program: MOLE BASIC                                              */
/*                                                                      */
/* --- Description -----------------------------------------------------*/
/* This is the lex part of the basic interperter.                       */
/*                                                                      */
/* --- Version information ---------------------------------------------*/
/* 0.1  1999-02-12  first version, no boundary checks and still buggy   */
/* 0.3  1999-02-20  Finaly a working version, still beta though.        */
/* 0.5  1999-02-26  First public beta release. Now runs under AIX too   */
/* 0.51 1999-02-28  Minor fixes to compile it on SUN and BSD.           */
/* 0.53 1999-02-28  Whoops, major memory bug when using READ.           */
/* 0.7  1999-03-01  Jumping drasticly changed, performance is better    */
/*                  and problems dealing with commands before or after  */
/*                  FOR..NEXT, WHILE..WEND and GOSUB are fixed now.     */
/*                  Also DO..UNTIL implemented.                         */
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <errno.h>
#include "basic_defs.h"

#ifdef WIN32
#else
#include <termios.h>
#endif

#include <gtk/gtk.h>
#include "judoshiai.h"

void yyerror(char *s);

static GtkTextBuffer *gtkbuf;
static void print_text_buffer(char *format, ...);
static int sql(char *cmd);

int checkignore(void);

char buf[STRING_SIZE*2+1];    /* buffer used by string operations      */
char vbuf[VARNAME_SIZE+1];    /* buffer to store varname               */
char nothing[]="";            /* empty string used as return values    */
int bcount=0;                 /* buffer pointer for various operations */
int indexcnt=0;               /* used to index jump tables             */
extern int did_jump;
extern FILE *yyin;
extern void yyterminate();
extern void pushback(char c);
extern int cmdcnt;            /* used to count commandblocks for jump  */

struct jumpstruct {
    int line;
    long pos;
};                            /* jumpinglist struct                    */
int jli=0;                    /* jumplist index                        */

struct _fnum{
    int len;
    FILE *fp;
} fnum[MAX_FILES];            /* Filenumber struct                     */

struct _d_vars{
    char name[VARNAME_SIZE+1];
    double value;
} d_vars[MAX_D_VARS+1];       /* These are the "double" vars           */
int d_vp=0;                   /* pointer to first free d_var           */

struct _str_vars{
    char name[VARNAME_SIZE+1];
    char *value;
} str_vars[MAX_STR_VARS+1];   /* And these are the strings             */
int str_vp=0;                 /* pointer to first free string          */

struct _dim_d_vars{
    char name[VARNAME_SIZE+1];
    int dim[MAX_DIM+1];
    double *values;
} dim_d_vars[MAX_DIM_VARS+1]; /* DIMensionsiond "double" vars          */
int dim_d_vp=0;               /* pointer to first free dim d_var       */

struct _dim_str_vars{
    char name[VARNAME_SIZE+1];
    int dim[MAX_DIM+1];
    char **values;
    int size;
} dim_str_vars[MAX_DIM_STR];  /* DIMensioned string vars               */
int dim_str_vp=0;             /* pointer to first free dim str_var     */

int gosub_stack[STACK_SIZE];  /* GOSUB stack                           */
int gosub_sp=0;               /* pointer to first free on gosub stack  */

int while_stack[STACK_SIZE];  /* WHILE stack                           */
int while_sp=0;               /* pointer to first free on while stack  */

int do_stack[STACK_SIZE];     /* DO stack                              */
int do_sp=0;                  /* pointer to first free on do stack     */


struct _for_stack {           /* FOR..NEXT stack, keeps track of wich  */
    int idx;                  /* variable we use, end value, step and  */
    int end;                  /* place to jump back on NEXT command    */
    double step;
    int jumpto;
    char dimflag;             /* flag if we use a dimvar as idx        */
    int pos;                  /* position in dim var                   */
} for_stack[STACK_SIZE];
int for_sp=0;                 /* pointer to first free on for stack    */

int dimensions[MAX_DIM];      /* buffer to store dimensions of DIM in  */
int dimc=0;                   /* counter of dimensions in DIM          */
int onlines[MAX_ON_LINES];    /* Buffer to store linenumbers from "on" */
int onlp=0;                   /* pointer to first free onlines         */
int lc=1;                     /* LineCounter, not useable at the moment*/
int current_line=0;           /* Current number of the line processed  */

int ignore_next_cmd=0;        /* ignore next command flag              */
int ignore_else_cmd=0;        /* ignore commands following "ELSE" flag */
int ignore_after_jump=0;      /* ignore everything, till jump is done  */
int use_stream=0;             /* if set, use value as io stream        */

extern int ignore_end;        /* END hit, ignore rest                  */
int ignore_till_wend=0;       /* flag set if we have to look for wend  */


struct l_list {                
    struct l_list *next;
    void *val;
};                             /* linked list structure                 */

struct l_list *jumplist=NULL;  /* this will store the readed data       */

struct l_list *readdata=NULL;  /* this will store the readed data       */
struct l_list *rd_p=NULL;      /* pointer to dataread                   */

/* reset all vars (including strings)     */
/* could be used someday as CLEAR command */

#if 0 //HJo
#define dojumpidx(idx) do { printf("DOJUMPIDX %s:%d idx=%d\n", __FUNCTION__, __LINE__, idx); dojumpidx1(idx); } while (0) 
#define for_push(idx,end,step,jumpto,type,pos) printf("FOR_PUSH %s:%d jumpto=%ld\n", __FUNCTION__, __LINE__,jumpto),for_push1(idx,end,step,jumpto,type,pos) 
#endif

void free_vars() 
{
    int count;
    for (count=0;count<MAX_D_VARS;count++) {
        str_vars[count].value=0;
    }
    for (count=0;count<MAX_STR_VARS;count++) {
        if (str_vars[count].value!=NULL) free(str_vars[count].value);
    }
}

/* free linked list's */

void free_ll(struct l_list *n) 
{
    if (n!=NULL) {
        free_ll(n->next);
        if (n->val!=NULL) free(n->val);
        free(n);
    }
}


/* translate linenumber to jumpindex */

int line2pos(int line) 
{
    int cnt;
    struct l_list *l=jumplist;
    for (cnt=0;cnt<jli;cnt++) {
        if (((struct jumpstruct *)(l->val))->line==line) return cnt+1;
        l=l->next;
    }
    return -1;
}

/* translate variable name to variable index */

int d_name2idx(char *lookup) 
{
    int x;
    for (x=0;x<=d_vp;x++) {
        if (strncmp(d_vars[x].name,lookup,VARNAME_SIZE)==0) return x;
    }
    return -1;
}

/* translate variable name to value corrosponding name */

double d_name2val(char *lookup) 
{
    int x=0;
    if ((x=d_name2idx(lookup))<0) return 0;
    return d_vars[x].value;
}

/* place value in named variable */

int d_val2name(double val,char *lookup) 
{
    int x=0;
    if ((x=d_name2idx(lookup))<0) {
        if (d_vp==MAX_D_VARS) {
            yyerror("Out of variable space");
            return (MAX_D_VARS+1);
        } else {
            strncpy(d_vars[d_vp].name,lookup,VARNAME_SIZE);
            d_vars[d_vp++].value=val;
            return (d_vp-1);
        }
    } else {
        d_vars[x].value=val;
        return x;
    }
}

/* translate string name to string index number */

int str_name2idx(char *lookup) 
{
    int x;
    for (x=0;x<=str_vp;x++) {
        if (strncmp(str_vars[x].name,lookup,VARNAME_SIZE)==0) return x;
    }
    return -1;
}

/* find content of string */

char* str_name2val(char *lookup) 
{
    int x=0;
    if ((x=str_name2idx(lookup))<0) return NULL;
    return str_vars[x].value;
}

/*  place content into string */

int str_val2name(char *val,char *lookup) 
{
    int x=0;
    if ((x=str_name2idx(lookup))<0) {
        if (str_vp==MAX_STR_VARS) {
            yyerror("Out of string variable space");
            return (MAX_STR_VARS+1);
        } else {
            strncpy(str_vars[str_vp].name,lookup,VARNAME_SIZE);
            str_vars[str_vp++].value=strdup(val);
            return (str_vp-1);
        }
    } else {
        str_vars[x].value=strdup(val);
        return x;
    }
}

/* translate dim variable name to dim variable index */

int dim_d_name2idx(char *lookup) 
{
    int x;
    for (x=0;x<=dim_d_vp;x++) {
        if (strncmp(dim_d_vars[x].name,lookup,VARNAME_SIZE)==0) return x;
    }
    return -1;
}

/* translate dim variable name to value corrosponding name */

double dim_d_name2val(char *lookup) 
{
    int pos=0;
    int mul=1;
    int x=0;
    int c;
    if ((x=dim_d_name2idx(lookup))<0) return 0;
    for (c=0;c<=dimc;c++) {
        if ((dimensions[c]>dim_d_vars[x].dim[c])||(dimensions[c]==0)) { 
            yyerror("Illegal dimension value(s)");
            return 0;
        }
        pos+=(dimensions[c]-1)*mul;
        mul*=dim_d_vars[x].dim[c];
    }
    return dim_d_vars[x].values[pos];
}

/* place value in named variable */

int dim_d_val2name(double val,char *lookup) 
{
    int x=0;
    int pos=0;
    int mul=1;
    int c;
    if ((x=dim_d_name2idx(lookup))<0) {
        yyerror("No such dim variable");
        return (MAX_D_VARS+1);
    } 
    for (c=0;c<=dimc;c++) {
        if ((dimensions[c]>dim_d_vars[x].dim[c])||(dimensions[c]==0)) { 
            yyerror("Illegal dimension value(s)");
            return 0;
        }
        pos+=(dimensions[c]-1)*mul;
        mul*=dim_d_vars[x].dim[c];
    }
    dim_d_vars[x].values[pos]=val;
    return x;
}

int dim_d_val2idx(double val,int x) 
{
    int pos=0;
    int mul=1;
    int c;
    for (c=0;c<=dimc;c++) {
        if ((dimensions[c]>dim_d_vars[x].dim[c])||(dimensions[c]==0)) { 
            yyerror("Illegal dimension value(s)");
            return 0;
        }
        pos+=(dimensions[c]-1)*mul;
        mul*=dim_d_vars[x].dim[c];
    }
    dim_d_vars[x].values[pos]=val;
    return pos;
}

/* place value y into dim var with index x */

int dodimvar(int x,double y) 
{
        if (checkignore()) return 0; // HJo ???
    if (x>=MAX_DIM_VARS) {
    yyerror("No such dim variable");
    return 0;
    }
    return dim_d_val2idx(y,x);
}

/* DIM */

void dovardim(char *name) 
{
    int c;
    int size=1;
    if (checkignore()) return;
    if (dim_d_vp==MAX_DIM_VARS) {
        yyerror("Out of dim variable space");
        return;
    }
    for (c=0;c<dim_d_vp;c++) {
        if (strcmp(name,dim_d_vars[dim_d_vp].name)==0) {
            yyerror("Can't re-DIM variable");
            return;
        }
    }
    for (c=0;c<=dimc;c++) {
        if ((c==MAX_DIM)||(dimensions[c]==0)) {
            yyerror("Illegal dimension(s)");
            return;
        }
        dim_d_vars[dim_d_vp].dim[c]=dimensions[c];
        size*=dimensions[c];
    }
    dim_d_vars[dim_d_vp].dim[++c]=0;
    dim_d_vars[dim_d_vp].values=(double *)malloc((sizeof(double))*size);
    strncpy(dim_d_vars[dim_d_vp++].name,name,VARNAME_SIZE);
}

/* translate dim variable name to dim variable index */

int dim_str_name2idx(char *lookup) 
{
    int x;
    for (x=0;x<=dim_str_vp;x++) {
        if (strncmp(dim_str_vars[x].name,lookup,VARNAME_SIZE)==0) return x;
    }
    return -1;
}

/* translate dim string name to value corrosponding name */

char *dim_str_name2val(char *lookup) 
{
    int pos=0;
    int mul=1;
    int x=0;
    int c;
    if ((x=dim_str_name2idx(lookup))<0) return 0;
    for (c=0;c<=dimc;c++) {
        if ((dimensions[c]>dim_str_vars[x].dim[c])||(dimensions[c]==0)) { 
            yyerror("Illegal dimension value(s)");
            return NULL;
        }
        pos+=(dimensions[c]-1)*mul;
        mul*=dim_str_vars[x].dim[c];
    }
    return dim_str_vars[x].values[pos];
}

/* place value in named string dim variable */

int dim_str_val2name(char *val,char *lookup) 
{
    int x=0;
    int pos=0;
    int mul=1;
    int c;
    if ((x=dim_str_name2idx(lookup))<0) {
        yyerror("No such dim variable");
        return (MAX_DIM_STR+1);
    } 
    for (c=0;c<=dimc;c++) {
        if ((dimensions[c]>dim_str_vars[x].dim[c])||(dimensions[c]==0)) { 
            yyerror("Illegal dimension value(s)");
            return 0;
        }
        pos+=(dimensions[c]-1)*mul;
        mul*=dim_str_vars[x].dim[c];
    }
    if (dim_str_vars[x].values[pos]==NULL) {
    dim_str_vars[x].values[pos]=strdup(val);
    } else {
    free(dim_str_vars[x].values[pos]);
    dim_str_vars[x].values[pos]=strdup(val);
    }
    return x;
}

int dim_str_val2idx(char *val,int x) 
{
    int pos=0;
    int mul=1;
    int c;
    for (c=0;c<=dimc;c++) {
        if ((dimensions[c]>dim_str_vars[x].dim[c])||(dimensions[c]==0)) { 
            yyerror("Illegal dimension value(s)");
            return 0;
        }
        pos+=(dimensions[c]-1)*mul;
        mul*=dim_str_vars[x].dim[c];
    }
    if (dim_str_vars[x].values[pos]==NULL) {
        dim_str_vars[x].values[pos]=strdup(val);
    } else {
        free(dim_str_vars[x].values[pos]);
        dim_str_vars[x].values[pos]=strdup(val);
    }

    return pos;
}

/* place value y into dim str with index x */

int dodimstr(int x,char *y) 
{
    if (checkignore()) return 0; // HJo ???
    if (x>=MAX_DIM_STR) {
    yyerror("No such dim variable");
    return 0;
    }
    return dim_str_val2idx(y,x);
}

/* DIM STR */

void dostrdim(char *name) 
{
    int c;
    int size=1;
    if (checkignore()) return;
    if (dim_str_vp==MAX_DIM_STR) {
        yyerror("Out of dim string space");
        return;
    }
    for (c=0;c<dim_str_vp;c++) {
        if (strcmp(name,dim_str_vars[dim_str_vp].name)==0) {
        yyerror("Can't re-DIM variable");
        return;
        }
    }
    for (c=0;c<=dimc;c++) {
        if ((c==MAX_DIM)||(dimensions[c]==0)) {
            yyerror("Illegal dimension(s)");
            return;
        }
        dim_str_vars[dim_str_vp].dim[c]=dimensions[c];
        size*=dimensions[c];
    }
    dim_str_vars[dim_str_vp].dim[++c]=0;
    dim_str_vars[dim_str_vp].values=(char **)malloc((sizeof(char *))*size);
    memset(dim_str_vars[dim_str_vp].values, 0, (sizeof(char *))*size);
    dim_str_vars[dim_str_vp].size=size;
    strncpy(dim_str_vars[dim_str_vp++].name,name,VARNAME_SIZE);
}

/* get variable, used by INPUT */

double fetchvar() 
{
    if (use_stream) {
        if ((use_stream > 0) && 
            (use_stream < MAX_FILES) &&
            (fnum[use_stream].fp != NULL)) {
                if (buf[0]) {
                    char *p = strchr(buf, '\t');
                    if (p)
                        strcpy(buf, p+1);
                    else
                        buf[0] = 0;
                }
                if (buf[0] == 0)
                    fgets(buf,sizeof(buf)-3,fnum[use_stream].fp);
            }
    } else {
        print_text_buffer("? ");
        fgets(buf,sizeof(buf)-3,stdin);
    }
    return atof(buf);
}

/* get string from stdin, used by INPUT */

char *fetchstring() 
{
    char *p;
    memset(buf,0,sizeof(buf));
    if (use_stream) {
        if ((use_stream>0)&&(use_stream<MAX_FILES))
            if (fnum[use_stream].fp!=NULL)
                fgets(buf,STRING_SIZE-1,fnum[use_stream].fp);
    } else {
        print_text_buffer("? ");
        fgets(buf,STRING_SIZE-1,stdin);
    }
    if ((p=strchr(buf,'\n'))!=NULL) *p='\0';
    return (&buf[0]);
}

/* check if we have to ignore current command */

int checkignore(void) 
{
    if (ignore_next_cmd|ignore_till_wend|ignore_after_jump|ignore_end) 
        return 1;
    return 0;
}

/* INKEY */

char doinkey()
{
#ifdef WIN32
    return 0;
#else
    char c;    
    struct termios new;
    struct termios stored;
    if (checkignore()) return ' ';
    tcgetattr(0,&stored);
    memcpy(&new, &stored, sizeof(struct termios));
    new.c_lflag &= (~ECHO);
    new.c_lflag &= (~ICANON);
    new.c_cc[VTIME] = 0;
    new.c_cc[VMIN] = 1;
    tcsetattr(0,TCSANOW,&new);
    c=getc(stdin);
    tcsetattr(0,TCSANOW,&stored);
    return c;
#endif
}

/* SYSTEM */

void dosystem(char *cmd) 
{
   if (checkignore()) return; 
   system(cmd);
}


/* CLS */

void docls() 
{
    if (checkignore()) return;
    print_text_buffer("\x1b[2J\x1b[r");
}

/* BEEP */

void dobeep()
{
    if (checkignore()) return;
    print_text_buffer("\a");
}

/* COLOR */

void docolor(int fg,int bg) 
{
    if (checkignore()) return;
    print_text_buffer("\x1b[%dm",(fg%8)+30);
    print_text_buffer("\x1b[%dm",(bg%8)+40);
}

/* LOCATE */

void dolocate(int line,int column) 
{
    GtkTextIter iter;
    if (checkignore()) return;
    gtk_text_buffer_get_iter_at_line_offset(gtkbuf, &iter, line - 1, column - 1);
    gtk_text_buffer_place_cursor(gtkbuf, &iter);
    //print_text_buffer("\x1b[%d;%dH",line,column);
}

/* set random seed */

void dorandomize(int seed) 
{
    if (checkignore()) return;
#ifdef sun
    seed%=32767;
#else
    seed%=RAND_MAX;
#endif
    srand(seed);
}

/* swap two variables */

void doswapd(int from,int to) 
{
    double dum;
    if (checkignore()) return;
    dum=d_vars[from].value;
    d_vars[from].value=d_vars[to].value;
    d_vars[to].value=dum;
}

/* swap two strings */

void doswapstr(int from,int to) 
{
    char b[STRING_SIZE+1];
    if (checkignore()) return;
    strcpy(b,str_vars[from].name);
    strcpy(str_vars[from].name,str_vars[to].name);
    strcpy(str_vars[to].name,b);
}

/* set environment variable */

void doenviron(char *to,char *from) 
{
    if (checkignore()) return;
    if (g_setenv(to,from,1)) yyerror("Insufficient environment space");
}

/* place value y into var with index x */

void dovar(int x,double y) 
{
    if (checkignore()) return;
    d_vars[x].value=y;
}

/* place string b into string with index idx */

void dostring(int idx,char *b) 
{
    if (checkignore()) return;
    if (str_vars[idx].value!=NULL) free(str_vars[idx].value);
    str_vars[idx].value=strdup(b);
}

/* print variable */

void dovarprint(double var) 
{
    if (checkignore()) return;
    if (use_stream) {
        if ((use_stream>0)&&(use_stream<MAX_FILES))
            if (fnum[use_stream].fp!=NULL)
                fprintf(fnum[use_stream].fp,"%g",var);
    } else {
        print_text_buffer("%g",var);
    }
}

/* print string by value */

void dostringprint(char *content) 
{
    if (checkignore()) return;
    if (use_stream) {
        if ((use_stream>0)&&(use_stream<MAX_FILES))
            if (fnum[use_stream].fp!=NULL)
                fprintf(fnum[use_stream].fp,"%s",content);
    } else {
        print_text_buffer("%s",content);
    }
}

/* get next DATA element */

char *fetchdata() 
{
    char *r;
    if (readdata==NULL) return &nothing[0];
    if (rd_p==NULL) rd_p=readdata;
    r=(char *)rd_p->val;
    rd_p=rd_p->next;
    return r;
}

/* put DATA element into var */

void doreadvar(int idx) 
{
    if (checkignore()) return;
    d_vars[idx].value=atof(fetchdata());
}

/* put DATA element into dim var */

void doreaddimvar(int idx) 
{
    if (checkignore()) return;
    dim_d_val2idx(atof(fetchdata()),idx);
}

/* put DATA element into string */

void doreadstr(int idx) 
{
    if (checkignore()) return;
    if (str_vars[idx].value!=NULL) free(str_vars[idx].value);
    str_vars[idx].value=strdup(fetchdata());
}

/* get input to store in variable indexed by var */

void doinput(int var) 
{
    if (checkignore()) return;
    d_vars[var].value=fetchvar();
}

/* get input to store in dim variable indexed by var */

void doinputdim(int var) 
{
    if (checkignore()) return;
    dim_d_val2idx(fetchvar(),var);
}

/* get input to store in dim string indexed by var */

void doinputstrdim(int var) 
{
    if (checkignore()) return;
    dim_str_val2idx(fetchstring(),var);
}

/* get input to store in string indexed by var */

void dostringinput(int var) 
{
    if (checkignore()) return;
    if (str_vars[var].value!=NULL) free(str_vars[var].value);
    str_vars[var].value=strdup(fetchstring());
}

/* I choosed to use seperate stacks to prevent serious jumping problems */
/* but still isn't recommended to mess around with it                   */

/* pop from GOSUB stack */

int gosub_pop() 
{
    /* printf("pop %d\n",gosub_stack[gosub_sp-1]); */
    return (gosub_sp)?gosub_stack[--gosub_sp]:-1;
}

/* push on GOSUB stack */

int gosub_push(int line) 
{
    /* printf("push %d\n",line); */
    if (gosub_sp<STACK_SIZE) {
        gosub_stack[gosub_sp++]=line;
        return 0;
    } 
    return -1;
}

/* pop from DO stack */

int do_pop() 
{
    return (do_sp)?do_stack[--do_sp]:-1;
}

/* push on DO stack */

int do_push(int line) 
{
    /* printf("push %d\n",line); */
    if (do_sp<STACK_SIZE) {
        do_stack[do_sp++]=line;
        return 0;
    } 
    return -1;
}

/* pop from WHILE stack */

int while_pop() 
{
    return (while_sp)?while_stack[--while_sp]:-1;
}

/* push on WHILE stack */

int while_push(int line) 
{
    if (while_sp<STACK_SIZE) {
        while_stack[while_sp++]=line;
        return 0;
    } 
    return -1;
}


void dojumpidx(int idx) 
{
    struct l_list *l;
    int cnt;
    ignore_after_jump=0;
    did_jump=1;
    l=jumplist;
    //printf("jumping to %d\n",idx); 
    for (cnt=1;cnt!=idx;cnt++) l=l->next;
    //printf("  jump idx=%d pos=%ld\n", idx, ((struct jumpstruct *)(l->val))->pos);
    fseek(yyin,((struct jumpstruct *)(l->val))->pos,SEEK_SET);
    yyrestart(yyin);
    pushback('\n');/* prevent yacc from going nuts after jump */
    cmdcnt=idx-2;
}

void dojump(int line_to_go)
{
    long result;
    ignore_after_jump=0;
    did_jump=1;
    if ((result=line2pos(line_to_go))<0) {
        yyerror("Can't jump to non-existend linenumber");
        return;
    }
    dojumpidx(result);
}

/* GOTO */

void dogoto(int line_to_go) 
{
    if (checkignore()) return;
    dojump(line_to_go);
}

/* ON .. GOTO */

void doongoto(int var) 
{
    if (checkignore()) return;
    ignore_next_cmd=1;ignore_else_cmd=1;
    if ((var>0)&&((var-2)<onlp)) {
        dojump(onlines[var-1]);
    } 
}

/* ON .. GOSUB */

void doongosub(int var) 
{
    if (checkignore()) return;
    ignore_next_cmd=1;ignore_else_cmd=1;
    if ((var>0)&&((var-2)<onlp)) {
    if (gosub_push(cmdcnt)<0) 
        yyerror("Can't go to subroutine; stack full");
    else
        dojump(onlines[var-1]);
    }
}

/* GOSUB */

void dogosub(int line_to_go) 
{
    if (checkignore()) return;
    if (gosub_push(cmdcnt+1)<0) 
        yyerror("Can't go to subroutine; stack full");
    else
        dojump(line_to_go);
}

/* DO */
void dodo() 
{
    if (checkignore()) return;
    if (do_push(cmdcnt)<0) yyerror("Can't do DO command; stack full");
}

/* UNTIL */
void dountil(double mode)
{
    int to;
    if (checkignore()) return;
    if ((to=do_pop())<0)  {
    	yyerror("No DO matching UNTIL");
	return;
    }	
    if (mode==B_FALSE) {
	dojumpidx(to+1);
	do_push(to);
    }
}

/* RETURN */

void doreturn() 
{
    int line_to_go;
    if (checkignore()) return;
    ignore_next_cmd=1;ignore_else_cmd=1;
    if ((line_to_go=gosub_pop())<0)  {
        yyerror("No GOSUB matching RETURN");
    } else {
        dojumpidx(line_to_go+1);
    }
}


/* push FOR..NEXT info on stack                                        */
/* 'cause counter could also be a DIM-ed var, position inside this var */
/* as to be passed too                                                 */

int for_push(int idx,int end,int step,int jumpto,int type,int pos) 
{
    if (for_sp<STACK_SIZE) {
        for_stack[for_sp].idx=idx;
        for_stack[for_sp].end=end;
        for_stack[for_sp].step=step;
        for_stack[for_sp].dimflag=type;
        for_stack[for_sp].pos=pos;
        for_stack[for_sp++].jumpto=jumpto;
        return 0;
    } 
    return -1;
}

/* pop FOR..NEXT info from stack */

int for_pop() 
{
    int idx,pos,dimflag;
    if (for_sp<1) return -1;
    idx=for_stack[--for_sp].idx;
    pos=for_stack[for_sp].pos;
    dimflag=for_stack[for_sp].dimflag;
    if (dimflag) {
        dim_d_vars[idx].values[pos]+=for_stack[for_sp].step;
        if (dim_d_vars[idx].values[pos]>for_stack[for_sp].end) return 0;
    } else {
        d_vars[idx].value+=for_stack[for_sp].step;
        if (d_vars[idx].value>for_stack[for_sp].end) return 0;
    }

    return (for_stack[for_sp++].jumpto);
}

/* FOR .. TO .. STEP */
void print_for_stack(void);

void dofor(int idx,int val,int end,int step,int type) 
{
    int pos=0;
    if (checkignore()) return;
    if (type)
        pos=dodimvar(idx,val);
    else
        dovar(idx,val);
    for_push(idx,end,step,cmdcnt+1,type,pos);
}

/* NEXT */

void donext() 
{
    int result;
    if (checkignore()) return;
    if ((result=for_pop())<0) {
        yyerror("no FOR matching NEXT");
    } else {
        if (result) {
            dojumpidx(result);
        } 
    }
}

/* WHILE */

void dowhile(int condition) 
{
    if (checkignore()) return;
    if (condition==B_TRUE) {
        if (while_push(cmdcnt)<0) yyerror("Can't do while; stack full");
    } else {
        ignore_till_wend=1;
    }
}

/* WEND */

void dowend()  
{
    int result;
    int old=ignore_till_wend;
    ignore_till_wend=0;
    if (checkignore()) return;
    if (!old) {
            if ((result=while_pop())>0) {
            dojumpidx(result);
            }
    }
}

/* END */

extern void quit_flex(void);

void doend() 
{
    if (checkignore()) return;
    ignore_end=1;
    quit_flex();
}
void doend1() 
{
        g_print("\n---DOEND1---\n\n");
    ignore_end=1;
}

/* KILL */

void dokill(char *file) 
{
    char er[400];
    if (checkignore()) return;
    if (unlink(file)<0) {
        sprintf(er,"Unable to do KILL (%s)",strerror(errno));
        yyerror(er);
    }
}

/* RMDIR */

void dormdir(char *dir) 
{
    char er[400];
    if (checkignore()) return;
    if (rmdir(dir)<0) {
        sprintf(er,"Unable to do RMDIR (%s)",strerror(errno));
        yyerror(er);
    }
}

/* NAME */

void doname(char *from,char *to) 
{
    char er[400];
    if (checkignore()) return;
    if (rename(from,to)<0) {
        sprintf(er,"Unable to do NAME (%s)",strerror(errno));
        yyerror(er);
    }
}

/* RESTORE */

void dorestore() 
{
    if (checkignore()) return;
    rd_p=readdata;
}

/* RSET */

void dorset(int idx,char *exp) 
{
    if (checkignore()) return;
    memset(buf,0,sizeof(buf));
    strcpy(buf,str_vars[idx].value);
    strncpy(&buf[strlen(buf)],exp,STRING_SIZE);
    buf[STRING_SIZE]='\0';
    dostring(idx,buf);
}        


/* LSET */

void dolset(int idx,char *exp) 
{
    if (checkignore()) return;
    memset(buf,0,sizeof(buf));
    strncpy(buf,exp,STRING_SIZE);
    strcpy(&buf[strlen(buf)],str_vars[idx].value);
    buf[STRING_SIZE]='\0';
    dostring(idx,buf);
}        


/* SEEK */

void doseek(int fno,double to) 
{
    /* the fun part is that I don't have to check if the range is higher */
    /* then the size of the file, because the gap will be filled with    */
    /* zero's...                                                         */
    long p=to;
    if (checkignore()) return;
    if ((fno>0)&&(fno<MAX_FILES))
    if (fnum[fno].fp!=NULL) fseek(fnum[fno].fp,p,SEEK_SET);
}        


/* CHDIR */

void dochdir(char *dir) 
{
    char er[400];
    if (checkignore()) return;
    if (chdir(dir)<0) {
        sprintf(er,"Unable to do CHDIR (%s)",strerror(errno));
        yyerror(er);
    }
}

/* MKDIR */

void domkdir(char *dir) 
{
    char er[400];
    if (checkignore()) return;
    if (g_mkdir(dir,DEF_UMASK)<0) {
        sprintf(er,"Unable to do MKDIR (%s)",strerror(errno));
        yyerror(er);
    }
}

/* OPEN */

void doopen(char *file, int mode, int no,int len) 
{
    char md[]="wra";
    char bf[2];
    char er[400];
    if (checkignore()) return;
    if (no<1) return;
    if (fnum[no].fp!=NULL) {
        yyerror("Filenumber already in use, close first");
        return;
    } 
    sprintf(bf,"%c",md[mode]);
    if ((no>0)&&(no<MAX_FILES)) {
        fnum[no].fp=fopen(file,bf);    
        if (fnum[no].fp==NULL) {
            sprintf(er,"Cannot open file (%s)",strerror(errno));
            yyerror(er);
        }
    if (len<1) len=1;
    if (len>STRING_SIZE) len=STRING_SIZE;
        fnum[no].len=len;
    }
}

/* CLOSE */

void doclose(int no) 
{
    if (checkignore()) return;
    if (fnum[no].fp!=NULL) {
        fclose(fnum[no].fp);
    }
    fnum[no].fp=NULL;
}

/* PUT is a problem, wat if recordlen is smaller then a double ?  */
/* I solved it by converting double to int, if thats to big       */
/* I convert it to unsigned char (=1 byte)                        */

/* PUT var */

void doputvar(int fno,int rec,double value) 
{
    unsigned char dum=0;
    int iv;
    unsigned char cv;
    if (checkignore()) return;
    if ((fno>0)&&(fno<MAX_FILES)&&(rec>0)) {
        if (fnum[fno].fp!=NULL) {
            fseek(fnum[fno].fp,fnum[fno].len*(rec-1),SEEK_SET);
            if (fnum[fno].len>=sizeof(double)) {
                if (fwrite(&value,sizeof(value),1,fnum[fno].fp)<1)
                    yyerror("File write error");
                if (fnum[fno].len>sizeof(double)) {
                    if (fwrite(&dum,1,fnum[fno].len-sizeof(double),
                        fnum[fno].fp)<1)
                    yyerror("File write error");
                }
                return;
            }
            iv=value;
            if (fnum[fno].len>=sizeof(int)) {
                if (fwrite(&iv,sizeof(iv),1,fnum[fno].fp)<1)
                    yyerror("File write error");
                if (fnum[fno].len>sizeof(int)) {
                    if (fwrite(&dum,1,fnum[fno].len-sizeof(int),fnum[fno].fp)<1)
                    yyerror("File write error");
                }
                return;
            }
            cv=iv%256;
            if (fwrite(&cv,1,1,fnum[fno].fp)<1) yyerror("File write error");
            return;
        } else { 
           yyerror("File not open");
        }
    } 
}

/* PUT string */

void doputstr(int fno,int rec,char *value) 
{
    if (checkignore()) return;
    memset(buf,0,sizeof(buf));
    strncpy(buf,value,STRING_SIZE);
    if ((fno>0)&&(fno<MAX_FILES)&&(rec>0)) {
        if (fnum[fno].fp!=NULL) {
            fseek(fnum[fno].fp,fnum[fno].len*(rec-1),SEEK_SET);
            if (fwrite(&buf[0],fnum[fno].len,1,fnum[fno].fp)<1)
                yyerror("File write error");
        }
    }
}

/* GET var */

void dogetvar(int fno,int rec,int idx) 
{
    int iv;
    unsigned char cv;
    double dv;
    if (checkignore()) return;
    if ((fno>0)&&(fno<MAX_FILES)&&(rec>0)) {
        if (fnum[fno].fp!=NULL) {
            fseek(fnum[fno].fp,fnum[fno].len*(rec-1),SEEK_SET);
            if (fnum[fno].len>=sizeof(double)) {
                if (fread(&dv,sizeof(dv),1,fnum[fno].fp)<1) {
                    yyerror("File read error");
                    return;
                }
                if (fnum[fno].len>sizeof(double)) {
                    if (fread(&buf,1,fnum[fno].len-sizeof(double),
                        fnum[fno].fp)<1) {
                        yyerror("File read error");
                        return;
                    }
                    dovar(idx,dv);                
                }
                return;
            }
            if (fnum[fno].len>=sizeof(int)) {
                if (fread(&iv,sizeof(iv),1,fnum[fno].fp)<1)
                    yyerror("File write error");
                if (fnum[fno].len>sizeof(int)) {
                    if (fread(&buf,1,fnum[fno].len-sizeof(int),fnum[fno].fp)<1)
                    yyerror("File write error");
                    dv=iv;
                    dovar(idx,dv);                
                }
                return;
            }
            if (fread(&cv,1,1,fnum[fno].fp)<1) {
                yyerror("File write error");
                return;
            }
            dv=cv;
            dovar(idx,dv);                
            return;
        } else { 
           yyerror("File not open");
        }
    } 
}

/* GET string */

void dogetstr(int fno,int rec,int idx) 
{
    if (checkignore()) return;
    if ((fno>0)&&(fno<MAX_FILES)&&(rec>0)) {
        if (fnum[fno].fp!=NULL) {
            fseek(fnum[fno].fp,fnum[fno].len*(rec-1),SEEK_SET);
            if (fread(&buf[0],fnum[fno].len,1,fnum[fno].fp)<1) {
                yyerror("Error reading file");
                return;
            }
            if (str_vars[idx].value!=NULL)
                free(str_vars[idx].value=strdup(buf));
            str_vars[idx].value=strdup(buf);
        }
    }
}


%}

%token LET REM CLS PRINT INPUT FOR TO NEXT STEP LOCATE DATA
%token IF THEN ELSE GOTO GOSUB RETURN ENVIRON ON RANDOMIZE
%token WHILE WEND END SWAP BEEP CHDIR OPEN AS OUTPUT MKDIR
%token RMDIR CLOSE KILL APPEND GET PUT NAME LSET RSET 
%token READ RESTORE FREEFILE DIM COLOR SYSTEM DO UNTIL

%token CHR HEX INSTR LEFT RIGHT MID OCT ENVIRONSTR 
%token SPACE STR STRINGSTR LOWER UPPER RTRIM LTRIM INKEY

%token ASC ABS LEN VAL ATN COS SIN RND SGN INT CINT EXP
%token TAN LOG SQR ASIN ACOS LOF LOC BEOF SEEK 

%token LINENUMBER NUMBER VAR DIMVAR DIMSTR STRING PASSEDTEXT

%token NL SEMICOLON
%token AND OR XOR NOT MOD

%token SQL

%left ','
%left SEMICOLON
%nonassoc UMINUS NOT

%left AND OR XOR 
%left '<' '>' '=' 
%left MOD
%left '-' '+' 
%left '\\'
%left '*' '/' 
%left '^'

%union {
    double d_val;
    char str[STRING_SIZE];
}

%type <d_val> LINENUMBER NUMBER SEMICOLON
%type <d_val> expression dimvaridx dimstridx dimvar varidx var goto 
%type <d_val> stringidx modes
%type <str>   STRING PASSEDTEXT VAR DIMVAR DIMSTR strexp 
%%



program: commandline
       | program commandline
       ;

commandline: linenumber commandline
           | commands newline
           | newline
           ; 

commands: single_command
        | single_command colon commands
        | ifthen commands
        | ifthen single_command else commands
        ;

single_command: CLS                {docls();}
              | stringidx '=' strexp                    {dostring($1,$3);}
              | varidx '=' expression                   {dovar($1,$3);}
              | dimvaridx '=' expression                {dodimvar($1,$3);}
              | dimstridx '=' strexp                    {dodimstr($1,$3);}
              | LET dimstridx '=' strexp                {dodimstr($2,$4);}
              | LET dimvaridx '=' expression            {dodimvar($2,$4);}
              | LET stringidx '=' strexp                {dostring($2,$4);}
              | LET varidx '=' expression               {dovar($2,$4);}
              | SQL strexp                              {sql($2);}
              | SQL fnum strexp                         {sql($3);use_stream=0;}
              | PRINT                                   {dostringprint("\n");}
              | PRINT printoptions                      {dostringprint("\n");}
              | PRINT printoptions SEMICOLON            {}        

              | PRINT fnum         {dostringprint("\n");use_stream=0;}
              | PRINT fnum printoptions  
                                   {dostringprint("\n");use_stream=0;}
              | PRINT fnum printoptions SEMICOLON       {}        
              | INPUT istrexp SEMICOLON invarlist       {}
              | INPUT invarlist                         {}
              | INPUT fnum invarlist                    {use_stream=0;}
              | BEEP               {dobeep();}
              | GOSUB LINENUMBER   {dogosub($2);}
              | RETURN             {doreturn();}
              | FOR varidx '=' expression TO expression {dofor($2,$4,$6,1,0);}
              | FOR varidx '=' expression TO expression STEP expression 
                                   {dofor($2,$4,$6,$8,0);}
              | FOR dimvaridx '=' expression TO expression 
                   {dofor($2,$4,$6,1,1);}
              | FOR dimvaridx '=' expression TO expression STEP expression 
                                   {dofor($2,$4,$6,$8,1);}
              | NEXT               {donext();}
              | LOCATE expression ',' expression        {dolocate($2,$4);}
              | COLOR expression ',' expression        {docolor($2,$4);}
              | goto               {dogoto($1);}
              | ENVIRON strexp '=' strexp               {doenviron($2,$4);}
              | ON expression GOTO numberlist           {doongoto($2);}
              | ON expression GOSUB numberlist          {doongosub($2);}
              | RANDOMIZE expression                    {dorandomize($2);}
              | SWAP varidx ',' varidx                  {doswapd($2,$4);}
              | SWAP stringidx ',' stringidx            {doswapstr($2,$4);}
              | WHILE expression                        {dowhile($2);}
              | WEND                                    {dowend();}
              | REM                                     {}
              | DATA                                    {}
              | END                                     {doend();}
              | CHDIR strexp                            {dochdir($2);}
              | MKDIR strexp                            {domkdir($2);}
              | RMDIR strexp                            {dormdir($2);}
              | KILL strexp                             {dokill($2);}
              | OPEN strexp FOR modes AS '#' NUMBER     
                         {doopen($2,$4,$7,DEFAULT_OPEN_LEN);}
              | OPEN strexp FOR modes AS '#' NUMBER LEN NUMBER
                         {doopen($2,$4,$7,$9);}
              | GET '#' NUMBER ',' NUMBER ',' varidx    {dogetvar($3,$5,$7);}
              | GET '#' NUMBER ',' NUMBER ',' stringidx {dogetstr($3,$5,$7);}
              | PUT '#' NUMBER ',' NUMBER ',' expression
                         {doputvar($3,$5,$7);}
              | PUT '#' NUMBER ',' NUMBER ',' strexp    {doputstr($3,$5,$7);}
              | CLOSE '#' NUMBER                        {doclose($3);}
              | NAME strexp AS strexp                   {doname($2,$4);}
              | SEEK '#' NUMBER ',' NUMBER              {doseek($3,$5);}
              | LSET stringidx '=' strexp               {dolset($2,$4);}
              | RSET stringidx '=' strexp               {dorset($2,$4);}
              | READ rdvarlist                          {}
              | RESTORE                                 {dorestore();}
              | DIM DIMVAR dimexp ')'                   {dovardim($2);}
              | DIM DIMSTR dimexp ')'                   {dostrdim($2);}
              | SYSTEM strexp ')'                       {dosystem($2);}
	      | DO                                      {dodo();}
	      | UNTIL expression                        {dountil($2);}
              ;

modes: OUTPUT {$$=0;}
     | INPUT  {$$=1;}
     | APPEND {$$=2;}
     ;

istrexp:strexp {dostringprint($1);}
       ;

invarlist: invarlist ',' varidx    {doinput($3);}
         | varidx                  {doinput($1);}
         | invarlist ',' dimvaridx {doinputdim($3);}
         | dimvaridx               {doinputdim($1);}
         | invarlist ',' dimstridx {doinputstrdim($3);}
         | dimstridx               {doinputstrdim($1);}
         | invarlist ',' stringidx {dostringinput($3);}
         | stringidx               {dostringinput($1);}
         ;

dimexp: dimexp ',' expression      {dimensions[++dimc]=$3;}
      | expression                 {dimc=0;dimensions[dimc]=$1;}
      ;

rdvarlist: rdvarlist ',' varidx    {doreadvar($3);}
         | varidx                  {doreadvar($1);}
         | rdvarlist ',' dimvaridx {doreaddimvar($3);}
         | dimvaridx               {doreaddimvar($1);}
         | rdvarlist ',' stringidx {doreadstr($3);}
         | stringidx               {doreadstr($1);}
         ;


goto: GOTO LINENUMBER {$$=$2;}
    | NUMBER      {$$=$1;/* only used in THEN xx [ELSE xx] statements */}
    ;

ifthen: IF expression THEN {
          ignore_next_cmd|=($2==B_TRUE)?0:1;
          ignore_else_cmd|=($2==B_TRUE)?1:0;
      }
      ;

else: ELSE { ignore_next_cmd=ignore_else_cmd;}
    ;

fnum: '#' NUMBER {use_stream=$2;}
    ;

printoptions: expression                          {dovarprint($1);}
            | strexp                              {dostringprint($1);}
            | printz printoptions  %prec SEMICOLON 
            ;

printz: printoptions SEMICOLON  
      {/* printz is a dirty trick to get 2 ways to concat strings */;}
      | printoptions ',' {dostringprint("\t");}
      ;

stringidx: STRING {
             $$=str_name2idx($1);
             if (!(checkignore())) {
                if ($$<0) $$=str_val2name("",$1);
             }
         }
         ;

varidx: VAR {
        $$=d_name2idx($1);
        if ($$<0) {
            if (!(checkignore())) {
                $$=d_val2name(0,$1);
            }
        }
      }
      ;

dimvaridx: DIMVAR dimexp ')' {$$=dim_d_name2idx($1);}
         ;

dimstridx: DIMSTR dimexp ')' {$$=dim_str_name2idx($1);}
         ;

var: VAR {$$=d_name2val($1);}
   ;

dimvar: DIMVAR dimexp ')' {$$=dim_d_name2val($1);}
      ;

expression: expression '+' expression     {$$=$1+$3;}
          | expression '-' expression     {$$=$1-$3;}
          | expression MOD expression     {
            int c1=$1;
            int c3=$3;
            $$=c1%c3;
          }
          | expression '*' expression     {$$=$1*$3;}
          | expression '^' expression     {$$=pow($1,$3);}
          | expression '/' expression     {
              if ($3==0.0) 
                yyerror("Error: division by zero\n");
            else
                $$=$1/$3;
            }
          | expression '\\' expression     {
              if ($3==0.0) 
                yyerror("Error: division by zero\n");
            else
                $$=rint($1/$3);
            }
          | '-' expression %prec UMINUS   {$$=-$2;}
          | '(' expression ')'            {$$= $2;}
          | NUMBER                        {$$=$1;}
          | var 
          | dimvar 
          | ASC strexp ')'            {$$=$2[0];}
          | ATN expression ')'       {$$=atan($2);}
          | ASIN expression ')'       {$$=asin($2);}
          | ACOS expression ')'       {$$=acos($2);}
          | CINT expression ')'       {$$=rint($2);}
          | INT expression ')'        {$$=floor($2);}
          | SIN expression ')'        {$$=sin($2);}
          | TAN expression ')'        {$$=tan($2);}
          | SQR expression ')'        {$$=sqrt($2);}
          | LOF '#' NUMBER                {
                int c=$3;
                long p;
                $$=0;
                if ((c>0)&&(c<MAX_FILES)) {
                    if (fnum[c].fp!=NULL) {
                        p=ftell(fnum[c].fp);
                        fseek(fnum[c].fp,0,SEEK_END);
                        $$=ftell(fnum[c].fp);
                        fseek(fnum[c].fp,p,SEEK_SET);
                    }
               }
          }
          | LOC '#' NUMBER {
            int c=$3;
            $$=0;
            if ((c>0)&&(c<MAX_FILES)) {
                if (fnum[c].fp!=NULL) {
                    $$=ftell(fnum[c].fp);
                    } 
                }
          }
          | BEOF '#' NUMBER {
            int c=$3;
            if ((c>0)&&(c<MAX_FILES)) {
                if (fnum[c].fp!=NULL) {
                    $$=(feof(fnum[c].fp))?B_TRUE:B_FALSE;
                } 
            }
          }        
          | LOG expression ')'        {$$=log($2);}
          | COS expression ')'        {$$=cos($2);}
          | EXP expression ')'        {$$=exp($2);}
          | SGN expression ')'        {$$=($2==0.0)?0.0:$2/abs($2);}
          | RND expression ')'        {int c=$2;$$=rand()%c;}
          | LEN strexp ')'            {$$=strlen($2);}
          | VAL strexp ')'            {$$=atof($2);}
          | INSTR strexp ',' strexp ')' {
              char *f;
              f=strstr($2,$4);
              if (f==NULL) {
                  $$=B_FALSE;
              } else {
                  $$=(f-$2)+1;
              }
          }
          | INSTR expression ',' strexp ',' strexp ')' {
              char *f;
              int c=$2;
              if (($2<=strlen($4))&&($2>0)) {
                  f=strstr(($4+c-1),$6);
                  if (f==NULL) {
                    $$=0.0;
                  } else {
                    $$=(f-$4)+1;
                  }
              } else {
                $$=0.0;
              }
          }
          | ABS expression ')'        {$$=($2<0)?-1*$2:$2;}
          | expression '=' expression     {$$=($1==$3)?B_TRUE:B_FALSE;}
          | expression '>' expression     {$$=($1 >$3)?B_TRUE:B_FALSE;}
          | expression '<' expression     {$$=($1 <$3)?B_TRUE:B_FALSE;}
          | expression '<' '>' expression {$$=($1!=$4)?B_TRUE:B_FALSE;}
          | expression '<' '=' expression {$$=($1<=$4)?B_TRUE:B_FALSE;}
          | expression '=' '<' expression {$$=($1<=$4)?B_TRUE:B_FALSE;}
          | expression '>' '=' expression {$$=($1>=$4)?B_TRUE:B_FALSE;}
          | expression '=' '>' expression {$$=($1>=$4)?B_TRUE:B_FALSE;}
          | expression AND expression     {
            int c1=$1;
            int c3=$3;
            if ((($1==B_TRUE)&&($1==B_FALSE))||(($3==B_TRUE)&&($3==B_FALSE))) {
                $$=($1==$3==B_TRUE)?B_TRUE:B_FALSE;
            } else {
                $$=c1&c3;
            }
          }
          | expression OR  expression     {
            int c1=$1;
            int c3=$3;
            if ((($1==B_TRUE)&&($1==B_FALSE))||(($3==B_TRUE)&&($3==B_FALSE))) {
                $$=(($1==B_TRUE)||($3==B_TRUE))?B_TRUE:B_FALSE;
            } else {
                $$=c1|c3;
            }
          }
          | expression XOR expression     {
            int c1=$1;
            int c3=$3;
            if ((($1==B_TRUE)&&($1==B_FALSE))||(($3==B_TRUE)&&($3==B_FALSE))) {
                $$=((($1==B_TRUE)&&($3==B_FALSE))||
                (($1==B_FALSE)&&($3==B_TRUE)))?B_TRUE:B_FALSE;
            } else {
                $$=c1^c3;
            }
          }
          | FREEFILE                      {
            int c=1;
            while ((c<MAX_FILES)&&(fnum[c].fp!=NULL)) c++;
            $$=c;        
          }
          | NOT expression                {
            int c2=$2;
            if (($2==B_TRUE)||($2==B_FALSE)) {
                $$=($2==B_FALSE)?B_TRUE:B_FALSE;
            } else {
                $$=~c2;
            }
          }
          | strexp '=' strexp           {$$=(strcmp($1,$3))?B_FALSE:B_TRUE;}
          | strexp '>' strexp           {$$=(strcmp($1,$3)>0)?B_TRUE:B_FALSE;}
          | strexp '<' strexp           {$$=(strcmp($1,$3)<0)?B_TRUE:B_FALSE;}
          | strexp '=' '>' strexp        {$$=(strcmp($1,$4)>=0)?B_TRUE:B_FALSE;}
          | strexp '>' '=' strexp        {$$=(strcmp($1,$4)>=0)?B_TRUE:B_FALSE;}
          | strexp '=' '<' strexp        {$$=(strcmp($1,$4)<=0)?B_TRUE:B_FALSE;}
          | strexp '<' '=' strexp        {$$=(strcmp($1,$4)<=0)?B_TRUE:B_FALSE;}
          | strexp '<' '>' strexp        {$$=(strcmp($1,$4)!=0)?B_TRUE:B_FALSE;}
          ;

strexp: strexp '+' strexp  {
          sprintf(buf,"%s%s",$1,$3);
          memset($$,0,sizeof($$));
          strncpy($$,buf,sizeof($$)-1);
      }
      | strexp '+' expression {
          if ($3-trunc($3) == 0) {
              int a = $3;
              sprintf(buf, "%s%d", $1, a);
          } else
              sprintf(buf, "%s%f", $1, $3);
          memset($$,0,sizeof($$));
          strncpy($$,buf,sizeof($$)-1);
      }
      | DIMSTR dimexp ')' {
         char *result;
         result=dim_str_name2val($1);
         if (result==NULL) { 
             $$[0]='\0';
         } else {
            strcpy($$,result);
         }
      }
      | STRING {
         char *result;
         result=str_name2val($1);
         if (result==NULL) { 
            str_val2name("",$1);
            result=str_name2val($1);
         }
         strcpy($$,result);
      }     
      | INKEY                  {$$[0]=doinkey();$$[1]='\0';}
      | PASSEDTEXT             {strncpy($$,$1,STRING_SIZE);}
      | STR expression ')' {sprintf($$,"%g",$2);}
      | CHR expression ')' {int c=$2;$$[0]=c%255;$$[1]='\0';}
      | STRINGSTR expression ',' expression ')' {
            int c=$2;
            if (c>STRING_SIZE) c=STRING_SIZE;
            memset($$,0,sizeof($$));
            while (c>0) $$[--c]=$4;
      }
      | STRINGSTR expression ',' strexp ')' {
        int c=$2;
        memset($$,0,sizeof($$));
          while(c>0) $$[--c]=$4[0];
      }
      | SPACE expression ')' {
        int c=$2;
        memset($$,0,sizeof($$));
        while(c>0) $$[--c]=' ';
      }
      | LOWER strexp ')' {
        int c=0;
        memset($$,0,sizeof($$));
        while((c<STRING_SIZE)&&($2[c])) {    
            $$[c]=tolower($2[c]);
            c++;
        }
      }
      | UPPER strexp ')' {
        int c=0;
        memset($$,0,sizeof($$));
        while((c<STRING_SIZE)&&($2[c])) {    
            $$[c]=toupper($2[c]);
            c++;
        }
      }
      | LTRIM strexp ')' {
        char *p;
        memset($$,0,sizeof($$));
        p=$2;
        while(isspace(*p)) p++;
        strncpy($$,p,STRING_SIZE);
      }
      | RTRIM strexp ')' {
        char *p;
        memset($$,0,sizeof($$));
        p=$2+strlen($2)-1;
        while(isspace(*p)) p--;
        *(p+1)='\0';
        strncpy($$,$2,STRING_SIZE);
      }
      | ENVIRONSTR strexp ')' {
        char *e;
        e=getenv($2);
        if (e==NULL) 
            $$[0]='\0';
        else
            strncpy($$,e,STRING_SIZE);
      }
      | MID strexp ',' expression ')' {
        int c=$4;
        if (($4>strlen($2))||($4<1)) {
            $$[0]='\0';
        } else {
            memset($$,0,sizeof($$));
        strcpy($$,$2+c-1);
        }
      }
      | MID strexp ',' expression ',' expression ')' {
        int c=$4;
        if (($4>strlen($2))||($4<1)||($6<1)) {
            $$[0]='\0';
        } else {
            memset($$,0,sizeof($$));
            strncpy($$,$2+c-1,$6);
        }
      }
      | LEFT strexp ',' expression ')' {
        memset($$,0,sizeof($$));
        if ($4>0)
            strncpy($$,$2,$4);
        else 
            $$[0]='\0';
      }
      | RIGHT strexp ',' expression ')' {
        int off;
        off=strlen($2)-$4;
        if (off<0) off=0;
        memset($$,0,sizeof($$));
        strncpy($$,$2+off,STRING_SIZE);
      }
      | HEX expression ')' {int c=$2;sprintf($$,"%X",c);}
      | OCT expression ')' {int c=$2;sprintf($$,"%o",c);}
      ;

numberlist: numberlist ',' NUMBER  {onlines[++onlp]=$3;}
          | NUMBER                 {onlp=0;onlines[0]=$1;}
          ;

linenumber: LINENUMBER {current_line=$1;}
          ;

newline: NL {ignore_else_cmd=0;ignore_next_cmd=0;lc++;}
       ;

colon: ':' {ignore_else_cmd=0;ignore_next_cmd=0;cmdcnt++;}
        ;


%%

void print_for_stack(void)
{
        int i;
        printf("FOR STACK:\n");
        for (i = 0; i < for_sp; i++)
                printf("%d: idx=%d end=%d step=%f jumpto=%d dimflag=0x%x pos=%d\n",
                       i, for_stack[i].idx, for_stack[i].end, for_stack[i].step, 
                       for_stack[i].jumpto, for_stack[i].dimflag, for_stack[i].pos);
}

char * do_index(char *line, int linenum) {
    char * dkwc=NULL;
    char *p;
    char old;
    int quote=0;
    int lineno=0;
    char dbuf[5];
    struct l_list *l=NULL;
    memset(dbuf,0,sizeof(dbuf));
    lc=0;
    p=&line[lc];
    while(isspace(*p)||isdigit(*p)) p++;
    old=*p;
    *p='\0';
    lineno=atof(line);
    if (lineno == 0) lineno = linenum+10000; // some value
    *p=old;
    while(isspace(*p)) p++;
    strncpy(buf,p,4);
    buf[0]=toupper(buf[0]);
    buf[1]=toupper(buf[1]);
    buf[2]=toupper(buf[2]);
    buf[3]=toupper(buf[3]);
    if (strcmp(buf,"DATA")==0) dkwc=p;
    if (jli==0) {
        jumplist=(struct l_list *)malloc(sizeof(struct l_list));
        l=jumplist;
    } else {
        l=jumplist;
        while((l->next)!=NULL) l=l->next;
        l->next=(struct l_list *)malloc(sizeof(struct l_list));
        l=l->next;
    }
    jli++;
    if (l!=NULL) {
        l->val=(void *)malloc(sizeof(struct jumpstruct));
        if (l->val==NULL) {
            yyerror("No memory left for program information");
	    return NULL;
	}
        ((struct jumpstruct *)(l->val))->line=lineno;
        ((struct jumpstruct *)(l->val))->pos=indexcnt;
        l->next=NULL;
    } else {
	yyerror("No memory left for program information");
	return NULL;
    }
    while(line[lc]) {
        if (line[lc]=='"') 
            quote^=1;
        else
            if ((line[lc]==':')&&(!(quote))) {
	    	p=&line[lc]+1;
		while(isspace(*p)) p++;
		strncpy(buf,p,4);
		buf[0]=toupper(buf[0]);
		buf[1]=toupper(buf[1]);
		buf[2]=toupper(buf[2]);
		buf[3]=toupper(buf[3]);
		if (strcmp(buf,"DATA")==0) dkwc=p;
                l->next=(struct l_list *)malloc(sizeof(struct l_list));
                l=l->next;
                if (l!=NULL) {
                    l->val=(void *)
                        malloc(sizeof(struct jumpstruct));
                    if (l->val==NULL) {
			yyerror("No memory left for program information");
			return NULL;
		    }
                    ((struct jumpstruct *)(l->val))->line=lineno;
                    ((struct jumpstruct *)(l->val))->pos=indexcnt+1;
                    l->next=NULL;
                } else {
		    yyerror("No memory left for program information");
		    return NULL;
                }
                jli++;
            }
        lc++;
        indexcnt++;
    }
    return dkwc;
}

int preparse() {
    char line[MAX_LINE_WIDTH+1];
    char *lp;
    struct l_list *p=NULL;
    int quotestarted=0;
    int linec=0;
    int bp=0;
    char *rslt;
    while(!(feof(yyin))) {
        lp=&line[0];
        memset(line,0,sizeof(line));
        fgets(line,MAX_LINE_WIDTH,yyin);
        linec++;
        if (line[MAX_LINE_WIDTH]) {
            fprintf(stderr,"Programline on line %d exceeds maximal characters\n"
                ,linec);
            return -1;
        }
	rslt=do_index(&line[0], linec);
	if (rslt!=NULL) {
	    lp=rslt;
	    memset(buf,0,sizeof(buf));
	    bp=0;
	    lp=lp+3;
	    while(*(lp++)) {
	    	if ((*lp==':')&&(!(quotestarted))) {
		    rewind(yyin);
		    return 1;
		}
		if (isalnum(*lp)|(*lp=='.')|quotestarted) {
		    if ((*lp)!='"') 
			buf[bp++]=*lp;
		    else
			quotestarted=0;
		} else {
		    if ((*lp)=='"') quotestarted=1;
		    if (bp) {
			if (p==NULL) {
			    p=(struct l_list *)
				malloc(sizeof(struct l_list));
			    readdata=p;
			} else {
			    p->next=(struct l_list *)
				malloc(sizeof(struct l_list));
			    p=p->next;
			}
			if (p!=NULL) {
			    p->val=(void *)strdup(buf);
			    p->next=NULL;
			} else {
			    yyerror("No memory left for DATA");
			    return -2;
			}
			if (p->val==NULL) {
			    yyerror("No memory left for DATA");
			    return -2;
			}
			memset(buf,0,sizeof(buf));
			bp=0;
		    }
		}
	    }
	}
    }
    rewind(yyin);

    return 1;
}

void yyerror(char *s) 
{
    extern char *yytext;
    print_text_buffer("\n*** Error on line %d: %s (0x%x)\n",cmdcnt+1,s, *yytext);
    //SHOW_MESSAGE("Error line %d: %s (0x%x)\n",cmdcnt+1,s, *yytext);
    //HJo fprintf(stderr,"Error line %d: %s\n",cmdcnt,s);
    fseek(yyin,0,SEEK_END); /* place seek on end, so lex could clean it up */
}

extern void init_flex(void);

int run_basic_script(char *file, GtkTextBuffer *buffer) 
{
    int count,count2;

    //yydebug = 1;

    bcount=0;                 /* buffer pointer for various operations */
    indexcnt=0;               /* used to index jump tables             */
    jli=0;                    /* jumplist index                        */
    d_vp=0;                   /* pointer to first free d_var           */
    str_vp=0;                 /* pointer to first free string          */
    dim_d_vp=0;               /* pointer to first free dim d_var       */
    dim_str_vp=0;             /* pointer to first free dim str_var     */
    gosub_sp=0;               /* pointer to first free on gosub stack  */
    while_sp=0;               /* pointer to first free on while stack  */
    do_sp=0;                  /* pointer to first free on do stack     */
    for_sp=0;                 /* pointer to first free on for stack    */
    dimc=0;                   /* counter of dimensions in DIM          */
    onlp=0;                   /* pointer to first free onlines         */
    lc=1;                     /* LineCounter, not useable at the moment*/
    current_line=0;           /* Current number of the line processed  */
    ignore_next_cmd=0;        /* ignore next command flag              */
    ignore_else_cmd=0;        /* ignore commands following "ELSE" flag */
    ignore_after_jump=0;      /* ignore everything, till jump is done  */
    use_stream=0;             /* if set, use value as io stream        */
    ignore_till_wend=0;       /* flag set if we have to look for wend  */
    jumplist=NULL;  /* this will store the readed data       */
    readdata=NULL;  /* this will store the readed data       */
    rd_p=NULL;      /* pointer to dataread                   */

    memset(fnum, 0, sizeof(fnum));
    memset(d_vars, 0, sizeof(d_vars));
    memset(str_vars, 0, sizeof(str_vars));
    memset(dim_d_vars, 0, sizeof(dim_d_vars));
    memset(dim_str_vars, 0, sizeof(dim_str_vars));
    memset(gosub_stack, 0, sizeof(gosub_stack));
    memset(while_stack, 0, sizeof(while_stack));
    memset(do_stack, 0, sizeof(do_stack));
    memset(for_stack, 0, sizeof(for_stack));
    memset(dimensions, 0, sizeof(dimensions));
    memset(onlines, 0, sizeof(onlines));

    gtkbuf = buffer;

    init_flex();

    memset(d_vars,0,sizeof(d_vars));
    memset(str_vars,0,sizeof(str_vars));
    for (count=0;count<MAX_FILES;count++) fnum[count].fp=NULL;
    for (count=0;count<MAX_STR_VARS;count++) str_vars[count].value=NULL;
    for (count=0;count<MAX_DIM_STR;count++) {
        memset(dim_str_vars[count].dim,0,sizeof(dim_str_vars[count].dim));
        dim_str_vars[count].values=NULL;
    }
    for (count=0;count<MAX_DIM_VARS;count++) {
        memset(dim_d_vars[count].dim,0,sizeof(dim_d_vars[count].dim));
        dim_d_vars[count].values=NULL;
    }
    yyin=fopen(file,"r"); 
    if (yyin!=NULL) {
        yyrestart(yyin);
        if (preparse()>=0) yyparse();
    } else { 
        fprintf(stderr,"Can't open file\n");
    }
    for (count=0;count<MAX_STR_VARS;count++) {
        if (str_vars[count].value!=NULL) free(str_vars[count].value);
    }
    for (count=0;count<MAX_DIM_VARS;count++) {
        if (dim_d_vars[count].values!=NULL) free(dim_d_vars[count].values);
    }
    for (count=0;count<MAX_DIM_STR;count++) {
        if (dim_str_vars[count].values!=NULL) {
            for (count2=0;count<dim_str_vars[count].size;count++) {
                if (dim_str_vars[count].values[count2]!=NULL)
                    free(dim_str_vars[count].values[count2]);
            }
        free(dim_str_vars[count].values);
        }
    }
    free_ll(readdata);
    free_ll(jumplist);

    return 0;
}

static void print_text_buffer(char *format, ...)
{
    va_list args;
    va_start(args, format);
    gchar *text = g_strdup_vprintf(format, args);
    va_end(args);

    if (use_stream) {
        if ((use_stream > 0) && (use_stream < MAX_FILES))
            if (fnum[use_stream].fp != NULL)
                fprintf(fnum[use_stream].fp, "%s", text);
    } else {
        gtk_text_buffer_insert_at_cursor(gtkbuf, text, strlen(text));
    }

    g_free(text);
}

static int sql(char *cmd)
{
    gchar *ret;

    ret = db_sql_command(cmd);

    if (ret == NULL || ret[0] == 0)
        print_text_buffer("OK\n", -1);
    else
        print_text_buffer(ret, -1);

    g_free(ret);

    return 0;
}

void basic_input_starts(void)
{
    buf[0] = 0;
}
