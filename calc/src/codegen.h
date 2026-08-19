#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

void codegen_generate(node_t *program, const char *out_path);

#endif
