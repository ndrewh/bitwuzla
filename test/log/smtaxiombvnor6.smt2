(set-logic QF_BV)
(declare-fun s () (_ BitVec 6))
(declare-fun t () (_ BitVec 6))
(assert (not (= (bvnor s t) (bvnot (bvor s t)))))
(check-sat)
(exit)
