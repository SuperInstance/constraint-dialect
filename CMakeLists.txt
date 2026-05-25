cmake_minimum_required(VERSION 3.20)
project(constraint-dialect LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find MLIR — this pulls in all the MLIR CMake infrastructure
find_package(MLIR REQUIRED CONFIG)

message(STATUS "Using MLIRConfig: ${MLIR_CONFIG}")
message(STATUS "MLIR include dirs: ${MLIR_INCLUDE_DIRS}")

include_directories(${MLIR_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_BINARY_DIR}/include)

# Helper: collect all MLIR libraries we depend on
set(MLIR_LIBS
  MLIRAnalysis
  MLIRIR
  MLIRParser
  MLIRPass
  MLIRTransforms
  MLIRAffineDialect
  MLIRAffineToStandard
  MLIRArithDialect
  MLIRMemRefDialect
  MLIRConversionPassIncGen
  MLIRLLVMDialect
  MLIRSupport
  MLIRTransformUtils
)

add_subdirectory(lib)
add_subdirectory(tools)

# ── Testing ────────────────────────────────────────────────
# Use FileCheck-based lit tests when lit is available
find_program(LIT_EXE lit)
if(LIT_EXE)
  add_custom_target(check-constraint
    COMMAND ${LIT_EXE} -v ${CMAKE_SOURCE_DIR}/test
    DEPENDS constraint-opt
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
else()
  add_custom_target(check-constraint
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS constraint-opt
  )
endif()
