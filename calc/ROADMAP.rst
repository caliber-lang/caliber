Caliber Bootstrap Roadmap
=========================

Overview
--------

This document outlines the bootstrap pathway from the C implementation of calc
(the Caliber compiler) through self-hosting in Caliber1.

.. contents::
   :local:
   :depth: 2

Stage 1: C Bootstrap (Current)
==============================

**Status**: Active development

**Goal**: Minimal working Caliber1 compiler in C

Lexer
-----

- [x] Tokenization (keywords, identifiers, literals, operators, punctuation)
- [x] Comment handling (semicolon-based)
- [x] Line/column tracking for error reporting

Parser
------

- [x] Expression parsing (arithmetic, comparisons, field access)
- [x] Statement parsing (var, if/then/else, function calls)
- [x] Function definitions
- [x] Data type definitions
- [x] Structure literals

AST
---

- [x] Node representation
- [x] Child management
- [x] Type annotation storage

Type System
-----------

- [ ] Type checker (basic)
- [ ] Type inference (minimal)
- [ ] Type error reporting

Caliber--
---------

- [x] IR emission (basic expressions and statements)
- [x] IR parser
- [x] Stack-based instruction representation

Code Generation
---------------

- [x] x86-64 AT&T assembly backend
- [ ] Function prologue/epilogue
- [ ] Stack frame management
- [ ] Register allocation

Features
--------

Completed:

- Arithmetic operators (+, -, *, /)
- Comparison operators (==, !=, <, >, <=, >=)
- Variable declaration and mutation
- Function definitions and calls
- Data type definitions
- Field access

In Progress:

- Stakes (@ allocation)
- References (ref syntax)
- Arrays

Not Yet:

- While loops
- Module system (import)
- File I/O
- Ranges
- Raw memory access

Stage 2: Caliber1 Compiler in Caliber1
=======================================

**Status**: Planning

**Goal**: Rewrite calc in Caliber1

**Dependencies**: Stage 1 must reach feature parity with subset of Caliber1 needed for a compiler

Lexer in Caliber1
-----------------

- [ ] String/buffer handling
- [ ] Tokenization
- [ ] Character classification
- [ ] Error reporting

Parser in Caliber1
------------------

- [ ] Recursive descent parser
- [ ] AST construction
- [ ] Error recovery

Type Checker in Caliber1
------------------------

- [ ] Symbol table management
- [ ] Type inference
- [ ] Type unification
- [ ] Error diagnostics

Caliber-- Emitter in Caliber1
-----------------------------

- [ ] AST traversal
- [ ] Instruction generation
- [ ] Function organization
- [ ] Constant pooling

Backend in Caliber1
-------------------

- [ ] x86-64 instruction selection
- [ ] Register allocation
- [ ] Assembly generation
- [ ] Linking

File I/O in Caliber1
--------------------

- [ ] Source file reading
- [ ] Output file writing
- [ ] Error file reporting

Standard Library
----------------

- [ ] String utilities
- [ ] Memory utilities
- [ ] I/O primitives
- [ ] Data structure support

Stage 3: Self-Compilation
==========================

**Status**: Not started

**Goal**: Achieve bootstrap cycle

**Prerequisites**: Stage 2 compiler must be feature complete

Bootstrap Cycle
---------------

- [ ] C bootstrap compiles Caliber1 compiler
- [ ] Resulting compiler (A) compiles Caliber1 compiler source
- [ ] Compiler A produces compiler B
- [ ] Compiler B produces compiler C
- [ ] Compiler B and C are functionally equivalent

Validation
----------

- [ ] Bitwise equivalence test
- [ ] Behavioral equivalence test
- [ ] Cross-compilation test
- [ ] Clean rebuild test

Stage 4: Stabilization
======================

**Status**: Not started

**Goal**: Stable self-hosted toolchain

Compiler Optimization
---------------------

- [ ] Basic optimization passes
- [ ] Dead code elimination
- [ ] Constant folding
- [ ] Peephole optimization

Standard Library Completion
----------------------------

- [ ] Collections (arrays, maps, sets)
- [ ] String manipulation
- [ ] Math utilities
- [ ] System interfaces

Documentation
--------------

- [ ] Language specification finalization
- [ ] API documentation
- [ ] Tutorial and examples
- [ ] Compiler internals documentation

Testing Infrastructure
----------------------

- [ ] Unit tests
- [ ] Integration tests
- [ ] Regression tests
- [ ] Benchmark suite

Milestone Timeline
==================

Phase 1: C Bootstrap Stabilization
-----------------------------------

**Target**: 3-4 months from start

- Complete lexer and parser for Caliber1 subset
- Implement basic type checking
- Caliber-- emission complete
- x86-64 backend functional
- Can compile simple programs

**Success Criteria**:

- Compile recursive functions
- Compile data structure operations
- Generate working x86-64 binaries

Phase 2: Caliber1 Self-Compilation
-----------------------------------

**Target**: 6-9 months from Phase 1 start

- Rewrite bootstrap compiler in Caliber1
- All major components functional
- File I/O and module system working
- Standard library adequate for compiler needs

**Success Criteria**:

- Caliber1 compiler compiles itself
- Generated binary produces equivalent output

Phase 3: Full Bootstrap Achievement
------------------------------------

**Target**: 9-12 months from Phase 2 start

- Bootstrap cycle completes successfully
- Compiler passes multi-generation testing
- C bootstrap becomes optional

**Success Criteria**:

- Clean rebuild produces stable compiler
- C bootstrap no longer required

Phase 4: Hardening and Expansion
---------------------------------

**Target**: Ongoing after Phase 3

- Performance optimization
- Expanded standard library
- Additional backends (if desired)
- Language stability

Critical Path
=============

The following are on the critical path to self-hosting:

1. **Stakes and References** (Stage 1)
   - Essential for memory management in compiler
   - Prerequisite for everything in Stage 2

2. **Arrays** (Stage 1)
   - Needed for buffers, token streams, AST children
   - Cannot defer to Stage 2

3. **File I/O** (Stage 1 or early Stage 2)
   - Needed to read source files
   - Needed to write output

4. **Module System** (early Stage 2)
   - Allows compiler decomposition
   - Enables library development

5. **Type Checker** (Stage 2)
   - Validates Caliber1 source
   - Detects compiler bugs early

Dependencies and Blockers
==========================

Current Blockers
----------------

None - C bootstrap can proceed independently.

Stage 1 → Stage 2 Blockers
---------------------------

- Must complete: stakes, references, arrays, file I/O
- Must have: basic type checking
- Must support: function composition for compiler architecture

Stage 2 → Stage 3 Blockers
---------------------------

- Must complete: full Caliber1 compiler in Caliber1
- Must support: self-compilation without C
- Must pass: bootstrap equivalence tests

Contingency
===========

If bootstrap cycle fails at Stage 3:

1. Identify semantic differences between compilers
2. Fix semantic issues in Stage 2 compiler
3. Use C bootstrap to re-test Stage 2
4. Re-enter Stage 3

If Stage 2 compiler is too slow or large:

1. Profile and optimize Caliber1 compiler
2. Consider intermediate bootstrap compiler (Caliber0)
3. Simplify IR or backend if necessary

Version History
===============

**v0.1** (Initial)
  First roadmap draft aligned with Caliber1 spec

Success Metrics
===============

- [ ] C bootstrap compiles non-trivial Caliber1 programs
- [ ] Caliber1 compiler compiles itself (Stage 2)
- [ ] Bootstrap cycle produces stable compiler (Stage 3)
- [ ] C bootstrap removed from critical path (Stage 4)
- [ ] Caliber compiler written entirely in Caliber (Completion)
