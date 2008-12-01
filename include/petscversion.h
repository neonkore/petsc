#if !defined(__PETSCVERSION_H)
#define __PETSCVERSION_H

#define PETSC_VERSION_RELEASE    0
#define PETSC_VERSION_MAJOR      2
#define PETSC_VERSION_MINOR      3
#define PETSC_VERSION_SUBMINOR   3
#define PETSC_VERSION_PATCH      15
#define PETSC_VERSION_DATE       "May, 23, 2007"
#define PETSC_VERSION_PATCH_DATE "unknown"

#if !defined (PETSC_VERSION_HG)
#define PETSC_VERSION_HG         "unknown"
#endif

#if !defined(PETSC_VERSION_DATE_HG)
#define PETSC_VERSION_DATE_HG    "unknown"
#endif

#define PETSC_VERSION_(MAJOR,MINOR,SUBMINOR) \
  ((PETSC_VERSION_MAJOR == (MAJOR)) &&       \
   (PETSC_VERSION_MINOR == (MINOR)) &&       \
   (PETSC_VERSION_SUBMINOR == (SUBMINOR)) && \
   (PETSC_VERSION_RELEASE  == 1))

#endif
