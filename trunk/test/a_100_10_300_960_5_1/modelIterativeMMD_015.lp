/* Objective function */
max: +C6;

/* Constraints */
+C1 +C2 +C3 +C4 = 1;

/* Variable bounds */
C1 <= 1;
C2 = 1;
C3 <= 1;
C4 <= 1;

/* Integer definitions */
int C1,C2,C3,C4;

/* SOS definitions */
SOS
SOS: C1,C2,C3,C4 <= 1;
