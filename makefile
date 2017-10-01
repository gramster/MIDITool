CFLAGS=-ml -Ic:\include\dflat -v -H

#LIBOBJ1=button.obj console.obj decomp.obj dialbox.obj dialogs.obj
#LIBOBJ2=fileopen.obj helpbox.obj htree.obj huffc.obj keys.obj listbox.obj lists.obj menu.obj
#LIBOBJ3=menubar.obj message.obj mouse.obj msgbox.obj normal.obj popdown.obj
#LIBOBJ4=rect.obj textbox.obj video.obj window.obj

LIBOBJ1=button.obj config.obj console.obj decomp.obj dialbox.obj dialogs.obj editbox.obj
LIBOBJ2=fileopen.obj helpbox.obj htree.obj keys.obj listbox.obj lists.obj menu.obj msgbox.obj
LIBOBJ3=menubar.obj message.obj mouse.obj msgbox.obj normal.obj popdown.obj
LIBOBJ4=rect.obj statbar.obj sysmenu.obj textbox.obj video.obj window.obj 

INC=c:\include\dflat

all: miditool.exe

MYOBJ = miditool.obj myapp.obj record.obj my401.obj mydbox.obj mem.obj

miditool.exe:  $(MYOBJ) dflatl.lib
	tcc $(CFLAGS) $(MYOBJ) dflatl.lib

miditool.obj: miditool.c
	tcc $(CFLAGS) -c miditool.c

myapp.obj: myapp.c
	tcc $(CFLAGS) -c myapp.c
	
record.obj: record.c
	tcc $(CFLAGS) -c record.c
	
my401.obj: my401.c
	tcc $(CFLAGS) -c my401.c

mydbox.obj: mydbox.c
	tcc $(CFLAGS) -c mydbox.c

mem.obj: mem.c
	tcc $(CFLAGS) -c mem.c
	
dflatl.lib: $(LIBOBJ1) $(LIBOBJ2) $(LIBOBJ3) $(LIBOBJ4)

button.obj: button.c $(INC)\dflat.h
	tcc $(CFLAGS) -c button.c
	tlib /C /E dflatl +-button.obj

config.obj: config.c $(INC)\dflat.h
	tcc $(CFLAGS) -c config.c
	tlib /C /E dflatl +-config.obj

console.obj: console.c $(INC)\dflat.h
	tcc $(CFLAGS) -c console.c
	tlib /C /E dflatl +-console.obj

decomp.obj: decomp.c $(INC)\dflat.h
	tcc $(CFLAGS) -c decomp.c
	tlib /C /E dflatl +-decomp.obj

dialbox.obj: dialbox.c $(INC)\dflat.h
	tcc $(CFLAGS) -c dialbox.c
	tlib /C /E dflatl +-dialbox.obj

dialogs.obj: dialogs.c $(INC)\dflat.h
	tcc $(CFLAGS) -c dialogs.c
	tlib /C /E dflatl +-dialogs.obj

editbox.obj: editbox.c $(INC)\dflat.h
	tcc $(CFLAGS) -c editbox.c
	tlib /C /E dflatl +-editbox.obj

fileopen.obj: fileopen.c $(INC)\dflat.h
	tcc $(CFLAGS) -c fileopen.c
	tlib /C /E dflatl +-fileopen.obj

helpbox.obj: helpbox.c $(INC)\dflat.h
	tcc $(CFLAGS) -c helpbox.c
	tlib /C /E dflatl +-helpbox.obj

htree.obj: htree.c $(INC)\dflat.h
	tcc $(CFLAGS) -c htree.c
	tlib /C /E dflatl +-htree.obj

huffc.obj: huffc.c $(INC)\dflat.h
	tcc $(CFLAGS) -c huffc.c
	tlib /C /E dflatl +-huffc.obj

keys.obj: keys.c $(INC)\dflat.h
	tcc $(CFLAGS) -c keys.c
	tlib /C /E dflatl +-keys.obj

listbox.obj: listbox.c $(INC)\dflat.h
	tcc $(CFLAGS) -c listbox.c
	tlib /C /E dflatl +-listbox.obj

lists.obj: lists.c $(INC)\dflat.h
	tcc $(CFLAGS) -c lists.c
	tlib /C /E dflatl +-lists.obj

menu.obj: menu.c $(INC)\dflat.h
	tcc $(CFLAGS) -c menu.c
	tlib /C /E dflatl +-menu.obj

menubar.obj: menubar.c $(INC)\dflat.h
	tcc $(CFLAGS) -c menubar.c
	tlib /C /E dflatl +-menubar.obj

message.obj: message.c $(INC)\dflat.h
	tcc $(CFLAGS) -c message.c
	tlib /C /E dflatl +-message.obj

mouse.obj: mouse.c $(INC)\dflat.h
	tcc $(CFLAGS) -c mouse.c
	tlib /C /E dflatl +-mouse.obj

msgbox.obj: msgbox.c $(INC)\dflat.h
	tcc $(CFLAGS) -c msgbox.c
	tlib /C /E dflatl +-msgbox.obj

normal.obj: normal.c $(INC)\dflat.h
	tcc $(CFLAGS) -c normal.c
	tlib /C /E dflatl +-normal.obj

popdown.obj: popdown.c $(INC)\dflat.h
	tcc $(CFLAGS) -c popdown.c
	tlib /C /E dflatl +-popdown.obj

rect.obj: rect.c $(INC)\dflat.h
	tcc $(CFLAGS) -c rect.c
	tlib /C /E dflatl +-rect.obj

statbar.obj: statbar.c $(INC)\dflat.h
	tcc $(CFLAGS) -c statbar.c
	tlib /C /E dflatl +-statbar.obj

sysmenu.obj: sysmenu.c $(INC)\dflat.h
	tcc $(CFLAGS) -c sysmenu.c
	tlib /C /E dflatl +-sysmenu.obj

textbox.obj: textbox.c $(INC)\dflat.h
	tcc $(CFLAGS) -c textbox.c
	tlib /C /E dflatl +-textbox.obj

video.obj: video.c $(INC)\dflat.h
	tcc $(CFLAGS) -c video.c
	tlib /C /E dflatl +-video.obj

window.obj: window.c $(INC)\dflat.h
	tcc $(CFLAGS) -c window.c
	tlib /C /E dflatl +-window.obj


