include ../mak/mkstand.mak
TARGET=upayposvr  upayfilesvr upaychkfilesvr

FUNC=\
unionpaypos\
F1200\
F200000

SOURCE=\
8583_impl\
ks_8583_reader\
ConfigFile\
config_imp\
logger_imp\
unitprocess\
ecardmain

UPAYFILESRC=\
8583_impl\
ks_8583_reader\
ConfigFile\
logger_imp\
unionpaypos\
upayfilesvr

UPAYCHKFILESRC=\
ConfigFile\
config_imp\
logger_imp\
upaychkfilesvr

OBJS=$(FUNC:=.o) $(SOURCE:=.o) 

APLIBS=-L$(HOME)/ykt/lib -lstdc++  -lbupubs -lxpack -lpubfunc -lpubdb -llog4cplus -lmessage -ljson_linux-gcc-4.1.2_libmt

# =====================================================
all:$(TARGET)

clean:
	@./.cleanfunc.sh
	@rm -f *.o *.bnd *.lis tp*

upayposvr:$(OBJS) $(HOME)/ykt/lib/libpubdb.a
	$(CXX) $(CCFLAGS) $(INCLD) $(LIBS) -o $@ $(OBJS) $(APLIBS) $(LIBSUDF)
	strip $@
	mv $@ ../../bin
upaychkfilesvr:$(UPAYCHKFILESRC:=.o) $(HOME)/ykt/lib/libpubdb.a
	$(CXX) $(CCFLAGS) $(INCLD) $(LIBS) -o $@ $(UPAYCHKFILESRC:=.o) $(APLIBS) $(LIBSUDF)
	strip $@
	mv $@ ../../bin
upayfilesvr:$(UPAYFILESRC:=.o) $(HOME)/ykt/lib/libpubdb.a
	$(CXX) $(CCFLAGS) $(INCLD) $(LIBS) -o $@ $(UPAYFILESRC:=.o) $(APLIBS) $(LIBSUDF)
	strip $@
	mv $@ ../../bin
