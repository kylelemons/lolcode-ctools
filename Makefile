CC=gcc
CPP=g++

CFLAGS=-Wall -fPIC -ggdb
CPPFLAGS=${CFLAGS}

CCOMPILE=${CC} ${CFLAGS} -c
CPPCOMPILE=${CPP} ${CPPFLAGS} -c

LINK=${CPP} ${LFLAGS} -o
LEX=flex
BISON=bison -v -d -k

LEX_PREFIX=lex.yy
BIS_PREFIX=grammar.tab

LEX_SOURCE_OUT=${LEX_PREFIX}.c
LEX_SOURCE_OBJ=${LEX_PREFIX}.o
BIS_SOURCE_OUT=${BIS_PREFIX}.c
BIS_HEADER_OUT=${BIS_PREFIX}.h
BIS_SOURCE_OBJ=${BIS_PREFIX}.o

MY_OBJ=ast.o lcc.o lolcode.o

all : lcc

lcc : ${LEX_SOURCE_OBJ} ${BIS_SOURCE_OBJ} ${MY_OBJ}
	${LINK} $@ ${LEX_SOURCE_OBJ} ${BIS_SOURCE_OBJ} ${MY_OBJ}

${LEX_SOURCE_OBJ} : ${LEX_SOURCE_OUT}

${LEX_SOURCE_OUT} : lexer.l ${BIS_HEADER_OUT}
	${LEX} -o ${LEX_SOURCE_OUT} $<

${BIS_SOURCE_OBJ} : ${BIS_SOURCE_OUT}

${BIS_HEADER_OUT} : ${BIS_SOURCE_OUT}

${BIS_SOURCE_OUT} : grammar.y
	${BISON} -o ${BIS_SOURCE_OUT} --defines=${BIS_HEADER_OUT} $<

clean :
	@echo "  CLEAN"
	rm .o lcc
	rm *.yy.* *.tab.* grammar.output 
	rm -rf help/

doc : doxygen.conf
	doxygen doxygen.conf

%.o : %.c %.h
	@echo "  CC [h]  $@"
	${CCOMPILE} $<

%.o : %.c
	@echo "  CC      $@"
	${CCOMPILE} $<

%.o : %.cpp %.hpp
	@echo "  CPP [h] $@"
	${CPPCOMPILE} $<

%.o : %.cpp
	@echo "  CPP     $@"
	${CPPCOMPILE} $<

% : %.o
	@echo "  LINK    $@"
	${LINK} $@ $^

