add_mlir_conversion_library(MLIRConstraintToAffine
  ConstraintToAffine.cpp

  ADDITIONAL_HEADER_DIRS
  ${PROJECT_SOURCE_DIR}/include/Constraint

  LINK_LIBS PUBLIC
  MLIRConstraintDialect
  MLIRAffineDialect
  MLIRArithDialect
  MLIRMemRefDialect
  MLIRTransforms
  MLIRIR
  MLIRPass
)

add_mlir_conversion_library(MLIRConstraintToLLVM
  ConstraintToLLVM.cpp

  ADDITIONAL_HEADER_DIRS
  ${PROJECT_SOURCE_DIR}/include/Constraint

  LINK_LIBS PUBLIC
  MLIRConstraintDialect
  MLIRLLVMDialect
  MLIRArithDialect
  MLIRMemRefDialect
  MLIRTransforms
  MLIRIR
  MLIRPass
)
