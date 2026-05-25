add_mlir_library(MLIRConstraintTransforms
  ConservationCheck.cpp

  ADDITIONAL_HEADER_DIRS
  ${PROJECT_SOURCE_DIR}/include/Constraint

  LINK_LIBS PUBLIC
  MLIRConstraintDialect
  MLIRIR
  MLIRPass
  MLIRAnalysis
)
