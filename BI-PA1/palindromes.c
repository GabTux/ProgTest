#include <stdio.h>
#include <assert.h>

int get_input(char *p_comm, int *p_radix, long long int *p_lo, long long int *p_hi)
{
  int res;
    
  res = scanf(" %c", p_comm);
  
  if (res == EOF)
    return 2;
    
  else if ( (res != 1) || !(*p_comm == 'c' || *p_comm == 'l') )
  {
    return 1;
  }
  
  res = scanf(" %d %lld %lld", p_radix, p_lo, p_hi);
  
  if (res == EOF)
    return 3;
  
  else if (res != 3 || *p_lo<0 || *p_hi<0 || *p_lo>*p_hi || *p_radix<2 || *p_radix>36)
  {
    return 1;
  }
  
  return 0;
}


void print_binary(long long int number)
{
  printf(" = ");
  if (number == 0)
    printf("%d", 0);
    
  while (number > 0)
  {
    printf("%lld", (number&1));
    number >>= 1;
  }
  printf("b\n");
  return;
}


int shift_bits(long long int number)
{
  int revNumber = 0;
  while (number > 0)
  {
    revNumber <<= 1;
    revNumber |= number&1;
    number >>= 1;
  }
  return revNumber;
}

char num_to_digit()
{
  return 1;
}

int to_radix(long long int number, int radix, int *p_radixNum)
{
  int i=-1;
  
  if (number==0)
  {
    p_radixNum[0]=0;
    return 0;
  }
  
  while (number>0)
  {
    ++i;
    p_radixNum[i] = number%radix;
    number /= radix;
  }
  return i;
}

void symetric_radix(char *p_comm, int *p_radix, long long int *p_lo, long long int *p_hi, int *p_radixNum, int *p_rotatedRadixNum)
{
  int counter = 0;
  
  if (*p_comm == 'c')
  {
    for (int i = *p_lo; i <= *p_hi; i++)
    {
      
      if (i==shift_bits(i))
        ++counter;
    }
    printf("Celkem: %d\n", counter);
  }
  
  else if (*p_comm == 'l')
  {
    for (int i = *p_lo; i <= *p_hi; i++)
    {
      if (i==shift_bits(i))
      {
        printf("%d", i);
        print_binary(i);
      }
    }
  }
  
  else
  {
    printf("Neco se pokazilo.\n");
  }
  
  return;
}

void symetric_bits(char *p_comm, long long int *p_lo, long long int *p_hi)
{
  int counter = 0;
  
  if (*p_comm == 'c')
  {
    for (int i = *p_lo; i <= *p_hi; i++)
    {
      if (i==shift_bits(i))
        ++counter;
    }
    printf("Celkem: %d\n", counter);
  }
  
  else if (*p_comm == 'l')
  {
    for (int i = *p_lo; i <= *p_hi; i++)
    {
      if (i==shift_bits(i))
      {
        printf("%d", i);
        print_binary(i);
      }
    }
  }
  
  else
  {
    printf("Neco se pokazilo.\n");
  }
  
  return;
}

void rotate_radix(int *p_radixNum, int *p_rotatedRadixNum, int length)
{
  for (int i = 0; i <= length; i++)
  {
    p_rotatedRadixNum[i]=p_radixNum[length-i];
  }
}

// inspired by https://codereview.stackexchange.com/questions/146288/calculate-the-number-of-palindrome-numbers-in-the-given-ranges
long long int calculate_symetric_nums(long long int number, int radix)
{
  int numLength = 0;
  long long int tempNumLength = number;
  long long int half = number;
  
  while (tempNumLength != 0)
  {
    numLength++;
    tempNumLength /= radix;
  }
  
  for (int i =0; i < numLength-(numLength+1)/2; i++)
  {
    half /= radix;
  }
  
  long long int fHalf = half;
  long long int sHalf = 0;
  long long int tempHalf = half;
  
  if (numLength % 2 == 1)
    tempHalf /= radix;
  
  while (tempHalf != 0)
  {
    int digit = tempHalf % radix;
    
    tempHalf /= radix;
    sHalf = sHalf * radix + digit;
    fHalf = fHalf * radix;
  }
  
  if (fHalf+sHalf>number)
    half--;
  
  half = half*2;
  if (numLength % 2 == 1)
  {
    long long int adj=1;
    
    for (int i = 1; i<(numLength+1)/2; i++)
      adj *= radix;
    half -= (half/2-adj+1);
  }
  else
  {
    long long int adj = 1;
    for (int i=0; i<(numLength+1)/2; i++)
      adj *= radix;
    half += (adj-half/2-1);
  }
  
  return half;
}

int main()
{
  char comm;
  long long int lo, hi;
  int radix, radixNum[64], rotatedRadixNum[64];

  
  printf("Vstupni intervaly:\n");
  
  while (1)
  {
    int res = get_input(&comm, &radix, &lo, &hi);
    
    if ( res == 1 )
    {
      printf("Nespravny vstup.\n");
      break;
    }
    
    else if (res == 2)
    {
      break;
    }
    
    else if (res == 3)
    {
      printf("Nespravny vstup.\n");
      break;
    }
    
    int Equal = 1;
  
    if (comm == 'c')
    {
      
        printf("Celkem: %lld\n", calculate_symetric_nums(hi, radix)-calculate_symetric_nums(lo-1, radix));
      
    }
  
    else if (comm == 'l')
    {
      for (int i = lo; i <= hi; i++)
      {
        int length = to_radix(i, radix, radixNum);
        rotate_radix(radixNum, rotatedRadixNum, length);
        Equal=1;
        for (int j = 0; j < length; j++)
        {
          if (radixNum[j]!=rotatedRadixNum[j])
          {
            Equal=0;
          }
        }
          
        if (Equal)
        {
          printf("%d = ", i);
          for (int f = 0; f <= length; f++)
          {
            if (radixNum[f]<=9)
              printf("%d", radixNum[f]);
            else
              printf("%c", radixNum[f] - 10 + 'a');
          }
          
          printf(" (%d)\n", radix);
        }
        
      }
    }
  }
  
  return 0;
}
