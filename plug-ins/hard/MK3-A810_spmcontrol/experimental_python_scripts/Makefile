LDLIBS=`pkg-config --libs glib-2.0`
CFLAGS=`pkg-config --cflags glib-2.0`
CC=c++ -Wall -Wextra 
PROJECT=mk3_reset

full: clean $(PROJECT)
	@echo usage: mk3_reset "</dev/sranger_mk2_xxx>"
	@echo small tool to reset MK3Pro boards
	@echo calling without optional device: /dev/sranger_mk2_0 is used

clean:
	rm -rf $(PROJECT)
	
$(PROJECT): $(PROJECT).o
	$(CC) $^ $(LDLIBS) -o $@
	
$(PROJECT).o: $(PROJECT).C
	$(CC) $(CFLAGS) -o $@ -c $^
	
default: $(PROJECT)

