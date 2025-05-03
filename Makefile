# Variable definitions
CC = g++
FLAGS = -Wall -Wextra -MMD -MP -std=c++20
DIRS = includes includes/parser core_details includes/analyzer
SRC_DIR = src/
INC_DIRS = ${addprefix -I, ${DIRS}}
FLAGS += ${flags}

OUTPUT_DIR = build/
OUTPUT_DEPS= build/

FILES_TO_COMPILE = ${foreach _D, ${SRC_DIR},${wildcard ${_D}*.cpp}}
OUTPUT_FILES_NAME = ${patsubst %.cpp, ${OUTPUT_DIR}%.o, ${FILES_TO_COMPILE}}
DEPS=${patsubst %.cpp, ${OUTPUT_DEPS}%.d, ${FILES_TO_COMPILE}}

all: directories ${OUTPUT_FILES_NAME}
	${CC} ${FLAGS} ${OUTPUT_FILES_NAME} masm.cpp ${INC_DIRS} -o ${OUTPUT_DIR}masm

WATCH_PROJECT: directories ${OUTPUT_FILES_NAME}

${OUTPUT_DIR}${SRC_DIR}%.o: ${SRC_DIR}%.cpp 
	${CC} ${FLAGS} ${INC_DIRS} -c $< -o $@

${OUTPUT_DIR}%.o: %.cpp
	${CC} ${FLAGS} ${INC_DIRS} -c $< -o $@

# Create necessary directories
directories:
	mkdir -p ${OUTPUT_DIR}
	${foreach f, ${SRC_DIR}, ${shell mkdir -p ${OUTPUT_DIR}${f}}}

clean:
	rm -rf ${OUTPUT_DIR}

.PHONY: all clean directories

-include $(DEPS)
