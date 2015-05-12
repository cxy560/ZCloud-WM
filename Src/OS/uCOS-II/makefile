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
GOAL  = $(LIB_DIR)/wmos.$(LIBTYPE)

#---------------------------------------------------------------------------
# Source section (User)
#---------------------------------------------------------------------------
ifeq ($(TOOL_GNU),1)
ASM_SRC				+= ports/os_cpu_gcc.s
else
ASM_SRC				+= ports/os_cpu_a.s
C_SRC				+= source/ucos_ii.c
endif
C_SRC				+= ports/os_cpu_c.c
C_SRC				+= source/os_core.c
C_SRC				+= source/os_dbg_r.c
C_SRC				+= source/os_flag.c
C_SRC				+= source/os_mbox.c
C_SRC				+= source/os_mem.c
C_SRC				+= source/os_mutex.c
C_SRC				+= source/os_q.c
C_SRC				+= source/os_sem.c
C_SRC				+= source/os_task.c
C_SRC				+= source/os_time.c
C_SRC				+= source/os_tmr.c

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
	