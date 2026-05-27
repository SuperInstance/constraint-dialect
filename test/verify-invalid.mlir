// RUN: not constraint-opt %s 2>&1 | FileCheck %s
//
// Negative verification tests: ensure ops fail verification with invalid inputs.

// CHECK-LABEL: @verify_bad_pitch_class_negative
module @verify_bad_pitch_class_negative {
  // Pitch class -1 is out of range [0, 11].
  // CHECK: pitch class must be in [0, 11]
  %seq = constraint.sequence notes = [-1, 4, 7] : !constraint.Sequence
}

// CHECK-LABEL: @verify_bad_pitch_class_too_large
module @verify_bad_pitch_class_too_large {
  // Pitch class 12 is out of range [0, 11].
  // CHECK: pitch class must be in [0, 11]
  %seq = constraint.sequence notes = [0, 4, 12] : !constraint.Sequence
}

// CHECK-LABEL: @verify_dial_harmonic_over
module @verify_dial_harmonic_over {
  // harmonic = 1.5 is out of range [0, 1].
  // CHECK: harmonic must be in [0, 1]
  %d = constraint.dial harmonic = 1.5, rhythmic = 0.5, spectral = 0.5 : !constraint.Dial
}

// CHECK-LABEL: @verify_dial_rhythmic_under
module @verify_dial_rhythmic_under {
  // rhythmic = -0.1 is out of range [0, 1].
  // CHECK: rhythmic must be in [0, 1]
  %d = constraint.dial harmonic = 0.5, rhythmic = -0.1, spectral = 0.5 : !constraint.Dial
}

// CHECK-LABEL: @verify_dial_spectral_over
module @verify_dial_spectral_over {
  // spectral = 2.0 is out of range [0, 1].
  // CHECK: spectral must be in [0, 1]
  %d = constraint.dial harmonic = 0.5, rhythmic = 0.5, spectral = 2.0 : !constraint.Dial
}

// CHECK-LABEL: @verify_conserve_negative_tolerance
module @verify_conserve_negative_tolerance {
  %seq = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence
  %t = constraint.tension %seq : f64
  %seq2 = constraint.sequence notes = [0, 5, 7] : !constraint.Sequence
  %vl = constraint.voice_lead %seq, %seq2 : f64

  // Negative tolerance is invalid.
  // CHECK: tolerance must be non-negative
  %ok = constraint.conserve %t, %vl expected_sum = 4.2, tolerance = -0.5 : i1
}
