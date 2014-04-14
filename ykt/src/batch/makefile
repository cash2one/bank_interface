include $(HOME)/ykt/src/mak/mkstand.mak
TARGET=upaybankpaydeal


COMMON_SRC = \
ConfigFile\
logger_imp

APLIBS=-L$(HOME)/ykt/lib -lstdc++  -lbupubs -lxpack -lpubfunc -lpubdb -llog4cplus

INCLD+=-I../tinyxml

# =====================================================
all:$(TARGET)

clean:
	@./.cleanfunc.sh
	@rm -f *.o *.bnd *.lis tp*

	
UPAYBANKPAY_SRC = \
upaybankpay \
yktclt\
$(COMMON_SRC)
	
upaybankpaydeal:$(UPAYBANKPAY_SRC:=.o) $(HOME)/ykt/lib/libpubdb.a
	$(CXX) $(CCFLAGS) $(INCLD) $(LIBS) -o $@ $(UPAYBANKPAY_SRC:=.o) $(APLIBS) -L../../bin -lbccclt $(LIBSUDF)
	strip $@
	mv $@ ../../bin
