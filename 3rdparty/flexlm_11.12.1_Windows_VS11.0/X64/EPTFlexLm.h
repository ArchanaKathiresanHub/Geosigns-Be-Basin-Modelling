/*********************************************************************************
 *
 * NAME:           EPTFlexLm.h
 *
 * COPYRIGHT:      Title to, ownership of and copyright of this software is
 *                 vested in Shell Internationale Research Maatschappij B.V.
 *                 All rights reserved.
 *                 Neither the whole nor any part of this software may be
 *                 reproduced, copied, stored in any retrieval system or
 *                 transmitted in any form or by any means without prior
 *                 written consent of the copyright owner.
 *                 This software and the information and data it contains,
 *                 is confidential. Neither the whole nor any part of the
 *                 software and the data and information it contains may
 *                 be disclosed to any third party without the prior written
 *                 consent of Shell Internationale Research Maatschappij B.V.,
 *                 The Hague, The Netherlands.
 *
 * FUNCTION:       This header file defines the constants and external
 *                 declarations which are used by the EPT - FlexLm interface.
 *
 * PACKAGE:        License Management, FLEXlm, interface.
 *
 * DEPENDENCIES:
 *
 * USAGE:          If a SEPTAR application wants to use license management
 *                 it has to call the API functions. To use these functions
 *                 properly this header file must be included in the source.
 *
 * AUTHOR           DATE      DESCRIPTION
 * ------------------------------------------------------------------
 * Details of change history, beginning with details of the original
 * version, followed by all subsequent changes, in date order.
 *
 * J.A. Forrer    Sept/Okt-96  Created and defined the API functions,
 * (Origin/Rotterdam)      Version 1.0.
 *
 * J.A. Forrer    Dec-96        Add incode comments.
 * (Origin/Rot.)
 *
 * D. Sickinghe	31-May-2000	Upgrade to FLEXlm version 7.0.
 *				Added and removed code.
 *
 * E. Kater     11-Dec-2003     Upgraded to FlexLm 9.2
 *                              Added call for num of days to expiration
 *
 * H. Kamsteeg  25-Jan-2005     Added Borrowing functionality
 *                              Added turn-off Heartbeats
 *
 *********************************************************************************/

#ifndef EPTFLEXLMAPI_H
#define EPTFLEXLMAPI_H

#define EPTFLEXLM_OK 0                /* return status successful, no warning */
#define EPTFLEXLM_WARN 1              /* return status successful with warning */

#define EPTFLEXLM_MAX_MESS_LEN 1025   /* array length for message string */
#define EPTFLEXLM_MAX_FEATURE_LEN 31  /* = MAX_FEATURE_LEN (in lmclient.h) + 1 */
#define EPTFLEXLM_MAX_VER_LEN 11      /* = MAX_VER_LEN (in lmclient.h) + 1     */


#ifdef __cplusplus
extern "C"
{
#endif

	int EPTFlexLmInit(char *);
        int EPTFlexLmGetHostid(char *);
	int EPTFlexLmCheckOut(char *, char *, char *) ;
	void EPTFlexLmCheckIn(char *);
        int EPTFlexLmExpDays(char *, char *);
        int EPTFlexLmMaxBorrowDays(char *);
        int EPTFlexLmSetBorrow(char *, char *);
        int EPTFlexLmSetBorrowDays(int, char *);
        int EPTFlexLmGetBorrow(	char *, char *);
        int EPTFlexLmShowBorrow(char *);
        int EPTFlexLmReturnBorrow( char *, char * );
	void EPTFlexLmTerminate(void);
        void EPTFlexLmAddDays(int, char *);
        int EPTFlexLmLeapYear(int);
        int EPTFlexLmDaysInYear(int);
	void EPTFlexLMVersionString(char *);

#ifdef __cplusplus
}
#endif

#endif

