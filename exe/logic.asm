

	ORG	$50

* AND

	AND.B	(A1),D5
	AND.W	D2,$900
	AND.L	D2,D4

* ANDI

	ANDI.B	#$AB,$900
	ANDI.W	#$ABCD,D5
	ANDI.L	#$ABCDEF12,(A2)

* ANDI_TO_CCR

	ANDI	#$12,CCR

* ANDI_TO_SR

	ANDI	#$ABCD,SR

* OR

	OR.B	(A1),D5
	OR.W	D2,$900
	OR.L	D2,D4

* ORI

	ORI.B	#$AB,$900
	ORI.W	#$ABCD,D5
	ORI.L	#$ABCDEF12,(A2)

* ORI_TO_CCR

	ORI	#$12,CCR

* ORI_TO_SR

	ORI	#$ABCD,SR

* EOR

	EOR.B	D2,$900
	EOR.W	D3,(A2)
	EOR.L	D6,-(A1)

* EORI

	EORI.B	#$AB,D2
	EORI.W	#$ABCD,D4
	EORI.L	#$ABCDEF12,$900

* EORI_TO_CCR

	EORI	#$76,CCR

* EORI_TO_SR

	EORI	#$ABCD,SR

* NOT

	NOT.B	D4
	NOT.W	(A5)
	NOT.L	$900
