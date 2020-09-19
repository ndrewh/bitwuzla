(set-logic QF_BV)
(declare-fun s () (_ BitVec 64))
(declare-fun t () (_ BitVec 64))
(assert (not (= (bvashr s t) (ite (= ((_ extract 63 63) s) (_ bv0 1)) (bvlshr s t) (bvnot (bvlshr (bvnot s) t))))))
(check-sat)
(exit)
