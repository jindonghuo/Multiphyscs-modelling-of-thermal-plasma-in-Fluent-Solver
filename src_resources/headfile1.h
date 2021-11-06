#include <stdio.h>
#include <stdlib.h>
#define CONSOLE 0
#define NumberOfColumns 10
#define NumberOfArguments 4
#define NumberOfConcentration 2
#define STRL 30
#define ARSTRL 10
#define TALC 100000
#define P1_ADD 1000

typedef struct
{
 void *parent;
 void *child;
 int NumberOfColums;
 int NumberOfChild;
 int NumberOfVariation;
 int StartLineInTable;
 int IfAlloc;
 int NumberOfSteps;
}ENTRANCE;
typedef struct
{
	double Y[NumberOfColumns];

} nSTRING;
typedef struct
{
	int* el_grid;
	double* Y;
	int l;
} LGRID;
typedef struct
{
	LGRID* elements;
	int allocated;
	int FinalSize;
} GRID;

typedef struct                           
{
	GRID* grid;
	nSTRING* Data;
	int used;
	int allocated;
	double InitialArg[NumberOfArguments];
	double FinalArg[NumberOfArguments];
	double StepArg[NumberOfArguments];
	int    NumberOfVariations[NumberOfArguments];
	int    NumberInVariation[NumberOfArguments];
	ENTRANCE* BasicEntrance;
} TABLE;

typedef struct
{
	char* str;
	int end;
	int length;
} cSTRING;

typedef struct
{
	cSTRING *strings;
	int used;
	int allocated;
} ARRAY_cSTRING;

typedef struct
{
	double P;
	double *dT;
	int* ip;
	int used;
	int allocated;
} P1_SLICE;

typedef struct
{
	int allocated;
	int used;
	double *dP;
    P1_SLICE *p1_slices;
} CONTAINER_P1_SLICES;

typedef struct
{
	double T;
	double P;
	double *Abs;
} P1_STRING;

typedef struct
{
	int NumberOfBounds;
	CONTAINER_P1_SLICES container_p1_slices;
	P1_STRING* Content;
	int used;
	int allocated;
	double (*ApproximationFunction)(void*table, double p, double T, int Nband);
} P1_TABLE;

int Init_ARRAY_cSTRING(ARRAY_cSTRING* ax,int al);
int AddTo_ARRAY_cSTRING(const cSTRING* xstr);
int Init_cSTRING(cSTRING* xstr,int xlength);
void Remove_cSTRING(cSTRING* xstr);
void CopyNotAlloc_cSTRING(cSTRING* xstr,const cSTRING* ystr);
int Copy_cSTRING(cSTRING* xstr,const cSTRING* ystr);
int IfHaveComment(const cSTRING* xstr);
int AddSymbolTo_cSTRING(cSTRING* xstr,char x);
int Add_cString_To_ARRAY_cSTRING(ARRAY_cSTRING* ax,const cSTRING* xstr);
void Remove_ARRAY_cSTRING(ARRAY_cSTRING* ax);
void Clear_ARRAY_cSTRING(ARRAY_cSTRING * ax);
void Remove_TABLE(TABLE* table);
int AddTo_Table(TABLE* table,const double *Y);
int CheckIntervalsAndSetIntervalsData(TABLE* table);
int CheckDataTable(TABLE *table);
void Remove_GRID(GRID *grid);
int Create_GRID(GRID* grid,int szx);
int ExtractColumns(ARRAY_cSTRING* ax,const cSTRING* xstr);
int FileRead(const char* NameOfFile,TABLE* table);
void Fill_GRID(GRID* grid,const double* Y,const TABLE *table);
double  FindApproximation(const double* X,const TABLE* table,int N);
double FindValue(int arg,const double* Yc, const TABLE* table);
int Init_TABLE(TABLE* table,int szx );
int Create_ENTRANCE(const TABLE* table,ENTRANCE* Entrance,const ENTRANCE* Parent,int NumberOfColumn,int NumberOfEntrance);
double GetSummOfConcentration(const TABLE* table, ENTRANCE* Entrance);
void Remove_ENTRANCE(ENTRANCE *Entrance);
void SetNumbersOfVariation(ENTRANCE* Entrance);
void SetStartLineInTableBegin(ENTRANCE* Entrance);
void reFill_GRID(GRID* grid,const double* Y,const TABLE* table,const ENTRANCE* Entrance,int i,int j);
int IfConcentration(int j_test);
void CopyNotAllocate_P1_STRINGs(P1_STRING* to,P1_STRING* from,int NumberOfBounds);
int AddDoubleArrayTo_P1_Table(P1_TABLE* p1_table, double* Ymass);
int Init_P1_SLICE(P1_SLICE* p1_slice,int length);
void Delete_P1_SLICE(P1_SLICE* p1_slice);
int Init_CONTAINER_P1_SLICES( CONTAINER_P1_SLICES* container, int length);
void Delete_CONTAINER_P1_SLICES( CONTAINER_P1_SLICES* container );
int Init_P1_TABLE(int NumberOfBounds,int InitialAlloc,P1_TABLE *p1_table);
int Read_P1_TABLE(const char* FileName,P1_TABLE *p1_table);
void Delete_P1_TABLE(P1_TABLE *p1_table);
void SortSlices_P1_TABLE(P1_TABLE* p1_table);
int FormSlices_P1_TABLE( P1_TABLE* p1_table, int* i1,int* i2);
int Set_dP_dTs_P1_TABLE( P1_TABLE* p1_table );
double Approximate_P1_TABLE_one_slice( void* p1_tableV, double p, double T,int Nband );
double Approximate_P1_TABLE_two_slices( void* p1_tableV, double p, double T,int Nband );
double Approximate_P1_TABLE_multiple_slices( void* p1_tableV, double p, double T,int Nband );
double Approximate_P1_TABLE( P1_TABLE* p1_table, double p, double T, int Nband );
double FindValue_P1_TABLE( P1_TABLE* p1_table, double P, double T, int Nband );

extern const char DelimSymbol;