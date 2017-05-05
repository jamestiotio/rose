//Test case for nodal accumulation pattern
//
//

void foo(double* x, int jp, int kp, int begin, int end, double rh1)
{
   //Condition 1: pointer declaration, 4 or 8 pointers
   double * x1, *x2, *x3, *x4; 

   //Condition 2:  pointer initialization, using other pointers on rhs
   x1 = x;
   x2 = x +1; 
   x3 = x1 + jp; 
   x4 = x1 + kp; 

   //Condition 3:  A regular loop or a RAJA loop
   for (int i = begin; i< end; i++)
   {
      // Condition 4: accumulation pattern: lhs accum-op rhs
      // lhs : array element access x[i]: x is pointer type, i is loop index 
      // rhs: a scalar double type
      // accum-op:   +=, -=, *=, /=, MIN (), MAX() 
      x1[i] += rh1; 
      x2[i] -= rh1; 
      x3[i] *= rh1; 
      x4[i] /= rh1; 
   } 
}
