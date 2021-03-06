# Copyright (c) 2017, Lawrence Livermore National Security, LLC. Produced at
# the Lawrence Livermore National Laboratory. LLNL-CODE-734707. All Rights
# reserved. See files LICENSE and NOTICE for details.
#
# This file is part of CEED, a collection of benchmarks, miniapps, software
# libraries and APIs for efficient high-order finite element and spectral
# element discretizations for exascale applications. For more information and
# source code availability see http://github.com/ceed.
#
# The CEED research is supported by the Exascale Computing Project 17-SC-20-SC,
# a collaborative effort of two U.S. Department of Energy organizations (Office
# of Science and the National Nuclear Security Administration) responsible for
# the planning and preparation of a capable exascale ecosystem, including
# software, applications, hardware, advanced system engineering and early
# testbed platforms, in support of the nation's exascale computing imperative.

# Ceed directory
CEED_DIR ?= ../..
CEED_FLAGS ?= -I$(CEED_DIR)
CEED_LIBS ?= -Wl,-rpath,$(abspath $(CEED_DIR)) -L$(CEED_DIR) -lceed

OCCA_DIR ?= ../../../occa
ifneq ($(wildcard $(OCCA_DIR)/lib/libocca.*),)
  OCCA_LIBS ?= -Wl,-rpath,$(abspath $(OCCA_DIR)/lib) -L$(OCCA_DIR)/lib -locca
endif

# Use an MFEM build directory or an MFEM install directory
MFEM_DIR ?= ../../../mfem
CONFIG_MK ?= $(or $(wildcard $(MFEM_DIR)/config/config.mk),$(wildcard\
  $(MFEM_DIR)/share/mfem/config.mk))

MFEM_LIB_FILE = mfem_is_not_built
-include $(CONFIG_MK)

MFEM_EXAMPLES = ex1

.SUFFIXES:
.SUFFIXES: .cpp
.PHONY: all clean

all: $(MFEM_EXAMPLES)

# Remove built-in rules
%: %.cpp

# Rules for building the examples
%: %.cpp $(MFEM_LIB_FILE) $(CONFIG_MK)
	$(MFEM_CXX) $(CEED_FLAGS) $(MFEM_FLAGS) $< -o $@ $(CEED_LIBS) \
	  $(MFEM_LIBS) $(OCCA_LIBS)

# Generate an error message if the MFEM library is not built and exit
$(MFEM_LIB_FILE):
	$(error The MFEM library is not built)

clean:
	rm -f *~ $(MFEM_EXAMPLES)
	rm -rf *.dSYM *.TVD.*breakpoints
