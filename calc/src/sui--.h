#ifndef SUI_IR_H
#define SUI_IR_H

#include "ast.h"
#include <stdio.h>

void sui_emit(node_t *program, FILE *out);

#endif
