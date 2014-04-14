include ../mak/mkstand.mak
TARGET=upaybanksvr

FUNC=\
ecbankfunc\
ecbankytc\
F3001\
F3002\
F3003\
F3005\
F3006\
F3007\
F3401\
F3402\
F3403\
F3404

SOURCE=\
ConfigFile\
config_imp\
logger_imp\
unitprocess\
ecardmain


OBJS=$(FUNC:=.o) $(SOURCE:=.o) 

APLIBS=-L$(HOME)/ykt/lib -lstdc++  -lbupubs -lxpack -lpubfunc -lpubdb -llog4cplus -lmessage -ljson_linux-gcc-4.1.2_libmt -ltinyxml

INCLD+=-I../tinyxml

# =====================================================
all:$(TARGET)

clean:
	@./.cleanfunc.sh
	@rm -f *.o *.bnd *.lis tp*

upaybanksvr:$(OBJS) $(HOME)/ykt/lib/libpubdb.a
	$(CXX) $(CCFLAGS) $(INCLD) $(LIBS) -o $@ $(OBJS) $(APLIBS) $(LIBSUDF)
	strip $@
	mv $@ ../../bin
