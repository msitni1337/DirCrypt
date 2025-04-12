SRC=.\src\*.cpp
INC=includes
OBJ_DIR=.\.obj
NAME=DirCrypt-Decryptor.exe

all: ${NAME}

${NAME}: clean
	-mkdir ${OBJ_DIR}
	rc includes/resource.rc
	cl /Fe"${NAME}" /Fo"${OBJ_DIR}\\" /I"${INC}" /O2 /Ob2 /Os /MT ${SRC} includes/resource.res
	mt -manifest .\includes\DirCrypt.manifest -outputresource:${NAME}
	
clean:
	-rmdir ${OBJ_DIR} /S /Q
	-del ${NAME} /Q