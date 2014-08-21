.SUFFIXES: .cc .C .d .o

CXX       = g++
# -O2 gives SIGSEGV when starting -> should be test under UNIX
#PROF      = -g -O2 -pg
# gprof rom.exe    profiles mud
#PROF      = -g -pg
# debug
PROF      = -g -O1
NOCRYPT   =
#CXXFLAGS = -fhandle-exceptions $(PROF) $(NOCRYPT)
#CXXFLAGS = -fexceptions $(PROF) $(NOCRYPT)
#Red Hat
#CXXFLAGS  = -Wall -Wno-sign-compare $(PROF) $(NOCRYPT)
#Suse, Cygwin
CXXFLAGS  = -Wall -Wno-unused $(PROF) $(NOCRYPT)

CFILES=act_comm.C act_enter.C act_info.C act_move.C act_obj.C \
	act_wiz.C act_wiz2.C punish.C \
	alias.C ban.C comm.C const.C db.C db2.C effects.C fight.C flags.C \
	handler.C healer.C interp.C lookup.C \
	magic.C magic2.C magic3.C \
	board.C copyover.C moons.C hints.C\
	recycle.C save.C scan.C skills.C special.C tables.C \
	update.C olc.C olc_act.C olc_save.C bit.C mem.C string.C olc_value.C\
	wizutil.C hunt.C classes.C clan.C affects.C \
	act_wiz3.C arena.C song.C qmaze.C data.C\
	restriction.C combatabilities.C noncombatabilities.C abilityupgrade.C \
	condition.C raceabilities.C language.C prereqs.C\
	script.C scrvalue.C dbscript.C scrpred.C act_scri.C scrhash.C \
	grammar.cc scanner.cc execute.cc cxx/parser_skel.cc \
	names.C power.C magic4.C smaug_load.C wiznet.C \
	magic5.C update_affect.C wearoff_affect.C brew.C string_space.C \
	faction.cc data_parser.cc data_execute.cc data_scanner.cc dbdata.C \
	ability.C group.C db3.C config.C gc_helper.C data_edit.C \
	ranged.C script_edit.C error.cc remort.C circle_load.C dsa_load.C \
	statistics.C utils.C html.C magic_schools.C damage.C asciimap.C \
	super_races.C weather.C

#NODEP=true

OFILES:=$(CFILES:.cc=.o) 
OFILES:=$(OFILES:.C=.o)

DEPFILES=$(OFILES:.o=.d)

GENFILES=grammar.cc grammar.hh prereq_parser.cc prereq_parser.hh

.PHONY: default fakeaut clean veryclean

default: rom

t:
	echo $(OFILES)

rom: $(OFILES) gc/gc.a
	rm -f ../rom.exe
	$(CXX) $(CXXFLAGS) -o ../rom $(OFILES) gc/gc.a

clean:
	rm ../rom.exe core ../area/core *.o *.d *~ cxx/*.o

veryclean: clean
	rm $(GENFILES)

%.d: %.cc
	set -e; $(CXX) -MM $(CPPFLAGS) $< \
	| sed 's/\($(subst /,\/,$*)\)\.o[ :]*/\1.o $(subst /,\/,$@) : /g' > $@; \
	[ -s $@ ] || rm -f $@

%.d: %.C
	set -e; $(CXX) -MM $(CPPFLAGS) $< \
	| sed 's/\($(subst /,\/,$*)\)\.o[ :]*/\1.o $(subst /,\/,$@) : /g' > $@; \
	[ -s $@ ] || rm -f $@

ifndef NODEP
include $(DEPFILES)
endif
