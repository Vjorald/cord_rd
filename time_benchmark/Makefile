APPLICATION = time_benchmark

BOARD ?= native

USEMODULE += rd
EXTERNAL_MODULE_DIRS += $(CURDIR)/../modules

USEMODULE += gnrc_sock_udp
USEMODULE += gnrc_ipv6_default
USEMODULE += sock_udp
USEMODULE += sock_util
USEMODULE += sock_async_event
USEMODULE += gcoap


USEMODULE += netdev_default
USEMODULE += auto_init_gnrc_netif

USEMODULE += gnrc_icmpv6_error
USEMODULE += gnrc_ipv6_router_default

USEMODULE += gnrc_rpl
USEMODULE += auto_init_gnrc_rpl

#USEMODULE += netdev_tap


USEMODULE += shell
USEMODULE += shell_cmds_default
USEMODULE += ps

USEMODULE += gnrc_icmpv6_echo
USEMODULE += shell_cmd_gnrc_udp
# Add also the shell, some shell commands
USEMODULE += netstats_l2
USEMODULE += netstats_ipv6
USEMODULE += netstats_rpl

RIOTBASE ?= $(CURDIR)/../RIOT


include $(RIOTBASE)/Makefile.include