//===- ConstraintOps.h - Constraint Operation Declarations ------*- C++ -*-===//
//
// Declares all operations in the Constraint dialect.
//
// In a full TableGen-driven project, these would be auto-generated from .td
// files. Here we declare them manually to keep the build simple while still
// following MLIR's Op<> patterns.
//
//===----------------------------------------------------------------------===//

#ifndef CONSTRAINT_CONSTRAINT_OPS_H
#define CONSTRAINT_CONSTRAINT_OPS_H

#include "mlir/IR/Builders.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

namespace mlir {
namespace constraint {

//===----------------------------------------------------------------------===//
// SequenceOp — constraint.sequence
//===----------------------------------------------------------------------===//
//
// Represents a musical sequence of pitch classes (integers 0–11).
//
//   %seq = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence
//
// Attributes:
//   notes: DenseI32ArrayAttr — pitch class values
//
// Results:
//   result: !constraint.Sequence
//
//===----------------------------------------------------------------------===//
class SequenceOp : public mlir::Op<SequenceOp,
                    mlir::OpTrait::ZeroOperands,
                    mlir::OpTrait::OneResult,
                    mlir::OpTrait::HasParent<mlir::RegionOp>::template Impl,
                    mlir::OpTrait::IsIsolatedFromAbove,
                    mlir::MemoryEffectOpInterface::Trait> {
public:
  using Op::Op;

  static llvm::StringRef getOperationName() {
    return "constraint.sequence";
  }

  // Accessors
  mlir::DenseI32ArrayAttr getNotesAttr();
  void setNotesAttr(mlir::DenseI32ArrayAttr value);
  mlir::Value getResult() { return getOperation()->getResult(0); }

  // Attribute name getters (used by the generic assembly format)
  static llvm::StringRef getNotesAttrName() { return "notes"; }

  // Build helpers
  static void build(mlir::OpBuilder &builder, mlir::OperationState &state,
                    mlir::DenseI32ArrayAttr notes);

  // Parse/print
  static mlir::ParseResult parse(mlir::OpAsmParser &parser,
                                  mlir::OperationState &result);
  void print(mlir::OpAsmPrinter &p);

  // Verify
  mlir::LogicalResult verify();
};

//===----------------------------------------------------------------------===//
// TensionOp — constraint.tension
//===----------------------------------------------------------------------===//
//
// Computes the harmonic tension (I_vert) of a sequence using the interval
// vector norm. I_vert = sqrt(sum of squared interval ratios).
//
//   %t = constraint.tension %seq : f64
//
// Operands:
//   sequence: !constraint.Sequence
//
// Results:
//   tension: f64
//
//===----------------------------------------------------------------------===//
class TensionOp : public mlir::Op<TensionOp,
                   mlir::OpTrait::OneOperand,
                   mlir::OpTrait::OneResult,
                   mlir::OpTrait::SameOperandsAndResultType,
                   mlir::MemoryEffectOpInterface::Trait> {
public:
  using Op::Op;

  static llvm::StringRef getOperationName() {
    return "constraint.tension";
  }

  mlir::Value getSequence() { return getOperation()->getOperand(0); }
  mlir::Value getTension() { return getOperation()->getResult(0); }

  static void build(mlir::OpBuilder &builder, mlir::OperationState &state,
                    mlir::Value sequence);

  static mlir::ParseResult parse(mlir::OpAsmParser &parser,
                                  mlir::OperationState &result);
  void print(mlir::OpAsmPrinter &p);

  mlir::LogicalResult verify();
};

//===----------------------------------------------------------------------===//
// DialOp — constraint.dial
//===----------------------------------------------------------------------===//
//
// Represents a 3D position in tradition space (harmonic, rhythmic, spectral).
//
//   %d = constraint.dial harmonic=0.8, rhythmic=0.5, spectral=0.3
//        : !constraint.Dial
//
// Attributes:
//   harmonic: f64
//   rhythmic:  f64
//   spectral:  f64
//
// Results:
//   result: !constraint.Dial
//
//===----------------------------------------------------------------------===//
class DialOp : public mlir::Op<DialOp,
                mlir::OpTrait::ZeroOperands,
                mlir::OpTrait::OneResult,
                mlir::MemoryEffectOpInterface::Trait> {
public:
  using Op::Op;

  static llvm::StringRef getOperationName() {
    return "constraint.dial";
  }

  mlir::FloatAttr getHarmonicAttr();
  mlir::FloatAttr getRhythmicAttr();
  mlir::FloatAttr getSpectralAttr();
  mlir::Value getResult() { return getOperation()->getResult(0); }

  static void build(mlir::OpBuilder &builder, mlir::OperationState &state,
                    mlir::FloatAttr harmonic, mlir::FloatAttr rhythmic,
                    mlir::FloatAttr spectral);

  static mlir::ParseResult parse(mlir::OpAsmParser &parser,
                                  mlir::OperationState &result);
  void print(mlir::OpAsmPrinter &p);

  mlir::LogicalResult verify();
};

//===----------------------------------------------------------------------===//
// TraditionOp — constraint.tradition
//===----------------------------------------------------------------------===//
//
// Looks up a named tradition's dial position from the tradition registry.
// Tradition names map to (harmonic, rhythmic, spectral) triplets.
//
//   %jazz = constraint.tradition name = "jazz" : !constraint.Dial
//
// Attributes:
//   name: StringAttr
//
// Results:
//   result: !constraint.Dial
//
//===----------------------------------------------------------------------===//
class TraditionOp : public mlir::Op<TraditionOp,
                     mlir::OpTrait::ZeroOperands,
                     mlir::OpTrait::OneResult,
                     mlir::MemoryEffectOpInterface::Trait> {
public:
  using Op::Op;

  static llvm::StringRef getOperationName() {
    return "constraint.tradition";
  }

  mlir::StringAttr getNameAttr();
  mlir::Value getResult() { return getOperation()->getResult(0); }

  static void build(mlir::OpBuilder &builder, mlir::OperationState &state,
                    mlir::StringAttr name);

  static mlir::ParseResult parse(mlir::OpAsmParser &parser,
                                  mlir::OperationState &result);
  void print(mlir::OpAsmPrinter &p);

  mlir::LogicalResult verify();
};

//===----------------------------------------------------------------------===//
// VoiceLeadOp — constraint.voice_lead
//===----------------------------------------------------------------------===//
//
// Computes the minimal voice-leading distance between two sequences using
// the Euclidean voice-leading metric.
//
//   %d = constraint.voice_lead %seq1, %seq2 : f64
//
// Operands:
//   from: !constraint.Sequence
//   to:   !constraint.Sequence
//
// Results:
//   distance: f64
//
//===----------------------------------------------------------------------===//
class VoiceLeadOp : public mlir::Op<VoiceLeadOp,
                     mlir::OpTrait::NOperands<2>::Impl,
                     mlir::OpTrait::OneResult,
                     mlir::MemoryEffectOpInterface::Trait> {
public:
  using Op::Op;

  static llvm::StringRef getOperationName() {
    return "constraint.voice_lead";
  }

  mlir::Value getFromSequence() { return getOperation()->getOperand(0); }
  mlir::Value getToSequence() { return getOperation()->getOperand(1); }
  mlir::Value getDistance() { return getOperation()->getResult(0); }

  static void build(mlir::OpBuilder &builder, mlir::OperationState &state,
                    mlir::Value from, mlir::Value to);

  static mlir::ParseResult parse(mlir::OpAsmParser &parser,
                                  mlir::OperationState &result);
  void print(mlir::OpAsmPrinter &p);

  mlir::LogicalResult verify();
};

//===----------------------------------------------------------------------===//
// ConserveOp — constraint.conserve
//===----------------------------------------------------------------------===//
//
// Applies the conservation constraint: |I_vert + I_horiz - expected_sum|
// must be within tolerance. This encodes the super-theoretic conservation
// law where harmonic tension and voice leading are complementary.
//
//   %ok = constraint.conserve %I_vert, %I_horiz {
//          expected_sum = 4.2, tolerance = 0.15
//        } : i1
//
// Operands:
//   I_vert:  f64 (harmonic tension)
//   I_horiz: f64 (voice-leading distance)
//
// Attributes:
//   expected_sum: f64
//   tolerance:    f64
//
// Results:
//   pass: i1 (true if conservation holds)
//
//===----------------------------------------------------------------------===//
class ConserveOp : public mlir::Op<ConserveOp,
                   mlir::OpTrait::NOperands<2>::Impl,
                   mlir::OpTrait::OneResult,
                   mlir::MemoryEffectOpInterface::Trait> {
public:
  using Op::Op;

  static llvm::StringRef getOperationName() {
    return "constraint.conserve";
  }

  mlir::Value getIVert() { return getOperation()->getOperand(0); }
  mlir::Value getIHoriz() { return getOperation()->getOperand(1); }
  mlir::FloatAttr getExpectedSumAttr();
  mlir::FloatAttr getToleranceAttr();
  mlir::Value getPass() { return getOperation()->getResult(0); }

  static void build(mlir::OpBuilder &builder, mlir::OperationState &state,
                    mlir::Value iVert, mlir::Value iHoriz,
                    mlir::FloatAttr expectedSum, mlir::FloatAttr tolerance);

  static mlir::ParseResult parse(mlir::OpAsmParser &parser,
                                  mlir::OperationState &result);
  void print(mlir::OpAsmPrinter &p);

  mlir::LogicalResult verify();
};

} // namespace constraint
} // namespace mlir

#endif // CONSTRAINT_CONSTRAINT_OPS_H
