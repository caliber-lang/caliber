#lang racket

(require "lexer.rkt" "ast.rkt")

(provide parse)

(define (parse tokens)
  (let ([toks (tokenize-file tokens)])
    (parse-program toks)))

(define (parse-program tokens)
  (let loop ([toks tokens] [defs '()])
    (cond
      [(null? toks) (program (reverse defs))]
      
      ;; data Type = { fields }
      [(eq? (token-e toks) 'KEYWORD) 
       (if (equal? (token-v (car toks)) "data")
         (let-values ([(def rest) (parse-data-def toks)])
           (loop rest (cons def defs)))
         ;; def name = body
         (let-values ([(def rest) (parse-func-def toks)])
           (loop rest (cons def defs))))]
      
      [else (error "unexpected token" (car toks))])))

(define (parse-data-def toks)
  (match toks
    [`((KEYWORD "data") (ID ,name) (LBRACE) . ,rest)
     ;; prse fields
     (let-values ([(fields rest) (parse-fields rest)])
       (values (data-def name fields) rest))]))

(define (parse-func-def toks)
  (match toks
    [`((KEYWORD "def") (ID ,name) . ,rest)
     ;; parse params and body
     (let-values ([(params rest) (parse-params rest)])
       (match rest
         [`((COLONEQUAL) . ,body-toks)
          (let-values ([(body rest) (parse-body body-toks)])
            (values (func-def name params body) rest))]))]))

;; stubs for now
(define (parse-fields toks) (values '() toks))
(define (parse-params toks) (values '() toks))
(define (parse-body toks) (values '() toks))
