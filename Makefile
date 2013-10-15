TARGET=iccflow
B=bin
S=src
O=obj
LIBS=-ljpeg -llcms2

all: $(B)/$(TARGET)

$(B)/$(TARGET): $(O)/iccflow.o $(O)/iccflowapp.o $(O)/iccconverter.o $(O)/iccprofile.o
	test -d $(B) || mkdir $(B)
	g++ -o $(B)/$(TARGET) $^ $(LIBS) 

$(O)/iccflow.o: $(S)/iccflow.cpp $(S)/iccflowapp.h
	test -d $(O) || mkdir $(O)
	g++ -c -o $(O)/iccflow.o $(S)/iccflow.cpp

$(O)/iccflowapp.o: $(S)/iccflowapp.cpp $(S)/iccflowapp.h $(S)/iccconverter.h $(S)/globals.h
	test -d $(O) || mkdir $(O)
	g++ -c -o $(O)/iccflowapp.o $(S)/iccflowapp.cpp

$(O)/iccconverter.o: $(S)/iccconverter.cpp $(S)/iccconverter.h $(S)/iccprofile.h $(S)/icc_fogra27.h $(S)/globals.h
	test -d $(O) || mkdir $(O)
	g++ -c -o $(O)/iccconverter.o $(S)/iccconverter.cpp

$(O)/iccprofile.o: $(S)/iccprofile.cpp $(S)/iccprofile.h $(S)/icc_adobergb.h
	test -d $(O) || mkdir $(O)
	g++ -c -o $(O)/iccprofile.o $(S)/iccprofile.cpp

clean:
	rm $(O)/*.o
	rm $(B)/*

