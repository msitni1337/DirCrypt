SRC=.\src\*.cpp
INC=includes
OBJ_DIR=.\.obj
NAME=dirdec.exe

all: ${NAME}

${NAME}: clean
	-mkdir ${OBJ_DIR}
	cl /Fe"${NAME}" /Fo"${OBJ_DIR}\\" /I"${INC}" ${SRC}
	${NAME}

clean:
	-rmdir ${OBJ_DIR} /S /Q
	-del ${NAME} /Q