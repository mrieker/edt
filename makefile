STATIC ?=

OBJS =	\
	chrutils.o		\
	ch_screen_vt100.o	\
	cmd_change.o		\
	cmd_copy.o		\
	cmd_delete.o		\
	cmd_exit.o		\
	cmd_find.o		\
	cmd_help.o		\
	cmd_include.o		\
	cmd_insert.o		\
	cmd_move.o		\
	cmd_open.o		\
	cmd_quit.o		\
	cmd_resequence.o	\
	cmd_set.o		\
	cmd_show.o		\
	cmd_substitute.o	\
	cmd_type.o		\
	cmd_write.o		\
	crash.o			\
	edt.o			\
	journaling.o		\
	keypad.o		\
	line.o			\
	ln_command.o		\
	md5.o			\
	os.linux.o		\
	output.o		\
	range.o			\
	read_file.o		\
	relposition.o		\
	representation.o	\
	string.o		\
	write_file.o

CC = cc -g -O2 -c

ifeq ($(STATIC),)
	LIBS := -lreadline
else
	LIBS := -static -lreadline -ltinfo
endif

edt:	$(OBJS)
	cc -o edt $(OBJS) $(LIBS)
	chmod 755 edt

chrutils.o: chrutils.c
	$(CC) chrutils.c

ch_screen_vt100.o: ch_screen_vt100.c
	$(CC) ch_screen_vt100.c

cmd_change.o: cmd_change.c
	$(CC) cmd_change.c

cmd_copy.o: cmd_copy.c
	$(CC) cmd_copy.c

cmd_delete.o: cmd_delete.c
	$(CC) cmd_delete.c

cmd_exit.o: cmd_exit.c
	$(CC) cmd_exit.c

cmd_find.o: cmd_find.c
	$(CC) cmd_find.c

cmd_help.o: cmd_help.c
	$(CC) cmd_help.c

cmd_include.o: cmd_include.c
	$(CC) cmd_include.c

cmd_insert.o: cmd_insert.c
	$(CC) cmd_insert.c

cmd_move.o: cmd_move.c
	$(CC) cmd_move.c

cmd_open.o: cmd_open.c
	$(CC) cmd_open.c

cmd_quit.o: cmd_quit.c
	$(CC) cmd_quit.c

cmd_resequence.o: cmd_resequence.c
	$(CC) cmd_resequence.c

cmd_set.o: cmd_set.c
	$(CC) cmd_set.c

cmd_show.o: cmd_show.c
	$(CC) cmd_show.c

cmd_substitute.o: cmd_substitute.c
	$(CC) cmd_substitute.c

cmd_type.o: cmd_type.c
	$(CC) cmd_type.c

cmd_write.o: cmd_write.c
	$(CC) cmd_write.c

crash.o: crash.c
	$(CC) crash.c

edt.o: edt.c
	$(CC) edt.c

journaling.o: journaling.c
	$(CC) journaling.c

keypad.o: keypad.c
	$(CC) keypad.c

line.o: line.c
	$(CC) line.c

ln_command.o: ln_command.c cmdtbl.h
	$(CC) ln_command.c

md5.o:	md5.c md5.h
	$(CC) md5.c

os.linux.o: os.c
	$(CC) -DUSE_LIBREADLINE -o os.linux.o os.c

output.o: output.c
	$(CC) output.c

range.o: range.c
	$(CC) range.c

read_file.o: read_file.c
	$(CC) read_file.c

relposition.o: relposition.c
	$(CC) relposition.c

representation.o: representation.c
	$(CC) representation.c

string.o: string.c
	$(CC) string.c

write_file.o: write_file.c
	$(CC) write_file.c

