MFLAGS="CFLAGS=-w -c"

all:
	@echo "Making Project [kernel]......"
	#@find ./ * -exec touch {} \;
	cd KBASE && $(MAKE) $(MFLAGS)
	#cd KDBAPI && $(MAKE) $(MFLAGS)
	#cd KNET && $(MAKE) $(MFLAGS)
	
clean:
	cd KBASE && $(MAKE) clean
	cd KDBAPI && $(MAKE) clean
	cd KNET && $(MAKE) clean
