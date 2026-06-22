# Makefile for NMake
# Usage: nmake ARCH=x64 CONFIG=Release

# Default to x64 if not specified
!IF "$(ARCH)" == ""
ARCH = x64
!ENDIF

# Default to Release if not specified
!IF "$(CONFIG)" == ""
CONFIG = Release
!ENDIF

SRC_DIR = ..\src
OBJ_DIR = obj\$(ARCH)\$(CONFIG)
OUT_DIR = bin\$(ARCH)\$(CONFIG)

CC = cl
RC = rc
LINK = link

# Compiler flags: Added /Zi for Debug symbols and /MTd for Debug runtime
!IF "$(CONFIG)" == "Debug"
CFLAGS = /nologo /c /Zi /MTd /Od /EHsc /std:c++17 /D_DEBUG /DUNICODE /D_UNICODE /D_CRT_SECURE_NO_WARNINGS /I$(SRC_DIR)
!ELSE
CFLAGS = /nologo /c /O2 /MT /EHsc /std:c++17 /DNDEBUG /DUNICODE /D_UNICODE /D_CRT_SECURE_NO_WARNINGS /I$(SRC_DIR)
!ENDIF

RCFLAGS = /nologo
LFLAGS = /nologo /SUBSYSTEM:WINDOWS user32.lib gdi32.lib shell32.lib ole32.lib comctl32.lib windowscodecs.lib advapi32.lib uxtheme.lib dwmapi.lib

OBJS = $(OBJ_DIR)\stacky.obj \
       $(OBJ_DIR)\logger.obj \
       $(OBJ_DIR)\util.obj \
       $(OBJ_DIR)\config.obj \
       $(OBJ_DIR)\bmp.obj \
       $(OBJ_DIR)\cache_manager.obj \
       $(OBJ_DIR)\app.obj

RESOURCES = $(OBJ_DIR)\stacky.res

all: $(OUT_DIR)\stacky.exe

$(OUT_DIR)\stacky.exe: $(OBJS) $(RESOURCES)
	@if not exist $(OUT_DIR) mkdir $(OUT_DIR)
	$(LINK) $(LFLAGS) /OUT:$@ $(OBJS) $(RESOURCES)

{$(SRC_DIR)}.cpp{$(OBJ_DIR)}.obj:
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	$(CC) $(CFLAGS) /Fo$@ $<

$(RESOURCES): $(SRC_DIR)\stacky.rc
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	$(RC) $(RCFLAGS) /fo$@ $(SRC_DIR)\stacky.rc

clean:
	@if exist obj rmdir /s /q obj
	@if exist bin rmdir /s /q bin