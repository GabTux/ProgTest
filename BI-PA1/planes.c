#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#define EPS 1e-10


typedef struct Plane
{
  double x;
  double y;
  char *pName;
} TPLANE;


void stripNL( char *s )
{
  int len = strlen(s);
  if (len > 0 && s[len-1] == '\n')
    s[len-1] = 0;
}


TPLANE * readInput(int *pCount)
{
  TPLANE * planes = NULL;
  char * line = NULL;
  size_t len = 0;
  ssize_t res;
  int max = 0;
  int nameStart;
  char trash;
  
  *pCount = 0;
  
  while ((res = getline(&line, &len, stdin)) != -1)
  {
    if (*pCount >= max)
    {
      max += (max < 96) ? 12 : max / 2;
      planes = (TPLANE *) realloc ( planes, max * sizeof ( *planes ) );
    }
    
    int ress = sscanf(line, " [ %lf , %lf ]%c %n", &(planes[*pCount] . x), &(planes[*pCount] . y), &trash, &nameStart);
    if (ress != 3)
    {
      free(line);
      for (int i = 0; i < *pCount; i++)
      {
        free(planes[i] . pName);
      }
      free(planes);
      return NULL;
    }
    
    planes[*pCount] . pName = (char *) malloc (res * sizeof (char) + 1);
    
    strncpy(planes[*pCount] . pName, (line+nameStart), res+1);
    free(line);
    line = NULL;
    stripNL(planes[*pCount] . pName);
    (*pCount)++;
  }
  
  free(line); 
  return planes;
}

double distance(double ax, double ay, double bx, double by)
{
  return sqrt((ax-bx)*(ax-bx)+(ay-by)*(ay-by));
}

int almostEqual(double x, double y)
{
  return fabs(x - y) <= EPS  * (fabs(x)+fabs(y));
}

void findPlanesQuadratic(TPLANE * planes, int planeCount)
{
  double min = distance( planes[0] . x, planes[0] .y , planes[1] . x, planes[1] . y);
  double actMin;
  
  for (int i = 0; i < planeCount; i++)
  {
    for (int j = i+1; j < planeCount; j++)
    {
      actMin = distance(planes[i] . x, planes[i] .y , planes[j] . x, planes[j] . y);
      if (actMin < min)
      {
        min = actMin;
      }
    }
  }
  
  printf("Nejmensi vzdalenost: %lf\n", min);
  
  for (int i = 0; i < planeCount; i++)
  {
    for (int j = i+1; j < planeCount; j++)
    {
      actMin = distance(planes[i] . x, planes[i] .y , planes[j] . x, planes[j] . y);
      if (almostEqual(actMin, min))
      {
        printf("%s - %s\n", planes[i] . pName, planes[j] . pName);
      }
    }
  }
  
}

int main()
{
  TPLANE * planes;
  int planeCount = 0;
  
  
  printf("Zadejte lety:\n");
  planes = readInput(&planeCount);
  
  if ( ! planes || planeCount < 2)
  {
    printf("Nespravny vstup.\n");
    return 1;
  }
  
  findPlanesQuadratic(planes, planeCount);
  
  for (int i = 0; i < planeCount; i++)
  {
    free(planes[i] . pName);
  }
  free(planes);
  return 0;
}
