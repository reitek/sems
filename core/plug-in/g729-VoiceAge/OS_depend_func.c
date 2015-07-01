/*--------------------------------------------------------------------------------*
 *                                                                                *
 * This material is strictly confidential and shall remain as such.               *
 *                                                                                *
 * Copyright © 2004 - VoiceAge Corporation. All Rights Reserved. No part of this  *
 * material may be reproduced, stored in a retrieval system, or transmitted, in   *
 * any form or by any means: photocopying, electronic, mechanical, recording, or  *
 * otherwise, without the prior written permission of the copyright holder.       *
 *                                                                                *
 * This material is subject to continuous developments and improvements. All      *
 * warranties implied or expressed, including but not limited to implied          *
 * warranties of merchantability, or fitness for purpose, are excluded.           *
 *                                                                                *
 * ACELP is registered trademark of VoiceAge Corporation in Canada and / or other *
 * countries. Any unauthorized use is strictly prohibited.                        *
 *                                                                                *
 *--------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h> 
#include <stdarg.h> 
//#include <string.h> 
#include <memory.h> 
#include "OS_depend_func.h"
#include "../../log.h"


//--------------------------------------------------------------------------------;
//  These are system or OS dependent functions. if these are completely 
//  different from standard functions called by codecLib eg. the embedded systems,
//  then, rewrite those functions for your owwn systems
//--------------------------------------------------------------------------------;


/* function print_msg ------------------------------------------- */
void print_msg(char *fmt,va_list ap,char* where)
{
    char buf[500];

    vsprintf(buf,fmt,ap);
    if (where==0) 
    INFO("%s",buf);
    else{
        printf("codecLib_printf problem \n");
        exit(1);
 }

}/* end print_msg */


/*---------------------------*/
/*----- codecLib_printf -----*/
/*---------------------------*/
CODEC_API void codecLib_printf( char *fmt, ...)
{
    va_list ap;

    va_start(ap,fmt);
    print_msg(fmt,ap,0);
    va_end(ap);
}/* end msg */


/*---------------------------*/
/*----- codecLib_malloc -----*/
/*---------------------------*/
CODEC_API void *codecLib_malloc(size_t n)
{
    void* memptr;
    memptr = malloc(n);    

   if (memptr)
	   codecLib_memset(memptr,0,n);

    return memptr;
}


/*-------------------------*/
/*----- codecLib_free -----*/
/*-------------------------*/
CODEC_API void codecLib_free(void* ptr)
{
    free(ptr);
}

/*----------------------------*/
/*----- codecLib_realloc -----*/
/*----------------------------*/
CODEC_API void *codecLib_realloc(void* ptr,size_t n)
{
    ptr = realloc(ptr,n);    
    return ptr;
}

/*---------------------------*/
/*----- codecLib_memcpy -----*/
/*---------------------------*/
CODEC_API void *codecLib_memcpy(void *ptr1, const void *ptr2, size_t n)
{
    void * rp;
    rp = memcpy((void*)ptr1, (const void*)ptr2, (size_t)n);
    return rp;
}

/*---------------------------*/
/*----- codecLib_memset -----*/
/*---------------------------*/
CODEC_API void *codecLib_memset(void *ptr, int i, size_t n)
{
    void* rp;
    rp = memset(ptr, i, n);
    return rp;
}

CODEC_API void codecLib_perror(const char *fmt)
{
    perror(fmt);
    ERROR("%s",fmt);
}

CODEC_API void * codecLib_calloc(size_t n, size_t t)
{
    return(calloc(n, t));
}

/*-------------------------*/
/*----- codecLib_exit -----*/
/*-------------------------*/
CODEC_API void codecLib_exit(size_t n)
{
    exit((int)n);
}
/*--------------------------*/
/*         end of file        */
/*--------------------------*/
