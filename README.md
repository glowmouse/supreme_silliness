# supreme_silliness
Parse a text description of a graph and count connected subgraphs at compile time

## Compiling

> g++ -O -fconstexpr-depth=10000 -fconstexpr-loop-limit=10000000 -fconstexpr-ops-limit=1000000000 main.cpp

gcc version 11.4.0 tested

## Manifest

main.cpp          | Most of the code to read the graph and count subgraphs
graph.h           | The graph as a literal string
text_partsing.h   | Utilities to do text partsing.

## Assembly output

```
main:
.LFB2291:
  .cfi_startproc
  endbr64
  subq  $8, %rsp
  .cfi_def_cfa_offset 16
  movl  $12, %esi
  leaq  _ZSt4cout(%rip), %rdi
  call  _ZNSolsEi@PLT
  movq  %rax, %rdi
  leaq  .LC0(%rip), %rsi
  call  _ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc@PLT
  movl  $0, %eax
  addq  $8, %rsp
  .cfi_def_cfa_offset 8
  ret
  .cfi_endproc
.LFE2291:
```

TLDR, the assembly prints 12, which is the number of subgraphs

## TODO

Some documentation for main.cpp.


