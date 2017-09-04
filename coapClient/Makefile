# name of your application
APPLICATION = coapClient

# If no BOARD is found in the environment, use this default:
BOARD ?= samr21-xpro

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT

BOARD_INSUFFICIENT_MEMORY := calliope-mini chronos microbit msb-430 msb-430h \
                             nucleo32-f031 nucleo32-f042 nucleo32-f303 nucleo32-l031 \
                             nucleo-f030 nucleo-f070 nucleo-f072 nucleo-f103 nucleo-f302 \
                             nucleo-f334 nucleo-l053 spark-core stm32f0discovery telosb \
                             weio wsn430-v1_3b wsn430-v1_4 z1

USEPKG += nanocoap
USEPKG += jsmn

USEMODULE += posix
USEMODULE += od
USEMODULE += fmt
USEMODULE += gcoap
USEMODULE += saul_default
USEMODULE += ipv6_addr
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
USEMODULE += gnrc_ipv6_router_default
USEMODULE += gnrc_icmpv6_echo
USEMODULE += gnrc_udp
USEMODULE += gnrc_sock_udp

USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

INCLUDES += -I$(CURDIR)/include
include $(RIOTBASE)/Makefile.include

# Set a custom channel if needed
DEFAULT_CHANNEL ?= 26
CFLAGS += -DIEEE802154_DEFAULT_CHANNEL=$(DEFAULT_CHANNEL)