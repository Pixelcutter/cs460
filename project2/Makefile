OPTIONS = -Wall -Werror
COMPILER = gcc
BUILD = build
OUT = p2 
CPATH = c_files
HPATH = h_files

$(OUT) : $(BUILD)/main.o \
	 $(BUILD)/utilFuncs.o \
	 $(BUILD)/io.o \
	 $(BUILD)/parser.o \
	 $(BUILD)/printStats.o \
	 $(BUILD)/rr.o \
	 $(BUILD)/sjf.o \
	 $(BUILD)/fcfs.o \
	 $(BUILD)/pr.o
	$(COMPILER) $(BUILD)/*.o -o $(OUT)

$(BUILD)/main.o : $(CPATH)/main.c $(HPATH)/*.h $(BUILD)
	$(COMPILER) $(CPATH)/main.c -c $(OPTIONS) -o $(BUILD)/main.o -pthread

$(BUILD)/parser.o : $(CPATH)/parser.c $(HPATH)/global.h $(HPATH)/utilFuncs.h $(BUILD)
	$(COMPILER) $(CPATH)/parser.c -c $(OPTIONS) -o $(BUILD)/parser.o -pthread

$(BUILD)/io.o : $(CPATH)/io.c $(HPATH)/global.h $(HPATH)/utilFuncs.h $(BUILD)
	$(COMPILER) $(CPATH)/io.c -c $(OPTIONS) -o $(BUILD)/io.o -pthread

$(BUILD)/utilFuncs.o : $(CPATH)/utilFuncs.c $(HPATH)/global.h $(BUILD)
	$(COMPILER) $(CPATH)/utilFuncs.c -c $(OPTIONS) -o $(BUILD)/utilFuncs.o -pthread

$(BUILD)/printStats.o : $(CPATH)/printStats.c $(HPATH)/global.h $(BUILD)
	$(COMPILER) $(CPATH)/printStats.c -c $(OPTIONS) -o $(BUILD)/printStats.o -pthread

$(BUILD)/sjf.o : $(CPATH)/sjf.c $(HPATH)/global.h $(HPATH)/utilFuncs.h $(BUILD)
	$(COMPILER) $(CPATH)/sjf.c -c $(OPTIONS) -o $(BUILD)/sjf.o -pthread

$(BUILD)/fcfs.o : $(CPATH)/fcfs.c $(HPATH)/global.h $(HPATH)/utilFuncs.h $(BUILD)
	$(COMPILER) $(CPATH)/fcfs.c -c $(OPTIONS) -o $(BUILD)/fcfs.o -pthread

$(BUILD)/pr.o : $(CPATH)/pr.c $(HPATH)/global.h $(HPATH)/utilFuncs.h $(BUILD)
	$(COMPILER) $(CPATH)/pr.c -c $(OPTIONS) -o $(BUILD)/pr.o -pthread

$(BUILD)/rr.o : $(CPATH)/rr.c $(HPATH)/global.h $(HPATH)/utilFuncs.h $(BUILD)
	$(COMPILER) $(CPATH)/rr.c -c $(OPTIONS) -o $(BUILD)/rr.o -pthread

$(BUILD) :
	mkdir -p $(BUILD)

clean :
	rm -rf $(BUILD) $(OUT)

run :
	./$(OUT)