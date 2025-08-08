# Intermediate representations (IR)
### Styles of IR
- Control flow graph
- Static single-assignment
- Continuation-passing style
- Three-address code

### GCC
Language front ends target one of a handful of IRs, mainly [GIMPLE](https://gcc.gnu.org/onlinedocs/gccint/GIMPLE.html) and [RTL](https://gcc.gnu.org/onlinedocs/gccint/RTL.html). Target back ends like the one for 68k then take those IRs and produce native code.