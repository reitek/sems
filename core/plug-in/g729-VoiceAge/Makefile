plug_in_name = g729
COREPATH =../..

VOIDEAGE_DIR = $(DEVELDIR)/vendors/VoiceAgeG729a_Linux
VOICEAGE_INC =	-I$(VOIDEAGE_DIR)/src
VOICEAGE_LIB =  $(VOIDEAGE_DIR)/libs

module_ldflags = $(VOICEAGE_LIB)/codecLib.a $(VOICEAGE_LIB)/g729a.a $(VOICEAGE_LIB)/g729.a $(VOICEAGE_LIB)/utilities.a 
module_cflags  = $(VOICEAGE_INC)

ifdef NOFPU
	module_cflags += -DNOFPU
endif

include ../Makefile.audio_module
