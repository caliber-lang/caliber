#lang racket

(require "lexer.rkt")

;; test tokenization
(define test-sui
  "def main =
     var x <- 5
     print x")

;; convert string to input port
(define (tokenize-string str)
  (tokenize (open-input-string str)))

;; run test
(let loop ([tok (tokenize-string test-sui)])
  (unless (eq? (token-type tok) 'eof)
    (printf "~a: ~a~n" (token-type tok) (token-value tok))
    (loop (tokenize-string test-sui))))
