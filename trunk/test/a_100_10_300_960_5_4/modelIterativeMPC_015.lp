/* Objective function */
min: +0.12972972973 C22 +0.0162162162162 C23 +0.135135135135 C24 +0.2 C26;

/* Constraints */
+C1 +C2 +C3 = 1;
+C5 +C6 +C7 = 1;
+C9 +C10 +C11 +C12 +C13 = 1;
+C15 +C16 = 1;
+C18 +C19 +C20 = 1;
+C3 +C10 -C26 <= 1;
+C9 +C15 -C22 <= 1;
+C2 +C19 -C25 <= 1;
+C9 +C18 -C23 <= 1;
+C13 +C20 -C23 <= 1;
+C15 +C18 -C24 <= 1;
0 C1 <= 0;

/* Variable bounds */
C1 <= 1;
C2 = 1;
C3 <= 1;
C5 = 1;
C6 <= 1;
C7 <= 1;
C9 <= 1;
C10 = 1;
C11 <= 1;
C12 <= 1;
C13 <= 1;
C15 <= 1;
C16 = 1;
C18 = 1;
C19 <= 1;
C20 <= 1;
C22 <= 1;
C23 <= 1;
C24 <= 1;
C25 <= 1;
C26 <= 1;

/* Integer definitions */
int C1,C2,C3,C5,C6,C7,C9,C10,C11,C12,C13,C15,C16,C18,C19,C20,C22,C23,C24,C25,C26;

/* SOS definitions */
SOS
SOS: C1,C2,C3 <= 1;
SOS: C5,C6,C7 <= 1:1;
SOS: C9,C10,C11,C12,C13 <= 1:1;
SOS: C15,C16 <= 1:1;
SOS: C18,C19,C20 <= 1:1;
SOS: C6,C12 <= 1:1;
