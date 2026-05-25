// RUN: constraint-opt %s --check-conservation | FileCheck %s
//
// Test the conservation transform pass.

module @conservation_test {
  // The pass should find these tension ops and sum them.
  %seq1 = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence
  %t1 = constraint.tension %seq1 : f64

  %seq2 = constraint.sequence notes = [2, 5, 9] : !constraint.Sequence
  %t2 = constraint.tension %seq2 : f64

  // Voice leading between the sequences
  %vl = constraint.voice_lead %seq1, %seq2 : f64

  // After --check-conservation, the pass should insert:
  // CHECK: constraint.conserve
  // The inserted conserve op will verify I_vert + I_horiz conservation.
}
