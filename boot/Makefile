#==============================================================================
# Copyright 2020 Daniel Boals & Michael Boals
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
# THE SOFTWARE.
#==============================================================================

#
# NOTE the boot laoder is itself a seperate binary, so we will build this
# with its own buidroot
#

.PHONY: clean local_clean_objs

# eventually these variables should be passed in 
SRCROOT	=$(CURDIR)/../../
ASRC	=startup.s
CSRC	=boot.c

include $(SRCROOT)/MAKE/RULES.MAK

INC+= -I../include
INC+= -I../../rtos/arc/aarch64/common_peripherals/include
LFLAGS      =  -L$(BUILDROOT)

#------------------------------------------------------------------------------
#  default rule.  Make the boot loader program blp.bin 
#------------------------------------------------------------------------------
blp.bin: $(OBJS) $(AOBJS) blp.ld
	$(LD) $(LFLAGS) -Map=$(BUILDROOT)/blp.map -o $(BUILDDIR)/blp.elf -T blp.ld $(AOBJS) $(OBJS)
	$(OBJCOPY) -O binary $(BUILDDIR)/blp.elf $(BUILDDIR)/blp.bin

include $(SRCROOT)/MAKE/TARGETS.MAK

#------------------------------------------------------------------------------
#  clean remove all the object and binary files. 
#------------------------------------------------------------------------------
clean: local_clean_objs local_clean_aobjs
	rm -f $(BUILDROOT)/*.map;
	rm -f $(BUILDROOT)/*.bin
	rm -f $(BUILDROOT)/*.elf