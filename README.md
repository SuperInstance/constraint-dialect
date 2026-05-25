# MLIR Constraint Dialect

An MLIR dialect for the **SuperInstance** ecosystem that encodes harmonic, rhythmic, and spectral constraints as first-class IR operations. It models musical sequences, voice leading, harmonic tension, and tradition-space dials, then lowers them through affine loops to LLVM IR.

## Architecture

```
┌─────────────────────────────────────────────────┐
│              Constraint MLIR Dialect            │
│  sequence · tension · dial · tradition          │
│  voice_lead · conserve                          │
└──────────────────────┬──────────────────────────┘
                       │  ConstraintToAffine
                       ▼
┌─────────────────────────────────────────────────┐
│              Affine + Arith + MemRef            │
│  (loop nests, memory ops, constants)            │
└──────────────────────┬──────────────────────────┘
                       │  ConvertToLLVM (upstream)
                       ▼
┌─────────────────────────────────────────────────┐
│                  LLVM IR                        │
└──────────────────────┬──────────────────────────┘
                       │  llc / JIT
                       ▼
                  Native Code
```

### Conservation Pipeline

```
constraint.tension ops ──► sum I_vert ──► compute I_horiz
       │                                      │
       └──── constraint.conserve ◄────────────┘
            (|I_vert + I_horiz - expected| < 0.15)
```

## Operations

| Operation | Description |
|---|---|
| `constraint.sequence` | Musical sequence of pitch classes (0–11) |
| `constraint.tension` | Harmonic tension I_vert of a sequence |
| `constraint.dial` | 3D position in tradition space |
| `constraint.tradition` | Named tradition dial lookup |
| `constraint.voice_lead` | Minimal voice-leading distance |
| `constraint.conserve` | Conservation constraint check |

## Custom Types

| Type | Lowered To |
|---|---|
| `!constraint.Sequence` | `memref<?xi32>` |
| `!constraint.Dial` | `tuple<f64, f64, f64>` |
| `!constraint.VoiceLeading` | `memref<?xi32>` |

## Build Instructions

**Requirements:** LLVM 17+ built with MLIR enabled.

```bash
# Clone
git clone https://github.com/SuperInstance/constraint-dialect.git
cd constraint-dialect

# Build against an existing LLVM/MLIR installation
mkdir build && cd build
cmake .. \
  -DMLIR_DIR=$(llvm-config --cmakedir) \
  -DCMAKE_BUILD_TYPE=Release
cmake --build .

# Run tests
cmake --build . --target check-constraint
```

### Using the `constraint-opt` Tool

```bash
# Parse and verify
./bin/constraint-opt test/basic.mlir

# Lower to affine
./bin/constraint-opt test/lowering.mlir --convert-constraint-to-affine

# Check conservation
./bin/constraint-opt test/conservation.mlir --check-conservation
```

## Example MLIR

```mlir
// Define a C major triad as a sequence
%seq = constraint.sequence notes = [0, 4, 7] : !constraint.Sequence

// Compute its harmonic tension
%t = constraint.tension %seq : f64

// Define a dial position
%dial = constraint.dial harmonic = 0.8, rhythmic = 0.5, spectral = 0.3
  : !constraint.Dial

// Look up a tradition
%jazz = constraint.tradition name = "jazz" : !constraint.Dial

// Voice lead between two sequences
%seq2 = constraint.sequence notes = [0, 5, 7] : !constraint.Sequence
%vl = constraint.voice_lead %seq, %seq2 : f64

// Conservation check
%ok = constraint.conserve %t, %vl {
  expected_sum = 4.2,
  tolerance = 0.15
} : i1
```

## Integration with SuperInstance Ecosystem

- **[constraint-toolkit](https://github.com/SuperInstance/constraint-toolkit)** — Python bindings and analysis tools that generate Constraint MLIR
- **[constraint-audio](https://github.com/SuperInstance/constraint-audio)** — Audio rendering pipeline that consumes lowered IR for real-time synthesis

The dialect acts as the compiler intermediate layer: Python tools emit `.mlir` files using this dialect, and the audio backend consumes the lowered LLVM IR for low-latency DSP.

## License

MIT
