#Makefile for M9

GAINC_DIR = ./spea2/src
GALIB_DIR = ./spea2/lib
GASRC_DIR = ./spea2/src

CC = gcc
CXX = g++

# Misc stuff for mpi environment support
# if you want, adapt it and export M9_MPI=1 before compiling
ifdef M9_MPI
MPICC = mpicxx
CFLAGS += -DM9_MPI -DMPICH_IGNORE_CXX_SEEK
#CFLAGS += -DM9_MPI -DMPICH_IGNORE_CXX_SEEK -DNDEBUG -Minform=severe Ktrap=fp
else
MPICC = ${CXX}
CFLAGS += -DNDEBUG 
#CFLAGS += -DTEST -DNDEBUG 
#CFLAGS += -O2 -DNDEBUG
CFLAGS += -g 
endif

#<aa> added by andrea.araldo@gmail.com
# If it is defined, a lot of redundant and overabundant checks will be performed to 
# check for inconsistent states or data. This can be useful when you edit the code to be
# sure that the modifications do not produce those inconcistencies</aa>
CFLAGS += -DSEVERE_DEBUG
#</aa>



all: M9

M9: ${GALIB_DIR}/libspea2.a explorer.o alg_dep.o alg_random.o alg_sensivity.o alg_genetic.o \
	estimator.o avg_err_ID.o time.o model_inverter.o \
	main.o user_interface.o matlabInterface.o \
	parameter.o common.o simulate_space.o \
	FuzzyApprox.o RuleList.o FuzzyWrapper.o alg_paramspace.o
	${MPICC} explorer.o simulate_space.o alg_dep.o alg_random.o alg_sensivity.o alg_genetic.o \
	user_interface.o matlabInterface.o estimator.o avg_err_ID.o time.o \
	model_inverter.o mem_hierarchy.o main.o parameter.o common.o \
	FuzzyApprox.o RuleList.o FuzzyWrapper.o alg_paramspace.o \
	-L${GALIB_DIR} -lspea2 -o M9

estimator.o: estimator.cpp estimator.h model_inverter.h mem_hierarchy.h \
	power_densities.h cacti_ID_interface.h
	${CXX} ${CFLAGS} -c estimator.cpp

explorer.o: explorer.cpp explorer.h model_inverter.h matlabInterface.h \
	estimator.h parameter.h common.h \
	FunctionApprox.h FuzzyApprox.h FannApprox.h
	${MPICC} -I${GAINC_DIR} ${CFLAGS} -c explorer.cpp

simulate_space.o: simulate_space.cpp explorer.h 
	${MPICC} -I${GAINC_DIR} ${CFLAGS} -c simulate_space.cpp

alg_dep.o: alg_dep.cpp explorer.h common.h
	${MPICC} -I${GAINC_DIR} ${CFLAGS} -c alg_dep.cpp

alg_sensivity.o: alg_sensivity.cpp explorer.h common.h
	${MPICC} -I${GAINC_DIR} ${CFLAGS} -c alg_sensivity.cpp

alg_random.o: alg_random.cpp explorer.h common.h
	${MPICC} -I${GAINC_DIR} ${CFLAGS} -c alg_random.cpp

alg_genetic.o: alg_genetic.cpp explorer.h common.h
	${MPICC} -I${GAINC_DIR} ${CFLAGS} -c alg_genetic.cpp

alg_paramspace.o: alg_paramspace.cpp paramspace.h
	 ${MPICC} -I${GAINC_DIR} ${CFLAGS} -c alg_paramspace.cpp

model_inverter.o: model_inverter.cpp model_inverter.h parameter.h
	${CXX} ${CFLAGS} -c model_inverter.cpp

mem_hierarchy.o: mem_hierarchy.cpp mem_hierarchy.h parameter.h
	${CXX} ${CFLAGS} -c mem_hierarchy.cpp

main.o: main.cpp user_interface.h
	${MPICC} -I${GAINC_DIR} ${CFLAGS} -c main.cpp

user_interface.o: user_interface.cpp user_interface.h \
	explorer.h estimator.h matlabInterface.h model_inverter.h \
	mem_hierarchy.h environment.h version.h
	${MPICC} -I${GAINC_DIR} ${CFLAGS} -c user_interface.cpp

matlabInterface.o: matlabInterface.cpp matlabInterface.h model_inverter.h
	${MPICC} ${CFLAGS} -c matlabInterface.cpp

avg_err_ID.o: cacti.h avg_err_ID.c
	${CC} ${CFLAGS} -c avg_err_ID.c

time.o: cacti.h time.c 
	${CC} ${CFLAGS} -c time.c

parameter.o: parameter.cpp parameter.h 
	${CXX} ${CFLAGS} -c parameter.cpp


common.o: common.cpp common.h 
	${MPICC} ${CFLAGS} -c common.cpp

FuzzyApprox.o: FuzzyApprox.cpp FuzzyApprox.h common.h RuleList.h FunctionApprox.h
	${CXX} ${CFLAGS} -c FuzzyApprox.cpp
#	${CXX} ${CFLAGS}  -funroll-loops -ffast-math -c FuzzyApprox.cpp

RuleList.o: RuleList.cpp RuleList.h common.h 
	${CXX} ${CFLAGS} -c RuleList.cpp
#	${CXX} ${CFLAGS} -funroll-loops -ffast-math -c RuleList.cpp

FuzzyWrapper.o: FuzzyWrapper.cpp FuzzyApprox.h
	${CXX} ${CFLAGS} -c FuzzyWrapper.cpp

.PHONY: spea2 clean cleanall

spea2: ${GALIB_DIR}/libspea2.a

${GALIB_DIR}/libspea2.a: ${GASRC_DIR}/*.cpp ${GASRC_DIR}/*.h
	${MAKE} -C ${GASRC_DIR}

clean: 
	rm -f *.o M9DSE *~ core

cleanall: clean
	${MAKE} -C ${GASRC_DIR} clean

# DO NOT DELETE
