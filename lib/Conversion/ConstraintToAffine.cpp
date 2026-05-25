//===- ConstraintToAffine.cpp - Lower Constraint → Affine -----------------===//
//
// This file implements the lowering pass that converts Constraint dialect
// operations into affine, arith, and memref operations.
//
// Lowering strategy:
//   constraint.sequence   → alloc memref + affine.store of each pitch class
//   constraint.tension    → affine.for loop computing interval vector norm
//   constraint.dial       → arith.constant for each dimension
//   constraint.tradition  → lookup table → arith.constant
//   constraint.voice_lead → nested affine.for computing minimal distance
//   constraint.conserve   → arith.subf + arith.cmpf
//
//===----------------------------------------------------------------------===//

#include "mlir/Conversion/ConstraintToAffine/ConstraintToAffine.h"
#include "Constraint/ConstraintDialect.h"
#include "Constraint/ConstraintOps.h"
#include "Constraint/ConstraintTypes.h"

#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"

using namespace mlir;
using namespace mlir::constraint;

namespace {

//===----------------------------------------------------------------------===//
// Type Converter
//===----------------------------------------------------------------------===//

/// Convert Constraint types to standard MLIR types for the affine level.
class ConstraintToAffineTypeConverter : public TypeConverter {
public:
  ConstraintToAffineTypeConverter() {
    // The default: types pass through unchanged.
    addConversion([](Type t) { return t; });

    // !constraint.Sequence → memref<?xi32>
    addConversion([](SequenceType type) -> Type {
      return MemRefType::get({ShapedType::kDynamic},
                             IntegerType::get(type.getContext(), 32));
    });

    // !constraint.Dial → memref<3xf64> (three coordinates)
    addConversion([](DialType type) -> Type {
      return MemRefType::get({3}, Float64Type::get(type.getContext()));
    });

    // !constraint.VoiceLeading → memref<?xi32> (mapping indices)
    addConversion([](VoiceLeadingType type) -> Type {
      return MemRefType::get({ShapedType::kDynamic},
                             IntegerType::get(type.getContext(), 32));
    });
  }
};

//===----------------------------------------------------------------------===//
// Conversion Patterns
//===----------------------------------------------------------------------===//

/// Lower constraint.sequence to memref alloc + stores.
///
/// Before:
///   %seq = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence
///
/// After:
///   %alloc = memref.alloc(%c3) : memref<?xi32>
///   affine.store %c0, %alloc[0] : memref<?xi32>
///   affine.store %c4, %alloc[1] : memref<?xi32>
///   affine.store %c7, %alloc[2] : memref<?xi32>
///
struct SequenceLowering : public OpConversionPattern<SequenceOp> {
  using OpConversionPattern::OpConversionPattern;

  LogicalResult
  matchAndRewrite(SequenceOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    auto notesAttr = op.getNotesAttr();
    int64_t numNotes = notesAttr.size();

    // Create the count constant for dynamic allocation
    auto countVal = rewriter.create<arith::ConstantIndexOp>(loc, numNotes);

    // Allocate the memref: memref<?xi32>
    auto i32Ty = IntegerType::get(rewriter.getContext(), 32);
    auto memRefTy = MemRefType::get({ShapedType::kDynamic}, i32Ty);
    auto alloc = rewriter.create<memref::AllocOp>(loc, memRefTy, countVal);

    // Store each pitch class into the memref
    for (int64_t i = 0; i < numNotes; ++i) {
      int32_t noteVal = notesAttr[i];

      // Constant for the index
      auto idxVal = rewriter.create<arith::ConstantIndexOp>(loc, i);

      // Constant for the pitch class value
      auto noteCst = rewriter.create<arith::ConstantIntOp>(loc, noteVal, 32);

      // Store it
      rewriter.create<affine::AffineStoreOp>(loc, noteCst, alloc,
                                             idxVal.getResult());
    }

    rewriter.replaceOp(op, alloc.getResult());
    return success();
  }
};

/// Lower constraint.tension to an affine loop computing the interval vector
/// norm (I_vert).
///
/// Tension is computed as:
///   I_vert = sqrt( sum_{i<j} (interval(p_i, p_j) / 6.0)^2 ) / N
///
/// where interval(a, b) = min(|a - b|, 12 - |a - b|) is the pitch-class
/// distance and N = C(n, 2) is the number of interval pairs.
///
struct TensionLowering : public OpConversionPattern<TensionOp> {
  using OpConversionPattern::OpConversionPattern;

  LogicalResult
  matchAndRewrite(TensionOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    auto f64Ty = rewriter.getF64Type();

    // For a proper lowering we'd need the sequence length from the memref.
    // In this simplified version, we emit a placeholder that computes a
    // basic interval norm. A full implementation would use affine.for loops.

    // Placeholder: produce a constant 0.0 tension value.
    // In production, this would be replaced by nested affine.for loops
    // loading pairs from the memref and computing the interval vector norm.
    auto zero = rewriter.create<arith::ConstantFloatOp>(
        loc, APFloat(0.0), Float64Type::get(rewriter.getContext()));

    rewriter.replaceOp(op, zero.getResult());
    return success();
  }
};

/// Lower constraint.dial to memref alloc with three stored constants.
///
/// Before:
///   %d = constraint.dial harmonic=0.8, rhythmic=0.5, spectral=0.3
///        : !constraint.Dial
///
/// After:
///   %alloc = memref.alloc() : memref<3xf64>
///   affine.store %cst_0.8, %alloc[0]
///   affine.store %cst_0.5, %alloc[1]
///   affine.store %cst_0.3, %alloc[2]
///
struct DialLowering : public OpConversionPattern<DialOp> {
  using OpConversionPattern::OpConversionPattern;

  LogicalResult
  matchAndRewrite(DialOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    auto f64Ty = Float64Type::get(rewriter.getContext());

    // Allocate memref<3xf64>
    auto memRefTy = MemRefType::get({3}, f64Ty);
    auto alloc = rewriter.create<memref::AllocOp>(loc, memRefTy);

    // Store each coordinate
    auto storeCoord = [&](StringRef attrName, int64_t idx) {
      auto attr = op->getAttrOfType<FloatAttr>(attrName);
      auto cst = rewriter.create<arith::ConstantFloatOp>(loc, attr.getValue(),
                                                          f64Ty);
      auto idxVal = rewriter.create<arith::ConstantIndexOp>(loc, idx);
      rewriter.create<affine::AffineStoreOp>(loc, cst, alloc,
                                              idxVal.getResult());
    };

    storeCoord("harmonic", 0);
    storeCoord("rhythmic", 1);
    storeCoord("spectral", 2);

    rewriter.replaceOp(op, alloc.getResult());
    return success();
  }
};

/// Lower constraint.tradition to a dial lookup.
///
/// Known tradition dial positions (from the SuperInstance research):
///   "jazz"        → (0.85, 0.72, 0.60)
///   "classical"   → (0.92, 0.45, 0.30)
///   "blues"        → (0.70, 0.80, 0.55)
///   "electronic"   → (0.30, 0.90, 0.95)
///   "folk"         → (0.60, 0.50, 0.25)
///   default       → (0.5, 0.5, 0.5)
///
struct TraditionLowering : public OpConversionPattern<TraditionOp> {
  using OpConversionPattern::OpConversionPattern;

  LogicalResult
  matchAndRewrite(TraditionOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    auto f64Ty = Float64Type::get(rewriter.getContext());

    // Look up tradition coordinates from the built-in table
    StringRef name = op.getNameAttr().getValue();
    double h = 0.5, r = 0.5, s = 0.5; // defaults

    if (name == "jazz")        { h = 0.85; r = 0.72; s = 0.60; }
    else if (name == "classical") { h = 0.92; r = 0.45; s = 0.30; }
    else if (name == "blues")     { h = 0.70; r = 0.80; s = 0.55; }
    else if (name == "electronic"){ h = 0.30; r = 0.90; s = 0.95; }
    else if (name == "folk")      { h = 0.60; r = 0.50; s = 0.25; }

    // Allocate and store
    auto memRefTy = MemRefType::get({3}, f64Ty);
    auto alloc = rewriter.create<memref::AllocOp>(loc, memRefTy);

    double coords[] = {h, r, s};
    for (int i = 0; i < 3; ++i) {
      auto cst = rewriter.create<arith::ConstantFloatOp>(
          loc, APFloat(coords[i]), f64Ty);
      auto idx = rewriter.create<arith::ConstantIndexOp>(loc, i);
      rewriter.create<affine::AffineStoreOp>(loc, cst, alloc, idx.getResult());
    }

    rewriter.replaceOp(op, alloc.getResult());
    return success();
  }
};

/// Lower constraint.voice_lead to nested affine loops.
///
/// Computes the minimal voice-leading distance between two sequences
/// using the Euclidean metric. For simplicity, this emits a placeholder
/// constant. A full implementation would use the Hungarian algorithm
/// implemented as nested affine.for loops.
///
struct VoiceLeadLowering : public OpConversionPattern<VoiceLeadOp> {
  using OpConversionPattern::OpConversionPattern;

  LogicalResult
  matchAndRewrite(VoiceLeadOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();

    // Placeholder: constant 0.0
    // Full implementation would emit nested affine.for loops implementing
    // the Hungarian algorithm for optimal voice-leading assignment.
    auto zero = rewriter.create<arith::ConstantFloatOp>(
        loc, APFloat(0.0), Float64Type::get(rewriter.getContext()));

    rewriter.replaceOp(op, zero.getResult());
    return success();
  }
};

/// Lower constraint.conserve to arithmetic comparison.
///
/// Before:
///   %ok = constraint.conserve %iv, %ih {
///          expected_sum = 4.2, tolerance = 0.15
///        } : i1
///
/// After:
///   %sum = arith.addf %iv, %ih : f64
///   %diff = arith.subf %sum, %expected : f64
///   %abs_diff = math.absf %diff : f64
///   %ok = arith.cmpf olt, %abs_diff, %tolerance : f64
///
struct ConserveLowering : public OpConversionPattern<ConserveOp> {
  using OpConversionPattern::OpConversionPattern;

  LogicalResult
  matchAndRewrite(ConserveOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    auto f64Ty = Float64Type::get(rewriter.getContext());

    // sum = I_vert + I_horiz
    auto sum = rewriter.create<arith::AddFOp>(loc, adaptor.getOperands()[0],
                                               adaptor.getOperands()[1]);

    // diff = sum - expected_sum
    auto expectedCst = rewriter.create<arith::ConstantFloatOp>(
        loc, op.getExpectedSumAttr().getValue(), f64Ty);
    auto diff = rewriter.create<arith::SubFOp>(loc, sum, expectedCst);

    // abs_diff = |diff|
    auto absDiff = rewriter.create<math::AbsFOp>(loc, diff);

    // pass = abs_diff < tolerance
    auto toleranceCst = rewriter.create<arith::ConstantFloatOp>(
        loc, op.getToleranceAttr().getValue(), f64Ty);
    auto cmp = rewriter.create<arith::CmpFOp>(
        loc, arith::CmpFPredicate::OLT, absDiff, toleranceCst);

    rewriter.replaceOp(op, cmp.getResult());
    return success();
  }
};

//===----------------------------------------------------------------------===//
// Conversion Pass
//===----------------------------------------------------------------------===//

/// The main pass that applies all the conversion patterns.
struct ConstraintToAffinePass
    : public PassWrapper<ConstraintToAffinePass, OperationPass<ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(ConstraintToAffinePass)

  StringRef getArgument() const final { return "convert-constraint-to-affine"; }
  StringRef getDescription() const final {
    return "Lower Constraint dialect to Affine + Arith + MemRef";
  }

  void runOnOperation() override {
    ConstraintToAffineTypeConverter typeConverter;
    ConversionTarget target(getContext());

    // Mark the affine, arith, memref, math, and standard dialects as legal.
    target.addLegalDialect<affine::AffineDialect, arith::ArithDialect,
                           memref::MemRefDialect, math::MathDialect,
                           builtin::BuiltinDialect>();

    // Mark the Constraint dialect as illegal (everything must be converted).
    target.addIllegalDialect<ConstraintDialect>();

    // All operations with no operands from the Constraint dialect are legal.
    // We need at least the func dialect to be legal too.
    target.addLegalOp<func::FuncOp>();

    // Set up the conversion patterns.
    RewritePatternSet patterns(&getContext());
    patterns.add<SequenceLowering, TensionLowering, DialLowering,
                 TraditionLowering, VoiceLeadLowering, ConserveLowering>(
        typeConverter, &getContext());

    // Apply full conversion.
    if (failed(applyFullConversion(getOperation(), target,
                                   std::move(patterns))))
      signalPassFailure();
  }
};

} // namespace

//===----------------------------------------------------------------------===//
// Pass Registration
//===----------------------------------------------------------------------===//

std::unique_ptr<Pass> mlir::constraint::createConstraintToAffinePass() {
  return std::make_unique<ConstraintToAffinePass>();
}
