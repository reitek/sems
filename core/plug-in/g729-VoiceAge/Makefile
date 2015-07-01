plug_in_name = g729
COREPATH =../..

VOICEAGE_DIR = $(DEVELDIR)/vendors/VoiceAgeG729a_Linux
VOICEAGE_INC =	-I$(VOICEAGE_DIR)/src
VOICEAGE_LIB =  $(VOICEAGE_DIR)/libs

module_ldflags = $(VOICEAGE_LIB)/codecLib.a $(VOICEAGE_LIB)/g729a.a $(VOICEAGE_LIB)/g729.a $(VOICEAGE_LIB)/utilities.a 
module_cflags  = $(VOICEAGE_INC)

ifdef NOFPU
	module_cflags += -DNOFPU
endif

include ../Makefile.audio_module
