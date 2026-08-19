Sui-- 1
=======

Sui-- is the compiler-oriented intermediate language used by Sui 1.

Sui-- is stack based. It is designed to be emitted quickly by a compiler
and consumed quickly by a backend. It is not intended to be pleasant for
humans to write.

Functions
=========

f <name> <params> <locals>
    Begin a function.

    ``name`` is the function name.

    ``params`` is the number of parameters.

    ``locals`` is the number of local slots.

e
    End a function.

Stack
=====

Expressions operate on an implicit value stack.

Instructions which produce a value push it onto the stack.

Instructions which consume values remove them from the stack.

The stack must be balanced at every control-flow boundary.

Constants
=========

i <type> <slot> <value>
    Push a constant onto the stack.

    ``type`` identifies the constant type.

    ``slot`` identifies the associated storage slot when required.

    ``value`` contains the constant value.

Locals
======

r <name>
    Push the value of local ``name`` onto the stack.

v <name>
    Pop the top value from the stack and store it in local ``name``.

Arithmetic
==========

b <op>
    Pop the right operand and left operand.

    Apply ``op`` to them.

    Push the result.

    Supported operators are:

    ``+``
        Addition.

    ``-``
        Subtraction.

    ``*``
        Multiplication.

    ``/``
        Division.

Comparisons
===========

c <op>
    Pop the right operand and left operand.

    Compare them using ``op``.

    Push the resulting boolean value.

    Supported operators are:

    ``==``
        Equality.

    ``!=``
        Inequality.

    ``<``
        Less than.

    ``>``
        Greater than.

    ``<=``
        Less than or equal.

    ``>=``
        Greater than or equal.

Structures
==========

g <field>
    Pop an object reference.

    Load ``field`` from the object.

    Push the field value.

m <field>
    Pop a value and an object reference.

    Store the value into ``field``.

Allocation
==========

a <slot> <type> <size>
    Allocate an object of ``type`` with ``size`` bytes.

    Store the resulting anchor in ``slot``.

Calls
=====

x <name> <argc>
    Pop ``argc`` arguments from the stack.

    Call function ``name``.

    If the function returns a value, push the result.

Output
======

p
    Pop the top value and print it.

Control flow
============

q
    Return the top value from the current function.

    If the function has no return value, return without consuming a value.

Design
======

Sui-- intentionally contains little source-level information.

It does not represent Sui syntax directly.

A Sui compiler should lower high-level constructs into simple stack
operations before emitting Sui--.

The backend is responsible for translating Sui-- into machine code.

Sui-- is an implementation detail of Sui 1 and is not intended to be
stable across major Sui versions.
