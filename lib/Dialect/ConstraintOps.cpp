//===- ConstraintOps.cpp - Operation Implementations ----------------------===//
//
// Implements parse, print, build, and verify for all Constraint dialect ops.
//
//===----------------------------------------------------------------------===//

#include "Constraint/ConstraintDialect.h"
#include "Constraint/ConstraintOps.h"
#include "Constraint/ConstraintTypes.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/OpImplementation.h"

using namespace mlir;
using namespace mlir::constraint;

//===----------------------------------------------------------------------===//
// Helpers
//===----------------------------------------------------------------------===//

/// Parse a keyword-attribute pair of the form `name = value`.
/// Returns the parsed attribute, or nullptr on failure.
static Attribute parseKeywordAttr(OpAsmParser &parser, StringRef keyword,
                                  Type attrType, MLIRContext *ctx) {
  return parser.getBuilder().getFloatAttr(attrType, 0.0); // placeholder
}

//===----------------------------------------------------------------------===//
// SequenceOp
//===----------------------------------------------------------------------===//

void SequenceOp::build(OpBuilder &builder, OperationState &state,
                       DenseI32ArrayAttr notes) {
  state.addAttribute("notes", notes);
  state.addTypes(SequenceType::get(builder.getContext()));
}

ParseResult SequenceOp::parse(OpAsmParser &parser, OperationState &result) {
  // Parse: notes = [0, 4, 7] : !constraint.Sequence
  DenseI32ArrayAttr notesAttr;
  if (parser.parseKeyword("notes") || parser.parseEqual() ||
      parser.parseAttribute(notesAttr, "notes", result.attributes))
    return failure();

  if (parser.parseColon())
    return failure();

  Type resultTy;
  if (parser.parseType(resultTy))
    return failure();

  result.addTypes(resultTy);
  return success();
}

void SequenceOp::print(OpAsmPrinter &p) {
  p << " notes = ";
  p.printAttribute(getNotesAttr());
  p << " : ";
  p.printType(getResult().getType());
}

LogicalResult SequenceOp::verify() {
  auto notes = getNotesAttr();
  for (int32_t val : notes.asArrayRef()) {
    if (val < 0 || val > 11) {
      return emitOpError("pitch class must be in [0, 11], got ")
             << val;
    }
  }
  return success();
}

DenseI32ArrayAttr SequenceOp::getNotesAttr() {
  return getOperation()->getAttrOfType<DenseI32ArrayAttr>("notes");
}

void SequenceOp::setNotesAttr(DenseI32ArrayAttr value) {
  getOperation()->setAttr("notes", value);
}

//===----------------------------------------------------------------------===//
// TensionOp
//===----------------------------------------------------------------------===//

void TensionOp::build(OpBuilder &builder, OperationState &state,
                      Value sequence) {
  state.addOperands(sequence);
  state.addTypes(builder.getF64Type());
}

ParseResult TensionOp::parse(OpAsmParser &parser, OperationState &result) {
  OpAsmParser::UnresolvedOperand seqOperand;
  if (parser.parseOperand(seqOperand))
    return failure();

  if (parser.parseColon())
    return failure();

  Type resultTy;
  if (parser.parseType(resultTy))
    return failure();

  Type seqTy = SequenceType::get(parser.getContext());
  if (parser.resolveOperand(seqOperand, seqTy, result.operands))
    return failure();

  result.addTypes(resultTy);
  return success();
}

void TensionOp::print(OpAsmPrinter &p) {
  p << " ";
  p.printOperand(getSequence());
  p << " : ";
  p.printType(getTension().getType());
}

LogicalResult TensionOp::verify() {
  auto seqTy = getSequence().getType().dyn_cast<SequenceType>();
  if (!seqTy)
    return emitOpError("operand must be !constraint.Sequence");
  return success();
}

//===----------------------------------------------------------------------===//
// DialOp
//===----------------------------------------------------------------------===//

void DialOp::build(OpBuilder &builder, OperationState &state,
                   FloatAttr harmonic, FloatAttr rhythmic, FloatAttr spectral) {
  state.addAttribute("harmonic", harmonic);
  state.addAttribute("rhythmic", rhythmic);
  state.addAttribute("spectral", spectral);
  state.addTypes(DialType::get(builder.getContext()));
}

ParseResult DialOp::parse(OpAsmParser &parser, OperationState &result) {
  // Parse: harmonic = 0.8, rhythmic = 0.5, spectral = 0.3 : !constraint.Dial
  FloatAttr harmonicAttr, rhythmicAttr, spectralAttr;

  if (parser.parseKeyword("harmonic") || parser.parseEqual() ||
      parser.parseAttribute(harmonicAttr, "harmonic", result.attributes))
    return failure();

  if (parser.parseComma() || parser.parseKeyword("rhythmic") ||
      parser.parseEqual() ||
      parser.parseAttribute(rhythmicAttr, "rhythmic", result.attributes))
    return failure();

  if (parser.parseComma() || parser.parseKeyword("spectral") ||
      parser.parseEqual() ||
      parser.parseAttribute(spectralAttr, "spectral", result.attributes))
    return failure();

  if (parser.parseColon())
    return failure();

  Type resultTy;
  if (parser.parseType(resultTy))
    return failure();

  result.addTypes(resultTy);
  return success();
}

void DialOp::print(OpAsmPrinter &p) {
  p << " harmonic = ";
  p.printAttribute(getHarmonicAttr());
  p << ", rhythmic = ";
  p.printAttribute(getRhythmicAttr());
  p << ", spectral = ";
  p.printAttribute(getSpectralAttr());
  p << " : ";
  p.printType(getResult().getType());
}

FloatAttr DialOp::getHarmonicAttr() {
  return getOperation()->getAttrOfType<FloatAttr>("harmonic");
}
FloatAttr DialOp::getRhythmicAttr() {
  return getOperation()->getAttrOfType<FloatAttr>("rhythmic");
}
FloatAttr DialOp::getSpectralAttr() {
  return getOperation()->getAttrOfType<FloatAttr>("spectral");
}

LogicalResult DialOp::verify() {
  // Verify each dial coordinate is in [0.0, 1.0].
  auto checkRange = [&](FloatAttr attr, StringRef name) -> LogicalResult {
    if (!attr) return emitOpError() << "missing " << name << " attribute";
    double val = attr.getValueAsDouble();
    if (val < 0.0 || val > 1.0)
      return emitOpError() << name << " must be in [0, 1], got " << val;
    return success();
  };
  if (failed(checkRange(getHarmonicAttr(), "harmonic"))) return failure();
  if (failed(checkRange(getRhythmicAttr(), "rhythmic"))) return failure();
  if (failed(checkRange(getSpectralAttr(), "spectral"))) return failure();
  return success();
}

//===----------------------------------------------------------------------===//
// TraditionOp
//===----------------------------------------------------------------------===//

void TraditionOp::build(OpBuilder &builder, OperationState &state,
                        StringAttr name) {
  state.addAttribute("name", name);
  state.addTypes(DialType::get(builder.getContext()));
}

ParseResult TraditionOp::parse(OpAsmParser &parser, OperationState &result) {
  StringAttr nameAttr;
  if (parser.parseKeyword("name") || parser.parseEqual() ||
      parser.parseAttribute(nameAttr, "name", result.attributes))
    return failure();

  if (parser.parseColon())
    return failure();

  Type resultTy;
  if (parser.parseType(resultTy))
    return failure();

  result.addTypes(resultTy);
  return success();
}

void TraditionOp::print(OpAsmPrinter &p) {
  p << " name = ";
  p.printAttribute(getNameAttr());
  p << " : ";
  p.printType(getResult().getType());
}

StringAttr TraditionOp::getNameAttr() {
  return getOperation()->getAttrOfType<StringAttr>("name");
}

LogicalResult TraditionOp::verify() {
  if (!getNameAttr() || getNameAttr().empty())
    return emitOpError("tradition name must not be empty");
  return success();
}

//===----------------------------------------------------------------------===//
// VoiceLeadOp
//===----------------------------------------------------------------------===//

void VoiceLeadOp::build(OpBuilder &builder, OperationState &state,
                        Value from, Value to) {
  state.addOperands({from, to});
  state.addTypes(builder.getF64Type());
}

ParseResult VoiceLeadOp::parse(OpAsmParser &parser, OperationState &result) {
  OpAsmParser::UnresolvedOperand fromOp, toOp;
  if (parser.parseOperand(fromOp) || parser.parseComma() ||
      parser.parseOperand(toOp))
    return failure();

  if (parser.parseColon())
    return failure();

  Type resultTy;
  if (parser.parseType(resultTy))
    return failure();

  Type seqTy = SequenceType::get(parser.getContext());
  if (parser.resolveOperand(fromOp, seqTy, result.operands) ||
      parser.resolveOperand(toOp, seqTy, result.operands))
    return failure();

  result.addTypes(resultTy);
  return success();
}

void VoiceLeadOp::print(OpAsmPrinter &p) {
  p << " ";
  p.printOperand(getFromSequence());
  p << ", ";
  p.printOperand(getToSequence());
  p << " : ";
  p.printType(getDistance().getType());
}

LogicalResult VoiceLeadOp::verify() {
  auto fromTy = getFromSequence().getType().dyn_cast<SequenceType>();
  auto toTy = getToSequence().getType().dyn_cast<SequenceType>();
  if (!fromTy || !toTy)
    return emitOpError("operands must be !constraint.Sequence");
  return success();
}

//===----------------------------------------------------------------------===//
// ConserveOp
//===----------------------------------------------------------------------===//

void ConserveOp::build(OpBuilder &builder, OperationState &state,
                       Value iVert, Value iHoriz,
                       FloatAttr expectedSum, FloatAttr tolerance) {
  state.addOperands({iVert, iHoriz});
  state.addAttribute("expected_sum", expectedSum);
  state.addAttribute("tolerance", tolerance);
  state.addTypes(builder.getI1Type());
}

ParseResult ConserveOp::parse(OpAsmParser &parser, OperationState &result) {
  OpAsmParser::UnresolvedOperand iVertOp, iHorizOp;
  if (parser.parseOperand(iVertOp) || parser.parseComma() ||
      parser.parseOperand(iHorizOp))
    return failure();

  // Parse attributes in braces: { expected_sum = 4.2, tolerance = 0.15 }
  if (parser.parseLSquare()) {
    // Try brace form instead
  }

  FloatAttr expectedSumAttr, toleranceAttr;
  if (parser.parseKeyword("expected_sum") || parser.parseEqual() ||
      parser.parseAttribute(expectedSumAttr, "expected_sum",
                            result.attributes))
    return failure();

  if (parser.parseComma() || parser.parseKeyword("tolerance") ||
      parser.parseEqual() ||
      parser.parseAttribute(toleranceAttr, "tolerance", result.attributes))
    return failure();

  if (parser.parseColon())
    return failure();

  Type resultTy;
  if (parser.parseType(resultTy))
    return failure();

  Type f64 = parser.getBuilder().getF64Type();
  if (parser.resolveOperand(iVertOp, f64, result.operands) ||
      parser.resolveOperand(iHorizOp, f64, result.operands))
    return failure();

  result.addTypes(resultTy);
  return success();
}

void ConserveOp::print(OpAsmPrinter &p) {
  p << " ";
  p.printOperand(getIVert());
  p << ", ";
  p.printOperand(getIHoriz());
  p << " expected_sum = ";
  p.printAttribute(getExpectedSumAttr());
  p << ", tolerance = ";
  p.printAttribute(getToleranceAttr());
  p << " : ";
  p.printType(getPass().getType());
}

FloatAttr ConserveOp::getExpectedSumAttr() {
  return getOperation()->getAttrOfType<FloatAttr>("expected_sum");
}
FloatAttr ConserveOp::getToleranceAttr() {
  return getOperation()->getAttrOfType<FloatAttr>("tolerance");
}

LogicalResult ConserveOp::verify() {
  if (!getExpectedSumAttr())
    return emitOpError("missing expected_sum attribute");
  if (!getToleranceAttr())
    return emitOpError("missing tolerance attribute");
  if (getToleranceAttr().getValueAsDouble() < 0.0)
    return emitOpError("tolerance must be non-negative");
  return success();
}

//===----------------------------------------------------------------------===//
// Generated op registration (would be populated by TableGen)
//===----------------------------------------------------------------------===//

// In a TableGen build, the following include pulls in the auto-generated
// registration code. For the manual build, registration is handled directly
// in ConstraintDialect.cpp.
// #include "Constraint/ConstraintOps.cpp.inc"
