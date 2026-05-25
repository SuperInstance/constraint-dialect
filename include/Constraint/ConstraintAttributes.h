//===- ConstraintAttributes.h - Constraint Attributes ----------*- C++ -*-===//
//
// Declares custom attributes for the Constraint dialect.
//
//===----------------------------------------------------------------------===//

#ifndef CONSTRAINT_CONSTRAINT_ATTRIBUTES_H
#define CONSTRAINT_CONSTRAINT_ATTRIBUTES_H

#include "mlir/IR/Attributes.h"
#include "mlir/IR/AttributeSupport.h"

namespace mlir {
namespace constraint {

// The Constraint dialect primarily uses standard MLIR attributes:
//   - DenseI32ArrayAttr  for pitch-class arrays (SequenceOp::notes)
//   - FloatAttr          for tension values and dial coordinates
//   - StringAttr         for tradition names
//
// Custom attributes can be added here as the dialect evolves. For example,
// a TraditionDefAttr could encode an entire tradition definition inline.

} // namespace constraint
} // namespace mlir

#endif // CONSTRAINT_CONSTRAINT_ATTRIBUTES_H
