Caliber1 Bootstrap Specification
================================

.. contents::
   :local:
   :depth: 2

Introduction
============

Caliber1 is the bootstrap language specification for Caliber.

Caliber1 defines the smallest practical subset of Caliber that is capable of
implementing the Caliber compiler itself. It is not intended to describe the
complete long term language.

The purpose of Caliber1 is to provide a stable foundation for bootstrapping
the language from its initial C implementation into a fully self hosted
compiler.

The primary requirement of Caliber1 is simple:

::

    A Caliber1 compiler must eventually be capable of compiling the Caliber
    compiler written entirely in Caliber1.

Caliber1 therefore prioritizes:

- simple implementation
- predictable semantics
- fast compilation
- sufficient expressive power
- systems programming capability
- straightforward translation to Caliber--
- suitability for implementing a compiler

Features that are not necessary for bootstrapping the compiler are outside
the initial Caliber1 specification.

Language philosophy
===================

Caliber1 follows the general philosophy of Caliber:

- memory should be safe by default
- aliasing should not require a borrow checker
- systems programmers should retain explicit control
- abstractions should have predictable costs
- the language should remain practical to implement

Caliber1 does not attempt to prove all memory safety properties statically.

Instead, stakes establish ownership and references provide checked access to
owned objects.

Caliber1 and future Caliber
===========================

Caliber1 is a subset of the eventual Caliber language.

The following features may exist in future versions of Caliber without being
part of Caliber1:

- advanced pattern matching
- generics
- traits
- macros
- pipelines
- advanced type inference
- concurrency abstractions
- asynchronous programming
- additional allocation strategies
- multiple architecture backends

Caliber1 must remain sufficiently small that its compiler can be implemented
without requiring these features.

Source files
============

Caliber1 implementation files use the ``.cal`` extension.

Caliber1 declaration files use the ``.calh`` extension.

A ``.cal`` file may contain declarations and implementations.

A ``.calh`` file contains declarations and type definitions but does not
contain function implementations.

Comments
========

Comments begin with ``;`` and continue until the end of the line.

Example::

    var value <- 42 ; this is a comment

Comments have no semantic meaning.

Lexical structure
=================

Identifiers
-----------

An identifier consists of letters, digits, and underscores.

An identifier must begin with a letter or underscore.

Examples::

    value
    user
    user_name
    parse_token
    node42

Keywords
--------

The following identifiers are reserved as Caliber1 keywords:

::

    def
    data
    var
    if
    then
    else
    while
    return
    print
    alloc
    ref
    import

Additional keywords may be reserved by future versions of Caliber.

Integer literals
----------------

Integer literals consist of decimal digits.

Examples::

    0
    1
    42
    1000

Caliber1 integers are signed machine-sized integers.

String literals
---------------

String literals are enclosed in double quotes.

The following escape sequences are defined:

::

    \n
    \t
    \\
    \"

An implementation must reject an unterminated string literal.

Primitive types
===============

Caliber1 defines the following primitive types.

Int
---

``Int`` represents a signed machine-sized integer.

Bool
----

``Bool`` represents a boolean value.

A boolean has one of two values:

::

    true
    false

String
------

``String`` represents a sequence of characters.

String values are immutable.

Char
----

``Char`` represents a single character.

Data types
==========

User-defined data types are declared using ``data``.

Example::

    data User = {
        name: String,
        age: Int
    }

A data type contains a fixed set of named fields.

Each field has a name and a type.

Field access
------------

Fields are accessed using ``.``.

Example::

    user.name
    user.age

Structure literals
------------------

A data value may be constructed using a structure literal.

Example::

    User {
        name: "Alice",
        age: 25
    }

The fields supplied by a structure literal must correspond to fields defined
by the data type.

Variables
=========

Local variables are declared using ``var``.

Example::

    var value <- 42

The ``<-`` operator binds the result of an expression to a new variable.

Variables are mutable.

Mutation
--------

A variable may be changed using ``:=``.

Example::

    value := 50

The new value must be compatible with the variable's type.

Functions
=========

Functions are declared using ``def``.

Example::

    def add(x: Int, y: Int) -> Int =
        return x + y

A function may contain zero or more parameters.

Parameters may optionally specify their types.

Return types are specified using ``->``.

The ``main`` function is the program entry point.

Function calls
--------------

Functions are called using their name and a list of arguments.

Example::

    add(10, 20)

Expressions
===========

Caliber1 expressions include:

- integer literals
- boolean literals
- string literals
- character literals
- identifiers
- stake references
- function calls
- arithmetic expressions
- comparison expressions
- field access
- parenthesized expressions

Arithmetic operators
--------------------

Caliber1 defines the following arithmetic operators:

::

    +
    -
    *
    /

Multiplication and division have higher precedence than addition and
subtraction.

Comparison operators
--------------------

Caliber1 defines the following comparison operators:

::

    ==
    !=
    <
    >
    <=
    >=

Comparison expressions produce ``Bool``.

Parenthesized expressions
-------------------------

Expressions may be grouped using parentheses.

Example::

    (x + y) * z

Stakes
======

A stake is the owner of a heap allocated object.

Stakes are written using ``@``.

Allocation
----------

An object is allocated using ``alloc``.

Example::

    @user <- alloc User {
        name: "Alice",
        age: 25
    }

The resulting stake owns the allocation.

As long as the stake remains alive, the allocation remains alive.

Stake lifetime
--------------

A stake has a lexical lifetime.

When a stake leaves its scope, its owned allocation becomes eligible for
destruction.

The implementation must ensure that references to the destroyed allocation
cannot silently access freed memory.

Stake mutation
--------------

Fields may be mutated through a stake.

Example::

    @user.age := 26

References
==========

A reference provides access to an object owned by a stake.

A reference is created using ``ref``.

Example::

    var user_ref <- ref @user

References do not own the object they reference.

Multiple references may refer to the same stake.

Example::

    var first <- ref @user
    var second <- ref @user

Both references refer to the same underlying allocation.

Aliasing
--------

Caliber1 permits unrestricted aliasing between valid references.

Mutation performed through a stake is visible through all valid references to
that object.

Example::

    @user.age := 26

    print first.age
    print second.age

Both accesses observe ``26``.

Reference validity
------------------

A reference is valid only while its associated allocation is alive.

When the owning stake is destroyed, references associated with that allocation
become stale.

Dereferencing a stale reference must not result in undefined behavior.

The implementation must instead report that the reference is invalid.

Generation counters
-------------------

An implementation may use generation counters to implement reference
validation.

A conceptual allocation may contain:

::

    generation
    object

A reference may contain:

::

    allocation
    generation

A reference is valid when its stored generation corresponds to the current
generation of the allocation.

The exact representation is implementation-defined.

Control flow
============

If statements
-------------

Conditional execution uses ``if``, ``then``, and ``else``.

Example::

    if age >= 18 then
        print "adult"
    else
        print "minor"

The condition must evaluate to ``Bool``.

While loops
-----------

Caliber1 provides ``while`` loops.

Example::

    while x < 10
        x := x + 1

The condition is evaluated before every iteration.

Blocks
------

A block contains a sequence of statements.

Statements execute in source order.

Memory management
=================

Caliber1 supports multiple memory management strategies.

Stakes
------

Stakes are the default mechanism for heap allocated objects.

A stake owns an allocation and determines its lifetime.

References may exist independently of the stake but become invalid when the
stake is destroyed.

Ranges
------

Ranges provide bulk allocation and bulk destruction.

The exact range interface is implementation-defined during the initial
Caliber1 bootstrap period.

Ranges are intended for workloads where many allocations share a lifetime.

Raw memory
----------

Caliber1 may provide controlled access to raw memory for systems programming.

Raw memory operations are outside the safety guarantees provided by stakes
and references.

The initial Caliber1 implementation may expose raw memory operations through
a small implementation-defined interface.

Arrays
======

Caliber1 provides arrays containing values of a single element type.

An array supports:

- creation
- indexing
- mutation
- length retrieval

Example::

    var values <- [1, 2, 3, 4]

    values[0] := 10

The exact runtime representation of arrays is implementation-defined.

Strings
=======

Strings represent sequences of characters.

Example::

    var message <- "Hello, world!"

Caliber1 strings must support:

- creation from literals
- length retrieval
- comparison
- output
- access to individual characters

String values are immutable.

I/O
===

The Caliber1 environment must provide enough I/O functionality for
implementing the compiler.

At minimum, the bootstrap environment must provide:

- standard output
- standard error
- file creation
- file opening
- file reading
- file writing
- file closing

The exact standard library interface may evolve during bootstrap.

Modules
=======

Caliber1 programs may consist of multiple source files.

Modules are imported using ``import``.

Example::

    import lexer
    import parser
    import ast

A module may expose declarations defined in a ``.calh`` file.

The module system must allow the Caliber compiler to be separated into
multiple source files.

Compiler requirements
=====================

Caliber1 must be expressive enough to implement the complete Caliber
compiler.

A bootstrap-capable compiler must be able to implement the following
components in Caliber1.

Lexer
-----

The lexer must be able to:

- read source text
- recognize identifiers
- recognize keywords
- recognize literals
- recognize operators
- recognize punctuation
- report lexical errors

Parser
------

The parser must be able to construct an AST representing valid Caliber1
source.

AST
---

Caliber1 must support recursive data structures.

A compiler must therefore be able to represent structures such as trees and
linked structures.

Example::

    data Node = {
        value: Int,
        next: Node
    }

The exact recursive type representation may be refined during bootstrap.

Type checker
------------

Caliber1 must be capable of implementing static type checking.

The language must provide sufficient type information to distinguish invalid
operations before code generation.

Code generator
--------------

Caliber1 must be capable of generating Caliber-- programs.

The compiler must be able to construct Caliber-- instructions, constants,
labels, and other required output.

Backend
-------

The compiler must be capable of translating Caliber-- into executable
machine code.

The initial backend may target x86-64.

File generation
---------------

The compiler must be able to create and write output files.

This includes generated Caliber-- files and, eventually, generated object
files or executable binaries.

Error reporting
---------------

The compiler must be capable of reporting errors with sufficient information
to identify their location and cause.

Caliber--
=========

Caliber-- is the intermediate language used by the Caliber compiler.

Caliber-- is intentionally not designed for human programmers.

Its primary goals are:

- extremely fast compiler generation
- extremely simple parsing
- compact representation
- explicit semantics
- direct translation to machine code

Caliber-- may use a textual representation during bootstrap.

A conceptual Caliber-- program may look like::

    f main 0 0
    a 0 User 24
    i 0 8 25
    i 0 16 26
    q 0
    e

Caliber-- instructions should represent compiler operations directly.

The Caliber compiler should not need to reconstruct high-level language
semantics from low-level instructions.

Compilation pipeline
--------------------

The intended Caliber compilation pipeline is::

    Caliber source
        |
        v
    Lexer
        |
        v
    Parser
        |
        v
    AST
        |
        v
    Type checker
        |
        v
    Caliber--
        |
        v
    Backend
        |
        v
    Machine code

Caliber-- is the boundary between the language frontend and the machine
backend.

Bootstrap compiler
==================

The first Caliber compiler may be written in C.

This compiler is the bootstrap compiler.

The bootstrap compiler exists only to provide the initial implementation of
Caliber1.

The C implementation is not considered part of the final Caliber toolchain.

Bootstrap stages
================

Stage 1: C bootstrap
--------------------

A minimal compiler is implemented in C.

It provides enough Caliber1 functionality to compile increasingly complex
Caliber1 programs.

Stage 2: Caliber compiler in Caliber1
-------------------------------------

The Caliber compiler is rewritten in Caliber1.

The Caliber compiler must eventually contain:

- lexer
- parser
- AST
- type checker
- Caliber-- generator
- Caliber-- backend
- standard library support
- file I/O

Stage 3: self compilation
-------------------------

The C bootstrap compiler compiles the Caliber compiler written in Caliber1.

The resulting compiler must then compile the same Caliber compiler source
again.

Conceptually::

    C bootstrap
        |
        v
    Caliber compiler
        |
        v
    Caliber compiler
        |
        v
    Caliber compiler
        |
        v
       ...

The resulting compiler must be functionally equivalent across successive
self compilations.

Stage 4: removal of C
---------------------

Once the Caliber compiler can compile itself, the C bootstrap compiler is no
longer required for normal Caliber development.

The Caliber compiler becomes the primary compiler implementation.

Self-hosting milestone
======================

Caliber1 is considered self hosted when all of the following conditions are
met:

- the Caliber compiler is written entirely in Caliber1
- the Caliber compiler can compile its own source
- the compiler generates Caliber--
- the Caliber-- backend produces executable programs
- the compiler can perform its required file I/O without C-specific compiler
  extensions
- a clean source tree can rebuild the compiler without using the C compiler

The final test is a bootstrap cycle::

    bootstrap compiler
        |
        v
    compiler A
        |
        v
    compiler B
        |
        v
    compiler C

If compiler B and compiler C can independently compile the same Caliber source
and produce equivalent behavior, the bootstrap process has reached a stable
self-hosted state.

Implementation limits
=====================

The initial Caliber1 specification intentionally leaves some implementation
details open.

These include:

- exact memory layout of data types
- exact representation of references
- generation counter representation
- range implementation
- raw memory interface
- array representation
- standard library ABI
- Caliber-- serialization details
- machine code representation
- linker implementation

These details may be fixed as the bootstrap implementation develops.

Caliber1 should not require implementation details that are irrelevant to the
language semantics.

Out of scope
============

The following are outside the initial Caliber1 specification:

- generics
- traits
- advanced pattern matching
- compile-time macros
- asynchronous programming
- advanced concurrency
- multiple architecture backends
- garbage collection
- advanced metaprogramming
- language reflection
- compiler plugins

These features may be introduced in later Caliber versions.

Versioning
==========

Caliber1 is the first bootstrap language specification.

Breaking changes are permitted while the bootstrap implementation is under
active development.

Once self hosting is achieved, Caliber1 should become stable.

Major incompatible language changes should result in a new language version.

The next major version should be designated Caliber2.

Final requirement
=================

The ultimate requirement of Caliber1 is not that the language be feature
complete.

The requirement is that it be powerful enough to replace the compiler that
created it.

Caliber1 succeeds when Caliber can be written in Caliber.
