//===- constraint-opt.cpp - Constraint Dialect Optimization Driver --------===//
//
// Standalone optimization driver for the Constraint dialect, analogous to
// mlir-opt. It parses a .mlir file, applies the requested passes, and
// prints the result.
//
// Usage:
//   constraint-opt input.mlir                           (parse + print)
//   constraint-opt input.mlir --convert-constraint-to-affine
//   constraint-opt input.mlir --check-conservation
//   constraint-opt input.mlir --convert-constraint-to-llvm
//
//===----------------------------------------------------------------------===//

#include "Constraint/ConstraintDialect.h"
#include "Constraint/ConstraintOps.h"
#include "Constraint/ConstraintTypes.h"

#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

#include "mlir/IR/AsmState.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Verifier.h"
#include "mlir/Parser/Parser.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Support/FileUtilities.h"
#include "mlir/Support/Timing.h"
#include "mlir/Support/ToolUtilities.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;
using namespace mlir::constraint;

//===----------------------------------------------------------------------===//
// Command-line options
//===----------------------------------------------------------------------===//

static llvm::cl::opt<std::string> inputFilename(
    llvm::cl::Positional, llvm::cl::desc("<input file>"),
    llvm::cl::init("-"));

static llvm::cl::opt<std::string> outputFilename(
    "o", llvm::cl::desc("Output filename"),
    llvm::cl::value_desc("filename"), llvm::cl::init("-"));

static llvm::cl::opt<bool> splitInputFile(
    "split-input-file",
    llvm::cl::desc("Split the input file into pieces and process each chunk"),
    llvm::cl::init(false));

static llvm::cl::opt<bool> verifyDiagnostics(
    "verify-diagnostics",
    llvm::cl::desc("Check that emitted diagnostics match expected-* lines"),
    llvm::cl::init(false));

static llvm::cl::opt<bool> verifyPasses(
    "verify-each",
    llvm::cl::desc("Run the verifier after each transformation pass"),
    llvm::cl::init(true));

static llvm::cl::opt<bool> allowUnregisteredDialects(
    "allow-unregistered-dialect",
    llvm::cl::desc("Allow operations from unregistered dialects"),
    llvm::cl::init(false));

//===----------------------------------------------------------------------===//
// Pass registration — forward declarations
//===----------------------------------------------------------------------===//

// These are defined in their respective .cpp files and linked in.
namespace mlir {
namespace constraint {
std::unique_ptr<Pass> createConstraintToAffinePass();
std::unique_ptr<Pass> createConstraintToLLVMPass();
std::unique_ptr<Pass> createConservationCheckPass();
} // namespace constraint
} // namespace mlir

//===----------------------------------------------------------------------===//
// Main
//===----------------------------------------------------------------------===//

int main(int argc, char **argv) {
  llvm::InitLLVM y(argc, argv);

  // Register all passes available via --pass-name flags.
  registerPass([]() -> std::unique_ptr<Pass> {
    return createConstraintToAffinePass();
  });
  registerPass([]() -> std::unique_ptr<Pass> {
    return createConstraintToLLVMPass();
  });
  registerPass([]() -> std::unique_ptr<Pass> {
    return createConservationCheckPass();
  });

  // Build the pass pipeline from command-line arguments.
  // This allows --pass-name style pass registration.
  PassPipelineCLParser passPipeline("", "Constraint dialect passes");

  // Parse command-line options.
  llvm::cl::ParseCommandLineOptions(argc, argv,
                                     "Constraint dialect optimizer\n");

  // Set up the MLIR context with all required dialects.
  DialectRegistry registry;
  registry.insert<ConstraintDialect>();
  registry.insert<affine::AffineDialect>();
  registry.insert<arith::ArithDialect>();
  registry.insert<LLVM::LLVMDialect>();
  registry.insert<memref::MemRefDialect>();
  registry.insert<math::MathDialect>();
  registry.insert<func::FuncDialect>();

  MLIRContext context(registry);
  context.allowUnregisteredDialects(allowUnregisteredDialects);

  // Parse the input file.
  auto file = openInputFile(inputFilename, &context);
  if (!file) {
    llvm::errs() << "Error: could not open input file: " << inputFilename
                 << "\n";
    return 1;
  }

  auto output = openOutputFile(outputFilename);
  if (!output) {
    llvm::errs() << "Error: could not open output file: " << outputFilename
                 << "\n";
    return 1;
  }

  // Process the input (possibly splitting into chunks).
  auto processBuffer = [&](std::unique_ptr<llvm::MemoryBuffer> buffer,
                           raw_ostream &os) {
    llvm::SourceMgr sourceMgr;
    sourceMgr.AddNewSourceBuffer(std::move(buffer), llvm::SMLoc());

    // Parse the MLIR module.
    auto module = parseSourceFile<ModuleOp>(sourceMgr, &context);
    if (!module) {
      llvm::errs() << "Error: could not parse input\n";
      return failure();
    }

    // Build the pass manager.
    PassManager pm(&context, module.getOperationName());
    pm.enableVerifier(verifyPasses);

    // Apply the pass pipeline specified on the command line.
    if (failed(passPipeline.addToPipeline(pm))) {
      llvm::errs() << "Error: could not add passes to pipeline\n";
      return failure();
    }

    // Run the passes.
    if (failed(pm.run(*module))) {
      llvm::errs() << "Error: pass pipeline failed\n";
      return failure();
    }

    // Print the result.
    module->print(os);
    os << "\n";
    return success();
  };

  if (splitInputFile) {
    if (failed(splitAndProcessBuffer(std::move(file), processBuffer,
                                      output->os())))
      return 1;
  } else {
    if (failed(processBuffer(std::move(file->getBuffer()), output->os())))
      return 1;
  }

  output->keep();
  return 0;
}
