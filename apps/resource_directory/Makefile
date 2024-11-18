APPLICATION = resource_directory

BOARD ?= native

# Folder of the RIOT-Submodule
RIOTBASE ?= $(CURDIR)/../../RIOT

USEMODULE += gnrc_sock_udp
USEMODULE += gnrc_ipv6_default
USEMODULE += sock_udp
USEMODULE += sock_util
USEMODULE += sock_async_event
USEMODULE += gcoap

USEMODULE += cord_ep

USEMODULE += netdev_default
USEMODULE += auto_init_gnrc_netif

USEMODULE += gnrc_icmpv6_error
USEMODULE += gnrc_ipv6_router_default

USEMODULE += gnrc_rpl
USEMODULE += auto_init_gnrc_rpl

USEMODULE += netdev_tap


USEMODULE += shell
USEMODULE += shell_cmds_default
USEMODULE += ps

USEMODULE += gnrc_icmpv6_echo
USEMODULE += shell_cmd_gnrc_udp
# Add also the shell, some shell commands
USEMODULE += netstats_l2
USEMODULE += netstats_ipv6
USEMODULE += netstats_rpl

IPV6_STATIC_LLADDR ?= '"fe80::cafe:cafe:cafe:1"'
CFLAGS += -DCONFIG_GNRC_IPV6_STATIC_LLADDR=$(IPV6_STATIC_LLADDR)
CFLAGS += -DCONFIG_GNRC_IPV6_STATIC_LLADDR_IS_FIXED


# add CoAP module
USEMODULE += gcoap


ifdef DBG_ID_COAP
  DEBUG_ADAPTER_ID=$(DBG_ID_COAP)
  PORT=/dev/ttyACM1
endif

include $(RIOTBASE)/Makefile.include

include $(RIOTMAKE)/default-radio-settings.inc.mk