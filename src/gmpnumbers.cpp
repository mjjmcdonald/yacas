

/* Implementation of the number classes (the functionality used
 * by yacas any way
 * 24 June 2001: being rewritten by Robert Schipper, still working on it
 */

#include <gmp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lisptype.h"
#include "numbers.h"
#include "standard.h"
#include "platmath.h"

struct GMPNumber {
  mpz_t man;
  long exp;
};

static void initGMPNumber(GMPNumber& x, GMPNumber& y);
static void initGMPNumber(GMPNumber& x, long y=0);
static void initGMPNumber(GMPNumber& x, char* str);
static void initGMPNumber(GMPNumber& x, mpz_t mpz);
static void clearGMPNumber(GMPNumber& x);
static void ConvToGMPNumber(GMPNumber& r, mpf_t x, long prec=0);
static char* getstrGMPNumber(GMPNumber& x, long prec=0);
static LispStringPtr GMPNumberToString(GMPNumber& x, LispHashTable& h, 
                                       LispInt prec=0);
static void GMPNumberCopy(GMPNumber& x, GMPNumber& y);
static void GMPNumberZeroFill(GMPNumber& r, GMPNumber& x);
static void GMPNumberTrunc(GMPNumber& r, GMPNumber& x, long prec);
static void GMPNumberAdd(GMPNumber& r, GMPNumber& x, GMPNumber& y);
static void GMPNumberSubtract(GMPNumber& r, GMPNumber& x, GMPNumber& y); 
static void GMPNumberMultiply(GMPNumber& r, GMPNumber& x, 
                              GMPNumber& y, long prec);
static void GMPNumberDivide(GMPNumber& r, GMPNumber& x, 
                                 GMPNumber& y, long prec);
static void GMPNumberInvert(GMPNumber& r, GMPNumber& x, long prec);
static void GMPNumberPower(GMPNumber& r, GMPNumber& x, long e, long prec);
static void GMPNumberNeg(GMPNumber& r, GMPNumber& x);
static void GMPNumberAbs(GMPNumber& r, GMPNumber& x);
static void GMPNumberFloor(GMPNumber& r, GMPNumber& x);
static void GMPNumberCeil(GMPNumber& r, GMPNumber& x);
static void GMPNumberRound(GMPNumber& r, GMPNumber& x);
static void GMPNumberDiv(GMPNumber& r, GMPNumber& x, GMPNumber& y);
static void GMPNumberMod(GMPNumber& r, GMPNumber& x, GMPNumber& y);
static void GMPNumberSqrt(GMPNumber& r, GMPNumber& x, long prec);
static void GMPNumberExp(GMPNumber& r, GMPNumber& x, long prec);
static void GMPNumberLog(GMPNumber& r, GMPNumber& x, long prec);
static void GMPNumberPi(GMPNumber& r, long prec);
static void GMPNumberShiftLeft(GMPNumber& r, GMPNumber& x, unsigned long d);
static void GMPNumberShiftRight(GMPNumber& r, GMPNumber& x, unsigned long d);
static void SetMPFPrecision(LispInt aPrecision);

static LispStringPtr IntegerToString(mpz_t& aInt, LispHashTable& aHashTable);
static LispStringPtr FloatToString(mpf_t& aInt, LispHashTable& aHashTable, 
                                   LispInt aBase = 10);

LispInt NumericSupportForMantissa()
{
    return LispTrue;
}

const LispCharPtr NumericLibraryName()
{
    return "Gmp";
}

void initGMPNumber(GMPNumber& x, GMPNumber& y)
{
  mpz_t man;
  mpz_init_set(man,y.man);
  x.man = man;
  x.exp = y.exp;
}

void initGMPNumber(GMPNumber& x, long y)
{
  mpz_t man;
  mpz_init_set_ui(man,y);
  x.man = man;
  x.exp = 0;
}

void initGMPNumber(GMPNumber& x, mpz_t mpz)
{
  mpz_t man;
  mpz_init_set(man,mpz);
  x.man = man;
  x.exp = 0;
}

void initGMPNumber(GMPNumber& x, char* str)
{
  size_t length = strlen(str);
  char *man = (char*)malloc(length+1);
  char* s = str;
  char* m = man;
  long exp = 0;
  int neg = 0;
  int enot = 0;
  int tiny = 0;
  if (*s=='-') {neg=1;s++;}
  while(*s=='0')s++;
  if(*s=='.') {
    s++;
    while(*s=='0') {
      s++;
      exp--;
    }
  } else {
    while(*s) {
      if(*s=='.'){s++;break;}
      *m=*s;
      m++;
      s++;
      if(*s=='E'||*s=='e'){s++;enot=1;break;}
    }
  }
  if (!enot) {
    while(*s) {
      *m=*s;
      m++;
      s++;
      exp--;
      if(*s=='E'||*s=='e'){s++;enot=1;break;}
    }
  }  
  if (enot) exp+=atol(s);
  *m='\0';
  m--;
  while (*m=='0') {
    *m='\0';
    m--;
    exp++;
  }
  if (!*man) {*man='0';*(man+1)='\0';neg=0;exp=0;}
  mpz_t ma;
  mpz_init_set_str(ma,man,10);
  free(man);
  if(neg)mpz_neg(ma,ma);
  x.man=ma;
  x.exp=exp;
}

void clearGMPNumber(GMPNumber& x)
{
  mpz_clear(x.man);
}

static void ConvToGMPNumber(GMPNumber& r, mpf_t x, long prec) {
  mp_exp_t expt;
  char* str=mpf_get_str(NULL,&expt,10,prec,x);
  mpz_set_str(r.man,str,10);
  r.exp=expt-strlen(str)+(mpz_sgn(r.man)<0);
  free(str);
}

char* getstrGMPNumber(GMPNumber& x, long prec=0)
{
  long rawsize = mpz_sizeinbase(x.man,10);
  size_t size;
  if (rawsize+x.exp<=0) size=-x.exp+4;
  else if (x.exp>0) size=rawsize+x.exp+4;
  else size=rawsize+4;
  char *s, *str=(char*)malloc(size);
  mpz_get_str(str,10,x.man);
  char * st=str;
  size_t length = strlen(str);
  if(mpz_sgn(x.man)<0) {st++;length--;}
  if (!x.exp) return str;
  if (x.exp>0) {
    s = st+length-1;
    int exp=x.exp;
    while (s>st && *s == '0') {s--; exp++; length--;}
    *++s='\0';
    if (exp>prec) sprintf(s,"E%ld\0",exp);
    else {
      while(exp) {
        *s++='0';
        exp--;
      }
      *s='\0';
    }
    return str;
  }
  size_t exp = -x.exp;
  if (length<=exp) {
    s = st + length - 1;
    while (s>st && *s == '0') {s--; exp--; length--;}
    *++s='\0';
    s = st;
    if (prec && length>prec) {
      exp-=length-prec;
      length=prec;
      *(s+length)='\0';
    }
    if (exp-length<=prec) {
      memmove(s+exp-length+2,s,length+1);
      *s++='0';
      *s++='.';
      exp-=length;
      while (exp--) *s++='0';
    } else {
      memmove(s+2,s,length+1);
      *s++='0';
      *s++='.';
      s+=length;
      exp-=length;
      sprintf(s,"E-%ld\0",exp);
    }
  } else {
    if (prec && exp>prec) {
      length-=exp-prec;
      exp=prec;
      *(st+length)='\0';
    }
    size_t index = length-exp;
    memmove(st+index+1, st+index, exp+1);
    st[index]='.';
    s = st+length;
    while (*s=='0') s--;
    if (*s=='.') *s='\0';
    else *++s='\0';
  }
  return str;
}

static LispStringPtr GMPNumberToString(GMPNumber& x, LispHashTable& h, 
                                       LispInt prec=0)
{
  char* result = getstrGMPNumber(x, prec);
  LispStringPtr toreturn = h.LookUp(result);
  free(result);
  return toreturn;
}
 
void GMPNumberCopy(GMPNumber& x, GMPNumber& y)
{
  mpz_set(x.man,y.man);
  x.exp=y.exp;
}

static void GMPNumberZeroFill(GMPNumber& r, GMPNumber& x)
{
  if (x.exp>0) {
    mpz_t factor;
    mpz_init_set_ui(factor,10);
    mpz_pow_ui(factor,factor,x.exp);
    mpz_mul(r.man,x.man,factor);
    mpz_clear(factor);
    r.exp=0;
  } else GMPNumberCopy(r,x);
}

void GMPNumberAdd(GMPNumber& r, GMPNumber& x, GMPNumber& y)
{
  if (!mpz_sgn(x.man)) {GMPNumberCopy(r,y); return;}
  if (!mpz_sgn(y.man)) {GMPNumberCopy(r,x); return;}
  int diff = x.exp-y.exp;
  if (diff) {
    mpz_t factor;
    mpz_init_set_ui(factor,10);
    GMPNumber tmp;
    if (diff<0) {
      mpz_pow_ui(factor,factor,-diff);
      initGMPNumber(tmp,y);
      mpz_mul(tmp.man,tmp.man,factor);
      mpz_add(r.man,x.man,tmp.man);
      r.exp=x.exp;
    } else if (diff>0) {
      mpz_pow_ui(factor,factor,diff);
      initGMPNumber(tmp,x);
      mpz_mul(tmp.man,tmp.man,factor);
      mpz_add(r.man,tmp.man,y.man);
      r.exp=y.exp;
    }
    clearGMPNumber(tmp);
    mpz_clear(factor);
  } else {
    mpz_add(r.man,x.man,y.man);
    r.exp=x.exp;
  }
  if (!mpz_sgn(r.man)) r.exp = 0;
}

void GMPNumberSubtract(GMPNumber& r, GMPNumber& x, GMPNumber& y)
{
  if (!mpz_sgn(x.man)) {mpz_neg(r.man,y.man); r.exp=y.exp; return;}
  if (!mpz_sgn(y.man)) {GMPNumberCopy(r,x); return;}
  int diff = x.exp-y.exp;
  if (diff) {
    mpz_t factor;
    mpz_init_set_ui(factor,10);
    GMPNumber tmp;
    if (diff<0) {
      mpz_pow_ui(factor,factor,-diff);
      initGMPNumber(tmp,y);
      mpz_mul(tmp.man,tmp.man,factor);
      mpz_sub(r.man,x.man,tmp.man);
      r.exp=x.exp;
    } else if (diff>0) {
      mpz_pow_ui(factor,factor,diff);
      initGMPNumber(tmp,x);
      mpz_mul(tmp.man,tmp.man,factor);
      mpz_sub(r.man,tmp.man,y.man);
      r.exp=y.exp;
    }
    clearGMPNumber(tmp);
    mpz_clear(factor);
  } else {
    mpz_sub(r.man,x.man,y.man);
    r.exp=x.exp;
  }
  if (!mpz_sgn(r.man)) r.exp = 0;
}

static void GMPNumberTrunc(GMPNumber& r, GMPNumber& x, long prec)
{
  long e = -x.exp;
  if (e>prec) {
    e -= prec;
    mpz_t factor;
    mpz_init_set_ui(factor,10);
    mpz_pow_ui(factor,factor,e);
    mpz_tdiv_q(r.man,x.man,factor);
    mpz_clear(factor);
    r.exp = -prec;
  }
}

void GMPNumberMultiply(GMPNumber& r, GMPNumber& x, GMPNumber& y, long prec)
{
  int neg = mpz_sgn(x.man)*mpz_sgn(y.man);
  if (neg) {
    mpz_mul(r.man,x.man,y.man);
    r.exp = x.exp+y.exp;
  } else {
    mpz_set_ui(r.man,0);
    r.exp = 0;
  }
}

void GMPNumberDivide(GMPNumber& r, GMPNumber& x, GMPNumber& y, long prec)
{
  int neg = mpz_sgn(x.man)/mpz_sgn(y.man);
  if (neg) {
    long e = prec + x.exp - y.exp;
    if (e<prec) e=prec;
    size_t xlen = mpz_sizeinbase(x.man,10);
    size_t ylen = mpz_sizeinbase(y.man,10);
    if (ylen>xlen) e+=ylen-xlen+1;
//    if (ylen>xlen) e+=ylen<<1-xlen+1;
//    else e+=xlen;
    mpz_t factor;
    mpz_init_set_ui(factor,10);
    mpz_pow_ui(factor,factor,e);
    mpz_mul(r.man,x.man,factor);
    mpz_tdiv_q(r.man,r.man,y.man);
    r.exp = mpz_sgn(r.man) ? x.exp-y.exp-e : 0;
    mpz_clear(factor);
  } else {
    mpz_set_ui(r.man,0);
    r.exp = 0;
  }
}

static void GMPNumberSqrt(GMPNumber& r, GMPNumber& x, long prec)
{
  int neg = mpz_sgn(x.man);
  if (neg==0) {
    mpz_set_ui(r.man,0);
    r.exp = 0;
  } else if (neg>0) {
    GMPNumberZeroFill(x,x);
    long exp = -x.exp;
    long l = mpz_sizeinbase(x.man,10);
    l+=l&1;
    long adjust = exp&1;
    mpz_t factor;
    mpz_init_set_ui(factor,10);
    mpz_pow_ui(factor,factor,(prec+l)<<1+adjust);
    mpz_mul(r.man,x.man,factor);
    mpz_sqrt(r.man,r.man);
    mpz_clear(factor);
    r.exp=-((exp-adjust)>>1)-prec-l;
    GMPNumberTrunc(r,r,prec);
  } else {
  //TODO Error: taking sqrt of negative number
  }
}

static void GMPNumberDiv(GMPNumber& r, GMPNumber& x, GMPNumber& y)
{
  GMPNumberZeroFill(x,x);
  GMPNumberZeroFill(y,y);
  mpz_set_ui(r.man,0);
  r.exp=0;
  if (!(x.exp || y.exp)) {
    int neg = mpz_sgn(x.man)/mpz_sgn(y.man);
    if (neg) mpz_tdiv_q(r.man,x.man,y.man);
  }
}

static void GMPNumberMod(GMPNumber& r, GMPNumber& x, GMPNumber& y)
{
  GMPNumberZeroFill(x,x);
  GMPNumberZeroFill(y,y);
  mpz_set_ui(r.man,0);
  r.exp=0;
  if (!(x.exp || y.exp)) {
    int neg = mpz_sgn(x.man)/mpz_sgn(y.man);
    if (neg) mpz_tdiv_r(r.man,x.man,y.man);
  }
}

void GMPNumberInvert(GMPNumber& r, GMPNumber& x, long prec)
{
  GMPNumber one;
  initGMPNumber(one, 1);
  GMPNumberDivide(r,one,x,prec); 
  clearGMPNumber(one);
}

void GMPNumberPower(GMPNumber& r, GMPNumber& x, long e, long prec)
{
    int inv = 0;
    if (e<0) {inv=1; e=-e;}
    mpz_pow_ui(r.man,x.man,e);
    r.exp = e*x.exp;
    if (inv) GMPNumberInvert(r,r,prec);
}

static void GMPNumberExp(GMPNumber& r, GMPNumber& x, long prec)
{
  mpf_set_default_prec(prec<<2);
  mpf_t e;
  mpf_init(e);
  mpf_set_z(e,x.man);
  mpz_t factor;
  mpz_init_set_ui(factor,10);
  long exp;
  int neg=0;
  if (x.exp<0) {neg=1;exp=-x.exp;}
  else exp=x.exp;
  mpz_pow_ui(factor,factor,exp);
  mpf_t f;
  mpf_init(f);
  mpf_set_z(f,factor);
  neg ? mpf_div(e,e,f) : mpf_mul(e,e,f);
  mpz_clear(factor);
  mpf_clear(f);
  unsigned long i;
  mpf_t s;
  mpf_t s1;
  mpf_t t;
  mpf_init_set_ui(s,1);
  mpf_init_set(t,e);
  mpf_init(s1);
  for(i=2;;i++) {
    mpf_add(s1,s,t);
    if (!mpf_cmp(s,s1)) break;
    mpf_set(s,s1);
    mpf_mul(t,t,e);
    mpf_div_ui(t,t,i);
  }
  mpf_clear(t);
  mpf_clear(s1);
  mpf_clear(e);
  ConvToGMPNumber(r,s,prec);
  mpf_clear(s);
}

static void GMPNumberLog(GMPNumber& r, GMPNumber& x, long prec)
{
  mpf_set_default_prec(prec<<2);
  mpf_t y;
  mpf_init(y);
  mpf_set_z(y,x.man);
  mpz_t factor;
  mpz_init_set_ui(factor,10);
  long exp;
  int neg=0;
  if (x.exp<0) {neg=1;exp=-x.exp;}
  else exp=x.exp;
  mpz_pow_ui(factor,factor,exp);
  mpf_t f;
  mpf_init(f);
  mpf_set_z(f,factor);
  neg ? mpf_div(y,y,f) : mpf_mul(y,y,f);
  mpz_clear(factor);
  mpf_clear(f);
  unsigned long n=0;
  mpf_t d;
  mpf_init_set_d(d,1.01);
  while (mpf_cmp(y,d)>0) {
    mpf_sqrt(y,y);
    n++;
  }
  mpf_clear(d);
  mpf_ui_sub(y,1,y);
  unsigned long i;
  mpf_t s;
  mpf_t s1;
  mpf_t t;
  mpf_t t1;
  mpf_init_set_ui(s,0);
  mpf_init_set(t,y);
  mpf_init_set(t1,y);
  mpf_init(s1);
  for(i=2;;i++) {
    mpf_add(s1,s,t);
    if (!mpf_cmp(s,s1)) break;
    mpf_set(s,s1);
    mpf_mul(t1,t1,y);
    mpf_div_ui(t,t1,i);
  }
  mpf_clear(t);
  mpf_clear(t1);
  mpf_clear(s1);
  mpf_clear(y);
  mpf_neg(s,s);
  mpf_mul_2exp(s,s,n);
  ConvToGMPNumber(r,s,0);
  mpf_clear(s);
}

static void GMPNumberPi(GMPNumber& r, long prec)
{
  mpf_set_default_prec(prec<<2);
  mpf_t sum1;
  mpf_t s;
  mpf_t s1;
  mpf_t t;
  mpf_t t1;
  mpf_t negfourth;
  mpf_init_set_ui(s,0);
  mpf_init_set_d(t,0.5);
  mpf_init_set_d(t1,0.5);
  mpf_init_set_d(negfourth,-0.25);
  mpf_init(sum1);
  mpf_init(s1);
  unsigned long i;
  for(i=3;;i+=2) {
     mpf_add(s1,s,t);
     if (!mpf_cmp(s,s1)) break;
     mpf_set(s,s1);
     mpf_mul(t1,t1,negfourth);
     mpf_div_ui(t,t1,i);
  }
  mpf_set(sum1,s);

  mpq_t tmp;
  mpq_init(tmp);
  mpq_set_ui(tmp,1,3);
  mpf_set_ui(s,0);
  mpf_set_q(t,tmp);
  mpf_set_q(t1,tmp);
  mpq_set_si(tmp,-1,9);
  mpf_t g;
  mpf_init(g);
  mpf_set_q(g,tmp);
  mpq_clear(tmp);
  for(i=3;;i+=2) {
     mpf_add(s1,s,t);
     if (!mpf_cmp(s,s1)) break;
     mpf_set(s,s1);
     mpf_mul(t1,t1,g);
     mpf_div_ui(t,t1,i);
  }
  mpf_add(s,s,sum1);
  mpf_mul_ui(s,s,4);
  mpf_clear(sum1);
  mpf_clear(t);
  mpf_clear(s1);
  mpf_clear(t1);
  mpf_clear(negfourth);
  mpf_clear(g);
  ConvToGMPNumber(r,s,prec+1);
  mpf_clear(s);
}

static void GMPNumberFloor(GMPNumber& r, GMPNumber& x)
{
  if (x.exp<0) {
    mpz_t factor;
    mpz_init_set_ui(factor,10);
    mpz_pow_ui(factor,factor,-x.exp);
    mpz_t rem;
    mpz_init(rem);
    mpz_fdiv_q(r.man,x.man,factor);
    mpz_clear(rem);
    mpz_clear(factor);
    r.exp=0;
  } else GMPNumberCopy(r,x);
}

static void GMPNumberCeil(GMPNumber& r, GMPNumber& x)
{
  if (x.exp<0) {
    mpz_t factor;
    mpz_init_set_ui(factor,10);
    mpz_pow_ui(factor,factor,-x.exp);
    mpz_cdiv_q(r.man,x.man,factor);
    mpz_clear(factor);
    r.exp=0;
  } else GMPNumberCopy(r,x);
}

static void GMPNumberRound(GMPNumber& r, GMPNumber& x)
{
  long exp = x.exp;
  if (exp<0) {
    exp = -exp-1;
    mpz_t factor;
    mpz_init_set_ui(factor,10);
    mpz_t half;
    mpz_init_set_ui(half,5);
    if (exp) {
      mpz_pow_ui(factor,factor,exp);
      mpz_mul(half,half,factor);
      mpz_mul_ui(factor,factor,10);
    }
    mpz_add(r.man,x.man,half);
    mpz_fdiv_q(r.man,r.man,factor);
    mpz_clear(factor);
    mpz_clear(half);
    r.exp=0;
  } else GMPNumberCopy(r,x);
}

void GMPNumberNeg(GMPNumber& r, GMPNumber& x)
{
  mpz_neg(r.man,x.man);
}

void GMPNumberAbs(GMPNumber& r, GMPNumber& x)
{
  mpz_abs(r.man,x.man);
}

static void GMPNumberShiftLeft(GMPNumber& r, GMPNumber& x, unsigned long d)
{
  GMPNumberZeroFill(r,x);
  mpz_mul_2exp(r.man,r.man,d);
}

static void GMPNumberShiftRight(GMPNumber& r, GMPNumber& x, unsigned long d)
{
  GMPNumberZeroFill(r,x);
  mpz_fdiv_q_2exp(r.man,r.man,d);
}

void SetMPFPrecision(LispInt aPrecision)
{
// FIXME this is still very primitive
    mpf_set_default_prec((unsigned long)aPrecision<<2);
}


LispStringPtr GcdInteger(LispCharPtr int1, LispCharPtr int2,
                         LispHashTable& aHashTable)
{
    mpz_t i1;
    mpz_t i2;
    mpz_init(i1);
    mpz_set_str(i1,int1,10);
    mpz_init(i2);
    mpz_set_str(i2,int2,10);

    mpz_t res;
    mpz_init(res);
    mpz_gcd(res,i1,i2);
    GMPNumber r;
    initGMPNumber(r,res);
    LispStringPtr result = GMPNumberToString(r, aHashTable);
    clearGMPNumber(r);
    mpz_clear(i1);
    mpz_clear(i2);
    mpz_clear(res);
    return result;
}

LispStringPtr LispFactorial(LispCharPtr int1, LispHashTable& aHashTable,LispInt aPrecision)
{
    LispInt nr = InternalAsciiToInt(int1);
    Check(nr>=0,KLispErrInvalidArg);
    GMPNumber r;
    initGMPNumber(r);
    mpz_fac_ui (r.man, nr);
    LispStringPtr result = GMPNumberToString(r, aHashTable);
    clearGMPNumber(r);
    return result;
}

LispStringPtr AddFloat(LispCharPtr int1, LispCharPtr int2,
                       LispHashTable& aHashTable,LispInt aPrecision)
{
  GMPNumber r,x,y;
  initGMPNumber(r);
  initGMPNumber(x,int1);
  initGMPNumber(y,int2);
  GMPNumberAdd(r,x,y);        
  LispStringPtr result = GMPNumberToString(r, aHashTable, aPrecision);
  clearGMPNumber(r);
  clearGMPNumber(x);
  clearGMPNumber(y);
  return result;
}

LispStringPtr PlusFloat(LispCharPtr int1,LispHashTable& aHashTable
                       ,LispInt aPrecision)
{
// RVS 16 June 2001:
// TODO: is this routine ever really used?
  GMPNumber x;
  initGMPNumber(x,int1);
  LispStringPtr result = GMPNumberToString(x, aHashTable, aPrecision);
  clearGMPNumber(x);
  return result;
}

LispStringPtr SubtractFloat(LispCharPtr int1, LispCharPtr int2,
                            LispHashTable& aHashTable,LispInt aPrecision)
{
  GMPNumber r,x,y;
  initGMPNumber(r);
  initGMPNumber(x,int1);
  initGMPNumber(y,int2);
  GMPNumberSubtract(r,x,y);        
  LispStringPtr result = GMPNumberToString(r, aHashTable, aPrecision);
  clearGMPNumber(r);
  clearGMPNumber(x);
  clearGMPNumber(y);
  return result;
}

LispStringPtr NegateFloat(LispCharPtr int1, LispHashTable& aHashTable
                          ,LispInt aPrecision)
{
  GMPNumber x;
  initGMPNumber(x,int1);
  GMPNumberNeg(x,x);
  LispStringPtr result = GMPNumberToString(x, aHashTable, aPrecision);
  clearGMPNumber(x);
  return result;
}

LispStringPtr DivideFloat(LispCharPtr int1, LispCharPtr int2,
                          LispHashTable& aHashTable,LispInt aPrecision)
{
  GMPNumber r,x,y;
  initGMPNumber(r);
  initGMPNumber(x,int1);
  initGMPNumber(y,int2);
  GMPNumberDivide(r,x,y,aPrecision);        
  LispStringPtr result = GMPNumberToString(r, aHashTable, aPrecision);
  clearGMPNumber(r);
  clearGMPNumber(x);
  clearGMPNumber(y);
  return result;
}

LispStringPtr MultiplyFloat(LispCharPtr int1, LispCharPtr int2,
                            LispHashTable& aHashTable,LispInt aPrecision)
{
  GMPNumber r,x,y;
  initGMPNumber(r);
  initGMPNumber(x,int1);
  initGMPNumber(y,int2);
  GMPNumberMultiply(r,x,y,aPrecision);        
  LispStringPtr result = GMPNumberToString(r, aHashTable, aPrecision);
  clearGMPNumber(r);
  clearGMPNumber(x);
  clearGMPNumber(y);
  return result;
}

LispStringPtr PowerFloat(LispCharPtr int1, LispCharPtr int2,
                         LispHashTable& aHashTable,LispInt aPrecision)
{
//TODO: handle case int2 != integer
  GMPNumber r,x;
  long y = atol(int2);
  initGMPNumber(r);
  initGMPNumber(x,int1);
  GMPNumberPower(r,x,y,aPrecision);        
  LispStringPtr result = GMPNumberToString(r, aHashTable, aPrecision);
  clearGMPNumber(r);
  clearGMPNumber(x);
  return result;
}



LispStringPtr SinFloat(LispCharPtr int1, LispHashTable& aHashTable,LispInt aPrecision)
{
    //TODO
    return PlatSin(int1, aHashTable, 0);
}

LispStringPtr CosFloat(LispCharPtr int1, LispHashTable& aHashTable,LispInt aPrecision)
{
    //TODO
    return PlatCos(int1, aHashTable, 0);
}

LispStringPtr TanFloat(LispCharPtr int1, LispHashTable& aHashTable,LispInt aPrecision)
{
    //TODO
    return PlatTan(int1, aHashTable, 0);
}

LispStringPtr ArcSinFloat(LispCharPtr int1, LispHashTable& aHashTable,LispInt aPrecision)
{
    //TODO
    return PlatArcSin(int1,  aHashTable, 0);
}

LispStringPtr ArcCosFloat(LispCharPtr int1, LispHashTable& aHashTable,LispInt aPrecision)
{
    //TODO
    return PlatArcCos(int1,  aHashTable, 0);
}

LispStringPtr ArcTanFloat(LispCharPtr int1, LispHashTable& aHashTable,LispInt aPrecision)
{
    //TODO
    return PlatArcTan(int1,  aHashTable, 0);
}

LispStringPtr ExpFloat(LispCharPtr int1, LispHashTable& aHashTable,LispInt aPrecision)
{
  GMPNumber x;
  initGMPNumber(x,int1);
  GMPNumberExp(x,x,aPrecision);
  LispStringPtr result = GMPNumberToString(x, aHashTable, aPrecision);
  clearGMPNumber(x);
  return result;
}

LispStringPtr LnFloat(LispCharPtr int1, LispHashTable& aHashTable,LispInt aPrecision)
{
  GMPNumber x;
  initGMPNumber(x,int1);
  GMPNumberLog(x,x,aPrecision);
  LispStringPtr result = GMPNumberToString(x, aHashTable, aPrecision);
  clearGMPNumber(x);
  return result;
}


LispStringPtr SqrtFloat(LispCharPtr int1, LispHashTable& aHashTable,LispInt aPrecision)
{
  GMPNumber x;
  initGMPNumber(x,int1);
  GMPNumberSqrt(x,x,aPrecision);
  LispStringPtr result = GMPNumberToString(x, aHashTable, aPrecision);
  clearGMPNumber(x);
  return result;
}

LispStringPtr AbsFloat(LispCharPtr int1, LispHashTable& aHashTable
                          ,LispInt aPrecision)
{
  GMPNumber x;
  initGMPNumber(x,int1);
  GMPNumberAbs(x,x);
  LispStringPtr result = GMPNumberToString(x, aHashTable, aPrecision);
  clearGMPNumber(x);
  return result;
}

LispBoolean LessThan(LispCharPtr int1, LispCharPtr int2,
                       LispHashTable& aHashTable,LispInt aPrecision)
{
//    SetMPFPrecision(aPrecision);
    mpf_t i1;
    mpf_t i2;
    mpf_init(i1);
    mpf_set_str(i1,int1,10);
    mpf_init(i2);
    mpf_set_str(i2,int2,10);

    LispBoolean result = mpf_cmp(i1,i2)<0;
    mpf_clear(i1);
    mpf_clear(i2);
    return result;
}

LispBoolean GreaterThan(LispCharPtr int1, LispCharPtr int2,
                       LispHashTable& aHashTable,LispInt aPrecision)
{
//    SetMPFPrecision(aPrecision);
    mpf_t i1;
    mpf_t i2;
    mpf_init(i1);
    mpf_set_str(i1,int1,10);
    mpf_init(i2);
    mpf_set_str(i2,int2,10);

    LispBoolean result = mpf_cmp(i1,i2)>0;
    mpf_clear(i1);
    mpf_clear(i2);
    return result;
}



LispStringPtr ShiftLeft( LispCharPtr int1, LispCharPtr int2, LispHashTable& aHashTable,LispInt aPrecision)
{
  GMPNumber x;
  initGMPNumber(x,int1);
  unsigned long bits = atol(int2);
  GMPNumberShiftLeft(x,x,bits);
  LispStringPtr result = GMPNumberToString(x, aHashTable, aPrecision);
  clearGMPNumber(x);
  return result;
}


LispStringPtr ShiftRight( LispCharPtr int1, LispCharPtr int2, LispHashTable& aHashTable,LispInt aPrecision)
{
  GMPNumber x;
  initGMPNumber(x,int1);
  unsigned long bits = atol(int2);
  GMPNumberShiftRight(x,x,bits);
  LispStringPtr result = GMPNumberToString(x, aHashTable, aPrecision);
  clearGMPNumber(x);
  return result;
}


LispStringPtr FromBase( LispCharPtr int1, LispCharPtr int2, LispHashTable& aHashTable,
                        LispInt aPrecision)
{
//TODO handle integer case
    SetMPFPrecision(aPrecision);
    LispInt base = InternalAsciiToInt(int1);
    mpf_t i1;
    mpf_init(i1);
    mpf_set_str(i1,int2,base);
    LispStringPtr result = FloatToString(i1, aHashTable,10);
    mpf_clear(i1);
    return result;

}


LispStringPtr ToBase( LispCharPtr int1, LispCharPtr int2, LispHashTable& aHashTable,
                    LispInt aPrecision)
{
//TODO handle integer case
    SetMPFPrecision(aPrecision);
    LispInt base = InternalAsciiToInt(int1);
    mpf_t i1;
    mpf_init(i1);
    mpf_set_str(i1,int2,10);
    LispStringPtr result = FloatToString(i1, aHashTable,base);
    mpf_clear(i1);
    return result;
}


LispStringPtr FloorFloat( LispCharPtr int1, LispHashTable& aHashTable,
                        LispInt aPrecision)
{
  GMPNumber x;
  initGMPNumber(x,int1);
  GMPNumberFloor(x,x);
  LispStringPtr result = GMPNumberToString(x, aHashTable, aPrecision);
  clearGMPNumber(x);
  return result;
}

LispStringPtr CeilFloat( LispCharPtr int1, LispHashTable& aHashTable,
                         LispInt aPrecision)
{
  GMPNumber x;
  initGMPNumber(x,int1);
  GMPNumberCeil(x,x);
  LispStringPtr result = GMPNumberToString(x, aHashTable, aPrecision);
  clearGMPNumber(x);
  return result;
}

LispStringPtr ModFloat( LispCharPtr int1, LispCharPtr int2, 
                        LispHashTable& aHashTable, LispInt aPrecision)
{
  GMPNumber r,x,y;
  initGMPNumber(r);
  initGMPNumber(x,int1);
  initGMPNumber(y,int2);
  GMPNumberMod(r,x,y);        
  LispStringPtr result = GMPNumberToString(r, aHashTable, aPrecision);
  clearGMPNumber(r);
  clearGMPNumber(x);
  clearGMPNumber(y);
  return result;
}

LispStringPtr DivFloat( LispCharPtr int1, LispCharPtr int2, LispHashTable& aHashTable,
                        LispInt aPrecision)
{
  GMPNumber r,x,y;
  initGMPNumber(r);
  initGMPNumber(x,int1);
  initGMPNumber(y,int2);
  GMPNumberDiv(r,x,y);        
  LispStringPtr result = GMPNumberToString(r, aHashTable, aPrecision);
  clearGMPNumber(r);
  clearGMPNumber(x);
  clearGMPNumber(y);
  return result;
}

LispStringPtr PiFloat( LispHashTable& aHashTable,
                        LispInt aPrecision)
{
  GMPNumber x;
  initGMPNumber(x);
  GMPNumberPi(x,aPrecision);
  LispStringPtr result = GMPNumberToString(x, aHashTable, aPrecision);
  clearGMPNumber(x);
  return result;
}

static LispStringPtr IntegerToString(mpz_t& aInt,
                                     LispHashTable& aHashTable)
{
    char* result = mpz_get_str(NULL,10,aInt);
    LispStringPtr toreturn = aHashTable.LookUp(result);
    free(result);
    return toreturn;
}

static void RemoveTrailingZeroes(LispString& printable)
{
    LispInt i;
    // remove trailing zeros
    i=printable.NrItems()-2;
    while (printable[i] == '0')
    {
        printable[i] = '\0';
        i--;
    }
    if (printable[i] == '.')
        printable[i] = '\0';
}

static LispStringPtr FloatToString(mpf_t& aInt,
                            LispHashTable& aHashTable, LispInt aBase)
{
    mp_exp_t expo;
    char* result = mpf_get_str(NULL,&expo,aBase,0,aInt);

    /* Now build our own version of a float number. Trying to keep
     * it integer
     */
    LispString printable;
    printable.SetNrItems(0);
    char* ptr=result;
    LispInt i;
    
    // Allow for sign
    if (*ptr == '-')
    {
        printable.Append('-');
        ptr++;
    }
    LispInt nrdigits = strlen(ptr);

    //Number is smaller than 0.1
    if(expo<0)
    {
        char exc[20];
        sprintf(exc,"%ld",expo);
        printable.Append('0');
        printable.Append('.');
        for (i=0;i<nrdigits;i++)
            printable.Append(ptr[i]);
        printable.Append('E');
        nrdigits=strlen(exc);
        for (i=0;i<nrdigits;i++)
            printable.Append(exc[i]);
        printable.Append('\0');
        RemoveTrailingZeroes(printable);
    }
    // Number is something like 0.xxx
    else if (expo == 0)
    {
        printable.Append('0');
        printable.Append('.');
        for (i=0;i<nrdigits;i++)
            printable.Append(ptr[i]);
        printable.Append('\0');
        RemoveTrailingZeroes(printable);
    }
    // Number has its decimal floating somewhere in the middle of the digits
    else if (expo<nrdigits)
    {
        for (i=0;i<nrdigits;i++)
        {
            if (i == expo)
                printable.Append('.');
            printable.Append(ptr[i]);
        }
        printable.Append('\0');
        RemoveTrailingZeroes(printable);
    }
    // Number is actually an integer.
    else
    {
        for (i=0;i<nrdigits;i++)
        {
            printable.Append(ptr[i]);
        }
        for (i=nrdigits;i<expo;i++)
        {
            printable.Append('0');
        }
        printable.Append('\0');
    }
    
    free(result);

    return aHashTable.LookUp(printable.String());
}


LispStringPtr BitAnd(LispCharPtr int1, LispCharPtr int2,
                     LispHashTable& aHashTable,LispInt aPrecision)
{
    mpz_t i1;
    mpz_t i2;
    mpz_init(i1);
    mpz_set_str(i1,int1,10);
    mpz_init(i2);
    mpz_set_str(i2,int2,10);

    mpz_t res;
    mpz_init(res);
    mpz_and(res,i1,i2);
    LispStringPtr result = IntegerToString(res, aHashTable);
    mpz_clear(i1);
    mpz_clear(i2);
    mpz_clear(res);
    return result;
}

LispStringPtr BitOr(LispCharPtr int1, LispCharPtr int2,
                     LispHashTable& aHashTable,LispInt aPrecision)
{
    mpz_t i1;
    mpz_t i2;
    mpz_init(i1);
    mpz_set_str(i1,int1,10);
    mpz_init(i2);
    mpz_set_str(i2,int2,10);

    mpz_t res;
    mpz_init(res);
    mpz_ior(res,i1,i2);
    LispStringPtr result = IntegerToString(res, aHashTable);
    mpz_clear(i1);
    mpz_clear(i2);
    mpz_clear(res);
    return result;
}

LispStringPtr BitXor(LispCharPtr int1, LispCharPtr int2,
                     LispHashTable& aHashTable,LispInt aPrecision)
{
    mpz_t i1;
    mpz_t i2;
    mpz_init(i1);
    mpz_set_str(i1,int1,10);
    mpz_init(i2);
    mpz_set_str(i2,int2,10);

    mpz_t res;
    mpz_t tmp1;
    mpz_t tmp2;
    mpz_init(res);
    mpz_init(tmp1);
    mpz_init(tmp2);
    // this works because i1 ^ i2 <=> (i1 & i2) | (!i1 & !i2)
    // but it's clumsy
    mpz_and(tmp1, i1, i2);
    mpz_com(i1, i1);
    mpz_com(i2, i2);
    mpz_and(tmp2, i1, i2);
    mpz_ior(res, tmp1, tmp2);
    LispStringPtr result = IntegerToString(res, aHashTable);
    mpz_clear(i1);
    mpz_clear(i2);
    mpz_clear(tmp1);
    mpz_clear(tmp2);
    mpz_clear(res);
    return result;

//  FOR THIS YOU NEED GMP >= 3.0  
/*    mpz_t i1;
    mpz_t i2;
    mpz_init(i1);
    mpz_set_str(i1,int1,10);
    mpz_init(i2);
    mpz_set_str(i2,int2,10);

    mpz_t res;
    mpz_init(res);
    mpz_xor(res, i1, i2);
    LispStringPtr result = IntegerToString(res, aHashTable);
    mpz_clear(i1);
    mpz_clear(i2);
    mpz_clear(res);
    return result;*/
}

//////////////////////////////////////////////////
///// BigNumber implementation by wrapping GMP
///// (coded by Serge Winitzki)
//////////////////////////////////////////////////

/// The number class describes either integers or floats, depending on the type_ flag. However, both the float part (float_, exponent_) and the integer part (int_) are always initialized for the life of the BigNumber object.
/// Wrapping of the GMP library is done using its mpz_t / mpf_t pointer types.
/// A special mode is that of "exp-float" number. Then the type_ flag has the value KExpFloat. The number is equal to float_ * 2^exponent_ where exponent_ is a big integer and float_ is a normal GMP float value. Otherwise the value of the exponent_ stays zero (actually it is ignored).
/// The exp-float mode is to be used only for really large or really small exponents.

// TO DO:
// - allow functions that will in the future implement better GMP wrapping. -?
// - replace _init by _init2 and specify explicit precision. (default precision is a non-reentrant global) (done)
// - use gmp_sprintf to print numbers rather than doing it by hand.
// - introduce an "universal functor" that is good for any floating-point functions; it uses a function object to encapsulate any details? Or use a void* instead of function pointers and then explicit casts?
// - optimize some arithmetic functions to use _ui or _si versions in case some operands are small integers. (IsSmall() checks that it fits into signed long.)


BigNumber::BigNumber() { init(); }

void BigNumber::init()
{
	turn_int();	// by default all numbers are created integer
	mpz_init2(int_, 32);	// default precision
	mpf_init2(float_, 32);
	mpz_init2(exponent_, 32);
}

BigNumber::~BigNumber()
{
	mpz_clear(int_);
	mpf_clear(float_);
	mpz_clear(exponent_);
}
// construct from string
BigNumber::BigNumber(const LispCharPtr aString,LispInt aPrecision,LispInt aBase)
{
	init();
	SetTo(aString, aPrecision, aBase);
}


/// copy constructor
BigNumber::BigNumber(const BigNumber& aOther)
{
	init();
	SetTo(aOther);
}


// assign from another number
void BigNumber::SetTo(const BigNumber& aOther)
{
  if (this == &aOther)
  	return;
  if (aOther.IsInt())
  {
  	turn_int();
	mpz_set(int_, aOther.int_);
  }
  else
  {
    	turn_float();
  	mpf_set(float_, aOther.float_);
  }
  
  type_ = aOther.type_;
}

// assign from string, result is always a float type
void BigNumber::SetTo(const LispCharPtr aString,LispInt aPrecision,LispInt aBase=10)
{
  Precision(aPrecision);	// FIXME: precision is set by hand here, instead of setting it from the string automatically... not sure whether to change the API about this.
  if (mpf_set_str(float_, aString, aBase)==0)
  {	// FIXME: we convert the string always to a float, while we could in principle decide whether to convert it to an integer or to a float
	turn_float();
  }
  else
  {// FIXME: this clause is executed when there is an error in the string. Need to signal the error somehow.
  
  }
}


// assign from a platform type
void BigNumber::SetTo(LispInt value)
{
  turn_int();
  mpz_set_si(int_, value);
}


void BigNumber::SetTo(double value)
{
  turn_float();
  mpf_set_d(float_, value);
}


// Convert back to other types
/// ToString : return string representation of number to given precision (base digits) in aResult

void BigNumber::ToString(LispString& aResult, LispInt aPrecision, LispInt aBase) const
{
  if (IsInt())
  {
	LispInt size=mpz_sizeinbase(int_, aBase);
	char* buffer=(char*)malloc(size+2);
	mpz_get_str(buffer, aBase, int_);
	// assign result
	aResult.SetStringCounted(buffer, strlen(buffer));
	free(buffer);
  }
  else
  {	// we have a floating-point number. Note that we cannot print exp-float numbers (except in base 2).
  // FIXME: maybe print exp-float numbers in bases 2, 8, 16?
	if (IsExpFloat()) return;
  	
	long size=aPrecision;	// FIXME: what if aPrecision<=0?
	// Find out the size needed to print the exponent
	long exp_small;
	size += 80;
	char* buffer=(char*)malloc(size);
	if (!buffer) return;	// error: cannot allocate memory
//	gmp_snprintf(buffer, size-1, "%*g", float_, aPrecision);
// cannot use gmp_printf because we need to print in a given base
	buffer[0] = '0';
	buffer[1] = '.';
	mpf_get_str(buffer+2, &exp_small, aBase, aPrecision, float_);
	// now print the exponent: either "e" or "@" and then the long integer
	size = strlen(buffer);
	buffer[size] = (aBase<=10) ? 'e' : '@';	// one character there, start exponent at offset size+1. Printing as per GMP instructions.
	// compute the total exponent for real now
	BigNumber exp_total;
	exp_total.SetTo((LispInt)exp_small);
	if (IsExpFloat()) mpz_add(exp_total.int_, exp_total.int_, exponent_);
	mpz_get_str(buffer+size+1, aBase, exp_total.int_);
	// assign result
	aResult.SetStringCounted(buffer, strlen(buffer));
	free(buffer);
  }
}

/// Give approximate representation as a double number
double BigNumber::Double() const
{
  if(IsInt())
	return mpz_get_d(int_);
  else
	return mpf_get_d(float_); // FIXME: this will not work with KExpFloat
}

/// get library name
const LispCharPtr BigNumber::NumericLibraryName()
{
	static char buf[20];
	snprintf(buf, 18, "GMP %s", gmp_version); 
	return buf;
}



//basic object manipulation
bool BigNumber::Equals(const BigNumber& aOther) const
{
  if (IsInt())
    if (aOther.IsInt())
    	return mpz_cmp(int_, aOther.int_)==0;
    else
    {
    	BigNumber temp(aOther);
	temp.BecomeInt();
	return this->Equals(temp);
    }
  else
    if (aOther.IsInt())
    {
    	BigNumber temp(*this);
	temp.BecomeInt();
	return temp.Equals(aOther);
    }
    else // two floats are equal when they are equal and both not ExpFloat, or if they are both ExpFloat and their exponents are also equal
    	return
		mpf_cmp(float_, aOther.float_)==0
		&& (!IsExpFloat() && !aOther.IsExpFloat() 
		  || IsExpFloat() && aOther.IsExpFloat() 
		    && mpz_cmp(exponent_, aOther.exponent_)==0);
    
}


bool BigNumber::IsInt() const
{
	return (type_ & KInt)!=0;
}


bool BigNumber::IsExpFloat() const
{
	return (type_ & KExpFloat)==KExpFloat;
}

bool BigNumber::IsIntValue() const
{
  if (IsInt())
  	return true;
  else
  	return mpf_integer_p(float_);
}


bool BigNumber::IsSmall() const
{
  if (IsInt())
  	return mpz_fits_slong_p(int_);
  else
  // this function is not present in GMP, need to code a workaround to determine whether a mpf_t fits into double.
  {
	long exp_small;
	(void) mpf_get_d_2exp(&exp_small, float_);
	return (!IsExpFloat() && mpf_get_prec(float_)<=64 && fabs(exp_small)<1021);
	// standard range of double precision is about 53 bits of mantissa and binary exponent of about 1021. But GMP sets the precision rather arbitrarily. Let's see how well this "64" works.
  }

}


void BigNumber::BecomeInt()
{
  if (!IsInt())
  {
	turn_int();
	mpz_set_f(int_, float_);
  }
}


void BigNumber::BecomeFloat()
{
  if (IsInt())
  {
  	turn_float();
	mpf_set_z(float_, int_);
  }
}


bool BigNumber::LessThan(const BigNumber& aOther) const
{
  if (IsInt())
    if (aOther.IsInt())
  	return mpz_cmp(int_, aOther.int_)<0;
    else
    {	// need to temporarily convert us to a float
    	BigNumber temp(*this);
	temp.BecomeFloat();
    	return temp.LessThan(aOther);
    }
  else
    if (aOther.IsInt())
    {	// need to temporarily convert aOther to a float
    	BigNumber temp(aOther);
	temp.BecomeFloat();
    	return this->LessThan(temp);
    }
    else
  	return mpf_cmp(float_, aOther.float_)<0;
	// FIXME: this does not work for KExpFloat
}


//arithmetic
/// Multiply two numbers at given precision and put result in *this
void BigNumber::Multiply(const BigNumber& aX, const BigNumber& aY, LispInt aPrecision)
{
  if (aX.IsInt())
    if (aY.IsInt())	// both integer
    {
      if (!IsInt()) turn_int(); // if we are float, then we are not aX or aY, can clear our values
      mpz_mul(int_, aX.int_, aY.int_);
    }
    else	// int + float, need to promote to float
    {
      BigNumber temp(aX);
      temp.BecomeFloat();
      Multiply(temp, aY, aPrecision);
    }
  else
    if (aY.IsInt())	// float + int, need to promote to float
    {
      BigNumber temp(aY);
      temp.BecomeFloat();
      Multiply(aX, temp, aPrecision);
    }
    else	// float + float
    {
      if (IsInt()) turn_float(); // if we are int, then we are not aX or aY, can clear our values
      Precision(aPrecision);	// GMP does calculations in target precision
      mpf_mul(float_, aX.float_, aY.float_);
      // FIXME: this does not work for KExpFloat
    }
}


/** Multiply two numbers, and add to *this (this is useful and generally efficient to implement).
*/
void BigNumber::MultiplyAdd(const BigNumber& aX, const BigNumber& aY, LispInt aPrecision)
{
  if (aX.IsInt() && aY.IsInt() && IsInt())
  {// all three are integers
	mpz_addmul(int_, aX.int_, aY.int_);
  }
  else
  if (!aX.IsInt() && !aY.IsInt() && !IsInt())
  {// all three are floats
  // FIXME: this does not work for KExpFloat
	mpf_t temp;
	mpf_init2(temp, aPrecision);
	mpf_mul(temp, aX.float_, aY.float_);
	Precision(aPrecision);
	mpf_add(float_, float_, temp);
	mpf_clear(temp);
  }
  else
  {	// some are integers and some are floats. Need to promote all to floats and then call MultiplyAdd again
    if (IsInt())
    {
    	this->BecomeFloat();
	MultiplyAdd(aX, aY, aPrecision);
    }
    else
    if (aX.IsInt())
    {
    	BigNumber temp(aX);
	temp.BecomeFloat();
	// temp.Precision(aPrecision);	// probably unnecessary
	MultiplyAdd(temp, aY, aPrecision);
    }
    else
    if (aY.IsInt())
    {	// need to promote both aX and aY to floats
      	BigNumber temp(aY);
	temp.BecomeFloat();
	// temp.Precision(aPrecision);
	MultiplyAdd(aX, temp, aPrecision);
    }
  }
}


/// Add two numbers at given precision and return result in *this
void BigNumber::Add(const BigNumber& aX, const BigNumber& aY, LispInt aPrecision)
{
  if (aX.IsInt())
    if (aY.IsInt())	// both integer
    {
      if (!IsInt()) turn_int(); // if we are float, then we are not aX or aY, can clear our values
      mpz_add(int_, aX.int_, aY.int_);
    }
    else	// int + float, need to promote to float
    {
      BigNumber temp(aX);
      temp.BecomeFloat();
      Add(temp, aY, aPrecision);
    }
  else
    if (aY.IsInt())	// float + int, need to promote to float
    {
      BigNumber temp(aY);
      temp.BecomeFloat();
      Add(aX, temp, aPrecision);
    }
    else	// float + float
    {
      if (IsInt()) turn_float(); // if we are int, then we are not aX or aY, can clear our values
      Precision(aPrecision);	// GMP does calculations in target precision
      mpf_add(float_, aX.float_, aY.float_);
      // FIXME: this does not work for KExpFloat
    }
}


/// Negate the given number, return result in *this
void BigNumber::Negate(const BigNumber& aX)
{
  if (aX.IsInt())
  {
    if (!IsInt()) turn_int();
    mpz_neg(int_, aX.int_);
  }
  else
  {
    if (IsInt()) turn_float();
    mpf_neg(float_, aX.float_);
  }

}


/// Divide two numbers and return result in *this. Note: if the two arguments are integer, it should return an integer result!
void BigNumber::Divide(const BigNumber& aX, const BigNumber& aY, LispInt aPrecision)
{
  if (aX.IsInt())
    if (aY.IsInt())
    {
    	if (!IsInt()) turn_int();
	mpz_tdiv_q(int_, aX.int_, aY.int_);	// divide -5/3 = -1
    }
    else
    {	// aX must be promoted to float
      BigNumber temp(aX);
      temp.BecomeFloat();
      Divide(temp, aY, aPrecision);
    }
  else
    if (aY.IsInt())
    {	// aY must be promoted to float
      BigNumber temp(aY);
      temp.BecomeFloat();
      Divide(aX, temp, aPrecision);
    }
    else
    {
    	if (IsInt()) turn_float();
	Precision(aPrecision);
	mpf_div(float_, aX.float_, aY.float_);
      // FIXME: this does not work for KExpFloat
    }

}



/// integer operation: *this = y mod z
void BigNumber::Mod(const BigNumber& aY, const BigNumber& aZ)
{
  if (aY.IsInt() && aZ.IsInt())
  {
  	if (!IsInt()) turn_int();
  	mpz_mod(int_, aY.int_, aZ.int_);
  }
}

/* this can be done later in plugins or in library-dependent wrappers
/// integer operation: *this = x^y mod z
void BigNumber::PowerMod(const BigNumber& aX, const BigNumber& aY, const BigNumber& aZ)
{
  if (aX.IsInt() && aY.IsInt() && aZ.IsInt())
  {
  	mpz_powm(int_, aX.int_, aY.int_, aZ.int_);
  }
}
*/

void BigNumber::Floor(const BigNumber& aX)
{

  if (!aX.IsInt() && !aX.IsExpFloat())
  {	// aX is float for which we can evaluate Floor()
    turn_float();	// just in case we are integer
    // we are float now
    mpf_floor(float_, aX.float_);
  }
  else if (this != &aX) // no change for integers or for exp floats, but need to assign values
  {
	SetTo(aX);
  }
}


void BigNumber::Precision(LispInt aPrecision)
{
  if (!IsInt())
  {
  	mpf_set_prec(float_, aPrecision);
  }
}



/// Bitwise operations, return result in *this.
void BigNumber::ShiftLeft(const BigNumber& aX, LispInt aNrToShift)
{
  if (aNrToShift>=0)
  {
    if (aX.IsInt() && aNrToShift >= 0)
    {
  	if (!IsInt()) turn_int();
	mpz_mul_2exp(int_, aX.int_, aNrToShift);
    }
    else
    {
  	if (IsInt()) turn_float();
	mpf_mul_2exp(float_, aX.float_, aNrToShift);
    }
  }	// do nothing if the shift amount is negative
}


void BigNumber::ShiftRight(const BigNumber& aX, LispInt aNrToShift)
{
  if (aNrToShift>=0)
  {
    if (aX.IsInt() && aNrToShift >= 0)
    {
  	if (!IsInt()) turn_int();
	mpz_tdiv_q_2exp(int_, aX.int_, aNrToShift);
    }
    else
    {
  	if (IsInt()) turn_float();
	mpf_div_2exp(float_, aX.float_, aNrToShift);
    }
  }	// do nothing if the shift amount is negative
}


void BigNumber::ShiftLeft(const BigNumber& aX, const BigNumber& aNrToShift)
{
// first, see if we can use short numbers
  if (aNrToShift.IsInt() && aNrToShift.Sign()>=0)
  {
    if (aNrToShift.IsSmall())
    {
  	long shift_amount=(long)aNrToShift.Double();
	ShiftLeft(aX, shift_amount);
    }
    else
    {// only floats can be shifted by a non-small number, so convert to float and use exponent_
    // FIXME: make this work for large shift amounts
    }
  }	// do nothing if shift amount is not integer or negative
}


void BigNumber::ShiftRight(const BigNumber& aX, const BigNumber& aNrToShift)
{
// first, see if we can use short numbers
  if (aNrToShift.IsInt() && aNrToShift.Sign()>=0)
  {
    if (aNrToShift.IsSmall())
    {
  	long shift_amount=(long)aNrToShift.Double();
	ShiftRight(aX, shift_amount);
    }
    else
    {// only floats can be shifted by a non-small number, so convert to float and use exponent_
    // FIXME: make this work for large shift amounts
    }
  }	// do nothing if shift amount is not integer or negative
}


void BigNumber::BitAnd(const BigNumber& aX, const BigNumber& aY)
{
  if (aX.IsInt() && aY.IsInt())
  {
  	if (!IsInt()) turn_int();
	mpz_and(int_, aX.int_, aY.int_);
  }	// do nothing if the arguments are not integer
}


void BigNumber::BitOr(const BigNumber& aX, const BigNumber& aY)
{
  if (aX.IsInt() && aY.IsInt())
  {
  	if (!IsInt()) turn_int();
	mpz_ior(int_, aX.int_, aY.int_);
  }
}


void BigNumber::BitXor(const BigNumber& aX, const BigNumber& aY)
{
  if (aX.IsInt() && aY.IsInt())
  {
  	if (!IsInt()) turn_int();
	mpz_xor(int_, aX.int_, aY.int_);
  }
}


/// Bit count operation: return the number of significant bits if integer, return the binary exponent if float (shortcut for binary logarithm)
void BigNumber::BitCount(const BigNumber& aX)
{
  long bit_count;
  if (aX.IsInt())
  {
  	(void) mpz_get_d_2exp(&bit_count, aX.int_);	// find the # of digits in base 2
  }
  else
  {
  	(void) mpf_get_d_2exp(&bit_count, aX.float_);
  }
  // now careful not to overwrite *this
  if (!IsInt()) turn_int();
  mpz_set_si(int_, bit_count);
  if (aX.IsExpFloat()) mpz_add(int_, int_, aX.exponent_);
}



/// Give sign (-1, 0, 1)
LispInt BigNumber::Sign() const
{
  if (IsInt())
  {
  	return mpz_sgn(int_);	// mutable cast (mpz_t) needed?
  }
  else
  {
  	return mpf_sgn(float_);
  }
}

/// 
void BigNumber::turn_float()
{
//  if (IsInt())
  {
    type_ = KFloat;
  }
}

void BigNumber::turn_int()
{
//  if (!IsInt())
  {
    type_ = KInt;
  }
}

// copy from gmp objects
void BigNumber::import_gmp(mpz_t gmp_int)
{
	turn_int();
	mpz_set(int_, gmp_int);
}
void BigNumber::import_gmp(mpf_t gmp_float)
{
	turn_float();
	mpf_set(float_, gmp_float);
}
// copy to gmp objects
void BigNumber::export_gmp(mpz_t gmp_int) const
{
	mpz_set(gmp_int, int_);
}
void BigNumber::export_gmp(mpf_t gmp_float) const
{
	mpf_set(gmp_float, float_);
}


//////////////////////////////////////////////////
///// End of BigNumber implementation
//////////////////////////////////////////////////
