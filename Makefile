#
# Roach Makefile
#
# Author:	Austin Archer
# Email:	aarcher73k@gmail.com
# Git URL:	https://github.com/GoodiesHQ/Roach
#

TARGET		= roach
SRCDIR		= src
INCDIR		= inc
OBJDIR		= obj
BINDIR		= bin

CC			= gcc
LD			= gcc
SRCFILES	:= $(wildcard $(SRCDIR)/*.c)
OBJFILES	:= $(SRCFILES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
CFLAGS		:= -std=c11 -Wall -I$(INCDIR)
CFLAGS		:= -Wall -I$(INCDIR)
RM			= rm -f

all:		$(OBJFILES)

debug:		CFLAGS += -DDEBUG -ggdb
debug:		$(OBJFILES)

$(BINDIR)/$(TARGET): $(OBJFILES)
	@$(LD) $(OBJFILES) $(LFLAGS) -o $@

$(OBJFILES): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY:		clean
clean:
	@$(RM) $(OBJFILES)
	@$(RM) $(BINDIR)/$(TARGET)
