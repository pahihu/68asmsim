

	ORG	$50

* MOVE
	MOVE.B	#$ea,D7
	MOVE.W	(A1),D0
	MOVE.L	A7,D4

* MOVEA

	MOVEA.W	#$7FFF,A0
	MOVEA.L	A1,A5

* MOVE_TO_CCR

	MOVE	D2,CCR

* MOVE_FR_SR

	MOVE	SR,D7

* MOVE_TO_SR

	MOVE	D1,SR

* MOVE_USP

	MOVE	A7,A0
	MOVE	A0,A7

* MOVEC

	MOVEC	SFC,D1
	MOVEC	D1,DFC
	MOVEC	D0,USP
	MOVEC	VBR,D0

* MOVEM

	MOVEM	D0-D1/A0,-(A1)
	MOVEM	(A0),D0-D2/A0

* MOVEP

	MOVEP.W	D0,10(A1)
	MOVEP.L	10(A1),D0

* MOVEQ

	MOVEQ	#$58,D7

* EXG

	EXG	D1,D2
	EXG	D7,A1
	EXG	A4,A6

* LEA

	LEA	(A1),A0

* PEA

	PEA	(A1)

* LINK

	LINK	A0,#-$12

* UNLK

	UNLK	A4

