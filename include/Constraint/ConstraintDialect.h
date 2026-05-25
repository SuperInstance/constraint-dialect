//===- ConstraintDialect.h - Constraint Dialect Declaration -----*- C++ -*-===//
//
// Declares the Constraint MLIR dialect for the SuperInstance ecosystem.
// This dialect models musical constraints (harmonic tension, voice leading,
// tradition-space dials) as first-class MLIR operations.
//
//===----------------------------------------------------------------------===//

#ifndef CONSTRAINT_CONSTRAINT_DIALECT_H
#define CONSTRAINT_CONSTRAINT_DIALECT_H

#include "mlir/IR/Dialect.h"
#include "mlir/IR/MLIRContext.h"

// Forward declarations for generated files
namespace mlir {
namespace constraint {
class ConstraintDialect;
} // namespace constraint
} // namespace mlir

//===----------------------------------------------------------------------===//
// ConstraintDialect
//===----------------------------------------------------------------------===//
//
// The ConstraintDialect is registered with the MLIRContext and provides
// the namespace "constraint" for all operations and types in this dialect.
//
// Operations:
//   constraint.sequence    — musical pitch-class sequence
//   constraint.tension     — harmonic tension (I_vert)
//   constraint.dial        — 3D tradition-space dial
//   constraint.tradition   — named tradition lookup
//   constraint.voice_lead  — voice-leading distance
//   constraint.conserve    — conservation check
//
// Types:
//   !constraint.Sequence      — pitch-class sequence
//   !constraint.Dial          — tradition-space position
//   !constraint.VoiceLeading  — voice-leading path
//
//===----------------------------------------------------------------------===//

namespace mlir {
namespace constraint {

class ConstraintDialect : public mlir::Dialect {
public:
  explicit ConstraintDialect(mlir::MLIRContext *ctx);

  /// Provide a utility to get the dialect namespace string.
  static constexpr llvm::StringLiteral getDialectNamespace() {
    return {"constraint"};
  }

  /// Parse a type registered to this dialect.
  mlir::Type parseType(mlir::DialectAsmParser &parser) const override;

  /// Print a type registered to this dialect.
  void printType(mlir::Type type,
                 mlir::DialectAsmPrinter &printer) const override;
};

} // namespace constraint
} // namespace mlir

#endif // CONSTRAINT_CONSTRAINT_DIALECT_H
