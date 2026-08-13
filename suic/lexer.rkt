#lang racket

(require parser-tools/lex
         (prefix-in : parser-tools/lex-sre))

(provide tokenize)

(define-tokens basic-tokens (id int string keyword))
(define-empty-tokens delims (
  lparen rparen lbrace rbrace lbracket rbracket
  dot colon colonequal arrow pipe comma
  at colonminus eof))

(define tokenize
  (lexer
    [(eof) (token-eof)]
    
    ;; whitespace
    [(:+ whitespace) (tokenize input-port)]
    
    ;; comments
    [(:seq ";" (:* (char-complement #\newline))) (tokenize input-port)]
    
    ;; keywords
    [(:or "def" "data" "var" "if" "then" "else" "match" "with"
          "import" "type" "do" "for" "while" "let") 
     (token-keyword lexeme)]
    
    ;; identifiers
    [(:+ (:or alphanumeric "_")) (token-id lexeme)]
    
    ;; numbers
    [(:+ digit) (token-int (string->number lexeme))]
    
    ;; strings
    [(:seq #\" (:* (char-complement #\")) #\")
     (token-string lexeme)]
    
    ;; symbols
    [#\@ (token-at)]
    [":" (token-colon)]
    [":=" (token-colonequal)]
    ["<-" (token-colonminus)]
    ["->" (token-arrow)]
    [#\. (token-dot)]
    [#\{ (token-lbrace)]
    [#\} (token-rbrace)]
    [#\[ (token-lbracket)]
    [#\] (token-rbracket)]
    [#\( (token-lparen)]
    [#\) (token-rparen)]
    [#\, (token-comma)]
    [#\| (token-pipe)]
    
    ;; operators
    [(:or "+" "-" "*" "/" "==" "!=" "<" ">" "<=" ">=")
     (token-id lexeme)]
))
