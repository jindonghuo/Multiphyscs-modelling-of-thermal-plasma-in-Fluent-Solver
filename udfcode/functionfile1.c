#include "headfile1.h"

#if CONSOLE == 0
#include "udf.h"
#endif /*CONSOLE*/

#include <math.h>
const char DelimSymbol='\'';

#if CONSOLE != 0
void Message(const char* str)
{
 printf("%s",str);
}
#endif /*CONSOLE*/

int Init_cSTRING(cSTRING* xstr,int xlength)
{
	int i;
	(*xstr).str=(char*)malloc(xlength*sizeof(char));
	
	if( (*xstr).str==NULL ) return 1;
	(*xstr).length=xlength;
	for( i=0;i<xlength;i++)
		(*xstr).str[i]=0;
	(*xstr).end = -1;
	return 0;
}

void Remove_cSTRING(cSTRING* xstr)
{
	
	free((*xstr).str);
	(*xstr).length=0;
	(*xstr).end=-1;
};

void Clear_cSTRING(cSTRING* xstr)
{
	int i;
	for(i=0;i<(*xstr).length;i++)
		(*xstr).str[i]=0;
	(*xstr).end=-1;
}

void CopyNotAlloc_cSTRING(cSTRING* xstr,const cSTRING* ystr)
{
	int i;
	if( (*xstr).length>=(*ystr).end+2 )
	{ 
		for(i=0;i<=(*ystr).end;i++)
			(*xstr).str[i]=(*ystr).str[i];
		(*xstr).end=(*ystr).end;
		for(i=(*xstr).end+1;i<(*xstr).length;i++)
			(*xstr).str[i]=0;
	}
};

int Copy_cSTRING(cSTRING *xstr,const cSTRING *ystr)
{
	char x;
	int i;
	Clear_cSTRING(xstr);
	for(i=0;i<=(*ystr).end;i++)
	{
		 x=(*ystr).str[i];
         if(AddSymbolTo_cSTRING(xstr,x)==1)
			 return 1;
	}
	return 0;
}

int AddSymbolTo_cSTRING(cSTRING* xstr,char x)
{
	int i;
	cSTRING tmpstr;
	int new_length;
    int new_end=(*xstr).end+1;
	if(new_end>(*xstr).length-2)
	{
		new_length=(*xstr).length+STRL;
		if( Init_cSTRING(&tmpstr,(*xstr).length)==1) return 1;
		else
		{
           CopyNotAlloc_cSTRING(&tmpstr,xstr);
		   Remove_cSTRING(xstr);
		   if(Init_cSTRING(xstr,new_length)==1) 
		   {
			   Remove_cSTRING(&tmpstr);
			   return 1;
		   }
           CopyNotAlloc_cSTRING(xstr,&tmpstr);
           Remove_cSTRING(&tmpstr);
		}
		for(i=new_end+1;i<new_length;i++)
             (*xstr).str[i]=0;
	}
	(*xstr).str[new_end]=x;

	(*xstr).end=new_end;
	return 0;
};

int ReadStringFromFile(FILE* file,cSTRING* xstr)
{
   char symbol='a';
   int cc;
   while(symbol!='\n')
   {
     cc=getc(file);
	 symbol=(char)cc;
     if(symbol==EOF) return 1;
	 if(AddSymbolTo_cSTRING(xstr,symbol)==1)
		 return 2;
   }
   return 0;
};

int IfHaveComment(const cSTRING* xstr)
{
	int i;
	int start=0;
	for(i=0;i<(*xstr).end;i++)
	{
		if(start==0)
		{
			if((*xstr).str[i]!=' ' && (*xstr).str[i+1]!='\t')
				start=1;
		}
		if(start==1)
		{
			if( (*xstr).str[i]=='/' && (*xstr).str[i+1]=='/' ) return 1;
			else return 0;
		}
	}
	return 0;
}

int IfHaveColumnsNames(const cSTRING* xstr)
{
  int i;
  int sp=0;
  
  for(i=0;i<=(*xstr).end && sp!=1;i++)
  {
   
	if((*xstr).str[i]==DelimSymbol && sp==0)
		return 1;
    if((*xstr).str[i]!=' ' && (*xstr).str[i]!='\t')
		sp=1;
  }
  return 0;
}

int ExtractColumnsNames(ARRAY_cSTRING* ar, const cSTRING* xstr)
{
    cSTRING tmp;
	int i;
	int start=0;

	i=STRL;
	if( Init_cSTRING(&tmp,i)==1 )
	{
		return 1;
	}
	for(i=0; i<=(*xstr).end ;i++)
	{
		if(start==0)
		{
			if( (*xstr).str[i]==DelimSymbol )
			{
				start=1;
				Clear_cSTRING(&tmp);
			}         
		}
		else
		{
          if( (*xstr).str[i]==DelimSymbol )
		  {
			  start=0;
			  if(Add_cString_To_ARRAY_cSTRING(ar,&tmp)==1)
			  {
                Remove_cSTRING(&tmp);
				return 1;
			  }
		  }
		  else
		  {
			  if(AddSymbolTo_cSTRING(&tmp,(*xstr).str[i])==1)
			  {
                Remove_cSTRING(&tmp);
				return 1;
			  }
		  }          
		}
      
	}
	Remove_cSTRING(&tmp);
	return 0;
}

int ExtractColumns(ARRAY_cSTRING* ax,const cSTRING* xstr)
{
   cSTRING tmp;
   int i=0;
   int start=0;
   if( Init_cSTRING(&tmp,3)==1 )
   {
	   return 1;
   }
   for(i=0;i<=(*xstr).end;i++)
   {
     if(start==0)
	 {
       if( (*xstr).str[i]==' ' || (*xstr).str[i]=='\t' || (*xstr).str[i]=='\n' )
	   {
		   ;
	   }
	   else
	   {
		   start=1;
		   if( AddSymbolTo_cSTRING(&tmp,(*xstr).str[i])==1)
		   {
			   Remove_cSTRING(&tmp);
			   return 1;
		   } 
	   }
	 }
	 else
	 {
		 if( (*xstr).str[i]==' ' || (*xstr).str[i]=='\t' || (*xstr).str[i]=='\n' )
		 {
		   start=0;
		   if( Add_cString_To_ARRAY_cSTRING(ax,&tmp)==1)
		   {
			   Remove_cSTRING(&tmp);
			   return 1;
		   }
		   Clear_cSTRING(&tmp);
		 }
		 else
		 {
            if( AddSymbolTo_cSTRING(&tmp,(*xstr).str[i])==1)
			{
				Remove_cSTRING(&tmp);
				return 1;
			} 
           
		 }

	 }
   }
   if(start==1)
   {
	   if( Add_cString_To_ARRAY_cSTRING(ax,&tmp)==1)
	   {
		   Remove_cSTRING(&tmp);
		   return 1;
	   }
   }
   Remove_cSTRING(&tmp);
   return 0;
}

void ConvertStringsToDouble(double* X,int szx,const ARRAY_cSTRING* ar)
{
	int i;
	for(i=0; i<szx;i++)
	{
		X[i]=atof((*ar).strings[i].str);
	}

}


int FileRead(const char* NameOfFile,TABLE* table) /* two points as formal point, which is unchangable*/
{
  ARRAY_cSTRING Array;
  cSTRING str;
  char Msg[256];
  int check;
  int i;
  int icor=0;
  double Yc[NumberOfColumns];
  FILE* file;
  
  file=fopen(NameOfFile,"r");
  
  if(file==NULL)
  {
	  sprintf(Msg,"Can not open file %s.\n",NameOfFile);
 	  Message(Msg);
	  return 1;
  }
  else
  {
    sprintf(Msg,"File %s was opened.\n",NameOfFile);
 	Message(Msg);
  }
  if(Init_cSTRING(&str,11)==1)
  {
      Message("Can not allocate memory.\n");
	  fclose(file);
	  return 1;
  }
  for( ; ;)
  {
	 Clear_cSTRING(&str);
     check=ReadStringFromFile(file,&str);
	 icor++;
	 if(check==1)
	 {
		 Message("Unexpected end of file.\n");
		 Remove_cSTRING(&str);
		 fclose(file);
		 return 1;
	 }
	if(check==2)
	{
		 Message("Can not allocate memory.\n");
		 Remove_cSTRING(&str);
		 fclose(file);
		 return 1;
	}
     if( IfHaveComment(&str)==1) continue;
	 else break;
  }
  
  if( IfHaveColumnsNames(&str)==1 )
  {
	  
	if( Init_ARRAY_cSTRING(&Array,1)==1 )
	{
	  Message("Can not allocate memory.\n");
	  Remove_cSTRING(&str);
	  fclose(file);
	  return 1;
	}
    if(ExtractColumnsNames(&Array,&str)==1)
	{
      Message("Can not allocate memory.\n");
	  Remove_cSTRING(&str);
	  Remove_ARRAY_cSTRING(&Array);
	  fclose(file);
	  return 1;
	};
	Message("Columns of data in file: \n");
	for(i=0;i<=Array.used;i++)
	{
		Message(Array.strings[i].str);
		Message(" \n");
	}
    Remove_ARRAY_cSTRING(&Array);

     Clear_cSTRING(&str);
     check=ReadStringFromFile(file,&str);
	 icor++;
	 if(check==1)
	 {
		 Message("Unexpected end of file.\n");
		 Remove_cSTRING(&str);
		 fclose(file);
		 return 1;
	 }
	if(check==2)
	{
		 Message("Can not allocate memory.\n");
		 Remove_cSTRING(&str);
		 fclose(file);
		 return 1;
	}
  }    
  if( Init_ARRAY_cSTRING(&Array,1)==1 )
  {
	  Message("Can not allocate memory.\n");
	  Remove_cSTRING(&str);
	  fclose(file);
	  return 1;
  }
  if( ExtractColumns(&Array,&str)==1)
  {
	  Message("Can not allocate memory.\n");
	  Remove_cSTRING(&str);
      Remove_ARRAY_cSTRING(&Array);
	  fclose(file);
	  return 1;
  };
  if(Array.used!=NumberOfArguments-1)
  {
      Message("Wrong number of initial values of arguments.\n");
	  Remove_cSTRING(&str);
      Remove_ARRAY_cSTRING(&Array);
	  fclose(file);
	  return 1;
  }
  ConvertStringsToDouble((*table).InitialArg,NumberOfArguments,&Array);
  Clear_ARRAY_cSTRING(&Array);
  Clear_cSTRING(&str);

  check=ReadStringFromFile(file,&str);
  icor++;
  if(check==1)
  {
	  Message("Unexpected end of file.\n");
	  Remove_cSTRING(&str);
	  fclose(file);
	  return 1;
  }
  if(check==2)
  {
	  Message("Can not allocate memory.\n");
	  Remove_cSTRING(&str);
	  fclose(file);
	  return 1;
  }

  if( ExtractColumns(&Array,&str)==1)
  {
	  Message("Can not allocate memory.\n");
	  Remove_cSTRING(&str);
      Remove_ARRAY_cSTRING(&Array);
	  fclose(file);
	  return 1;
  };
  if(Array.used!=NumberOfArguments-1)
  {
      Message("Wrong number of final values of arguments.\n");
	  Remove_cSTRING(&str);
      Remove_ARRAY_cSTRING(&Array);
	  fclose(file);
	  return 1;
  }
  ConvertStringsToDouble((*table).FinalArg,NumberOfArguments,&Array);
  Clear_ARRAY_cSTRING(&Array);
  Clear_cSTRING(&str);

  check=ReadStringFromFile(file,&str);
  icor++;
  if(check==1)
  {
	  Message("Unexpected end of file.\n");
	  Remove_cSTRING(&str);
	  fclose(file);
	  return 1;
  }
  if(check==2)
  {
	  Message("Can not allocate memory.\n");
	  Remove_cSTRING(&str);
	  fclose(file);
	  return 1;
  }

  if( ExtractColumns(&Array,&str)==1)
  {
	  Message("Can not allocate memory.\n");
	  Remove_cSTRING(&str);
      Remove_ARRAY_cSTRING(&Array);
	  fclose(file);
	  return 1;
  };
  if(Array.used!=NumberOfArguments-1)
  {
      Message("Wrong number of step values of arguments.\n");
	  Remove_cSTRING(&str);
      Remove_ARRAY_cSTRING(&Array);
	  fclose(file);
	  return 1;
  }
  ConvertStringsToDouble((*table).StepArg,NumberOfArguments,&Array);
  Clear_ARRAY_cSTRING(&Array);
  Clear_cSTRING(&str);

  check=0;
  i=icor;
  while(check!=1)
  {
	  check=ReadStringFromFile(file,&str);
	  i++;
      
	  if(check==2)
	  {
		  Message("Can not allocate memory.\n");
		  Remove_cSTRING(&str);
		  fclose(file);
		  return 1;
	  }
	  if( ExtractColumns(&Array,&str)==1)
	  {
		  Message("Can not allocate memory.\n");
		  Remove_cSTRING(&str);
		  Remove_ARRAY_cSTRING(&Array);
		  fclose(file);
		  return 1;
	  };
	  if(Array.used==-1)
		  {
			  /*skip empty string*/
			  Clear_ARRAY_cSTRING(&Array);
	          Clear_cSTRING(&str);
			  continue;
	      }
	  if(Array.used!=NumberOfColumns-1)
	  {
		  sprintf(Msg,"Wrong number of columns in string number %i",i);
		  Message(Msg);
		  Message(".\n");
		  Remove_cSTRING(&str);
		  Remove_ARRAY_cSTRING(&Array);
		  fclose(file);
		  return 1;
	  }
	  ConvertStringsToDouble(Yc,NumberOfColumns,&Array);
      if(AddTo_Table(table,Yc)==1)
	  {
          Message("Can not allocate memory.\n");
		  Remove_cSTRING(&str);
		  Remove_ARRAY_cSTRING(&Array);
		  fclose(file);
		  return 1;
	  };
	  Clear_ARRAY_cSTRING(&Array);
	  Clear_cSTRING(&str);
  }
  fclose(file);
  Remove_cSTRING(&str);
  Remove_ARRAY_cSTRING(&Array);
 if(CheckIntervalsAndSetIntervalsData(table)==1)
	 {
	    Message("Wrong values of intervals or steps of arguments.\n");
		return 1;
	 }

 check=CheckDataTable(table);
 if( check != -1)
 {
	 if(check==-2)
	 {
		 Message("Table do not end with finals values of arguments.\n");
	 }
	 else
	 {
		 sprintf(Msg,"Wrong arguments in string number %i",icor+check+1);
		 Message(Msg);
		 Message(".\n");
	 }
	 return 1;
 }
 if( Create_ENTRANCE(table,table->BasicEntrance,NULL,-1,0) == 1 )
 {
   Message("Can not allocate memory.\n");
   return 1;
 }
 SetNumbersOfVariation(table->BasicEntrance);
 SetStartLineInTableBegin(table->BasicEntrance);

  return 0;
}

int Init_ARRAY_cSTRING(ARRAY_cSTRING* ax,int al)
{
	int i;
	int j;
    
	(*ax).strings=(cSTRING*)malloc(al*sizeof(cSTRING));
	if((*ax).strings==NULL)
		return 1;
	(*ax).allocated = al;
	(*ax).used = -1;
	
	for(i=0;i<(*ax).allocated;i++)
	{
		
		if( Init_cSTRING(&(*ax).strings[i],30)==1 )
		{
			for(j=0;j<i;j++)
				Remove_cSTRING(&(*ax).strings[j]);
			free( (*ax).strings );
			return 1;
		}
	}

    return 0; 
};

void Remove_ARRAY_cSTRING(ARRAY_cSTRING* ax)
{
	int i;
	for(i=0;i<(*ax).allocated;i++)
		Remove_cSTRING(&(*ax).strings[i]);
	free( (*ax).strings );
	(*ax).allocated=0;
	(*ax).used=-1;
}

int  Copy_ARRAY_cSTRING(ARRAY_cSTRING* ax,const ARRAY_cSTRING* ay)
{
	int i;
	Remove_ARRAY_cSTRING(ax);
	if(Init_ARRAY_cSTRING(ax,(*ay).allocated)==1)
		return 1;
    for(i=0;i<=(*ay).used;i++)
	{
		if( Copy_cSTRING(&(*ax).strings[i],&(*ay).strings[i])==1)
		{
			Remove_ARRAY_cSTRING(ax);
			return 1;
		}
	}
    (*ax).used=(*ay).used;
	return 0;
}; 

int Add_cString_To_ARRAY_cSTRING(ARRAY_cSTRING* ax,const cSTRING* xstr)
{
 ARRAY_cSTRING tmp; 
 int new_alloc;
 int new_used=(*ax).used+1;
 int i;
 if( new_used>=(*ax).allocated )
 {
   new_alloc = (*ax).allocated + ARSTRL;
   if( Init_ARRAY_cSTRING(&tmp,1)==1 )
	   return 1;
   if( Copy_ARRAY_cSTRING(&tmp,ax)==1 )
	   return 1;
   Remove_ARRAY_cSTRING(ax);
   if( Init_ARRAY_cSTRING(ax,new_alloc)==1 )
   {
	   Remove_ARRAY_cSTRING(&tmp);
       return 1;  
   }
   for(i=0;i<=tmp.used;i++)
   {
     if( Copy_cSTRING(&(*ax).strings[i],&tmp.strings[i])==1 )
	 {
		 Remove_ARRAY_cSTRING(&tmp);
         return 1;
	 }
   }
   Remove_ARRAY_cSTRING(&tmp);
 }
   if(   Copy_cSTRING(&(*ax).strings[new_used],xstr)==1 )
   {
      
      return 1;
   }
   (*ax).used=new_used;
   return 0;
};

int Init_TABLE(TABLE* table,int szx )
{
	   (*table).Data=(nSTRING*)malloc(szx*sizeof(nSTRING));
	   if((*table).Data==NULL)
		   return 1;
	   (*table).grid = (GRID*)malloc(sizeof(GRID));
	   if( (*table).grid==NULL )
	   {
		   free( (*table).Data );
		   return 1;
	   }
	   if(Create_GRID((*table).grid,NumberOfArguments)==1)
	   {
		   free( (*table).Data );
		   free( (*table).grid );
		   return 1;
	   };
	   (*table).BasicEntrance = (ENTRANCE*)malloc( sizeof(ENTRANCE) );
	   if( (*table).BasicEntrance == NULL )
	   {
		   free( (*table).Data );
	       Remove_GRID( (*table).grid );
		   free( (*table).grid );
		   return 1;
	   }
	   (*table).BasicEntrance->NumberOfColums=-1;
	   (*table).allocated=szx;
	   (*table).used=-1;
	   return 0;
};

void Remove_TABLE(TABLE* table)
{
	free((*table).Data);
	Remove_GRID( (*table).grid );
	free((*table).grid);
    Remove_ENTRANCE(table->BasicEntrance);
	free(table->BasicEntrance);
	(*table).allocated=0;
	(*table).used=-1;
	Remove_ENTRANCE(table->BasicEntrance);
}

int AddTo_Table(TABLE* table,const double *Y)
{  
  nSTRING* tmp;
  int i,j; 
  int new_alloc;
  int new_used =(*table).used + 1;
  
  if( new_used >= (*table).allocated )
  {
	  new_alloc = (*table).allocated + TALC;
      tmp=(nSTRING*)malloc((*table).allocated*sizeof(nSTRING));
	  if( tmp==NULL)
		  return 1;
	  for( i=0;i<(*table).allocated;i++)
	  {
         for(j=0;j<NumberOfColumns;j++)
		 {
			 tmp[i].Y[j]=(*table).Data[i].Y[j];
		 }
	  }
      free((*table).Data);
	  (*table).Data=(nSTRING*)malloc(new_alloc*sizeof(nSTRING));
	  if((*table).Data == NULL)
	  {
		  free(tmp);
		  return 1;
	  }
      for( i=0;i<(*table).allocated;i++)
	  {
         for(j=0;j<NumberOfColumns;j++)
		 {
			 (*table).Data[i].Y[j] = tmp[i].Y[j];
		 }
	  }
	  (*table).allocated=new_alloc;
	  free(tmp);
     
  }
  for(j=0;j<NumberOfColumns;j++)
  {
	  (*table).Data[new_used].Y[j] = Y[j];
  }
  (*table).used=new_used;
  return 0;  
}

void Clear_ARRAY_cSTRING(ARRAY_cSTRING * ax)
{
	int i;
	for(i=0;i<=(*ax).used;i++)
	{
		Clear_cSTRING(&(*ax).strings[i]);
	}
	(*ax).used=-1;
}

double distY(const double* X,const double* Y,int sz)
{
   int i=0;
   double ret=0;
   for(i=0;i<sz;i++)
   {
	   ret=ret+(X[i]-Y[i])*(X[i]-Y[i]);     
   }
   ret=sqrt(ret);
   return ret;
}

double FindValue(int arg,const double* Yc, const TABLE* table)
{
  double rets=1e36;
  double retj=1e36;
  double dist;
  double tmp;
  int i;
  int js=-1;
  int jj=-1;
  for(i=0;i<=(*table).used;i++)
  {
    dist=distY(Yc,(*table).Data[i].Y,NumberOfArguments);
	if(dist<retj)
	{
		rets=retj;
        js=jj;
		retj=dist;
		jj=i;
	}
  }
  if(js==-1 || jj==-1)
	  return 1;
  tmp = ( (*table).Data[js].Y[arg]*retj + (*table).Data[jj].Y[arg]*rets ) / (rets+retj);
  return tmp;
}

int CheckIntervalsAndSetIntervalsData(TABLE* table)
{
	int i,j;
	int ret;
	for(i=0;i<NumberOfArguments;i++)
	{
		if( (*table).FinalArg[i]-(*table).InitialArg[i]<=0.0 || (*table).StepArg[i]<=0 )
			return 1;
		(*table).NumberOfVariations[i]=(int) ( ((*table).FinalArg[i]-(*table).InitialArg[i])/ (*table).StepArg[i] ) +1; 
	}
	for(i=0;i<NumberOfArguments;i++)
	{
		ret=1;
		for(j=NumberOfArguments-1;j>i;j--)
			ret=ret*(*table).NumberOfVariations[j];
		(*table).NumberInVariation[i]=ret;
	}

	return 0;
}

int CheckArray(const double* X,const double* Y,int szx)
{
	int i;
	for(i=0;i<szx;i++)
	{
		if(X[i]!=Y[i])
			return 1;
	}
	return 0;
}

double GetFinalArgForCheck(double* Yc,const TABLE* table,int i)
{
	int j;
	double ret;
	if(i>=NumberOfConcentration || i==0)
		return (*table).FinalArg[i];

	ret=0.0;
	for( j=i-1;j>=0;j--)
	{
       ret += Yc[j]; 
	}
	ret = (*table).FinalArg[0] - ret;
    return ret;
}

void AddStepToArguments(double* Yc, const TABLE* table)
{
	double FinalArg;
	int i;
	i = NumberOfArguments-1;
	while(i>-1)
	{
		FinalArg = GetFinalArgForCheck(Yc,table,i); 
		if(Yc[i]>=FinalArg)
		{
			Yc[i]=(*table).InitialArg[i];
			i--;
		}
		else 
		{
			Yc[i]=Yc[i]+(*table).StepArg[i];
			if(Yc[i]>(*table).FinalArg[i]) Yc[i]=(*table).FinalArg[i];
			break;
		}
	}
}

void SetFinalArg(double* Yc,TABLE* table)
{
	int i;
	Yc[0] = table->FinalArg[0];
	for(i=1;i<NumberOfArguments;i++)
	{
		Yc[i] = GetFinalArgForCheck(Yc,table,i);
	}
}

int CheckDataTable(TABLE *table)
{
	
	int i;
	double Ycheck[NumberOfArguments];
	
	for(i=0;i<NumberOfArguments;i++)
		Ycheck[i]=(*table).InitialArg[i];
	for(i=0;i<=(*table).used;i++)
	{
		
		if( CheckArray(Ycheck,(*table).Data[i].Y,NumberOfArguments)==1)
		{
						return i;
		}
		AddStepToArguments(Ycheck,table);
	}

    SetFinalArg(Ycheck,table);
	if( CheckArray(Ycheck,(*table).Data[(*table).used].Y,NumberOfArguments)==1 )
	  return -2;

	return -1;
}

int Create_GRID(GRID* grid,int szx)
{
	int i,j;
	(*grid).allocated=szx;
	(*grid).elements=(LGRID*)malloc(szx*sizeof(LGRID));
	if( (*grid).elements==NULL ) return 1;
	for(i=0,j=2;i<szx;i++,j=j*2)
	{
		(*grid).elements[i].l=j;
		(*grid).elements[i].el_grid=(int*)malloc(j*sizeof(int));
		if( (*grid).elements[i].el_grid==NULL )
		{
			for(j=0;j<i;j++) free((*grid).elements[j].el_grid);
			free((*grid).elements);
			return 1;
		}
		(*grid).elements[i].Y=(double*)malloc(j*sizeof(double));
		if( (*grid).elements[i].Y==NULL )
		{
			for(j=0;j<=i;j++) free((*grid).elements[j].el_grid);
			free((*grid).elements);
			for(j=0;j<i;j++) free((*grid).elements[j].Y);
			free((*grid).elements);
			return 1;
		}
	}
	(*grid).FinalSize=j/2;
	return 0;
}

void Remove_GRID(GRID *grid)
{
	int i;
	for( i=0 ;i < (*grid).allocated ; i++)
	{
		free((*grid).elements[i].el_grid);
		free((*grid).elements[i].Y);
	}
	free( (*grid).elements );
}

/*void Fill_GRID(GRID* grid,const double* Y,const TABLE *table)
{
	int I[NumberOfArguments];
	int i,j,k,l;
	double LeftPoint;
	for(i=0;i<NumberOfArguments;i++)
	{
		if(Y[i]<(*table).InitialArg[i]) LeftPoint=(*table).InitialArg[i];
		else LeftPoint=Y[i];
		if(Y[i]>=(*table).FinalArg[i])
		{
			I[i]=(*table).NumberOfVariations[i]-2;
			LeftPoint=(*table).InitialArg[i]+(*table).StepArg[i]*I[i];
		}
		else I[i] = (int)( (LeftPoint-(*table).InitialArg[i])/(*table).StepArg[i] );
	}
	(*grid).elements[0].el_grid[0]=I[0]*(*table).NumberInVariation[0];
	(*grid).elements[0].el_grid[1]=(I[0]+1)*(*table).NumberInVariation[0];
	for(i=1,j=4;i<NumberOfArguments;i++,j=j*2)
	{
		l=j/2;
		for(k=0;k<j;k++)
		{
			(*grid).elements[i].el_grid[k]=(*grid).elements[i-1].el_grid[k/l]+(I[i]+k%2)*(*table).NumberInVariation[i];
		}
	}
}*/

void reFill_GRID(GRID* grid,const double* Y,const TABLE* table,const ENTRANCE* Entrance,int i,int j)
{
	int jj;
    ENTRANCE* Child_l;
	int I;
	int j1,j2;
	if( i == NumberOfArguments )
		return;
    I = (int)((Y[i]-table->InitialArg[i])/table->StepArg[i] );
    
	if(I>=Entrance->NumberOfChild-1)
		I=Entrance->NumberOfChild-2;
    if(I<0) I = 0;
    Child_l=(ENTRANCE*)Entrance->child;
	j1 = 2*j;
	j2 = 2*j + 1;
	(*grid).elements[i].el_grid[j1]=Child_l[I].StartLineInTable;
    (*grid).elements[i].el_grid[j2]=Child_l[I+1].StartLineInTable;
	if(i==1 && j2==3)
	{
		jj=3;
	}
	if(Entrance->NumberOfChild==1)
	{
		reFill_GRID(grid,Y,table,&Child_l[0],i+1,j1);
		reFill_GRID(grid,Y,table,&Child_l[0],i+1,j2);
	}
	else
	{
		reFill_GRID(grid,Y,table,&Child_l[I],i+1,j1);
		reFill_GRID(grid,Y,table,&Child_l[I+1],i+1,j2);
	}
}

void Fill_GRID(GRID* grid,const double* Y,const TABLE *table)
{
	ENTRANCE* Child_l;
	int i;
	int I;
    I = (int)((Y[0]-table->InitialArg[0])/table->StepArg[0] );
    if(I>=table->BasicEntrance->NumberOfChild-1)
		I=table->BasicEntrance->NumberOfChild-2;
    if(I<0) I = 0;
    Child_l=(ENTRANCE*)table->BasicEntrance->child;
    
	(*grid).elements[0].el_grid[0]=Child_l[I].StartLineInTable;
    (*grid).elements[0].el_grid[1]=Child_l[I+1].StartLineInTable;
	i=1;
	if(table->BasicEntrance->NumberOfChild==1)
	{
		reFill_GRID(grid,Y,table,&Child_l[0],i,0);
		reFill_GRID(grid,Y,table,&Child_l[0],i,1);
	}
	else
	{
		reFill_GRID(grid,Y,table,&Child_l[I],i,0);
		reFill_GRID(grid,Y,table,&Child_l[I+1],i,1);
	}
}

void SetFirstLayerOfApproximation(const TABLE* table,int N)
{
	int i;
	for(i=0;i<(*table).grid->FinalSize;i++)
	{
		(*table).grid->elements[NumberOfArguments-1].Y[i]=(*table).Data[(*table).grid->elements[NumberOfArguments-1].el_grid[i]].Y[N];
	}
}

void NextLayerOfApproximation(const double* Y,const TABLE* table,int L)
{
	int i;
	double X,Y1,Y2,X1,X2_X1;
	int u1,u2;
	for(i=0;i<(*table).grid->elements[L].l;i++)
	{
		u1=i*2;
		u2=i*2+1;
		X = Y[L+1];
		Y1 = (*table).grid->elements[L+1].Y[u1];
		Y2 = (*table).grid->elements[L+1].Y[u2];
		X1 = (*table).Data[(*table).grid->elements[L+1].el_grid[u1]].Y[L+1];
		X2_X1 = (*table).StepArg[L+1];
		(*table).grid->elements[L].Y[i]=Y1+(Y2-Y1)*(X-X1)/X2_X1;
	}
}

double FromLastLayerOfApproximation(const double* Y,const TABLE* table)
{
	double ret;
	double X,X1,X2_X1,Y1,Y2;
	X = Y[0];
	Y1 = (*table).grid->elements[0].Y[0];
	Y2 = (*table).grid->elements[0].Y[1];
	X1 = (*table).Data[(*table).grid->elements[0].el_grid[0]].Y[0];
	X2_X1 = (*table).StepArg[0];
	ret=Y1+(Y2-Y1)*(X-X1)/X2_X1;
	return ret;
}

double  FindApproximation(const double* X,const TABLE* table,int N)
{
	double ret;
	int i;
	Fill_GRID((*table).grid,X,table);
	SetFirstLayerOfApproximation(table,N);
	for( i=NumberOfArguments-2; i>=0; i--)
	     NextLayerOfApproximation(X,table,i);

	ret=FromLastLayerOfApproximation(X,table);
	return ret;
}

int ReadNameFromFile(const char* FileName,cSTRING *str)
{
	FILE* file;
	char Msg[256];
	file=fopen(FileName,"r");
	if( file == NULL)
	{
		sprintf_s(Msg,254,"Can not open file \'%s\' with name of the table.\n",FileName);
		Message(Msg);
		return 1;
	}
	if(ReadStringFromFile(file,str)==2)
	{   sprintf_s(Msg,254,"Can not allocate memory while opening file \'%s\' with name of the table.\n",FileName);
		Message(Msg);
		return 1;
	}
	return 0;
}

void RemoveDelimFormString(cSTRING *str)
{
	int i,j;
	
	for(i=0;i<(*str).end+1;i++)
	{
		if( (*str).str[i]==' ' || (*str).str[i]=='\t' || (*str).str[i]=='\n')
		{
			for(j=i;j<=(*str).end+1;j++)
				(*str).str[j]=(*str).str[j+1];
			(*str).end--;
		}
	}
	i=(*str).end;
	if( (*str).str[i]==' ' || (*str).str[i]=='\t' || (*str).str[i]=='\n')
		{
			for(j=i;j<=(*str).end+1;j++)
				(*str).str[j]=(*str).str[j+1];
			(*str).end--;
		}
}

double GetFinalArg(const TABLE* table, ENTRANCE* Entrance)
{
    if( IfConcentration( Entrance->NumberOfColums + 1 )==1 )
		return table->FinalArg[ Entrance->NumberOfColums + 1 ];
	else
		return table->FinalArg[0] - GetSummOfConcentration(table,Entrance);
	return 1.0;
}

int IfConcentration(int j_test)
{
	if(j_test<NumberOfConcentration)
		return 0;
	return 1;
}

double GetSummOfConcentration(const TABLE* table, ENTRANCE* Entrance)
{
	double ret;
	ENTRANCE* parent = (ENTRANCE*) (*Entrance).parent;
	if( Entrance->NumberOfColums == -1 )
		return 0.0;
	else ret=GetSummOfConcentration(table,parent);
	if( IfConcentration(parent->NumberOfColums)==0 )
	{
       ret+=(*table).InitialArg[Entrance->NumberOfColums]+Entrance->NumberOfSteps * table->StepArg[Entrance->NumberOfColums];
	}
	
	return ret;
}

int Create_ENTRANCE(const TABLE* table,ENTRANCE* Entrance,const ENTRANCE* Parent,int NumberOfColumn,int NumberOfEntrance)
{
	double FinalArg;
	int NextColumn;
	int NumberOfChild;
    ENTRANCE* Child ;
	int i;
    Entrance->parent = (void*)Parent;
	Entrance->NumberOfColums = NumberOfColumn;
	Entrance->NumberOfSteps = NumberOfEntrance;
	Entrance->IfAlloc = 1;
    if( Entrance->NumberOfColums == NumberOfArguments - 1 )
	{
		Entrance->NumberOfChild = 1;
		Entrance->child = (void*)Entrance;
		return 0;
	}
	FinalArg = GetFinalArg(table,Entrance);
	NextColumn = NumberOfColumn + 1;
    NumberOfChild = 1 + (int)((FinalArg - (*table).InitialArg[ NextColumn])/(*table).StepArg[NextColumn]);
	(*Entrance).NumberOfChild = NumberOfChild;
    (*Entrance).child = (ENTRANCE*)malloc(NumberOfChild*sizeof(ENTRANCE)); 
	if( (*Entrance).child == NULL ) return 1;
	else (*Entrance).IfAlloc = 0;
    Child = (ENTRANCE*)(*Entrance).child ;
	for(i=0;i<NumberOfChild;i++)
	{
		if( Create_ENTRANCE(table,&Child[i],Entrance,NumberOfColumn+1,i)==1 )
			return 1;
	}
	return 0;
}

void Remove_ENTRANCE(ENTRANCE *Entrance)
{
	int i;
	ENTRANCE* Child;
	if( Entrance->IfAlloc==0 )
	{
		Child = (ENTRANCE*)Entrance->child;
		for( i=0;i<Entrance->NumberOfChild;i++)
			Remove_ENTRANCE(&Child[i]);
		free(Entrance->child);
	}
}

int GetNumberOfVariations(ENTRANCE* Entrance)
{
	int i;
	int ret;
    ENTRANCE* Child;
	if(Entrance->NumberOfColums==NumberOfArguments-1)
	{
        Entrance->NumberOfVariation = 1;
		return 1;
	}
	Child = (ENTRANCE*)Entrance->child;
    for(i=0,ret=0;i<Entrance->NumberOfChild;i++)
	{
		ret+=GetNumberOfVariations(&Child[i]);
	}
	Entrance->NumberOfVariation = ret;
    return ret;
}

void SetNumbersOfVariation(ENTRANCE* Entrance)
{
    int i;
	int ret;
    ENTRANCE* Child;

	Child = (ENTRANCE*)Entrance->child;
	for(i=0,ret=0;i<Entrance->NumberOfChild;i++)
	{
		ret+=GetNumberOfVariations(&Child[i]);
	}
    Entrance->NumberOfVariation = ret;
}

void SetStartLineInTable(int cshx,ENTRANCE* Entrance)
{
	int i;
	ENTRANCE* Child;
    int csh;
	Entrance->StartLineInTable = cshx;
    if(Entrance->NumberOfColums==NumberOfArguments-1)
		return;
    Child=(ENTRANCE*)Entrance->child;
	for(i=0,csh=cshx;i<Entrance->NumberOfChild;i++)
	{
       SetStartLineInTable(csh,&Child[i]);
       csh += Child[i].NumberOfVariation;
	}
	
}

void SetStartLineInTableBegin(ENTRANCE* Entrance)
{
  int i;
  int csh;
  ENTRANCE* Child;
  Entrance->StartLineInTable=0;
  Child=(ENTRANCE*)Entrance->child;
  for(i=0,csh=0;i<Entrance->NumberOfChild;i++)
  {
    SetStartLineInTable(csh,&Child[i]);
	csh += Child[i].NumberOfVariation;
  }
}

int Init_P1_STRING(int NumberOfBounds,P1_STRING* p1_string)
{
	p1_string->Abs = (double*)malloc( NumberOfBounds*sizeof(double) );
	if( p1_string == NULL )
		return 1;
	return 0;
}

void Delete_P1_STRING(P1_STRING* p1_string)
{
	if( p1_string->Abs != NULL )
		free(p1_string->Abs);	
}

int Init_P1_TABLE(int NumberOfBounds,int InitialAlloc,P1_TABLE *p1_table)
{
	int i,j;
	p1_table->NumberOfBounds = NumberOfBounds;
	p1_table->Content = (P1_STRING*) malloc(InitialAlloc*sizeof(P1_STRING));
	p1_table->used = -1;
	if(p1_table->Content == NULL)
	{
		p1_table->allocated=0;
		return 1;
	}
    for( i=0;i<InitialAlloc;i++ )
	{
		if( Init_P1_STRING(NumberOfBounds,&p1_table->Content[i])==1)
		{
			for( j=0;j<i;j++)
				Delete_P1_STRING(&p1_table->Content[j]);
			free(p1_table->Content);
			p1_table->allocated = 0;
			return 1;
		}
	}
	p1_table->allocated = InitialAlloc;
	if( Init_CONTAINER_P1_SLICES( &p1_table->container_p1_slices,4) == 1 )
	{
		for( i=0; i<p1_table->allocated;i++)
		{
			Delete_P1_STRING(&p1_table->Content[i]);
		}
		free(p1_table->Content);
		p1_table->allocated = 0;
		return 1;
	}
	return 0;
}

void Delete_P1_TABLE(P1_TABLE *p1_table)
{
	int i;
	if( p1_table == NULL)
		return;
	for( i=0;i<p1_table->allocated;i++ )
	{
      Delete_P1_STRING(&p1_table->Content[i]);
	}
	free( p1_table->Content );
	p1_table->used = -1;
	p1_table->allocated = 0;
	Delete_CONTAINER_P1_SLICES(&p1_table->container_p1_slices );
}

int Read_P1_TABLE(const char* FileName,P1_TABLE *p1_table)
{
	FILE* File;
	char Msg[256];
	int npos;
	int check;
	cSTRING str_tmp;
    ARRAY_cSTRING Array_str;
	int i;
	int i1,i2;
	int last;
	int SizeOfString;
	int SizeOfString_1;
	double* Ymass;

	File = fopen(FileName,"r+");
	if(File == NULL)
	{
		npos = sprintf(Msg,"Can not open file %s while reading a P1 model table.\n",FileName);
		Message(Msg);
		return 1;
	}
	else
	{
		npos = sprintf(Msg,"File %s was opened while reading a P1 model table.\n",FileName);
		Message(Msg);
	}

	if( Init_cSTRING(&str_tmp,256) == 1)
	{
		npos = sprintf(Msg,"Can not allocate memory while reading file %s with P1 model table.\n",FileName);
		Message(Msg);
		Remove_cSTRING(&str_tmp);
        fclose(File);
        return 1;
	}

	for(i=0;i<2;i++)
	{
		check = ReadStringFromFile(File,&str_tmp);
		if(check ==1)
		{
			npos = sprintf(Msg,"Unexpacted end of file %s while reading a P1 model table.\n",FileName);
			Message(Msg);
            Remove_cSTRING(&str_tmp);
            fclose(File);
			return 1;
		}
		if(check == 2)
		{
			npos = sprintf(Msg,"Can not allocate memory while reading file %s with P1 model table.\n",FileName);
			Message(Msg);
            Remove_cSTRING(&str_tmp);
			fclose(File);
			return 1;
		}
        Clear_cSTRING(&str_tmp);
	}
	
	SizeOfString = p1_table->NumberOfBounds+2;
	SizeOfString_1 = SizeOfString-1;
	Ymass = (double*) malloc( SizeOfString*sizeof(double) );
	if( Ymass == NULL )
	{
		npos = sprintf(Msg,"Can not allocate memory while reading file %s with P1 model table.\n",FileName);
		Message(Msg);
        Remove_cSTRING(&str_tmp);
		fclose(File);
		return 1;
	}
	if( Init_ARRAY_cSTRING(&Array_str,SizeOfString) ==1)
	{
		npos = sprintf(Msg,"Can not allocate memory while reading file %s with P1 model table.\n",FileName);
		Message(Msg);
		Remove_cSTRING(&str_tmp);
		free(Ymass);
		fclose(File);
		return 1;
	}

	for(i=3,last=0; last!=1 ;i++)
	{
       check = ReadStringFromFile(File,&str_tmp);
		if(check ==1)
			last = 1;
		if(check == 2)
		{
			npos = sprintf(Msg,"Can not allocate memory while reading file %s with P1 model table.\n",FileName);
			Message(Msg);
            Remove_cSTRING(&str_tmp);
			Remove_ARRAY_cSTRING(&Array_str);
            free(Ymass);
            fclose(File);
			return 1;

		}
       if( ExtractColumns(&Array_str,&str_tmp)==1 )
	   {
		   npos = sprintf(Msg,"Can not allocate memory while reading file %s with P1 model table.\n",FileName);
		   Message(Msg);
           Remove_cSTRING(&str_tmp);
		   Remove_ARRAY_cSTRING(&Array_str);
           free(Ymass);
           fclose(File);
		   return 1;
	   };
	   if(Array_str.used==-1)
	   {
		  Clear_ARRAY_cSTRING(&Array_str);
		  Clear_cSTRING(&str_tmp);
		  continue;
	   }
	   if(Array_str.used!=SizeOfString_1)
	   {
		   npos = sprintf(Msg,"Wrong number of values in string %i while reading file %s with P1 model table.\n",i,FileName);
		   Message(Msg);
           Remove_cSTRING(&str_tmp);
		   Remove_ARRAY_cSTRING(&Array_str);
           free(Ymass);
           fclose(File);
		   return 1;
	   }
	   ConvertStringsToDouble(Ymass,SizeOfString,&Array_str);
	   if( AddDoubleArrayTo_P1_Table(p1_table,Ymass) == 1 )
	   {
		   npos = sprintf(Msg,"Can not allocate memory while reading file %s with P1 model table.\n",FileName);
		   Message(Msg);
           Remove_cSTRING(&str_tmp);
		   Remove_ARRAY_cSTRING(&Array_str);
           free(Ymass);
           fclose(File);
		   return 1;
	   }
	   Clear_ARRAY_cSTRING(&Array_str);
	   Clear_cSTRING(&str_tmp);
	}
    Remove_cSTRING(&str_tmp);
    Remove_ARRAY_cSTRING(&Array_str);
    free(Ymass);
    fclose(File);
	if( p1_table->used <= 1)
	{
		npos = sprintf(Msg,"Number of string in p1 table %s is not enough for approximation",FileName);
        Message(Msg);
        return 1; 
	}
	/*Forming and checking slices: */
	check = FormSlices_P1_TABLE(p1_table,&i1,&i2);
	if( check==1 )
	{
		npos = sprintf(Msg,"Can not allocate memory while reading file %s with P1 model table.\n",FileName);
		Message(Msg);
		return 1;
	}
	if( check==2 )
	{
        npos = sprintf(Msg,"There are  two strings (numbers %i and %i) with same tempretures and pressures in file %s with P1 model table.\n",i1,i2,FileName);
		Message(Msg);
		return 1;      
	}
	if( Set_dP_dTs_P1_TABLE( p1_table ) == 1 )
	{
        npos = sprintf(Msg,"Can not allocate memory while reading file %s with P1 model table.\n",FileName);
		Message(Msg);
		return 1;
	}
	if( p1_table->container_p1_slices.used == -1 )
	{
        npos = sprintf(Msg,"Can not resolve contant pressure slices while reading file %s with P1 model table.\n",FileName);
		Message(Msg);
		return 1;
     }
	else if( p1_table->container_p1_slices.used == 0 )
	{
		p1_table->ApproximationFunction = &Approximate_P1_TABLE_one_slice;
	}
	else if( p1_table->container_p1_slices.used == 1 )
	{
        p1_table->ApproximationFunction = &Approximate_P1_TABLE_two_slices;
	}
	else if( p1_table->container_p1_slices.used >= 2 )
	{
        p1_table->ApproximationFunction = &Approximate_P1_TABLE_multiple_slices;
	}
	return 0;
}

int AddDoubleArrayTo_P1_Table(P1_TABLE* p1_table, double* Ymass)
{
  int new_used;
  int new_allocated;
  P1_STRING* p1_strings;
  int i,j;

  new_used = p1_table->used + 1;
  if( new_used>=p1_table->allocated )
  {
	  new_allocated = p1_table->allocated + P1_ADD;
      p1_strings = (P1_STRING*) malloc( new_allocated * sizeof(P1_STRING) );
	  if( p1_strings == NULL )
		  return 1;
	  
	  for( i=0; i<new_allocated; i++)
	  {
        if(  Init_P1_STRING(p1_table->NumberOfBounds,&p1_strings[i]) == 1 )
		{
			for(j=0;j<i;j++)
				Delete_P1_STRING(&p1_strings[j]);
			free(p1_strings);
			
			return 1;
		}
	  }
	  for( i=0; i<p1_table->allocated; i++)
	  {
		  CopyNotAllocate_P1_STRINGs( &p1_strings[i], &p1_table->Content[i],p1_table->NumberOfBounds);
		  Delete_P1_STRING(&p1_table->Content[i]);
	  }
	  free(p1_table->Content);
	  p1_table->Content = p1_strings;
	  p1_table->allocated = new_allocated;
  }
  p1_table->Content[new_used].T = Ymass[0];
  p1_table->Content[new_used].P = Ymass[1];
  for( i=2,j=0;j<p1_table->NumberOfBounds;j++,i++)
	  p1_table->Content[new_used].Abs[j] = Ymass[i];
  p1_table->used = new_used;
  return 0;
}

void CopyNotAllocate_P1_STRINGs(P1_STRING* to,P1_STRING* from,int NumberOfBounds)
{
	int u;
	to->T = from->T;
    to->P = from->P;
	for( u=0; u< NumberOfBounds; u++)
		to->Abs[u] = from->Abs[u];     
}

int Init_P1_SLICE(P1_SLICE* p1_slice,int length)
{
	p1_slice->ip = (int*) malloc( length * sizeof(int) );
    p1_slice->used = -1;
	if(p1_slice==NULL)
	{
		p1_slice->allocated = 0;
		return 1;
	}
    p1_slice->allocated = length;
	p1_slice->dT = NULL;
	return 0;
}

void Delete_P1_SLICE(P1_SLICE* p1_slice)
{
	if( p1_slice->ip != NULL )
		free( p1_slice->ip );
	if( p1_slice->dT != NULL )
		free( p1_slice->dT );
}

int Init_CONTAINER_P1_SLICES( CONTAINER_P1_SLICES* container, int length)
{
	int i,j;
	container->p1_slices = (P1_SLICE*)malloc( length * sizeof( P1_SLICE ) );
	container->used = -1;
	if( container->p1_slices == NULL)
	{
		container->allocated =0;
       	return 1;
	}
    container->allocated = length;
	for( i=0; i<container->allocated;i++)
	{
		if( Init_P1_SLICE( &container->p1_slices[i],1) == 1 )
		{
          for(j=0;j<i;j++)
			  Delete_P1_SLICE(&container->p1_slices[j]);
		  container->allocated = 0;
		  free(container->p1_slices);
		  return 1;
		}
	}
	container->dP = NULL;
	return 0;
}

void Delete_CONTAINER_P1_SLICES( CONTAINER_P1_SLICES* container )
{
	int i;
	if( container->p1_slices == NULL)
		return;
	for( i=0; i<container->allocated; i++)
	{
		Delete_P1_SLICE(&container->p1_slices[i]);
	}

}

int AddInt_P1_SLICE(P1_SLICE* slice,int add)
{
	int new_allocated;
	int new_used;
	int* new_ip;
	int i;

	new_used = slice->used + 1;
	if( new_used >= slice->allocated )
	{
		new_allocated = slice->allocated + 100;
		new_ip = (int*) malloc( new_allocated * sizeof(int) );
		if( new_ip == NULL )
			return 1;
		for(  i=0; i<slice->allocated;i++)
		{
			new_ip[i] = slice->ip[i];
		}
        free(slice->ip);
		slice->ip = new_ip;
        slice->allocated = new_allocated;
	}
	slice->ip[new_used] = add;
	slice->used = new_used;
	return 0;
}

int Copy_P1_SLICE( P1_SLICE* To, P1_SLICE* From )
{
	int i;
	if( To->ip != NULL)
		free(To->ip);
	To->ip = (int*) malloc( From->allocated * sizeof(int) );
	if( To->ip == NULL )
	{
	  To->allocated=0;
	  To->used=-1;
	  return 1;
	}
	To->allocated = From->allocated;
	To->used = From->used;
	for( i=0; i<From->allocated; i++)
	{
		To->ip[i] = From->ip[i];
	}
	return 0;
}

int AddSlice_CONTAINER_P1_SLICES( CONTAINER_P1_SLICES* container, double p)
{
	int i,j;
	int new_allocated;
	int new_used;
	P1_SLICE* new_slices;
	new_used = container->used + 1;
	if( new_used>=container->allocated )
	{
		new_allocated = container->allocated + 100;
		new_slices = (P1_SLICE*) malloc( new_allocated * sizeof(P1_SLICE) );
        if( new_slices == NULL)
			return 1;
		for( i=0; i<new_allocated; i++)
		{
			if( Init_P1_SLICE( &new_slices[i],1)==1 )
			{
				for(j=0;j<=i;j++)
					Delete_P1_SLICE(&new_slices[j]);
				free(new_slices);
				return 1;
			}
			
		}
		for( i=0; i<container->allocated; i++ )
		{
			if( Copy_P1_SLICE(&new_slices[i],&container->p1_slices[i]) == 1)
			{
				for( j=0;j<new_allocated;j++)
					Delete_P1_SLICE(&new_slices[i]);
				free(new_slices);
				return 1;            
			}
		}
		for( i=0; i<=container->used; i++ )
					Delete_P1_SLICE(&container->p1_slices[i]);
		container->p1_slices = new_slices;
		container->allocated = new_allocated;

	}
	container->used = new_used;
	if( Init_P1_SLICE(&container->p1_slices[new_used],1)==1 )
			return 1;
	container->p1_slices[new_used].P = p;
   	return 0;
}

int CheckSlice_P1_SLICE( P1_TABLE* p1_table, P1_SLICE* slice, double T, int* i1 )
{
	int j;
	double Tsl;
	int ip;
	for(j=0;j<=slice->used;j++)
	{
		ip = slice->ip[j];
		Tsl = p1_table->Content[ip].T;
		if( Tsl == T )
		{
			(*i1) = ip;
			return 1;
		}
	}
	(*i1) = -1;
    return 0;
}

int FormSlices_P1_TABLE( P1_TABLE* p1_table, int* i1,int* i2)
{
	int i,j;
	int test;
	double p;
	double T;
	for( i=0; i <= p1_table->used; i++)
	{
		test = 1;
		p =	p1_table->Content[i].P;
		for( j=0; j<=p1_table->container_p1_slices.used; j++)
		{
          if( p1_table->container_p1_slices.p1_slices[j].P == p )
		  {  
			  T = p1_table->Content[i].T;
			  if( CheckSlice_P1_SLICE(p1_table,&p1_table->container_p1_slices.p1_slices[j],T,i1) == 1 )
			  {
				  (*i2) = i;
				  return 2;
			  }
			  if( AddInt_P1_SLICE(&p1_table->container_p1_slices.p1_slices[j],i)==1 )
				  return 1;
			  test=0;            
		  }          
		}
		if( test == 1 )
		{
			if( AddSlice_CONTAINER_P1_SLICES( &p1_table->container_p1_slices,p) ==1 )
				return 1;
			if( AddInt_P1_SLICE(&p1_table->container_p1_slices.p1_slices[p1_table->container_p1_slices.used],i) == 1)
				return 1;
		}
	}
    SortSlices_P1_TABLE( p1_table);
	return 0;
}

void Exchange_P1_SLICE(P1_SLICE* sl1, P1_SLICE* sl2)
{
  int allocated;
  int used;
  int* ip;
  double p;

  allocated = sl1->allocated;
  used = sl1->used;
  ip = sl1->ip;
  p = sl1->P;

  sl1->allocated = sl2->allocated;
  sl1->used = sl2->used;
  sl1->ip = sl2->ip;
  sl1->P = sl2->P;
  
  sl2->allocated = allocated;
  sl2->used = used;
  sl2->ip = ip;
  sl2->P = p;
}

void SortInSlice_P1_SLICE(P1_TABLE* p1_table, P1_SLICE* slice)
{
	int i;
	int ip1,ip2;
	double T1,T2;
	for( i=0; i<slice->used;)
	{
      ip1 = slice->ip[i];
	  ip2 = slice->ip[i+1];
	  T1 =  p1_table->Content[ip1].T;
	  T2 =  p1_table->Content[ip2].T;
	  if( T1 > T2)
	  {
		  slice->ip[i+1] = ip1;
		  slice->ip[i] = ip2;
		  if(i>0) i--;
	  }
	  else
		  i++;
	}	
}

void SortSlices_P1_TABLE(P1_TABLE* p1_table)
{
  P1_SLICE* sl1;
  P1_SLICE* sl2;
  int i;
  for( i=0; i<p1_table->container_p1_slices.used;)
  {
	  sl1 = &p1_table->container_p1_slices.p1_slices[i];
	  sl2 = &p1_table->container_p1_slices.p1_slices[i+1];
	  if( sl1->P > sl2->P)
	  {
		  Exchange_P1_SLICE(sl1,sl2);
		  if(i>0) i--;
	  }
	  else
		  i++;
  }
  for( i=0; i<p1_table->container_p1_slices.used;i++)
  {
	  sl1 = &p1_table->container_p1_slices.p1_slices[i];
      SortInSlice_P1_SLICE(p1_table, sl1);
  }
}

double FindValue_P1_TABLE( P1_TABLE* p1_table, double P, double T, int Nband )
{
	int j,jmin;
	double nearest = 1e10;
	double dim,dT,dP;
	double ret = 0.0;
	for( j=0; j < p1_table->used; j++ )
	{
      dT = p1_table->Content[j].T - T;
	  dT *= dT;
	  dP = p1_table->Content[j].P - P;
	  dP *= dP;
	  dim = sqrt( dP + dT );
      if( dim<=nearest )
	  {
		  ret = p1_table->Content[j].Abs[Nband];
		  nearest = dim;
		  jmin = j;
	  }
	}
    return ret;
}

double Approximate_P1_TABLE( P1_TABLE* p1_table, double p, double T, int Nband )
{
	void* p1_tableV;
    double ret; 
	p1_tableV = (void*) p1_table;
	ret = p1_table->ApproximationFunction(p1_tableV,p,T,Nband);
    return ret;
}

int FindInterval_P1_SLICE(P1_TABLE* p1_table, P1_SLICE* p1_slice, double T)
{
	int Left,Right,Dif;
	int Test,ip;
	double Tcomp;
	Left = 0, Right = p1_slice->used;
    Dif = Right;
	while(Dif!=1)
	{
      Test = (Left + Right)/2;
	  ip = p1_slice->ip[Test];
	  Tcomp = p1_table->Content[ip].T;
      if( T >= Tcomp )
		  Left = Test;
	  else
		  Right = Test;
	  Dif = Right - Left;
	}
	return Left;
}

double Approximate_P1_SLICE(P1_TABLE* p1_table,P1_SLICE* p1_slice,double T,int Nband)
{
	int Left;
	int ip1,ip2;
	double Tleft,dT;
	double AbsLeft,AbsRight;
	double ret;
	if( T <= p1_table->Content[ p1_slice->ip[0] ].T )
		return  p1_table->Content[ p1_slice->ip[0] ].Abs[Nband];
	Left = FindInterval_P1_SLICE(p1_table,p1_slice,T);
    ip1 = p1_slice->ip[Left];
	ip2 = p1_slice->ip[Left+1];
	dT = p1_slice->dT[Left];
	Tleft = p1_table->Content[ip1].T;
    AbsLeft = p1_table->Content[ip1].Abs[Nband];
	AbsRight = p1_table->Content[ip2].Abs[Nband];
	ret = AbsLeft + ( AbsRight - AbsLeft ) * ( T - Tleft ) / dT; 
    return ret;
}

int FindInterval_CONTAINER_P1_SLICES(CONTAINER_P1_SLICES* container, double p)
{
	int Left, Right;
	int Dif,Test;
	double Pcomp;
	Left = 0;
	Right = container->used;
	Dif = Right;
	while( Dif!=1 )
	{
		Test = ( Right + Left ) / 2;
		Pcomp = container->p1_slices[Test].P;
		if( p >= Pcomp)
			Left = Test;
		else
			Right = Test;
		Dif = Right - Left;
	}
	return Left;
}

double Approximate_P1_TABLE_one_slice( void* p1_tableV, double p, double T,int Nband )
{
	double ret;
    P1_TABLE* p1_table;
	P1_SLICE* p1_slice;
	p1_table = (P1_TABLE*) p1_tableV;
	p1_slice = &p1_table->container_p1_slices.p1_slices[0];
	ret = Approximate_P1_SLICE(p1_table, p1_slice, T, Nband);
    return ret;
}

double Approximate_P1_TABLE_two_slices( void* p1_tableV, double p, double T,int Nband )
{
	double ret,dP,Pleft;
    double AbsLeft,AbsRight;
	P1_TABLE* p1_table;
	P1_SLICE* p1_sliceLeft;
	P1_SLICE* p1_sliceRight;
	p1_table = (P1_TABLE*) p1_tableV;
	p1_sliceLeft = & p1_table->container_p1_slices.p1_slices[0];
	p1_sliceRight = & p1_table->container_p1_slices.p1_slices[1];
	AbsLeft = Approximate_P1_SLICE(p1_table, p1_sliceLeft, T, Nband);
    AbsRight = Approximate_P1_SLICE(p1_table, p1_sliceRight, T, Nband);
    dP = p1_table->container_p1_slices.dP[0];
    Pleft = p1_table->container_p1_slices.p1_slices[0].P;
	ret = AbsLeft + ( AbsRight -AbsLeft ) * ( p - Pleft ) / dP;
    return ret;
}

double Approximate_P1_TABLE_multiple_slices( void* p1_tableV, double p, double T,int Nband )
{
	double ret,dP,Pleft;
    double AbsLeft,AbsRight;
    P1_TABLE* p1_table;
	P1_SLICE* p1_sliceLeft;
	P1_SLICE* p1_sliceRight;
	int Left;
	p1_table = (P1_TABLE*) p1_tableV;
    Left = FindInterval_CONTAINER_P1_SLICES( &p1_table->container_p1_slices, p);
    p1_sliceLeft = &p1_table->container_p1_slices.p1_slices[Left];
	p1_sliceRight = &p1_table->container_p1_slices.p1_slices[Left+1];
    AbsLeft = Approximate_P1_SLICE(p1_table, p1_sliceLeft, T, Nband);
    AbsRight = Approximate_P1_SLICE(p1_table, p1_sliceRight, T, Nband);
    dP = p1_table->container_p1_slices.dP[Left];
    Pleft = p1_table->container_p1_slices.p1_slices[Left].P;
	ret = AbsLeft + ( AbsRight -AbsLeft ) * ( p - Pleft ) / dP;
    return ret;
}

int Set_dP_dTs_P1_TABLE( P1_TABLE* p1_table )
{
	int i,j;
	int ip1,ip2;
	double T1,T2;
	p1_table->container_p1_slices.dP = (double*) malloc( p1_table->container_p1_slices.used * sizeof(double) );
	if( p1_table->container_p1_slices.dP == NULL )
		return 1;
    for( i=0; i<= p1_table->container_p1_slices.used; i++ )
	{
		p1_table->container_p1_slices.p1_slices[i].dT = (double*) malloc( p1_table->container_p1_slices.p1_slices[i].used * sizeof(double) ); 
        if( p1_table->container_p1_slices.p1_slices[i].dT == NULL )
			return 1;
	}
    for( i = 0; i < p1_table->container_p1_slices.used; i++)
		  	p1_table->container_p1_slices.dP[i] = p1_table->container_p1_slices.p1_slices[i+1].P - p1_table->container_p1_slices.p1_slices[i].P;
    for( i=0; i <= p1_table->container_p1_slices.used; i++)
		for( j=0;j<p1_table->container_p1_slices.p1_slices[i].used;j++ )
		{
			ip1 = p1_table->container_p1_slices.p1_slices[i].ip[j];
			ip2 = p1_table->container_p1_slices.p1_slices[i].ip[j+1];
			T1 = p1_table->Content[ip1].T;
            T2 = p1_table->Content[ip2].T;
            p1_table->container_p1_slices.p1_slices[i].dT[j] = T2 - T1;
		}
	return 0;
}

