#lang racket

(require parser-tools/lex
         (prefix-in : parser-tools/lex-sre))

(provide tokenize)

(define-tokens basic-tokens (ID INT STRING KEYWORD))
(define-empty-tokens delims (
  LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
  DOT COLON COLONEQUAL ARROW PIPE COMMA
  AT COLONMINUS EOF))

(define tokenize
  (lexer
    [(eof) (token-EOF)]
    
    ;; whitespace
    [(:+ whitespace) (tokenize input-port)]
    
    ;; comments
    ["--" (:* (char-complement #\newline)) (tokenize input-port)]
    
    ;; keywords
    [(:or "def" "data" "var" "if" "then" "else" "match" "with"
          "import" "type" "do" "for" "while" "let") 
     (token-KEYWORD lexeme)]
    
    ;; identifiers & operators
    [(:+ (:or alphanumeric "_")) (token-ID lexeme)]
    
    ;; numbers
    [(:+ digit) (token-INT (string->number lexeme))]
    
    ;; strings
    [#\" (:* (char-complement #\")) #\"
     (token-STRING lexeme)]
    
    ;; symbols
    [#\@ (token-AT)]
    [#\: (token-COLON)]
    [":=" (token-COLONEQUAL)]
    ["<-" (token-COLONMINUS)]
    ["->" (token-ARROW)]
    [#\. (token-DOT)]
    [#\{ (token-LBRACE)]
    [#\} (token-RBRACE)]
    [#\[ (token-LBRACKET)]
    [#\] (token-RBRACKET)]
    [#\( (token-LPAREN)]
    [#\) (token-RPAREN)]
    [#\, (token-COMMA)]
    [#\| (token-PIPE)]
    
    ;; operators
    [(:or "+" "-" "*" "/" "==" "!=" "<" ">" "<=" ">=")
     (token-ID lexeme)]
))
