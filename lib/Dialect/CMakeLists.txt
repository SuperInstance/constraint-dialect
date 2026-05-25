add_mlir_dialect_library(MLIRConstraintDialect
  ConstraintDialect.cpp
  ConstraintOps.cpp
  ConstraintTypes.cpp

  ADDITIONAL_HEADER_DIRS
  ${PROJECT_SOURCE_DIR}/include/Constraint

  DEPENDS
  MLIRConstraintOpsIncGen

  LINK_LIBS PUBLIC
  MLIRAnalysis
  MLIRIR
  MLIRParser
  MLIRPass
  MLIRSupport
  MLIRMemRefDialect
)
