#!/bin/bash
app=$5

echo "#ifndef __PROJECT_CONF_H__
#define __PROJECT_CONF_H__

#define TRAFFIC_PATTERN $1	// 0: Periodic, 1: Event-driven /* jk */
#if TRAFFIC_PATTERN == 0 // If periodic
#define PERIOD	$2 /* jk */
#else	// If event driven (assume poisson)
#define INTENSITY $3 // lambda /* jk */
#endif

#define RPL_CONF_OF rpl_mrhof
// #define RPL_CONF_OF rpl_of0


#undef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE	$4

#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     csma_driver
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     cxmac_driver
#undef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER  framer_802154


#endif" > ../../../lanada_$app/project-conf.h
