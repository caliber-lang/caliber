===
Sui
===

Sui is a systems programming language built around a simple idea: memory
should be safe by default, without forcing the programmer into an ownership
and borrowing discipline to get there. *(Right, rust?)*

Sui is trying to be a 100% independent language thats easy, powerful and beatiful.

The core idea
==============

In Sui, memory is managed through anchors and references.

.. code-block:: haskell

    @user <- alloc User { name: "Alice", email: "alice@example.com", age: 25 }

An anchor, written with ``@``, is the owner of a heap allocated object. As
long as the anchor is alive, the object is guaranteed to be alive.

.. code-block:: haskell

    var ref_a <- ref @user
    var ref_b <- ref @user

References are created freely from an anchor. There is no limit on how many
references exist, and no compiler complaining about aliasing. All references
see the same underlying object.

.. code-block:: haskell

    @user.name := "Bob"
    print ref_a.name   ; Bob
    print ref_b.name   ; Bob

Mutation through the anchor is visible to every reference, because they all
point back to the same owned object.

When the anchor goes out of scope, the object is freed automatically, and
every reference tied to it becomes stale. Sui uses generation counters to
detect this: each reference remembers the generation of the object at the
time it was created, and dereferencing checks that generation against the
anchor's current one. A stale reference does not crash the program or invoke
undefined behavior. It simply reports that it is no longer valid.

This gives you:

- C's freedom to alias and mutate
- Rust's guarantee that a dangling reference cannot silently corrupt memory
- The convenience of not writing a borrow checker proof for every function
  signature
- The option to turn automatic management off and manage memory by hand
  where it matters

The philosophy in one line: make things safe by default, but never take
control away from the programmer who wants it.

Syntax
======

Sui borrows the readability of Haskell and the symbolic minimalism of
Scheme, without the parenthesis heavy structure of either. The result is an
imperative language with functional syntax sugar: statements execute in
order, but pattern matching, type inference, and pipelines are all first
class.

.. code-block:: haskell

    data User = {
        name: String,
        email: String,
        age: Int
    }

    def isAdult (@user: User) -> Bool =
        @user.age >= 18

    def main =
        @alice <- alloc User { name: "Alice", email: "alice@example.com", age: 25 }
        var ref_a <- ref @alice

        @alice.age := 26
        print ref_a.age              ; 26

        if isAdult @alice then
            print "Alice is an adult"
        else
            print "Alice is a minor"

Notable pieces of the syntax:

- ``def`` defines a function, ``var`` binds a local value
- ``@name`` marks an anchor, the single owner of a heap allocation
- ``ref`` creates a reference to an anchor; reference types are inferred
  unless you annotate them explicitly
- ``:=`` mutates through an anchor, ``<-`` binds the result of an
  allocation or a computation
- ``data`` defines a struct-like type, ``match`` provides pattern matching
  over it
- Macros expand entirely at compile time and carry no runtime cost

File extensions
================

- ``.sui`` is the extension for implementation files.
- ``.kon`` is the extension for header files, containing only type and
  function declarations, no implementations, imported by ``.sui`` files
  that need them.

Memory model in practice
=========================

Sui does not force a single allocation strategy. Three modes exist depending
on what you are writing:

- **Anchors**, the default, for heap allocated, scope managed objects with
  reference support
- **Arenas**, for bulk allocation and bulk teardown, useful in tight loops
  such as game frames
- **Raw pointers**, for the rare cases where systems code needs direct,
  unchecked control

This means the same language can reasonably be used to write a command line
tool, a GUI, a game engine, or a kernel level component, choosing the right
allocation strategy for the job without switching languages.

Compiler architecture
======================

The long term goal is a fully self hosted toolchain: a Sui compiler written
in Sui, with no C anywhere in the final chain, and a hand rolled backend
rather than a dependency on an external assembler and linker.

The path there:

1. **Bootstrap compiler in C.** A minimal lexer, parser, and code
   generator, just enough to compile the subset of Sui needed to write a
   real compiler. This exists only to get off the ground and is not meant
   to survive long term.
2. **Self hosted compiler in Sui.** Once the bootstrap can handle
   functions, structs, control flow, recursion, and basic file I/O, the
   compiler itself gets rewritten in Sui and compiled by the bootstrap.
3. **Hand rolled backend.** The compiler emits x86-64 machine code
   directly and writes ELF objects itself, removing the dependency on an
   external assembler. Linking against libc remains for now.
4. **No libc.** The final stage drops libc entirely, replacing it with raw
   syscalls and a small allocator written in Sui.

The compiler pipeline itself is layered::

    Sui source (.sui / .kon)
        -> Lexer
        -> Parser -> AST
        -> Type checker
        -> WIR (Water Intermediate Representation)
        -> Code generator
        -> Machine code

WIR is Sui's own intermediate representation, sitting between the AST and
machine code. It exists so that optimization, portability to other
architectures, and the semantics of anchors, references, and generation
counters can all be handled in one place, independent of the final target.

Status
======

Sui is under active design and development. The language specification, the
standard library, and the compiler are all evolving together. Expect
breaking changes until the self hosting milestone is reached.
