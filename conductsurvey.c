/*--------------------------------------------------------------------*/
/* conductsurvey.c                                                    */
/* Authors: Iasonas Petras and Bob Dondero                            */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>
 
/*--------------------------------------------------------------------*/

/* Use the current time to compute and return the current academic
   year. */
   
static int getCurrAcademicYear(void)
{
   enum {MAX_YEAR_LENGTH = 5};
   enum {MAX_MONTH_LENGTH = 3};
   
   char acYear[MAX_YEAR_LENGTH];
   char acMonth[MAX_MONTH_LENGTH];
   int iYear;
   int iMonth;
   time_t iTime;
   struct tm *psTime;
   
   iTime = time(NULL);
   psTime = localtime(&iTime);
   
   (void)strftime(acYear, sizeof(acYear), "%Y", psTime);
   sscanf(acYear, "%d", &iYear);
   
   (void)strftime(acMonth, sizeof(acMonth), "%m", psTime);
   sscanf(acMonth, "%d", &iMonth);
   if (iMonth >= 8)
      iYear++;
   
   return iYear; 
}

/*--------------------------------------------------------------------*/

/* Return 1 (TRUE) iff pcAnswer is a valid expected graduation year. */

static int isValidYear(const char *pcAnswer)
{
   int iCurrAcademicYear; 
   int iAnswer;
   
   assert(pcAnswer != NULL);
   
   if (strcmp(pcAnswer, "N/A") == 0)
      return 1;
   
   iCurrAcademicYear = getCurrAcademicYear(); 
   iAnswer = 0;
   
   if ((strlen(pcAnswer) != 4)
      || (sscanf(pcAnswer, "%d", &iAnswer) != 1)
      || (iAnswer < iCurrAcademicYear) 
      || (iAnswer > iCurrAcademicYear+4))
      return 0;

   return 1;
}

/*--------------------------------------------------------------------*/

/* 
   Return 1 (TRUE) iff pcAnswer is a valid course code. 
   There are some codes that are not valid concentrations.
*/

static int isValidAffiliation(const char *pcAnswer)
{ 
   const char* apcAffiliations[] = {
      "AAS", "AFS", "AMS", "ANT", "AOS", "APC", "ARA", "ARC", "ART",
      "ASA", "AST", "ATL", "BCS", "CBE", "CEE", "CGS", "CHI", "CHM",
      "CHV", "CLA", "CLG", "COM", "COS", "CTL", "CWR", "CZE", "DAN",
      "EAS", "ECO", "ECS", "EEB", "EGR", "ELE", "ENE", "ENG", "ENT",
      "ENV", "EPS", "FIN", "FRE", "FRS", "GEO", "GER", "GHP", "GLS",
      "GSS", "HEB", "HIN", "HIS", "HLS", "HOS", "HPD", "HUM", "ISC",
      "ITA", "JDS", "JPN", "JRN", "KOR", "LAO", "LAS", "LAT", "LCA",
      "LIN", "MAE", "MAT", "MED", "MOD", "MOG", "MOL", "MPP", "MSE",
      "MTD", "MUS", "NES", "NEU", "ORF", "PAW", "PER", "PHI", "PHY",
      "PLS", "POL", "POP", "POR", "PSY", "QCB", "REL", "RES", "RUS",
      "SAN", "SAS", "SLA", "SML", "SOC", "SPA", "SPI", "STC", "SWA",
      "THR", "TPP", "TRA", "TUR", "TWI", "URB", "URD", "VIS", "WRI", 
      "undeclared", "other", "N/A", NULL};
   int i = 0;
 
   assert(pcAnswer != NULL);

   while (apcAffiliations[i] != NULL)
   {
      if (strcmp(pcAnswer, apcAffiliations[i]) == 0)
         return 1;
      i++;
   }
   return 0;
}

/*--------------------------------------------------------------------*/

/* Return 1 (TRUE) iff pcAnswer is a valid degree. */

static int isValidDegree(const char *pcAnswer)
{
   const char *apcDegrees[] =
      {"AB", "BSE", "grad", "HS", "N/A", NULL};
   int i = 0;
  
   assert(pcAnswer != NULL);   

   while (apcDegrees[i] != NULL)
   {
      if (strcmp(pcAnswer, apcDegrees[i]) == 0)
         return 1;
      i++;
   }
   return 0;
}

/*--------------------------------------------------------------------*/

/* Return 1 (TRUE) iff pcAnswer is a valid numeric rating. */

static int isValidRating(const char *pcAnswer)
{
   const char *apcRatings[] = {"0", "1", "2", "3", "4", "5", NULL};
   int i = 0;
   
   assert(pcAnswer != NULL);   

   while (apcRatings[i] != NULL)
   {
      if (strcmp(pcAnswer, apcRatings[i]) == 0)
         return 1;
      i++;
   }
   return 0;
}

/*--------------------------------------------------------------------*/

/* In string pc replace '\n' with '\0'. */

static void stripNewline(char *pc)
{
   assert(pc != NULL);
   
   while (*pc != '\0')
   {
      if (*pc == '\n')
      {
         *pc = '\0';
         return;
      }
      pc++;
   }
}

/*--------------------------------------------------------------------*/

/* Write the string pcQuestion to stdout. Read a string answer
   from stdin. Repeat until the answer is valid as defined by 
   *pfIsValid. Then write string pcQuestion and the answer to
   psSurveyFile. */
   
static void handleQuestion(const char *pcQuestion,
   int (*pfIsValid)(const char *pcAnswer),
   FILE *psSurveyFile)
{   
   enum {MAX_ANSWER_SIZE = 1000};  /* Too large, but safe. */
   
   char acAnswer[MAX_ANSWER_SIZE];
   
   static int iQuestionNum = 1;
   
   assert(pcQuestion != NULL);
   assert(pfIsValid != NULL);
   assert(psSurveyFile != NULL);
   
   printf("(%d) %s\n", iQuestionNum, pcQuestion);
   (void)fgets(acAnswer, MAX_ANSWER_SIZE, stdin);
   stripNewline(acAnswer);
   while (! (*pfIsValid)(acAnswer))
   {
      printf("Please try again.\n");
      printf("%s\n", pcQuestion);
      (void)fgets(acAnswer, MAX_ANSWER_SIZE, stdin);
      stripNewline(acAnswer);
   }
   
   iQuestionNum++;
   
   fprintf(psSurveyFile, "%s\n", pcQuestion);
   fprintf(psSurveyFile, "%s\n", acAnswer);
}

/*--------------------------------------------------------------------*/

/* Conduct an introductory survey of COS 217 students. Write questions
   to stdout, read answers from stdin, and write questions and answers
   to a file named survey. As usual argc is the command-line argument
   count and argv contains command-line arguments. Return 0 if
   successful, and EXIT_FAILURE otherwise. */

int main(int argc, char *argv[])
{       
   enum {QUESTION_COUNT = 34};
   
   FILE *psSurveyFile;

   /* Validate command-line arguments. */
   
   if (argc != 1)
   {
      fprintf(stderr, "Usage: %s\n", argv[0]);
      exit(EXIT_FAILURE);
   }
   
   /* Open the survey file. */
   
   psSurveyFile = fopen("survey", "w");
   if (psSurveyFile == NULL)
   {
      fprintf(stderr, "Failed to open survey file.\n");
      exit(EXIT_FAILURE);
   }  
   
   /* Conduct the survey, writing the questions and answers
      to the survey file. */
   
   printf("Welcome to the Introductory Survey for COS 217.\n");

   printf("\n");

   fprintf(psSurveyFile, "Student's username\n");
   fprintf(psSurveyFile, "%s\n", getlogin());

   printf("There are %d questions.\n", QUESTION_COUNT);

   printf("\n");
   
   handleQuestion(
      "What is your concentration (or program, affiliation, etc.)?"
      "(COS, MAT, PHI, URB, ..., undeclared, other, N/A)?",
      isValidAffiliation, psSurveyFile);

   printf("\n");

   handleQuestion(
      "What degree are you pursuing (AB, BSE, grad, HS, N/A)?",
      isValidDegree, psSurveyFile); 

   printf("\n");

   handleQuestion(
      "What is your expected graduation year (4 digits, N/A)?",
      isValidYear, psSurveyFile);

   printf("\n");

   printf("State your expertise in each topic. Use a 5-point scale\n");
   printf("where 5 means \"I know this topic very well\" and 0\n");
   printf("means \"I know nothing about this topic\". Please enter");
   printf("only integers (no decimal points).\n");

   printf("\n");
   printf("Topic 1: Number systems\n");
   printf("\n");

   handleQuestion(
      "Binary number system (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "Hexadecimal number system (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "Representation of signed integers (two's comp notation) (0-5)?",
     isValidRating, psSurveyFile);

   printf("\n");
   printf("Topic 2: Linux operating system\n");
   printf("\n");

   handleQuestion(
      "Linux operating system, in general (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "Fundamental commands (cd, ls, cat, etc.) (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "Redirection (< and >) and pipes ( | ) (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "Background processes via Ctrl-z (0-5)?",
      isValidRating, psSurveyFile);
   
   printf("\n");
   printf("Topic 3: GNU Programming Environment\n");
   printf("\n");

   handleQuestion(
      "GNU programming environment, in general (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "The emacs editor (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "The gcc compiler driver (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "The gdb debugger (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "The make project maintenance tool (0-5)?",
      isValidRating, psSurveyFile);
  
   printf("\n");
   printf("Topic 4: Java programming language\n");
   printf("\n");
   
   handleQuestion(
      "Java programming language, in general (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "Java control stmts (if, switch, for, while, do, break) (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(  
      "Java methods (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "Java arrays (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "Java classes (0-5)?",
      isValidRating, psSurveyFile);

   printf("\n");
   printf("Topic 5: C programming language\n");
   printf("\n");
   
   handleQuestion(
      "C programming language, in general (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "C control stmts (if, switch, for, while, do, break) (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "C functions (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "C arrays (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "C structures (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "C preprocessor directives (#include, #define, etc.) (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "C interface (.h) files (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "C pointers and pointer operators (* and &) (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "C dynamic memory mgmt (malloc, calloc, realloc, free) (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "C void pointers (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "C function pointers (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "C abstract data types (ADTs) (0-5)?",
      isValidRating, psSurveyFile);
   
   printf("\n");
   printf("Topic 6: ARMv8 architecture and assembly language\n");   
   printf("\n");

   handleQuestion(
      "ARMv8 architecture (0-5)?",
      isValidRating, psSurveyFile);
   handleQuestion(
      "ARMv8 assembly language (0-5)?",
      isValidRating, psSurveyFile);
   
   printf("\n");
      
   fclose(psSurveyFile);
   
   printf("Congratulations on completing the Introductory Survey.\n");
   return 0;   
}
