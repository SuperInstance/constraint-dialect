// RUN: constraint-opt %s --convert-constraint-to-affine | FileCheck %s
//
// Test lowering from Constraint dialect to Affine + Arith + MemRef.

module @lowering_test {
  // CHECK-LABEL: @lowering_test
  // Test sequence lowering → memref.alloc + affine.store
  // CHECK: memref.alloc
  // CHECK: affine.store
  %seq = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence

  // Test tension lowering → arithmetic
  // CHECK: arith.constant
  %t = constraint.tension %seq : f64

  // Test voice lead lowering
  %seq2 = constraint.sequence notes = [0, 5, 7] : !constraint.Sequence
  // CHECK: arith.constant
  %vl = constraint.voice_lead %seq, %seq2 : f64

  // Test dial lowering → memref<3xf64>
  // CHECK: memref.alloc
  // CHECK: affine.store
  %d = constraint.dial harmonic = 0.8, rhythmic = 0.5, spectral = 0.3
    : !constraint.Dial

  // Test tradition lowering → memref<3xf64>
  // CHECK: memref.alloc
  %jazz = constraint.tradition name = "jazz" : !constraint.Dial

  // Test conserve lowering → arith.addf + arith.subf + arith.cmpf
  // CHECK: arith.addf
  // CHECK: arith.subf
  // CHECK: arith.cmpf
  %ok = constraint.conserve %t, %vl expected_sum = 4.2, tolerance = 0.15 : i1
}
