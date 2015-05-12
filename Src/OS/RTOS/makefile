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
GOAL  = $(LIB_DIR)/wmrtos.$(LIBTYPE)

#---------------------------------------------------------------------------
# Source section (User)
#---------------------------------------------------------------------------
ifeq ($(TOOL_GNU),1)
ASM_SRC				+= ports/portASM_gcc.s
else
ASM_SRC				+= ports/portASM.s
endif
C_SRC				+= ports/port.c
C_SRC				+= source/croutine.c
C_SRC				+= source/heap_2.c
C_SRC				+= source/heap_3.c
C_SRC				+= source/list.c
C_SRC				+= source/queue.c
C_SRC				+= source/rtostimers.c
C_SRC				+= source/tasks.c

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
	
