#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct Phrase
{
  double count;
  char *pPhrase;
} TPHRASE;


void stripNL( char *s )
{
  int len = strlen(s);
  if (len > 0 && s[len-1] == '\n')
    s[len-1] = 0;
}

TPHRASE * readInput( int *pCount )
{
  TPHRASE * phrases = NULL;
  char * line = NULL;
  size_t len = 0;
  ssize_t lineLen;
  int max = 0;
  int phrStart;
  int phrLen;
  char trash;
  *pCount = 0;
  
  
  while ((lineLen = getline(&line, &len, stdin)) != -1)
  {
    if (*line == '\n')
      break;
    
    if (*pCount >= max)
    {
      max += (max < 96) ? 12 : max / 2;
      phrases = (TPHRASE *) realloc ( phrases, max * sizeof ( *phrases ) );
    }
    
    int ress = sscanf(line, "%lf%c%n", &(phrases[*pCount] . count), &trash, &phrStart);
    if (ress != 2 || trash != ':')
    {
      free(line);
      for (int i = 0; i < *pCount; i++)
        free(phrases[i] . pPhrase);
      free(phrases);
      return NULL;
    }
    
    phrLen = lineLen-phrStart;
    phrases[*pCount] . pPhrase = (char *) malloc (phrLen * sizeof (char) + 1);
    strcpy(phrases[*pCount] . pPhrase, (line+phrStart));
    free(line);
    line = NULL;
    stripNL( phrases[*pCount] . pPhrase );
    (*pCount)++;
  }
  
  free(line);
  
  if ( feof( stdin ) )
  {
    for (int i = 0; i < *pCount; i++)
      free(phrases[i] . pPhrase);
    free(phrases);
    return NULL;
  }
  
  return phrases;
}

char * readText()
{
  char * pSearch = NULL;
  size_t len = 0;
  
  if (getline(&pSearch, &len, stdin) == -1)
  {
    free(pSearch);
    return NULL;
  }
  
  stripNL(pSearch);
  return pSearch;
}

int strContains(char *s, char *p)
{
  int lenS = strlen(s);
  int lenP = strlen(p);
  int match = 0;
  
  for (int i = 0; i < lenS-lenP+1; i++)
  {
    match = 0;
    for (int j = 0; j < lenP; j++)
    {
      if (toupper( s[i+j] ) == toupper( p[j] ))
      {
        match++;
      }
    }
    if (match == lenP)
      return 1;
  }
  return 0;
}

int findText( TPHRASE * phrases, char * pSearch, int phrCount )
{
  int hits = 0;
  char **resPhr;
  resPhr = (char **) malloc(phrCount * sizeof(*resPhr));
  
  for (int i = 0; i < phrCount; i++)
  {
    if ( strContains( phrases[i] . pPhrase, pSearch ) )
    {
      resPhr[hits] = phrases[i] . pPhrase;
      hits++;
    }
  }
  
  printf("Nalezeno: %d\n", hits);
  for (int i = 0; i < hits; i++)
  {
    printf("> %s\n", resPhr[i]);
  }
  
  free(resPhr);
  
  return 0;
}

int sortPhrases( double *a, double *b )
{
  if (*a > *b)
    return -1;
  else if (*a < *b)
    return 1;
  return 0;
}

int main()
{
  TPHRASE * phrases;
  int phrCount = 0;
  char *pSearch = NULL;
  
  printf("Casto hledane fraze:\n");
  phrases = readInput( &phrCount );
  
  if ( ! phrases )
  {
    printf("Nespravny vstup.\n");
    return 1;
  }
  
  qsort ( phrases, phrCount, sizeof ( *phrases ), (int(*)(const void *, const void *))sortPhrases );
  
  printf("Hledani:\n");
  
  while ( 1 )
  {
    pSearch = readText();
    if ( ! pSearch )
      break;
    
    findText( phrases, pSearch, phrCount );
    free(pSearch);
    pSearch = NULL;
  }
  
  free(pSearch);
  for (int i = 0; i < phrCount; i++)
    free( phrases[i] . pPhrase );
  free(phrases);
  
  return 0;
}
