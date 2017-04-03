/* Benjamin Ellerby
 * Ray Luow
 * Evan Ricks
 * Oliver Smith-Denny
 * spr.h, header file for Portcullis, the Secure Port Reservation (SPR) Daemon
 * Published under the MIT License copyright (C) 2017 by the aformentioned authors
 * Defines functions related to interacting with the SPR Daemon
*/

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS }
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif

#undef __P
#if defined (__STDC__) || defined (_AIX) \
  || (defined (__mips) && defined (_SYSTYPE_SVR4)) \
  || defined(__cplusplus)
#define __P(protos) protos
#else
#define __P(protos) ()
#endif

#ifndef _SPR_H_
#define _SPR_H_ 1

__BEGIN_DECLS

#define _GNU_SOURCE

int secureBind __P((int portNum, char *udsName));
int secureClose __P((int portNum));

__END_DECLS

#endif
