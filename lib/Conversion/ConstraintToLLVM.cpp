//===- ConstraintToLLVM.cpp - Lower Constraint → LLVM IR ------------------===//
//
// This file implements the conversion pass from the Constraint dialect
// (already lowered to affine/arith/memref) to LLVM IR. In practice this
// pass composes with the upstream affine-to-standard and standard-to-llvm
// passes, adding only the Constraint-specific type lowering.
//
//===----------------------------------------------------------------------===//

#include "mlir/Conversion/ConstraintToLLVM/ConstraintToLLVM.h"
#include "Constraint/ConstraintDialect.h"
#include "Constraint/ConstraintTypes.h"

#include "mlir/Conversion/AffineToStandard/AffineToStandard.h"
#include "mlir/Conversion/ArithToLLVM/ArithToLLVM.h"
#include "mlir/Conversion/ControlFlowToLLVM/ControlFlowToLLVM.h"
#include "mlir/Conversion/FuncToLLVM/ConvertFuncToLLVMPass.h"
#include "mlir/Conversion/MemRefToLLVM/MemRefToLLVM.h"
#include "mlir/Conversion/SCFToControlFlow/SCFToControlFlow.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Builders.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Transforms/Passes.h"

using namespace mlir;
using namespace mlir::constraint;

namespace {

/// The Constraint-to-LLVM pass composes multiple upstream conversion passes
/// to produce LLVM IR. The Constraint dialect ops should already have been
/// lowered to affine/arith/memref by the ConstraintToAffine pass.
struct ConstraintToLLVMPass
    : public PassWrapper<ConstraintToLLVMPass, OperationPass<ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(ConstraintToLLVMPass)

  StringRef getArgument() const final { return "convert-constraint-to-llvm"; }
  StringRef getDescription() const final {
    return "Lower Constraint dialect (via affine) to LLVM IR";
  }

  void runOnOperation() override {
    auto module = getOperation();

    // Build a pass pipeline that lowers through the standard stages:
    //   affine → scf → cf → llvm
    PassManager pm(&getContext(), module.getOperationName());

    // Lower affine constructs to scf (structured control flow)
    pm.addPass(createConvertAffineToStandardPass());

    // Lower scf to cf (branch-based control flow)
    pm.addPass(createConvertSCFToControlFlowPass());

    // Final lowering to LLVM IR
    pm.addPass(createConvertControlFlowToLLVMPass());
    pm.addPass(createConvertArithToLLVMPass());
    pm.addPass(createConvertFuncToLLVMPass());
    pm.addPass(createFinalizeMemRefToLLVMConversionPass());

    if (failed(pm.run(module)))
      signalPassFailure();
  }
};

} // namespace

std::unique_ptr<Pass> mlir::constraint::createConstraintToLLVMPass() {
  return std::make_unique<ConstraintToLLVMPass>();
}
