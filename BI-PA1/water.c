#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
 * This program is divided to three segments
 * 
 * 1) READ INPUT
 *   - use double sized array for reservoirs
 *   - [0] - the altitude of reservoir
 *   - [1] - the height of reservoir
 *   - [2] - is the 2D content of reservoir
 * 
 * STRUCTURE:
 * ReservoirNum    1    2     3    4
 *        [0]  -   A |  A  |  A  | A
 *        [1]  -   H |  H  |  H  | H
 *        [2]  - W*D | W*D | W*D | W*D 
 * 
 * 
 * 2) GENERATE MAP
 *   - index represent metres of height
 *   - value of index represents sum of Volume at this height
 * example: map[0] = 5 --> from bottom bound to (bottom bound + 1 meter), is Volume of reservoirs 5L
 * 
 * 
 * 3) CALCULATE WATER HEIGHT
 *   - go throuht whole map
 *   - Water Volume = Water Volume - Volume of reservoirs at this height
 *   - check if water will still fill whole segment
 *   - if not, divide the rest, to know the height of water
 * 
 * EXAMPLE
 *  - two Reservoirs
 *  - -1 5 5 1
 *  - 1 4 4 1
 *  - water Volume is 6
 *   
 *   min(A)=-1
 *   max(A+H)=5
 *   the map will be 5-(-1)=6 indexes long
 * 
 *  map will look like this:
 *   map[5] = 4
 *   map[4] = 4+5=9
 *   map[3] = 4+5=9
 *   map[2] = 4+5=9
 *   map[1] = 5
 *   map[0] = 5
 * 
 * calculation:
 *   at first meter the water will fit to the whole segments
 *   water Volume = 6 - 5 = 1
 *   at second meter it will not fit (1<5)
 *   divide it to know the rest: 1/5=0.2
 *   so the final answer is: min(A)+index in map(in this ex. it is 1)+0.2
 * 
 * NOTE
 *   - this algorithm is quite fast, because its complexity is Linear
 *   - It will be useless, when the reservoirs will be small but, there will be huge gap between them
 *   - in better case, there will be a lot of zeros
 *   - in worst scenario, the size of array will not be enough
 *   - so idea for effectivity is, to get rid of spaces
 */

#define MAX_ARRAY 200001
#define true 1
#define false 0


/*---------------------------------------------------------------------------*/
/** Function will save input to array and determine min(A) and max(A+H)
 * 
 *
 * @param[out] arr there will be saved input
 * @param[in] len how much reservoirs it will read
 * @param[out] p_maxA_H there will be saved the max(A+H)
 * @param[out] p_minA there will be saved the min(A)
 * @return =false failed, =true ok
 */
int readArray(int arr[3][MAX_ARRAY], int len, int *p_minA, int *p_maxA_H)
{
  int Alt, H, W, D;
  
  printf("Zadejte parametry nadrzi:\n");
  
  for (int i = 0; i < len; i++)
  {
    if ( scanf(" %d %d %d %d", &Alt, &H, &W, &D) != 4 
         || H<=0
         || W<=0
         || D<=0 )
    {
      return false;
    }
    if (i==0)
    {
      *p_minA = Alt;
      *p_maxA_H = Alt+H;
    }
    arr[0][i] = Alt;
    arr[1][i] = H;
    arr[2][i] = W*D;
    
    // find bottom bound && top bound
    if (arr[0][i]<*p_minA)
      *p_minA = arr[0][i];
    if ((arr[1][i]+arr[0][i])>*p_maxA_H)
      *p_maxA_H = arr[1][i]+arr[0][i];
  }
  
  return true;
}
/*---------------------------------------------------------------------------*/
/** Function will initialize the array to zeros
 * 
 *
 * @param[out] arr this will be initialized
 * @param[in] len the legth of array
 */
void nullArr(int arr[], int len)
{
  for (int i =0; i < len; i++)
    arr[i] = 0;
}
/*---------------------------------------------------------------------------*/
/** Function will generate map of reservoirs
 * 
 *
 * @param[in] Reservoirs array of reservoirs
 * @param[out] ReservoirMap array where should be map saved
 * @param[in] ReservoirsCount number of reservoirs
 * @param[in] minA the bottom bound of map
 * @param[in] maxA_H the top bound of map
 */
void generateMap(int Reservoirs[3][MAX_ARRAY], int ReservoirMap[], int ReservoirsCount, int minA, int maxA_H)
{
  /* a is index in map
   * b is index in reservoir
   */
  for (int a = 0; a < ReservoirsCount; a++)
    for (int b = 0; b < Reservoirs[1][a]; b++)
      ReservoirMap[abs(minA-Reservoirs[0][a])+b] += Reservoirs[2][a];
}
/*---------------------------------------------------------------------------*/
/** Function will calculate the water height
 * 
 *
 * @param[in] ReservoirMap map of reservoirs
 * @param[in] ReservoirMap array where should be map saved
 * @param[in] waterVol the volume of water
 * @param[in] minA the bottom bound of map
 * @param[in] maxA_H the top bound of map
 */
void calculateWaterHeight(int ReservoirMap[], int waterVol, int minA, int maxA_H)
{
  for (int i = 0; i < maxA_H-minA; i++)
  {
    if (waterVol-ReservoirMap[i]==0)
    {
      printf("h = %lf\n", (double)minA+i+1);
      return;
    }
    
    else if (waterVol-ReservoirMap[i]<0)
    {
      printf("h = %lf\n", (double)minA+i+((double)waterVol/ReservoirMap[i]));
      return;
    }
    waterVol -= ReservoirMap[i];
  }
  printf("Pretece.\n");
  return;
}
/*---------------------------------------------------------------------------*/
/** Function for wrong input
 * 
 * @return =false
 */
int wrongInput()
{
  printf("Nespravny vstup.\n");
  return EXIT_FAILURE;
}
/*---------------------------------------------------------------------------*/
int main()
{
  int Reservoirs[3][MAX_ARRAY];
  int ReservoirMap[MAX_ARRAY];
  int ReservoirsCount = 0;
  int minA, maxA_H;
  int waterVol;
  
  printf("Zadejte pocet nadrzi:\n");
  if (scanf(" %d", &ReservoirsCount) != 1 || ReservoirsCount <= 0 || ReservoirsCount >= MAX_ARRAY)
    return wrongInput();
    
  if ( ! readArray(Reservoirs, ReservoirsCount, &minA, &maxA_H) )
    return wrongInput();
  
  nullArr(ReservoirMap, maxA_H-minA);
  generateMap(Reservoirs, ReservoirMap, ReservoirsCount, minA, maxA_H);
  
  printf("Zadejte objem vody:\n");
  while ( scanf(" %d", &waterVol) == 1 && waterVol >= 0 )
  {
    if (waterVol == 0)
      printf("Prazdne.\n");
    else
      calculateWaterHeight(ReservoirMap, waterVol, minA, maxA_H);
  }
  
  if ( ! feof(stdin) )
    return wrongInput();
  
  return 0;
}
