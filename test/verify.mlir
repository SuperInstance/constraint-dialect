// RUN: constraint-opt %s | FileCheck %s
//
// Verification tests: ensure ops pass verification with valid inputs.

// CHECK-LABEL: @verify_sequence_boundary_notes
module @verify_sequence_boundary_notes {
  // Boundary pitch classes 0 and 11 should verify.
  // CHECK: constraint.sequence
  %seq0 = constraint.sequence notes = [0] : !constraint.Sequence
  // CHECK: constraint.sequence
  %seq11 = constraint.sequence notes = [11] : !constraint.Sequence
  // CHECK: constraint.sequence
  %seqFull = constraint.sequence notes = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11] : !constraint.Sequence
}

// CHECK-LABEL: @verify_dial_boundary_values
module @verify_dial_boundary_values {
  // Boundary dial values 0.0 and 1.0 should verify.
  // CHECK: constraint.dial
  %d0 = constraint.dial harmonic = 0.0, rhythmic = 0.0, spectral = 0.0 : !constraint.Dial
  // CHECK: constraint.dial
  %d1 = constraint.dial harmonic = 1.0, rhythmic = 1.0, spectral = 1.0 : !constraint.Dial
}

// CHECK-LABEL: @verify_tradition_all_known
module @verify_tradition_all_known {
  // All known traditions should parse and verify.
  // CHECK: constraint.tradition
  %jazz = constraint.tradition name = "jazz" : !constraint.Dial
  // CHECK: constraint.tradition
  %classical = constraint.tradition name = "classical" : !constraint.Dial
  // CHECK: constraint.tradition
  %blues = constraint.tradition name = "blues" : !constraint.Dial
  // CHECK: constraint.tradition
  %electronic = constraint.tradition name = "electronic" : !constraint.Dial
  // CHECK: constraint.tradition
  %folk = constraint.tradition name = "folk" : !constraint.Dial
}

// CHECK-LABEL: @verify_conserve_zero_tolerance
module @verify_conserve_zero_tolerance {
  %seq = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence
  %t = constraint.tension %seq : f64
  %seq2 = constraint.sequence notes = [0, 5, 7] : !constraint.Sequence
  %vl = constraint.voice_lead %seq, %seq2 : f64

  // Zero tolerance should be valid (non-negative).
  // CHECK: constraint.conserve
  %ok = constraint.conserve %t, %vl expected_sum = 4.2, tolerance = 0.0 : i1
}

// CHECK-LABEL: @verify_voice_lead_same_sequence
module @verify_voice_lead_same_sequence {
  %seq = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence
  // Voice leading from a sequence to itself should be valid.
  // CHECK: constraint.voice_lead
  %vl = constraint.voice_lead %seq, %seq : f64
}

// CHECK-LABEL: @verify_multiple_tensions
module @verify_multiple_tensions {
  %s1 = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence
  // CHECK: constraint.tension
  %t1 = constraint.tension %s1 : f64
  %s2 = constraint.sequence notes = [2, 5, 9] : !constraint.Sequence
  // CHECK: constraint.tension
  %t2 = constraint.tension %s2 : f64
  %s3 = constraint.sequence notes = [11, 3, 6] : !constraint.Sequence
  // CHECK: constraint.tension
  %t3 = constraint.tension %s3 : f64
}
