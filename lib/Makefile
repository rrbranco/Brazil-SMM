# Really poor makefile, don't forget to use make install before run the test
# programs

all:
	gcc -Wall -Werror libSMM.c -o libSMM.so -shared
	@echo
	@echo
	@echo "Don't forget to run make install before run test programs"
	@echo
	@echo

install:
	cp -f libSMM.so /usr/lib

clean:
	rm -f libSMM.so

cleanall:
	rm -f /usr/lib/libSMM.so
	make clean
