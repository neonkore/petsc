/* $Id: petscconf.h,v 1.1 1998/04/09 20:26:47 balay Exp balay $ */

/*
    Defines the configuration for this machine
*/

#if !defined(INCLUDED_PETSCCONF_H)
#define INCLUDED_PETSCCONF_H
 

#define HAVE_PWD_H 
#define HAVE_STRING_H 
#define HAVE_STROPTS_H 
#define HAVE_MALLOC_H 
#define HAVE_X11  
#define HAVE_DRAND48  
#define HAVE_GETDOMAINNAME
#define HAVE_UNAME 
#define HAVE_UNISTD_H 
#define HAVE_SYS_TIME_H

#define HAVE_FORTRAN_UNDERSCORE 

#undef HAVE_READLINK
#define NEEDS_GETTIMEOFDAY_PROTO

#endif
