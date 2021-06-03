/////////////////////////////////////////////////////////////////////////////////////
//Root Based Data Visualization and Analysis Utility Library
//
//Version 000
//
//RPlotList source file
/////////////////////////////////////////////////////////////////////////////////////


#include "RPlotList.h"
#include <assert.h>

RPlotList::RPlotList()
{
  FirstElement=NULL;
  LastElement=NULL;
  CurrentElement=NULL;
  NewElement=NULL;

  dErrVal = ERR_PTR_VAL;
  dCount = 0;
};

RPlotList::~RPlotList()
{
  EmptyList();
}

void RPlotList::EmptyList()
{
  CurrentElement = FirstElement;
  ListElement* Temp = NULL;

  while(CurrentElement != NULL)
    {
      Temp = CurrentElement;
      CurrentElement = CurrentElement->NextElement;
      Cleanup(Temp);
      delete Temp;
    };
  
  FirstElement = NULL;
  LastElement=NULL;
  CurrentElement=NULL;
  NewElement=NULL;

  dCount = 0;
}

void RPlotList::Cleanup(ListElement *elem)
{
  if(!elem) return;

  if(elem->fGraph       != NULL){delete elem->fGraph;      };
  if(elem->fGraphEr     != NULL){delete elem->fGraphEr;    };
  if(elem->fGraphAsymEr != NULL){delete elem->fGraphAsymEr;};
  if(elem->fS1Histo     != NULL){delete elem->fS1Histo;    };
  if(elem->fF1Histo     != NULL){delete elem->fF1Histo;    };
  if(elem->fD1Histo     != NULL){delete elem->fD1Histo;    };
  if(elem->fS2Histo     != NULL){delete elem->fS2Histo;    };
  if(elem->fF2Histo     != NULL){delete elem->fF2Histo;    };
  if(elem->fD2Histo     != NULL){delete elem->fD2Histo;    };
  if(elem->fS3Histo     != NULL){delete elem->fS3Histo;    };
  if(elem->fF3Histo     != NULL){delete elem->fF3Histo;    };
  if(elem->fD3Histo     != NULL){delete elem->fD3Histo;    };
  if(elem->fF1Func      != NULL){delete elem->fF1Func;     };
  if(elem->fMGraph      != NULL){delete elem->fMGraph;     };
}

TObject *RPlotList::GetNewPlotObjectPtr(Char_t *type)
{
  if(!type || !*type) return (TObject*)&dErrVal;
  
  NewElement = new ListElement;

  assert(NewElement);
  
  if(NewElement != NULL){
    if(dCount == 0){
      NewElement->NextElement = NULL;
      NewElement->PrevElement = NULL;
      FirstElement = LastElement = NewElement;	  
      dCount = 1;
      SetElement(FirstElement,type);
      return GetObject(FirstElement);
    }
    else{
      NewElement->NextElement = NULL;
      NewElement->PrevElement = LastElement;
      LastElement->NextElement = NewElement;
      LastElement = LastElement->NextElement;
      dCount++;
      SetElement(LastElement,type);
      return GetObject(LastElement);
    }
  }
  return (TObject*)&dErrVal;  
}

TObject *RPlotList::GetObject(ListElement *elem)
{
  if(!elem) return (TObject*)&dErrVal;

  if(!strcmp(elem->dType,"TGraphErrors"))     return (TObject*)(elem->fGraphEr);
  if(!strcmp(elem->dType,"TGraphAsymmErrors"))return (TObject*)(elem->fGraphAsymEr);
  if(!strcmp(elem->dType,"TGraph"))           return (TObject*)(elem->fGraph);
  if(!strcmp(elem->dType,"TH1S"))             return (TObject*)(elem->fS1Histo);
  if(!strcmp(elem->dType,"TH1F"))             return (TObject*)(elem->fF1Histo);
  if(!strcmp(elem->dType,"TH1D"))             return (TObject*)(elem->fD1Histo);
  if(!strcmp(elem->dType,"TH2S"))             return (TObject*)(elem->fS2Histo);
  if(!strcmp(elem->dType,"TH2F"))             return (TObject*)(elem->fF2Histo);
  if(!strcmp(elem->dType,"TH2D"))             return (TObject*)(elem->fD2Histo);
  if(!strcmp(elem->dType,"TH3S"))             return (TObject*)(elem->fS3Histo);
  if(!strcmp(elem->dType,"TH3F"))             return (TObject*)(elem->fF3Histo);
  if(!strcmp(elem->dType,"TH3D"))             return (TObject*)(elem->fD3Histo);
  if(!strcmp(elem->dType,"TF1"))              return (TObject*)(elem->fF1Func);
  if(!strcmp(elem->dType,"TMultiGraph"))      return (TObject*)(elem->fMGraph);

  return (TObject*)&dErrVal;
}

void RPlotList::SetElement(ListElement* elem, Char_t *type)
{
  if(!elem) return;
  if(!type || !*type) return;
  elem->fGraph       = NULL;
  elem->fGraphEr     = NULL;
  elem->fGraphAsymEr = NULL;
  elem->fS1Histo     = NULL;
  elem->fF1Histo     = NULL;
  elem->fD1Histo     = NULL;
  elem->fS2Histo     = NULL;
  elem->fF2Histo     = NULL;
  elem->fD2Histo     = NULL;
  elem->fS3Histo     = NULL;
  elem->fF3Histo     = NULL;
  elem->fD3Histo     = NULL;
  elem->fF1Func      = NULL;
  elem->fMGraph      = NULL;

  elem->dIndex = dCount;
  strcpy(elem->dType,type);
}


TObject *RPlotList::GetPlotObject(int ind)
{
  assert(FirstElement);

  CurrentElement = FirstElement;
  ListElement *Temp = NULL;
  
  if(ind > 0 && ind <= dCount){
    while(CurrentElement != NULL){
      if(CurrentElement->dIndex == ind){
	return GetObject(Temp);
      }
      CurrentElement = CurrentElement->NextElement;
    } 
  }
  return (TObject*)&dErrVal;
}

TObject *RPlotList::GetPlotObject(Char_t *name)
{
  assert(FirstElement);

  CurrentElement = FirstElement;

  while(CurrentElement != NULL){
    TObject *obj = GetObject(CurrentElement);
    if(obj){
      if(!strcmp(obj->GetName(),name)){
	return obj;
      }
    }
    CurrentElement = CurrentElement->NextElement;
  }
  return(NULL);
}
