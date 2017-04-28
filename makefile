CC = gcc
RM = rm -rf
OBJECTS = driver.o utils.o
EXE = capture
OTHERS = *.temp *.txt *.png capture

.PHONY: packetcapture
all: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXE)

driver.o: utils.h 
utils.o: utils.h

.PHONY: clean
clean:
	$(RM) $(OTHERS) $(OBJECTS) packetcapture
