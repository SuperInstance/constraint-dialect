// RUN: constraint-opt %s --convert-constraint-to-affine | FileCheck %s
//
// Lowering edge-case tests: verify lowering works for corner cases.

// CHECK-LABEL: @lowering_single_note
module @lowering_single_note {
  // Single-note sequence should lower to a single-element memref.
  // CHECK: memref.alloc
  // CHECK: affine.store
  %seq = constraint.sequence notes = [7] : !constraint.Sequence
  // CHECK: arith.constant
  %t = constraint.tension %seq : f64
}

// CHECK-LABEL: @lowering_many_notes
module @lowering_many_notes {
  // Large sequence should lower with multiple stores.
  // CHECK: memref.alloc
  // CHECK: affine.store
  // CHECK: affine.store
  // CHECK: affine.store
  // CHECK: affine.store
  // CHECK: affine.store
  // CHECK: affine.store
  %seq = constraint.sequence notes = [0, 2, 4, 5, 7, 9] : !constraint.Sequence
  // CHECK: arith.constant
  %t = constraint.tension %seq : f64
}

// CHECK-LABEL: @lowering_dial_corners
module @lowering_dial_corners {
  // Boundary dial values should lower correctly.
  // CHECK: memref.alloc
  // CHECK: affine.store
  %d0 = constraint.dial harmonic = 0.0, rhythmic = 0.0, spectral = 0.0 : !constraint.Dial
  // CHECK: memref.alloc
  // CHECK: affine.store
  %d1 = constraint.dial harmonic = 1.0, rhythmic = 1.0, spectral = 1.0 : !constraint.Dial
}

// CHECK-LABEL: @lowering_tradition_all
module @lowering_tradition_all {
  // All known traditions should lower to memref with constants.
  // CHECK: memref.alloc
  %jazz = constraint.tradition name = "jazz" : !constraint.Dial
  // CHECK: memref.alloc
  %classical = constraint.tradition name = "classical" : !constraint.Dial
  // CHECK: memref.alloc
  %blues = constraint.tradition name = "blues" : !constraint.Dial
  // CHECK: memref.alloc
  %electronic = constraint.tradition name = "electronic" : !constraint.Dial
  // CHECK: memref.alloc
  %folk = constraint.tradition name = "folk" : !constraint.Dial
}

// CHECK-LABEL: @lowering_tradition_unknown
module @lowering_tradition_unknown {
  // Unknown tradition should default to (0.5, 0.5, 0.5).
  // CHECK: memref.alloc
  %unknown = constraint.tradition name = "reggae" : !constraint.Dial
}

// CHECK-LABEL: @lowering_voice_lead_same
module @lowering_voice_lead_same {
  // Voice leading from a sequence to itself.
  %seq = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence
  // CHECK: arith.constant
  %vl = constraint.voice_lead %seq, %seq : f64
}

// CHECK-LABEL: @lowering_conserve_boundary
module @lowering_conserve_boundary {
  %seq = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence
  %t = constraint.tension %seq : f64
  %seq2 = constraint.sequence notes = [0, 5, 7] : !constraint.Sequence
  %vl = constraint.voice_lead %seq, %seq2 : f64

  // Zero tolerance should lower to a strict comparison.
  // CHECK: arith.addf
  // CHECK: arith.subf
  // CHECK: arith.cmpf
  %ok = constraint.conserve %t, %vl expected_sum = 0.0, tolerance = 0.0 : i1
}

// CHECK-LABEL: @lowering_multiple_ops
module @lowering_multiple_ops {
  // Multiple operations of each type should all lower correctly.
  // CHECK: memref.alloc
  %s1 = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence
  // CHECK: memref.alloc
  %s2 = constraint.sequence notes = [2, 5, 9] : !constraint.Sequence
  // CHECK: arith.constant
  %t1 = constraint.tension %s1 : f64
  // CHECK: arith.constant
  %t2 = constraint.tension %s2 : f64
  // CHECK: arith.constant
  %vl = constraint.voice_lead %s1, %s2 : f64
  // CHECK: arith.addf
  // CHECK: arith.subf
  // CHECK: arith.cmpf
  %ok = constraint.conserve %t1, %vl expected_sum = 4.2, tolerance = 0.15 : i1
}
