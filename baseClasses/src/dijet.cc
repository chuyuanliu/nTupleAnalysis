#include "nTupleAnalysis/baseClasses/interface/dijet.h"

using namespace nTupleAnalysis;

//dijet object
//dijet::dijet(){}
dijet::dijet(std::shared_ptr<jet> &jet1, std::shared_ptr<jet> &jet2){

  if(jet1->pt > jet2->pt){
    lead = jet1;
    subl = jet2;
  }else{
    lead = jet2;
    subl = jet1;
  }

  dR  = jet1->p.DeltaR(jet2->p);
  st  = jet1->pt + jet2->pt;
  p   = jet1->p  + jet2->p;
  pt  = p.Pt();
  eta = p.Eta();
  phi = p.Phi();
  m   = p.M();
  e   = p.E();

  pW  = p*(mW/m);
  pZ  = p*(mZ/m);
  pH  = p*(mH/m);

  xW  = m>0 ? (m-mW)/(0.10*m) : -1;
  xZ  = m>0 ? (m-mZ)/(0.10*m) : -1;
  xH  = m>0 ? (m-mH)/(0.10*m) : -1;

}

dijet::~dijet(){}


