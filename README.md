# constraint-dialect

MLIR dialect for constraint operations — custom ops for `constraint.sequence`, `constraint.tension`, `constraint.conserve` with lowering to Affine and LLVM IR.

## What This Gives You

- **Custom MLIR ops** — first-class constraint operations in the MLIR framework
- **Constraint-specific types** — lattice points, deadband funnels, tension graphs
- **Progressive lowering** — constraint dialect → Affine → LLVM IR → machine code
- **Optimization passes** — constraint-aware constant folding, dead code elimination, and fusion
- **LLVM integration** — plugs into the standard MLIR/LLVM pipeline

## Quick Start

```mlir
// constraint.sequence — a temporal constraint pipeline
func.func @verify_system(%points: memref<100xf64>) -> i1 {
  %snapped = constraint.sequence(%points) {
    constraint.snap { lattice = #constraint.eisenstein_a2 },
    constraint.funnel { decay = 0.1 : f64, tolerance = 0.001 : f64 },
    constraint.conserve { modulus = 48 : i32 }
  }
  return %snapped : i1
}
```

### Building the Dialect

```bash
mkdir build && cd build
cmake .. -G Ninja -DLLVM_ENABLE_PROJECTS=mlir
ninja constraint-dialect
ninja check-constraint-dialect  # run tests
```

## Dialect Operations

| Op | Description |
|---|---|
| `constraint.snap` | Quantize to lattice (Eisenstein A₂ or Z²) |
| `constraint.tension` | Compute tension in constraint graph |
| `constraint.conserve` | Check conservation invariant |
| `constraint.sequence` | Chain of constraint operations |
| `constraint.funnel` | Apply deadband funnel with decay |

## Architecture

```
constraint.dialect
       │
       ▼
    Affine Dialect
       │
       ▼
    LLVM IR
       │
       ▼
   Machine Code
```

## How It Fits

The **compiler integration** layer of the constraint theory ecosystem:

- [constraint-theory-core](https://github.com/SuperInstance/constraint-theory-core) — theory primitives that the dialect encodes
- [constraint-theory-engine-cpp-lua](https://github.com/SuperInstance/constraint-theory-engine-cpp-lua) — C++ engine with LLVM IR emitter
- [flux-compiler-workspace](https://github.com/SuperInstance/flux-compiler-workspace) — FLUX compiler using this dialect
- [constraint-substrate](https://github.com/SuperInstance/constraint-substrate) — cross-language primitives for lowering targets

## Testing

```bash
ninja check-constraint-dialect
```

## Installation

Requires LLVM/MLIR development build. See [MLIR getting started](https://mlir.llvm.org/getting_started/).

## License

MIT
