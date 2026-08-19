project('suic', 'c',
  version : '0.1.0',
  default_options : ['warning_level=3', 'c_std=c99'])

add_project_arguments('-D_POSIX_C_SOURCE=200809L', language : 'c')

sources = [
  'src/main.c',
  'src/lexer.c',
  'src/ast.c',
  'src/parser.c',
  'src/typetab.c',
  'src/codegen.c',
  'src/utils.c',
  'src/sui--.c',
  'src/sui---parse.c',
  'src/x86_64.c',
]

executable('suic', sources,
  install : true)
