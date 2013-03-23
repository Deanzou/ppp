/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 *
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/* -----------------------------------------------------------------------------
includes
----------------------------------------------------------------------------- */
#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <CoreFoundation/CoreFoundation.h>

#ifdef	USE_SYSTEMCONFIGURATION_PUBLIC_APIS
#include <SystemConfiguration/SystemConfiguration.h>
#else	/* USE_SYSTEMCONFIGURATION_PUBLIC_APIS */
#include <SystemConfiguration/v1Compatibility.h>
#include <SystemConfiguration/SCSchemaDefinitions.h>
#endif	/* USE_SYSTEMCONFIGURATION_PUBLIC_APIS */

#include "ppp_msg.h"
#include "ppp_privmsg.h"

#include "ppp_client.h"
#include "ppp_option.h"
#include "ppp_command.h"
#include "ppp_manager.h"

/* -----------------------------------------------------------------------------
definitions
----------------------------------------------------------------------------- */



/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
u_long set_long_opt (struct opt_long *option, u_long opt, u_long mini, u_long maxi, u_long limit)
{
    if (opt < mini) {
        if (limit) opt = mini;
        else return EINVAL;
    }
    else if (opt > maxi) {
        if (limit) opt = maxi;
        else return EINVAL;
    }
    option->set = 1;
    option->val = opt;
    return 0;
}

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
u_long set_str_opt (struct opt_str *option, char *opt, int len)
{

    if (len < OPT_STR_LEN) {
        option->set = 1;
        strncpy(option->str, opt, len);
        option->str[len] = 0;
            return 0;
    }

    return EMSGSIZE;
}


/* -----------------------------------------------------------------------------
id must be a valid client 
----------------------------------------------------------------------------- */
u_long ppp_setoption (struct client *client, struct msg *msg)
{
    struct ppp_opt 	*opt = (struct ppp_opt *)&msg->data[0];
    struct options	*opts;
    u_long		err = 0, len = msg->hdr.m_len - sizeof(struct ppp_opt_hdr);
    u_long		speed;
    struct ppp 		*ppp = ppp_findbyref(msg->hdr.m_link);
    
    if (!ppp) {
        msg->hdr.m_result = ENODEV;
        msg->hdr.m_len = 0;
        return 0;
    }

    // not connected, set the client options that will be used.
    opts = client_findoptset(client, msg->hdr.m_link);
    if (!opts) {
        // first option used by client, create private set
        opts = client_newoptset(client, msg->hdr.m_link);
        if (!opts) {
            msg->hdr.m_result = ENOMEM;
            msg->hdr.m_len = 0;
            return 0;
        }
    }

    switch (opt->o_type) {

        // COMM options
        case PPP_OPT_DEV_NAME:
            err = set_str_opt(&opts->dev.name, &opt->o_data[0], len);
            break;
        case PPP_OPT_DEV_SPEED:
            // add flexibility and adapt the speed to the immediatly higher speed
            speed = *(u_long *)(&opt->o_data[0]);
            if (speed <= 1200) speed = 1200;
            else if ((speed > 1200) && (speed <= 2400)) speed = 2400;
            else if ((speed > 2400) && (speed <= 9600)) speed = 9600;
            else if ((speed > 9600) && (speed <= 19200)) speed = 19200;
            else if ((speed > 19200) && (speed <= 38400)) speed = 38400;
            else if ((speed > 38400) && (speed <= 57600)) speed = 57600;
            else if ((speed > 38400) && (speed <= 57600)) speed = 57600;
            else if ((speed > 57600) && (speed <= 0xFFFFFFFF)) speed = 115200;
            err = set_long_opt(&opts->dev.speed, speed, 0, 0xFFFFFFFF, 0);
            break;
        case PPP_OPT_DEV_CONNECTSCRIPT:
            err = set_str_opt(&opts->dev.connectscript, &opt->o_data[0], len);
            break;
        case PPP_OPT_COMM_TERMINALMODE:
	    err = set_long_opt(&opts->comm.terminalmode, *(u_long *)(&opt->o_data[0]), 0, 0xFFFFFFFF, 1);
	    break;
        case PPP_OPT_COMM_TERMINALSCRIPT:
            err = set_str_opt(&opts->comm.terminalscript, &opt->o_data[0], len);
            break;
        case PPP_OPT_COMM_REMOTEADDR:
        //printf("XXXXXXXXXXXXXXXXXXXXXXx SETOPTION REMOTE ADDRESS : %s, len = %d\n", opt->o_data, len);
            err = set_str_opt(&opts->comm.remoteaddr, &opt->o_data[0], len);
            break;
        case PPP_OPT_COMM_IDLETIMER:
            err = set_long_opt(&opts->comm.idletimer, *(u_long *)(&opt->o_data[0]), 0, 0xFFFFFFFF, 1);
            break;
        case PPP_OPT_COMM_SESSIONTIMER:
            err = set_long_opt(&opts->comm.sessiontimer, *(u_long *)(&opt->o_data[0]), 0, 0xFFFFFFFF, 1);
            break;
        case PPP_OPT_COMM_CONNECTDELAY:
            err = set_long_opt(&opts->comm.connectdelay, *(u_long *)(&opt->o_data[0]), 0, 0xFFFFFFFF, 1);
            break;

            // LCP options
        case PPP_OPT_LCP_HDRCOMP:
            err = set_long_opt(&opts->lcp.pcomp, ((*(u_long *)(&opt->o_data[0])) & PPP_LCP_HDRCOMP_PROTO) ? 1 : 0, 0, 1, 1);
            if (!err)
                err = set_long_opt(&opts->lcp.accomp, ((*(u_long *)(&opt->o_data[0])) & PPP_LCP_HDRCOMP_ADDR) ? 1 : 0, 0, 1, 1);
            break;
        case PPP_OPT_LCP_MRU:
            err = set_long_opt(&opts->lcp.mru, *(u_long *)(&opt->o_data[0]), 0, 0xFFFFFFFF, 1);
            break;
        case PPP_OPT_LCP_MTU:
            err = set_long_opt(&opts->lcp.mtu, *(u_long *)(&opt->o_data[0]), 0, 0xFFFFFFFF, 1);
            break;
        case PPP_OPT_LCP_RCACCM:
            err = set_long_opt(&opts->lcp.rcaccm, *(u_long *)(&opt->o_data[0]), 0, 0xFFFFFFFF, 1);
            break;
        case PPP_OPT_LCP_TXACCM:
            err = set_long_opt(&opts->lcp.txaccm, *(u_long *)(&opt->o_data[0]), 0, 0xFFFFFFFF, 1);
            break;
        case PPP_OPT_LCP_ECHO:
            err = set_long_opt(&opts->lcp.echointerval, ((struct ppp_opt_echo *)opt->o_data)->interval, 0, 0xFFFFFFFF, 1);
            err = set_long_opt(&opts->lcp.echofailure, ((struct ppp_opt_echo *)opt->o_data)->failure, 0, 0xFFFFFFFF, 1);
            break;

            // SEC options
        case PPP_OPT_AUTH_PROTO:
            err = set_long_opt(&opts->auth.proto, *(u_long *)(&opt->o_data[0]), 0, PPP_AUTH_CHAP, 1);
            break;
        case PPP_OPT_AUTH_NAME:
           err = set_str_opt(&opts->auth.name, &opt->o_data[0], len);
            break;
        case PPP_OPT_AUTH_PASSWD:
            err = set_str_opt(&opts->auth.passwd, &opt->o_data[0], len);
            break;

            // IPCP options
        case PPP_OPT_IPCP_HDRCOMP:
            err = set_long_opt(&opts->ipcp.hdrcomp, *(u_long *)(&opt->o_data[0]), 0, 1, 1);
            break;
        case PPP_OPT_IPCP_REMOTEADDR:
            err = set_long_opt(&opts->ipcp.remoteaddr, *(u_long *)(&opt->o_data[0]), 0, 0xFFFFFFFF, 1);
            break;
        case PPP_OPT_IPCP_LOCALADDR:
            err = set_long_opt(&opts->ipcp.localaddr, *(u_long *)(&opt->o_data[0]), 0, 0xFFFFFFFF, 1);
            break;
            // MISC options
        case PPP_OPT_LOGFILE:
           err = set_str_opt(&opts->misc.logfile, &opt->o_data[0], len);
            break;
        case PPP_OPT_COMM_REMINDERTIMER:
            err = set_long_opt(&opts->comm.remindertimer, *(u_long *)(&opt->o_data[0]), 0, 0xFFFFFFFF, 1);
            break;
        case PPP_OPT_ALERTENABLE:
            //err = set_long_opt(&opts->misc.alertenable, *(u_long *)(&opt->o_data[0]), 0, 1, 1);
            // this option has immediate effect, at least on alert/dialogs
            ppp->alertenable = *(u_long *)(&opt->o_data[0]);
            break;
        default:
            err = EOPNOTSUPP;
    };
    
    msg->hdr.m_result = err;
    msg->hdr.m_len = 0;
    return 0;
}
