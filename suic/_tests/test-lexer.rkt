#lang racket

(require "../lexer.rkt")

(define test-sui
  "def main =
     var x <- 5
     print x")

(display (tokenize test-sui))
