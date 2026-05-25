//===- ConstraintDialect.cpp - Dialect Registration -----------------------===//
//
// Registers the Constraint dialect with the MLIR context. The dialect
// provides the "constraint" namespace and registers all operations and
// types defined in the dialect.
//
//===----------------------------------------------------------------------===//

#include "Constraint/ConstraintDialect.h"
#include "Constraint/ConstraintOps.h"
#include "Constraint/ConstraintTypes.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/DialectImplementation.h"

using namespace mlir;
using namespace mlir::constraint;

//===----------------------------------------------------------------------===//
// Dialect Construction
//===----------------------------------------------------------------------===//

ConstraintDialect::ConstraintDialect(MLIRContext *ctx)
    : Dialect(getDialectNamespace(), ctx, TypeID::get<ConstraintDialect>()) {
  // Register all operations defined in the dialect.
  addOperations<
#define GET_OP_LIST
#include "Constraint/ConstraintOps.cpp.inc"
      >();

  // Register custom types.
  addTypes<SequenceType, DialType, VoiceLeadingType>();
}

//===----------------------------------------------------------------------===//
// Type Parsing
//===----------------------------------------------------------------------===//
//
// Parse types of the form:
//   !constraint.Sequence
//   !constraint.Dial
//   !constraint.VoiceLeading
//
Type ConstraintDialect::parseType(DialectAsmParser &parser) const {
  StringRef keyword;
  if (parser.parseKeyword(&keyword))
    return Type();

  if (keyword == SequenceType::getMnemonic())
    return SequenceType::get(getContext());
  if (keyword == DialType::getMnemonic())
    return DialType::get(getContext());
  if (keyword == VoiceLeadingType::getMnemonic())
    return VoiceLeadingType::get(getContext());

  parser.emitError(parser.getNameLoc(), "unknown constraint type: ")
      << keyword;
  return Type();
}

//===----------------------------------------------------------------------===//
// Type Printing
//===----------------------------------------------------------------------===//
void ConstraintDialect::printType(Type type, DialectAsmPrinter &printer) const {
  if (auto seqTy = type.dyn_cast<SequenceType>()) {
    printer << seqTy.getMnemonic();
    return;
  }
  if (auto dialTy = type.dyn_cast<DialType>()) {
    printer << dialTy.getMnemonic();
    return;
  }
  if (auto vlTy = type.dyn_cast<VoiceLeadingType>()) {
    printer << vlTy.getMnemonic();
    return;
  }
  llvm_unreachable("unhandled Constraint type");
}

//===----------------------------------------------------------------------===//
// Dialect Hook — register the dialect so mlir::registerDialect<ConstraintDialect>
// or ctx->getOrLoadDialect<ConstraintDialect>() works.
//===----------------------------------------------------------------------===//

// Static registration entry point used by the MLIR context.
// The dialect is automatically loaded on first use.
static DialectRegistration<ConstraintDialect> constraintDialectReg;
