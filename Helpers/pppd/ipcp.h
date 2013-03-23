/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved.
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * ipcp.h - IP Control Protocol definitions.
 *
 * Copyright (c) 1989 Carnegie Mellon University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Carnegie Mellon University.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Id: ipcp.h,v 1.4 2003/08/14 00:00:30 callie Exp $
 */

/*
 * Options.
 */
#define CI_ADDRS	1	/* IP Addresses */
#define CI_COMPRESSTYPE	2	/* Compression Type */
#define	CI_ADDR		3

#define CI_MS_DNS1	129	/* Primary DNS value */
#define CI_MS_WINS1	130	/* Primary WINS value */
#define CI_MS_DNS2	131	/* Secondary DNS value */
#define CI_MS_WINS2	132	/* Secondary WINS value */

#define MAX_STATES 16		/* from slcompress.h */

#define IPCP_VJMODE_OLD 1	/* "old" mode (option # = 0x0037) */
#define IPCP_VJMODE_RFC1172 2	/* "old-rfc"mode (option # = 0x002d) */
#define IPCP_VJMODE_RFC1332 3	/* "new-rfc"mode (option # = 0x002d, */
                                /*  maxslot and slot number compression) */

#define IPCP_VJ_COMP 0x002d	/* current value for VJ compression option*/
#define IPCP_VJ_COMP_OLD 0x0037	/* "old" (i.e, broken) value for VJ */
				/* compression option*/ 

typedef struct ipcp_options {
    bool neg_addr;		/* Negotiate IP Address? */
    bool old_addrs;		/* Use old (IP-Addresses) option? */
    bool req_addr;		/* Ask peer to send IP address? */
    bool default_route;		/* Assign default route through interface? */
    bool proxy_arp;		/* Make proxy ARP entry for peer? */
    bool neg_vj;		/* Van Jacobson Compression? */
    bool old_vj;		/* use old (short) form of VJ option? */
    bool accept_local;		/* accept peer's value for ouraddr */
    bool accept_remote;		/* accept peer's value for hisaddr */
    bool req_dns1;		/* Ask peer to send primary DNS address? */
    bool req_dns2;		/* Ask peer to send secondary DNS address? */
    int  vj_protocol;		/* protocol value to use in VJ option */
    int  maxslotindex;		/* values for RFC1332 VJ compression neg. */
    bool cflag;
    u_int32_t ouraddr, hisaddr;	/* Addresses in NETWORK BYTE ORDER */
    u_int32_t dnsaddr[2];	/* Primary and secondary MS DNS entries */
    u_int32_t winsaddr[2];	/* Primary and secondary MS WINS entries */
} ipcp_options;

extern fsm ipcp_fsm[];
extern ipcp_options ipcp_wantoptions[];
extern ipcp_options ipcp_gotoptions[];
extern ipcp_options ipcp_allowoptions[];
extern ipcp_options ipcp_hisoptions[];

char *ip_ntoa __P((u_int32_t));

extern struct protent ipcp_protent;
