include $(HOME)/ykt/src/mak/mkstand.mak
TARGET=libpubfunc.a
LIBDIR=../../lib

SOURCE= \
acctrans\
mac\
pubfunc\
dbfunc\
dbfunc_foo\
interfacefunc\
transfunc\
doorfunc\
d3des\
Base64\
md5\
rsa\
rijndael-alg-fst\
rijndael-api-fst\
gnudes\
cpucard\
bufunc


# =====================================================
all:$(TARGET)
	mv $(TARGET) $(LIBDIR)
	@echo $(TARGET) is moved to $(LIBDIR)
	
clean:
	@./.cleanpublic.sh
	rm -f *.o $(TARGET) tp*


$(TARGET):$(SOURCE:=.o)  $(LIBDIR)/libpubdb.a
	$(AR) $(ARFLAGS) $@ $(SOURCE:=.o)
