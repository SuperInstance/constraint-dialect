// RUN: constraint-opt %s | FileCheck %s
//
// Basic Constraint dialect usage test.

// CHECK-LABEL: @test_sequence
module @test_sequence {
  // CHECK: constraint.sequence
  %seq = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence

  // CHECK: constraint.tension
  %t = constraint.tension %seq : f64

  // CHECK: constraint.sequence
  %seq2 = constraint.sequence notes = [0, 5, 7] : !constraint.Sequence

  // CHECK: constraint.voice_lead
  %vl = constraint.voice_lead %seq, %seq2 : f64
}

// CHECK-LABEL: @test_dial
module @test_dial {
  // CHECK: constraint.dial
  %d = constraint.dial harmonic = 0.8, rhythmic = 0.5, spectral = 0.3
    : !constraint.Dial

  // CHECK: constraint.tradition
  %jazz = constraint.tradition name = "jazz" : !constraint.Dial
}

// CHECK-LABEL: @test_conserve
module @test_conserve {
  %seq = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence
  %t = constraint.tension %seq : f64

  %seq2 = constraint.sequence notes = [0, 5, 7] : !constraint.Sequence
  %vl = constraint.voice_lead %seq, %seq2 : f64

  // CHECK: constraint.conserve
  %ok = constraint.conserve %t, %vl expected_sum = 4.2, tolerance = 0.15 : i1
}
