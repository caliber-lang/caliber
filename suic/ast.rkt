#lang racket

(provide (all-defined-out))

;; types
(struct type-def (name fields) #:transparent)
(struct type-alias (name body) #:transparent)

;; expressions
(struct lit (value) #:transparent)              ; 42, "hello"
(struct var (name) #:transparent)               ; x
(struct anchor (name) #:transparent)            ; @user
(struct binop (op left right) #:transparent)    ; x + y
(struct call (func args) #:transparent)         ; func(arg)
(struct if-expr (cond then else) #:transparent)
(struct match-expr (expr cases) #:transparent)

;; statements
(struct var-bind (name init) #:transparent)     ; var x <- 5
(struct anchor-bind (name type init) #:transparent)  ; @x <- alloc T
(struct mutation (anchor field value) #:transparent) ; @x.f := v
(struct func-call (name args) #:transparent)

;; definitions
(struct func-def (name params body) #:transparent)
(struct data-def (name fields) #:transparent)

;; program
(struct program (defs) #:transparent)
