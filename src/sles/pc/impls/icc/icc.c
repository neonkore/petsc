/*$Id: icc.c,v 1.74 2000/09/25 17:29:19 balay Exp balay $*/
/*
   Defines a Cholesky factorization preconditioner for any Mat implementation.
  Presently only provided for MPIRowbs format (i.e. BlockSolve).
*/

#include "src/sles/pc/impls/icc/icc.h"   /*I "petscpc.h" I*/

EXTERN_C_BEGIN
#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"PCICCSetMatOrdering_ICC"
int PCICCSetMatOrdering_ICC(PC pc,MatOrderingType ordering)
{
  PC_ICC *dir = (PC_ICC*)pc->data;
  int    ierr;
 
  PetscFunctionBegin;
  ierr = PetscStrfree(dir->ordering);CHKERRQ(ierr);
  ierr = PetscStrallocpy(ordering,&dir->ordering);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
EXTERN_C_END

EXTERN_C_BEGIN
#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"PCICCSetFill_ICC"
int PCICCSetFill_ICC(PC pc,PetscReal fill)
{
  PC_ICC *dir;

  PetscFunctionBegin;
  dir       = (PC_ICC*)pc->data;
  dir->fill = fill;
  PetscFunctionReturn(0);
}
EXTERN_C_END

EXTERN_C_BEGIN
#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"PCICCSetLevels_ICC"
int PCICCSetLevels_ICC(PC pc,int levels)
{
  PC_ICC *icc;

  PetscFunctionBegin;
  icc = (PC_ICC*)pc->data;
  icc->levels = levels;
  PetscFunctionReturn(0);
}
EXTERN_C_END

#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"PCICCSetMatOrdering"
/*@
    PCICCSetMatOrdering - Sets the ordering routine (to reduce fill) to 
    be used it the ICC factorization.

    Collective on PC

    Input Parameters:
+   pc - the preconditioner context
-   ordering - the matrix ordering name, for example, MATORDERING_ND or MATORDERING_RCM

    Options Database Key:
.   -pc_icc_mat_ordering_type <nd,rcm,...> - Sets ordering routine

    Level: intermediate

.seealso: PCLUSetMatOrdering()

.keywords: PC, ICC, set, matrix, reordering

@*/
int PCICCSetMatOrdering(PC pc,MatOrderingType ordering)
{
  int ierr,(*f)(PC,MatOrderingType);

  PetscFunctionBegin;
  PetscValidHeaderSpecific(pc,PC_COOKIE);
  ierr = PetscObjectQueryFunction((PetscObject)pc,"PCICCSetMatOrdering_C",(void **)&f);CHKERRQ(ierr);
  if (f) {
    ierr = (*f)(pc,ordering);CHKERRQ(ierr);
  } 
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"PCICCSetLevels"
/*@
   PCICCSetLevels - Sets the number of levels of fill to use.

   Collective on PC

   Input Parameters:
+  pc - the preconditioner context
-  levels - number of levels of fill

   Options Database Key:
.  -pc_icc_levels <levels> - Sets fill level

   Level: intermediate

   Concepts: ICC^setting levels of fill

@*/
int PCICCSetLevels(PC pc,int levels)
{
  int ierr,(*f)(PC,int);

  PetscFunctionBegin;
  PetscValidHeaderSpecific(pc,PC_COOKIE);
  if (levels < 0) SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,0,"negative levels");
  ierr = PetscObjectQueryFunction((PetscObject)pc,"PCICCSetLevels_C",(void **)&f);CHKERRQ(ierr);
  if (f) {
    ierr = (*f)(pc,levels);CHKERRQ(ierr);
  } 
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"PCICCSetFill"
/*@
   PCICCSetFill - Indicate the amount of fill you expect in the factored matrix,
   where fill = number nonzeros in factor/number nonzeros in original matrix.

   Collective on PC

   Input Parameters:
+  pc - the preconditioner context
-  fill - amount of expected fill

   Options Database Key:
$  -pc_icc_fill <fill>

   Note:
   For sparse matrix factorizations it is difficult to predict how much 
   fill to expect. By running with the option -log_info PETSc will print the 
   actual amount of fill used; allowing you to set the value accurately for
   future runs. But default PETSc uses a value of 1.0

   Level: intermediate

.keywords: PC, set, factorization, direct, fill

.seealso: PCLUSetFill()
@*/
int PCICCSetFill(PC pc,PetscReal fill)
{
  int ierr,(*f)(PC,PetscReal);

  PetscFunctionBegin;
  PetscValidHeaderSpecific(pc,PC_COOKIE);
  if (fill < 1.0) SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,1,"Fill factor cannot be less than 1.0");
  ierr = PetscObjectQueryFunction((PetscObject)pc,"PCICCSetFill_C",(void **)&f);CHKERRQ(ierr);
  if (f) {
    ierr = (*f)(pc,fill);CHKERRQ(ierr);
  } 
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"PCSetup_ICC"
static int PCSetup_ICC(PC pc)
{
  PC_ICC *icc = (PC_ICC*)pc->data;
  IS     perm,cperm;
  int    ierr;

  PetscFunctionBegin;
  ierr = MatGetOrdering(pc->pmat,icc->ordering,&perm,&cperm);CHKERRQ(ierr);

  if (!pc->setupcalled) {
    ierr = MatIncompleteCholeskyFactorSymbolic(pc->pmat,perm,icc->fill,icc->levels,&icc->fact);CHKERRQ(ierr);
  } else if (pc->flag != SAME_NONZERO_PATTERN) {
    ierr = MatDestroy(icc->fact);CHKERRQ(ierr);
    ierr = MatIncompleteCholeskyFactorSymbolic(pc->pmat,perm,icc->fill,icc->levels,&icc->fact);CHKERRQ(ierr);
  }
  ierr = ISDestroy(cperm);CHKERRQ(ierr);
  ierr = ISDestroy(perm);CHKERRQ(ierr);
  ierr = MatCholeskyFactorNumeric(pc->pmat,&icc->fact);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"PCDestroy_ICC"
static int PCDestroy_ICC(PC pc)
{
  PC_ICC *icc = (PC_ICC*)pc->data;
  int    ierr;

  PetscFunctionBegin;
  if (icc->fact) {ierr = MatDestroy(icc->fact);CHKERRQ(ierr);}
  ierr = PetscStrfree(icc->ordering);CHKERRQ(ierr);
  ierr = PetscFree(icc);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"PCApply_ICC"
static int PCApply_ICC(PC pc,Vec x,Vec y)
{
  PC_ICC *icc = (PC_ICC*)pc->data;
  int    ierr;

  PetscFunctionBegin;
  ierr = MatSolve(icc->fact,x,y);CHKERRQ(ierr);
  PetscFunctionReturn(0);  
}

#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"PCApplySymmetricLeft_ICC"
static int PCApplySymmetricLeft_ICC(PC pc,Vec x,Vec y)
{
  int    ierr;
  PC_ICC *icc = (PC_ICC*)pc->data;

  PetscFunctionBegin;
  ierr = MatForwardSolve(icc->fact,x,y);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"PCApplySymmetricRight_ICC"
static int PCApplySymmetricRight_ICC(PC pc,Vec x,Vec y)
{
  int    ierr;
  PC_ICC *icc = (PC_ICC*)pc->data;

  PetscFunctionBegin;
  ierr = MatBackwardSolve(icc->fact,x,y);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"PCGetFactoredMatrix_ICC"
static int PCGetFactoredMatrix_ICC(PC pc,Mat *mat)
{
  PC_ICC *icc = (PC_ICC*)pc->data;

  PetscFunctionBegin;
  *mat = icc->fact;
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"PCSetFromOptions_ICC"
static int PCSetFromOptions_ICC(PC pc)
{
  PC_ICC     *icc = (PC_ICC*)pc->data;
  char       tname[256];
  PetscTruth flg;
  int        ierr;
  FList      ordlist;

  PetscFunctionBegin;
  ierr = MatOrderingRegisterAll(PETSC_NULL);CHKERRQ(ierr);
  ierr = OptionsHead("ICC Options");CHKERRQ(ierr);
    ierr = OptionsInt("-pc_icc_levels","levels of fill","PCICCSetLevels",icc->levels,&icc->levels,&flg);CHKERRQ(ierr);
    ierr = OptionsDouble("-pc_icc_fill","Expected fill in factorization","PCICCSetFill",icc->fill,&icc->fill,&flg);CHKERRQ(ierr);
    ierr = MatGetOrderingList(&ordlist);CHKERRQ(ierr);
    ierr = OptionsList("-pc_icc_mat_ordering_type","Reorder to reduce nonzeros in ICC","PCICCSetMatOrdering",ordlist,icc->ordering,tname,256,&flg);CHKERRQ(ierr);
    if (flg) {
      ierr = PCICCSetMatOrdering(pc,tname);CHKERRQ(ierr);
    }
  ierr = OptionsTail();CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

EXTERN_C_BEGIN
#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"PCCreate_ICC"
int PCCreate_ICC(PC pc)
{
  int    ierr;
  PC_ICC *icc;

  PetscFunctionBegin;
  icc = PetscNew(PC_ICC);CHKPTRQ(icc);
  PLogObjectMemory(pc,sizeof(PC_ICC));

  icc->fact	          = 0;
  ierr = PetscStrallocpy(MATORDERING_NATURAL,&icc->ordering);CHKERRQ(ierr);
  icc->levels	          = 0;
  icc->fill               = 1.0;
  icc->implctx            = 0;
  pc->data	          = (void*)icc;

  pc->ops->apply	       = PCApply_ICC;
  pc->ops->setup               = PCSetup_ICC;
  pc->ops->destroy	       = PCDestroy_ICC;
  pc->ops->setfromoptions      = PCSetFromOptions_ICC;
  pc->ops->view                = 0;
  pc->ops->getfactoredmatrix   = PCGetFactoredMatrix_ICC;
  pc->ops->applysymmetricleft  = PCApplySymmetricLeft_ICC;
  pc->ops->applysymmetricright = PCApplySymmetricRight_ICC;

  ierr = PetscObjectComposeFunctionDynamic((PetscObject)pc,"PCICCSetLevels_C","PCICCSetLevels_ICC",
                    PCICCSetLevels_ICC);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunctionDynamic((PetscObject)pc,"PCICCSetFill_C","PCICCSetFill_ICC",
                    PCICCSetFill_ICC);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunctionDynamic((PetscObject)pc,"PCICCSetMatOrdering_C","PCICCSetMatOrdering_ICC",
                    PCICCSetMatOrdering_ICC);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
EXTERN_C_END


