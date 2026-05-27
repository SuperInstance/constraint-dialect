// RUN: constraint-opt %s | FileCheck %s
//
// Roundtrip test: parse → print should reproduce the same operations.
// This verifies that parse/print are symmetric.

// CHECK-LABEL: @roundtrip_sequence
module @roundtrip_sequence {
  // CHECK: constraint.sequence notes = [0, 4, 7]
  %seq = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence
  // CHECK: constraint.tension
  %t = constraint.tension %seq : f64
}

// CHECK-LABEL: @roundtrip_dial
module @roundtrip_dial {
  // CHECK: constraint.dial harmonic = 5.000000e-01, rhythmic = 5.000000e-01, spectral = 5.000000e-01
  %d = constraint.dial harmonic = 0.5, rhythmic = 0.5, spectral = 0.5 : !constraint.Dial
}

// CHECK-LABEL: @roundtrip_tradition
module @roundtrip_tradition {
  // CHECK: constraint.tradition name = "jazz"
  %jazz = constraint.tradition name = "jazz" : !constraint.Dial
}

// CHECK-LABEL: @roundtrip_voice_lead
module @roundtrip_voice_lead {
  %seq1 = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence
  %seq2 = constraint.sequence notes = [2, 5, 9] : !constraint.Sequence
  // CHECK: constraint.voice_lead
  %vl = constraint.voice_lead %seq1, %seq2 : f64
}

// CHECK-LABEL: @roundtrip_conserve
module @roundtrip_conserve {
  %seq = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence
  %t = constraint.tension %seq : f64
  %seq2 = constraint.sequence notes = [0, 5, 7] : !constraint.Sequence
  %vl = constraint.voice_lead %seq, %seq2 : f64
  // CHECK: constraint.conserve
  %ok = constraint.conserve %t, %vl expected_sum = 4.200000e+00, tolerance = 1.500000e-01 : i1
}

// CHECK-LABEL: @roundtrip_single_note
module @roundtrip_single_note {
  // Single-note sequence should roundtrip correctly.
  // CHECK: constraint.sequence notes = [7]
  %seq = constraint.sequence notes = [7] : !constraint.Sequence
}

// CHECK-LABEL: @roundtrip_chromatic
module @roundtrip_chromatic {
  // All 12 pitch classes.
  // CHECK: constraint.sequence notes = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
  %seq = constraint.sequence notes = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11] : !constraint.Sequence
}
