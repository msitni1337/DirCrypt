SRC=.\src\*.cpp
INC=includes
OBJ_DIR=.\.obj
NAME=DirCrypt-Encryptor.exe

all: ${NAME}

${NAME}: clean
	-mkdir ${OBJ_DIR}
	rc includes/resource.rc
	cl /Fe"${NAME}" /Fo"${OBJ_DIR}\\" /I"${INC}" /O2 /Ob2 /Os /MT  includes/resource.res ${SRC}
	${NAME}

clean:
	-rmdir ${OBJ_DIR} /S /Q
	-del ${NAME} /Q