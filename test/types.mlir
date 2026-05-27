// RUN: constraint-opt %s | FileCheck %s
//
// Type parsing test: verify all custom types are recognized.

// CHECK-LABEL: @type_sequence
module @type_sequence {
  // CHECK: constraint.sequence
  %seq = constraint.sequence notes = [0] : !constraint.Sequence
}

// CHECK-LABEL: @type_dial
module @type_dial {
  // CHECK: constraint.dial
  %d = constraint.dial harmonic = 0.5, rhythmic = 0.5, spectral = 0.5 : !constraint.Dial
}

// CHECK-LABEL: @type_all_together
module @type_all_together {
  // Use all three types in one module.
  // Sequence type
  %s = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence

  // Dial type
  %d = constraint.dial harmonic = 0.8, rhythmic = 0.6, spectral = 0.4 : !constraint.Dial

  // Tension produces f64 (not a custom type, but uses Sequence input)
  %t = constraint.tension %s : f64

  // Voice lead produces f64
  %s2 = constraint.sequence notes = [2, 5, 9] : !constraint.Sequence
  %vl = constraint.voice_lead %s, %s2 : f64

  // Consume all values to ensure they type-check together
  %ok = constraint.conserve %t, %vl expected_sum = 1.0, tolerance = 0.5 : i1
}
