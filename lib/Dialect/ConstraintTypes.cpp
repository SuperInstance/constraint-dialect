//===- ConstraintTypes.cpp - Type Implementations -------------------------===//
//
// Implements the custom types for the Constraint dialect.
//
//===----------------------------------------------------------------------===//

#include "Constraint/ConstraintTypes.h"
#include "Constraint/ConstraintDialect.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"

using namespace mlir;
using namespace mlir::constraint;

//===----------------------------------------------------------------------===//
// SequenceType
//===----------------------------------------------------------------------===//

Type SequenceType::parse(AsmParser &parser) {
  // Parse the keyword "Sequence" — the mnemonic was already consumed by
  // the dialect parser, so there's nothing more to parse for this
  // parameterless type.
  return SequenceType::get(parser.getContext());
}

void SequenceType::print(AsmPrinter &printer) const {
  // The mnemonic is printed by the dialect's printType.
}

//===----------------------------------------------------------------------===//
// DialType
//===----------------------------------------------------------------------===//

Type DialType::parse(AsmParser &parser) {
  return DialType::get(parser.getContext());
}

void DialType::print(AsmPrinter &printer) const {}

//===----------------------------------------------------------------------===//
// VoiceLeadingType
//===----------------------------------------------------------------------===//

Type VoiceLeadingType::parse(AsmParser &parser) {
  return VoiceLeadingType::get(parser.getContext());
}

void VoiceLeadingType::print(AsmPrinter &printer) const {}
