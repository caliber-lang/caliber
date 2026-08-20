Caliber
=======

Caliber is a systems programming language built around a simple idea: memory
should be safe by default, without forcing the programmer into an ownership
and borrowing discipline.

Caliber is a fully independent language designed to be easy, powerful and
beautiful.

The core idea
=============

In Caliber, memory is managed through stakes and references.

.. code-block:: haskell

    @user <- alloc User { name: "Alice", email: "alice@example.com", age: 25 }

A stake, written with ``@``, is the owner of a heap allocated object. As long
as the stake is alive, the object is guaranteed to be alive.

.. code-block:: haskell

    var ref_a <- ref @user
    var ref_b <- ref @user

References are created freely from a stake. There is no limit on how many
references exist, and no compiler restriction on aliasing. All references see
the same underlying object.

.. code-block:: haskell

    @user.name := "Bob"
    print ref_a.name   ; Bob
    print ref_b.name   ; Bob

Mutation through the stake is visible to every reference, because they all
point back to the same allocated object.

When the stake goes out of scope, the object is freed automatically, and every
reference tied to it becomes stale. Caliber uses generation counters to detect
this: each reference remembers the generation of the object at the time it was
created, and dereferencing checks that generation against the stake's current
one. A stale reference does not crash the program or invoke undefined
behavior. It simply reports that it is no longer valid.

This gives you:

- Direct aliasing and mutation
- Protection against stale references silently corrupting memory
- Automatic memory management without requiring lifetime proofs
- The option to turn automatic management off and manage memory by hand where
  it matters

The philosophy in one line: make things safe by default, but never take
control away from the programmer who wants it.

Syntax
======

Caliber borrows the readability of Haskell and the symbolic minimalism of
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
- ``@name`` marks a stake, the owner of a heap allocation
- ``ref`` creates a reference to a stake; reference types are inferred unless
  you annotate them explicitly
- ``:=`` mutates through a stake, ``<-`` binds the result of an allocation or
  a computation
- ``data`` defines a struct-like type, ``match`` provides pattern matching
  over it
- Macros expand entirely at compile time and carry no runtime cost

File extensions
===============

- ``.cal`` is the extension for implementation files.
- ``.calh`` is the extension for header files, containing only type and
  function declarations, no implementations, imported by ``.cal`` files
  that need them.

Memory model in practice
=========================

Caliber does not force a single allocation strategy. Three modes exist
depending on what you are writing:

- **Stakes**, the default, for heap allocated, scope managed objects with
  reference support
- **Ranges**, for bulk allocation and bulk teardown, useful in tight loops
  such as game frames
- **Raw pointers**, for the rare cases where systems code needs direct,
  unchecked control

This means the same language can reasonably be used to write a command line
tool, a GUI, a game engine, or a kernel level component, choosing the right
allocation strategy for the job without switching languages.

Compiler architecture
======================

The long term goal is a fully self hosted toolchain: a Caliber compiler
written in Caliber, with no C anywhere in the final chain, and a hand rolled
backend rather than a dependency on an external assembler and linker.

The path there:

1. **Bootstrap compiler in C.** A minimal lexer, parser, and code generator,
   just enough to compile the subset of Caliber needed to write a real
   compiler. This exists only to get off the ground and is not meant to
   survive long term.
2. **Self hosted compiler in Caliber.** Once the bootstrap can handle
   functions, structs, control flow, recursion, and basic file I/O, the
   compiler itself gets rewritten in Caliber and compiled by the bootstrap.
3. **Hand rolled backend.** The compiler emits x86-64 machine code directly
   and writes ELF objects itself, removing the dependency on an external
   assembler. Linking against libc remains for now.
4. **No libc.** The final stage drops libc entirely, replacing it with raw
   syscalls and a small allocator written in Caliber.

The compiler pipeline itself is layered::

    Caliber source (.cal / .calh)
        -> Lexer
        -> Parser -> AST
        -> Type checker
        -> Caliber--
        -> Code generator
        -> Machine code

Caliber--
=========

Caliber-- is Caliber's own intermediate representation, sitting between the
AST and machine code. It exists so that optimization, portability to other
architectures, and the semantics of stakes, references, ranges, and generation
counters can all be handled in one place, independent of the final target.

Status
======

Caliber is under active design and development. The language specification,
the standard library, and the compiler are all evolving together. Expect
breaking changes until the self hosting milestone is reached.
