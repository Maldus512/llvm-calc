# Basic LLVM Code Generation Example with C Bindings

Implementation of the "calc" arithmetic language presented in chapter 2 of the "Learn LLVM 17" book. Based on the code found [here](https://github.com/PacktPublishing/Learn-LLVM-17/tree/main/Chapter02/calc) with some (not so) slight variations:

 - Written in C instead of C++
 - Using the LLVM C API
 - Tested with LLVM 20 instead of 17 (everything worked out of the box)
 - Uses algebraic data types (i.e. structs and unions) instead of objects and inheritance
 - Integrates syntax errors in the AST instead of simply reporting and halting; an error is translated into an error node and the parsing continues as if everything was fine. The error reporting is moved to the sematic check section and the code generation step is avoided.

## Requirements

 - A working LLVM installation
 - `ninja`

## Building

```
$ ninja         # builds the project, binaries are found under `build/`
$ ninja run     # builds and run the `test.expr` example
```
