// RUN: constraint-opt %s --check-conservation | FileCheck %s
//
// Conservation pass edge-case tests.

// CHECK-LABEL: @conservation_single_tension
module @conservation_single_tension {
  // A single tension op with no voice_lead should still produce a conserve op
  // (using 0.0 for I_horiz).
  %seq = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence
  %t = constraint.tension %seq : f64

  // CHECK: constraint.conserve
}

// CHECK-LABEL: @conservation_multiple_tensions_and_vl
module @conservation_multiple_tensions_and_vl {
  // Multiple tension and voice_lead ops should all be collected.
  %s1 = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence
  %t1 = constraint.tension %s1 : f64

  %s2 = constraint.sequence notes = [2, 5, 9] : !constraint.Sequence
  %t2 = constraint.tension %s2 : f64

  %s3 = constraint.sequence notes = [1, 3, 6] : !constraint.Sequence
  %t3 = constraint.tension %s3 : f64

  %vl1 = constraint.voice_lead %s1, %s2 : f64
  %vl2 = constraint.voice_lead %s2, %s3 : f64

  // The pass should insert a single conserve op summing all tensions and VLs.
  // CHECK: constraint.conserve
}

// CHECK-LABEL: @conservation_no_tension
module @conservation_no_tension {
  // If there are no tension ops, the pass should not insert anything.
  %seq = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence

  // CHECK-NOT: constraint.conserve
}
