#/**************************************************************************
# * Kevin 2014-02-24                                                       *
# **************************************************************************/

#---------------------------------------------------------------------------
# Constant Variable definition
#---------------------------------------------------------------------------
ROOT = ../../..
include $(ROOT)/Tools/tool_chain.def
LIB_DIR =	$(ROOT)/lib/$(LIBDIR)

#---------------------------------------------------------------------------
# Target definition (User)
#---------------------------------------------------------------------------
GOAL  = $(LIB_DIR)/wmlwip.$(LIBTYPE)

#---------------------------------------------------------------------------
# Source section (User)
#---------------------------------------------------------------------------
ASM_SRC			+= 
C_SRC				+= ethernet.c
C_SRC				+= sys_arch.c

C_SRC				+= api/api_lib.c
C_SRC				+= api/api_msg.c
C_SRC				+= api/err.c
C_SRC				+= api/netbuf.c
C_SRC				+= api/netdb.c
C_SRC				+= api/netifapi.c
C_SRC				+= api/sockets.c
C_SRC				+= api/tcpip.c

C_SRC				+= netif/etharp.c
C_SRC				+= netif/ethernetif.c

C_SRC				+= core/tcp.c
C_SRC				+= core/tcp_in.c
C_SRC				+= core/tcp_out.c
C_SRC				+= core/timers.c
C_SRC				+= core/udp.c
C_SRC				+= core/def.c
C_SRC				+= core/dhcp.c
C_SRC				+= core/dns.c
C_SRC				+= core/init.c
C_SRC				+= core/mem.c
C_SRC				+= core/memp.c
C_SRC				+= core/netif.c
C_SRC				+= core/pbuf.c
C_SRC				+= core/raw.c
C_SRC				+= core/stats.c
C_SRC				+= core/sys.c

C_SRC				+= core/ipv4/autoip.c
C_SRC				+= core/ipv4/icmp.c
C_SRC				+= core/ipv4/igmp.c
C_SRC				+= core/ipv4/inet.c
C_SRC				+= core/ipv4/inet_chksum.c
C_SRC				+= core/ipv4/ip.c
C_SRC				+= core/ipv4/ip_addr.c
C_SRC				+= core/ipv4/ip_frag.c
#---------------------------------------------------------------------------
# Implicit rules
#---------------------------------------------------------------------------
.c.o:
	@$(ECHO) "Compiling $<..."
	@$(CC) $(CCFLAGS) -c -o $*.o $< $(INCLUDE_DIR)

.s.o:
	@$(ECHO) "Compiling $<..."
	@$(ASM) $(ASMFLAGS) -c -o $*.o $< $(INCLUDE_DIR)

#---------------------------------------------------------------------------
# Explicit ruls
#---------------------------------------------------------------------------
OBJ_FILES	= $(C_SRC:%.c=%.o) $(ASM_SRC:%.s=%.o)

all: $(GOAL)

$(GOAL): $(OBJ_FILES)
	@$(AR) $(ARFLAGS) $@ $(OBJ_FILES)
	@$(ECHO) "***" $(GOAL) "is built successfully! ***"
	@$(ECHO) "---------"
	@rm -f $(OBJ_FILES:.o=.d) $(OBJ_FILES)
	
.PHONY: clean
clean:
	@rm -f $(GOAL)
	@rm -f $(OBJ_FILES:.o=.d) $(OBJ_FILES)
	