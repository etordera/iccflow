TARGET=iccflow
B=bin
S=src
O=obj
LIBS=-ljpeg -llcms2

all: $(B)/$(TARGET)

$(B)/$(TARGET): $(B) $(O)/iccflow.o $(O)/iccflowapp.o $(O)/iccconverter.o $(O)/iccprofile.o
	g++ -o $(B)/$(TARGET) $(O)/iccflow.o $(O)/iccflowapp.o $(O)/iccconverter.o $(O)/iccprofile.o $(LIBS) 

$(O)/iccflow.o: $(O) $(S)/iccflow.cpp $(S)/iccflowapp.h
	g++ -c -o $(O)/iccflow.o $(S)/iccflow.cpp

$(O)/iccflowapp.o: $(O) $(S)/iccflowapp.cpp $(S)/iccflowapp.h $(S)/iccconverter.h $(S)/globals.h
	g++ -c -o $(O)/iccflowapp.o $(S)/iccflowapp.cpp

$(O)/iccconverter.o: $(O) $(S)/iccconverter.cpp $(S)/iccconverter.h $(S)/iccprofile.h $(S)/icc_fogra27.h $(S)/globals.h
	g++ -c -o $(O)/iccconverter.o $(S)/iccconverter.cpp

$(O)/iccprofile.o: $(O) $(S)/iccprofile.cpp $(S)/iccprofile.h $(S)/icc_adobergb.h
	g++ -c -o $(O)/iccprofile.o $(S)/iccprofile.cpp

$(B):
	mkdir -p $(B)

$(O):
	mkdir -p $(O)

clean:
	rm $(O)/*.o
	rm $(B)/$(TARGET)

