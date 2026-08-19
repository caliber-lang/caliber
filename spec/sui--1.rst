Sui-- instructions

f <name> <params> <locals>
    Begin function.

e
    End function.

i <type> <slot> <value>
    Push constant.

r <name>
    Load local.

v <name>
    Store top of stack into local.

b <op>
    Binary operation on the top two stack values.

c <op>
    Comparison.

g <field>
    Load a field from the object on the stack.

m <field>
    Store a value into a field.

a <slot> <type> <size>
    Allocate an anchor.

x <name> <argc>
    Call function.

p
    Print top of stack.

q
    Return top of stack.
