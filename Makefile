TARGET=bin/iccflow
SRCDIR=src/
OBJDIR=obj/


all: $(TARGET)

$(TARGET): $(OBJDIR)iccflow.o $(OBJDIR)iccflowapp.o $(OBJDIR)iccconverter.o
	g++ -o $(TARGET) $^ 

$(OBJDIR)iccflow.o: $(SRCDIR)iccflow.cpp
	g++ -c -o $(OBJDIR)iccflow.o $(SRCDIR)iccflow.cpp

$(OBJDIR)iccflowapp.o: $(SRCDIR)iccflowapp.cpp $(SRCDIR)iccflowapp.h
	g++ -c -o $(OBJDIR)iccflowapp.o $(SRCDIR)iccflowapp.cpp

$(OBJDIR)iccconverter.o: $(SRCDIR)iccconverter.cpp $(SRCDIR)iccconverter.h
	g++ -c -o $(OBJDIR)iccconverter.o $(SRCDIR)iccconverter.cpp

clean:
	rm $(OBJDIR)*.o
	rm $(TARGET)
