#ifndef _CONFIG_H_
#define _CONFIG_H_
//-----------------------------------------------------------------------------|
// Copyright (C) 2020, All rights reserved.
// embeddedKeith
// -*^*-
//-----------------------------------------------------------------------------|
//
//	Module:	config
//
//	Description:	
//
//	FileName:	config.h
//
//	Originator:	Keith DeWald
//
//  Initials  Date           Description
//  --------  ---------      -------------------------------------------------
//  KRD       10/24/2020     Created file and began implementation
//
//
//-----------------------------------------------------------------------------|

/* use the following line to configure a production build */
/*#define PRODUCTION
*/


#ifdef PRODUCTION

/* get rid of the following 2 defines before shipping! */
#define ALLOW_PDOG              /* for now, can use psuedo dog when
                                   real WDT is absent
                                 */
#define USE_FILE_FOR_NVRAM      /* for now, use a file instead of nvram */

#define NDEBUG                  /* make sure assertions are disabled */
#include <assert.h>             /* even if assert.h was included earlier */

#else

#define DUMPCORE                /* for now dump core */
#define ALLOW_PDOG              /* can use psuedo dog when real WDT is absent
                                 */
#define USE_FILE_FOR_NVRAM      /* use a file instead of nvram */
#endif

/*
** define the priority levels for the production processes
*/
#define TOP_RT_PRIO 5
#define HIGH_RT_PRIO 10
#define NORMAL_RT_PRIO 15
#define LOW_RT_PRIO 20

#endif // _CONFIG_H_
