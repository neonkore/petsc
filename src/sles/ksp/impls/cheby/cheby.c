/*
    This is a first attempt at a Chebychev Routine, it is not 
    necessarily well optimized.
*/
#include <stdio.h>
#include <math.h>
#include "petsc.h"
#include "kspimpl.h"    /*I "ksp.h" I*/
#include "chebctx.h"


int KSPiChebychevSetUp(KSP itP)
{
  int ierr;
  if (itP->method != KSPCHEBYCHEV) {
     SETERR(1,"Attempt to use Chebychev Setup on wrong context");
  }
  if (ierr = KSPCheckDef(itP)) return ierr;
  return KSPiDefaultGetWork( itP, 3 );
}
/*@
     KSPChebychevSetEigenvalues - sets estimates for the extreme eigenvalues
              of the preconditioned problem.

  Input Parameters:
.  itP - the Krylov space context
.  emax, emin - the eigenvalue estimates
@*/
int KSPChebychevSetEigenvalues(KSP itP,double emax,double emin)
{
  ChebychevCntx *chebychevP = (ChebychevCntx *) itP->MethodPrivate;
  VALIDHEADER(itP,KSP_COOKIE);
  if (itP->method != KSPCHEBYCHEV) return 0;
  chebychevP->emax = emax;
  chebychevP->emin = emin;
  return 0;
}

int  KSPiChebychevSolve(KSP itP,int *its)
{
  int              k,kp1,km1,maxit,ktmp,i = 0,pres,brokeout = 0, hist_len,cerr;
  Scalar           alpha,omegaprod;
  Scalar           mu,omega,Gamma,c[3],scale;
  double           rnorm,*history;
  Vec              x,b,p[3],r;
  ChebychevCntx    *chebychevP = (ChebychevCntx *) itP->MethodPrivate;
  Scalar           mone = -1.0, tmp;

  history = itP->residual_history;
  hist_len= itP->res_hist_size;
  maxit   = itP->max_it;
  pres    = itP->use_pres;
  cerr    = 1;

  /* These three point to the three active solutions, we
     rotate these three at each solution update */
  km1 = 0; k = 1; kp1 = 2;
  x = itP->vec_sol;
  b = itP->vec_rhs;
  p[km1] = x;
  p[k]   = itP->work[0];
  p[kp1] = itP->work[1];
  r      = itP->work[2];

  /* use scale*B as our preconditioner */
  scale = 2.0/( chebychevP->emax + chebychevP->emin );

  /*   -alpha <=  scale*lambda(B^{-1}A) <= alpha   */
  alpha = 1.0 - scale*(chebychevP->emin); ;
  Gamma = 1.0;
  mu = 1.0/alpha; 
  omegaprod = 2.0/alpha;

  c[km1] = 1.0;
  c[k] = mu;

  if (!itP->guess_zero) {
    MatMult(itP->A,x,r);                      /*  r = b - Ax     */
    VecAYPX(&mone,b,r);       
  }
  else VecCopy(b,r);
                  
  PCApply(itP->B,r,p[k]);                    /*  p[k] = scale B^{-1}r  + x */
  VecAYPX(&scale,x,p[k]);                        

  for ( i=0; i<maxit; i++) {
    c[kp1] = 2.0*mu*c[k] - c[km1];
    omega = omegaprod*c[k]/c[kp1];

    MatMult(itP->A,p[k],r);                     /*  r = b - Ap[k]    */
    VecAYPX(&mone,b,r);                        
    PCApply(itP->B,r,p[kp1]);                   /*  p[kp1] = B^{-1}z  */

    /* calculate residual norm if requested */
    if (itP->calc_res) {
      if (!pres) VecNorm(r,&rnorm);
      else VecNorm(p[kp1],&rnorm);
      if (history && hist_len > i) history[i] = rnorm;
      itP->vec_sol = p[k]; 
      MONITOR(itP,rnorm,i);
      if (CONVERGED(itP,rnorm,i)) {brokeout = 1; break;}
    }

    /* y^{k+1} = omega( y^{k} - y^{k-1} + Gamma*r^{k}) + y^{k-1} */
    tmp = omega*Gamma*scale;
    VecScale(&tmp,p[kp1]);
    tmp = 1.0-omega; VecAXPY(&tmp,p[km1],p[kp1]);
    VecAXPY(&omega,p[k],p[kp1]);

    ktmp = km1;
    km1  = k;
    k    = kp1;
    kp1  = ktmp;
  }
  if (!brokeout && itP->calc_res) {
    MatMult(itP->A,p[k],r);                      /*  r = b - Ap[k]    */
    VecAYPX(&mone,b,r);                        
    if (!pres) VecNorm(r,&rnorm);
    else {
      PCApply(itP->B,r,p[kp1]);                 /*  p[kp1] = B^{-1}z  */
      VecNorm(p[kp1],&rnorm);
    }
    if (history && hist_len > i) history[i] = rnorm;
    itP->vec_sol = p[k]; 
    MONITOR(itP,rnorm,i);
  }
if (history) itP->res_act_size = (hist_len < i) ? hist_len : i;

  /* make sure solution is in vector x */
  itP->vec_sol = x;
  if (k != 0) {
    VecCopy(p[k],x);
  }

  /* Get floating point work */
  itP->namult   += (i+1);
  itP->nbinv    += (i+1);
  itP->nvectors += (i+1)*7;

  *its = RCONV(itP,i+1); return 0;
}

int KSPiChebychevCreate(KSP itP)
{
  ChebychevCntx *chebychevP;

  chebychevP = NEW(ChebychevCntx); CHKPTR(chebychevP);
  itP->MethodPrivate = (void *) chebychevP;

  itP->method               = KSPCHEBYCHEV;
  itP->right_pre            = 0;
  itP->calc_res             = 1;

  chebychevP->emin          = 1.e-2;
  chebychevP->emax          = 1.e+2;

  itP->setup                = KSPiChebychevSetUp;
  itP->solver               = KSPiChebychevSolve;
  itP->adjustwork           = KSPiDefaultAdjustWork;
  itP->destroy              = KSPiDefaultDestroy;
  itP->converged            = KSPDefaultConverged;
  itP->BuildSolution        = KSPDefaultBuildSolution;
  itP->BuildResidual        = KSPDefaultBuildResidual;
  return 0;
}
