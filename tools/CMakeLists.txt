add_mlir_tool(constraint-opt
  constraint-opt.cpp

  LINK_LIBS PRIVATE
  MLIRConstraintDialect
  MLIRConstraintToAffine
  MLIRConstraintToLLVM
  MLIRConstraintTransforms
  MLIRParser
  MLIRPass
  MLIRIR
  MLIRSupport
)
