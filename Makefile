TARGET=bin/iccflow
S=src/
O=obj/
LIBS=-ljpeg -llcms2

all: $(TARGET)

$(TARGET): $(O)iccflow.o $(O)iccflowapp.o $(O)iccconverter.o $(O)iccprofile.o
	g++ -o $(TARGET) $^ $(LIBS) 

$(O)iccflow.o: $(S)iccflow.cpp
	g++ -c -o $(O)iccflow.o $(S)iccflow.cpp

$(O)iccflowapp.o: $(S)iccflowapp.cpp $(S)iccflowapp.h
	g++ -c -o $(O)iccflowapp.o $(S)iccflowapp.cpp

$(O)iccconverter.o: $(S)iccconverter.cpp $(S)iccconverter.h
	g++ -c -o $(O)iccconverter.o $(S)iccconverter.cpp

$(O)iccprofile.o: $(S)iccprofile.cpp $(S)iccprofile.h
	g++ -c -o $(O)iccprofile.o $(S)iccprofile.cpp

clean:
	rm $(O)*.o
	rm $(TARGET)
