cmake_minimum_required(VERSION 3.10)
project(M9DSE)

#set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "-DSEVERE_DEBUG")
include_directories(.)
include_directories(spea2/src)

add_executable(M9DSE
        spea2/lib/libspea2.a
        spea2/src/containers.cpp
        spea2/src/containers.h
        spea2/src/Makefile
        spea2/src/selector.cpp
        spea2/src/selector.h
        spea2/src/utility.cpp
        spea2/src/utility.h
        spea2/src/variator.cpp
        spea2/src/variator.h
        alg_genetic.cpp
        alg_random.cpp
        alg_sensivity.cpp
        common.cpp
        common.h
        environment.h
        estimator.cpp
        estimator.h
        explorer.cpp
        explorer.h
        FunctionApprox.h
        FuzzyApprox.cpp
        FuzzyApprox.h
        FuzzyWrapper.cpp
        hash.h
        main.cpp
        Makefile
        MPI.README
        parameter.cpp
        parameter.h
        model_inverter.cpp
        model_inverter.h
        RuleList.cpp
        RuleList.h
        simulate_space.cpp
        matlab_interface.cpp
        matlab_interface.h
        user_interface.cpp
        user_interface.h
        version.h)
