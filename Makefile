INCLUDE=./include
SRC=./src
OBJDIR=./obj
DISTDIR=./dist/cinot

CC=gcc
PYTHON_INCLUDES=$(shell python-config --includes)
CFLAGS=-std=c11 -I$(INCLUDE) $(PYTHON_INCLUDES)
# CXXFLAGS=$(CFLAGS)

ifdef DEBUG
	X=$(shell echo toto)
	CFLAGS:=$(CFLAGS) -g -DDEBUG
endif

DEPS=$(wildcard $(INCLUDE)/*.h)
OBJS=$(addprefix $(OBJDIR)/, $(patsubst %.c,%.o,$(notdir $(wildcard $(SRC)/*.c))))
PY=$(wildcard $(SRC)/*.py)


.PHONY: clean

all: cinot $(PY)
	@for py in $(PY); do \
		echo [CP] $(DISTDIR)$$(basename $${py}); \
		cp $${py} $(DISTDIR); \
	done

$(OBJDIR)/%.o: $(SRC)/%.c $(DEPS)
	@[ -d $(OBJDIR) ] || mkdir $(OBJDIR)
	@echo [CC] $@
	@$(CC) $(CFLAGS) -fPIC -o $@ -c $<

cinot: $(OBJS)
	@[ -d $(DISTDIR) ] || mkdir -p $(DISTDIR)
	@echo [LD] $(DISTDIR)/_$@.so
	@$(CC) $(CFLAGS) -shared -o $(DISTDIR)/_$@.so $^
	
clean:
	@rm -rf $(dir $(DISTDIR))
	@rm -rf $(OBJDIR)
