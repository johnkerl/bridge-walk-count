# ================================================================
# Makefile for project count_walks
# Automatically generated from "count_walks.mki" at Fri Feb  5 22:31:58 2010

# yamm v1.0
# John Kerl
# 2002/05/04
# ================================================================


INCLUDE_DIRS =
LIB_DIRS = -L.
DEFINES =
MISC_CFLAGS =
MISC_LFLAGS = -lm
EXTRA_DEPS =
COMPILE_FLAGS = -c $(INCLUDE_DIRS) $(DEFINES) $(MISC_CFLAGS)
LINK_FLAGS =  $(LIB_DIRS) $(MISC_LFLAGS)

build: mk_obj_dir ./count_walks

mk_obj_dir:
	mkdir -p ./objs

./objs/count_walks.o:  count_walks.c walk_count_lib.h
	gcc $(OPTCFLAGS) -Wall -Werror $(COMPILE_FLAGS)  count_walks.c -o ./objs/count_walks.o

./objs/walk_count_lib.o:  walk_count_lib.c walk_count_lib.h
	gcc $(OPTCFLAGS) -Wall -Werror $(COMPILE_FLAGS)  walk_count_lib.c -o ./objs/walk_count_lib.o

OBJS = \
	./objs/count_walks.o \
	./objs/walk_count_lib.o

./count_walks: $(OBJS) $(EXTRA_DEPS)
	gcc $(OPTLFLAGS) $(OBJS) -o ./count_walks $(LINK_FLAGS)

clean:
	-@rm -f $(OBJS)
	-@rm -f ./count_walks
