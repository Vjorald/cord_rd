APPLICATION = endpoint

BOARD ?= native

# Folder of the RIOT-Submodule
RIOTBASE ?= $(CURDIR)/../../RIOT

USEMODULE += cord_ep
USEMODULE += cord_common

USEMODULE += ztimer
USEMODULE += ztimer_sec
#USEMODULE += cord_lc
#USEMODULE += cord_common
USEMODULE += gcoap      # RIOT's CoAP implementation
#USEMODULE += netdev_default
#USEMODULE += auto_init_saul  # For sensor/actuator abstraction
#USEMODULE += saul_default    # For using sensors/actuators with SAUL
#USEMODULE += saul
#USEMODULE += gnrc_netif
#USEMODULE += gnrc_ipv6
#USEMODULE += gnrc_icmpv6
#USEMODULE += gnrc_udp

USEMODULE += gnrc_sock_udp
USEMODULE += sock_udp
USEMODULE += sock_util
USEMODULE += sock_async_event
USEMODULE += gnrc_ipv6_default

USEMODULE += netdev_default
USEMODULE += auto_init_gnrc_netif
# Activate ICMPv6 error messages
USEMODULE += gnrc_icmpv6_error
# Specify the mandatory networking module for a IPv6 routing node
USEMODULE += gnrc_ipv6_router_default
# Add a routing protocol
USEMODULE += gnrc_rpl
USEMODULE += auto_init_gnrc_rpl

USEMODULE += shell
USEMODULE += shell_cmds_default
USEMODULE += ps

USEMODULE += gnrc_icmpv6_echo
USEMODULE += shell_cmd_gnrc_udp
# Add also the shell, some shell commands
USEMODULE += netstats_l2
USEMODULE += netstats_ipv6
USEMODULE += netstats_rpl

#USEMODULE += usbus_cdc_ecm
#USB_VID = ${USB_VID_TESTING}
#USB_PID = ${USB_PID_TESTING}

IPV6_STATIC_LLADDR ?= '"fe80::cafe:cafe:cafe:2"'
CFLAGS += -DCONFIG_GNRC_IPV6_STATIC_LLADDR=$(IPV6_STATIC_LLADDR)
CFLAGS += -DCONFIG_GNRC_IPV6_STATIC_LLADDR_IS_FIXED

#USEMODULE += socket_zep
#USEMODULE += socket_zep_hello

#USEMODULE += nimble_riot

#USEMODULE += netdev_tap


include $(RIOTBASE)/Makefile.include

include $(RIOTMAKE)/default-radio-settings.inc.mk