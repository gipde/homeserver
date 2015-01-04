CXX=avr-g++

test: Ds18x20libTest.o TestBase.o
	$(CXX) -o test Ds18x20libTest.o TestBase.o
Ds18x20libTest.E: Ds18x20libTest.cpp test.h
	-rm $@
	$(CXX) -E Ds18x20libTest.cpp

clean:
	rm *.o
	rm test
