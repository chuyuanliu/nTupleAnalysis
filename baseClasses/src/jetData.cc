#include "TChain.h"

#include "nTupleAnalysis/baseClasses/interface/jetData.h"

using namespace nTupleAnalysis;



//jet object
jet::jet(){}
jet::jet(UInt_t i, jetData* data){

  cleanmask = data->cleanmask[i];

  pt  = data->pt [i];
  eta = data->eta[i];
  phi = data->phi[i];
  m   = data->m  [i];
  p = TLorentzVector();
  p.SetPtEtaPhiM(pt, eta, phi, m);
  e = p.E();

  bRegCorr = data->bRegCorr[i];

  deepB     = data->deepB[i];
  CSVv2     = data->CSVv2[i];
  deepFlavB = data->deepFlavB[i];

  nFirstTrack = data->nFirstTrack[i];
  nLastTrack = data->nLastTrack[i];
  
  if(data->trkData){
    tracks = data->trkData->getTracks(nFirstTrack,nLastTrack);
    for(const trackPtr& track: tracks){
      track->dR = track->p.DeltaR(p);
    }
  }


  if(data->btagData->haveSVs){
    svs = data->btagData->getSecondaryVertices(data->nFirstSV[i],data->nLastSV[i]);
  }

  if(data->btagData->haveTrkTagVars){
    trkTagVars = data->btagData->getTrkTagVars(data->nFirstTrkTagVar[i],data->nLastTrkTagVar[i]);
  }

  //
  // Hack to fix trkTagVas phi which is not filled in cmssw
  //
  //for(const trkTagVarPtr& trkTagVar: trkTagVars){
  //  //std::cout << "Matching " << trkTagVar->trackEta << " " << trkTagVar->pt << std::endl;
  //  for(const trackPtr& track: tracks){
  //    //std::cout << "\t " << track->eta << " " << track->pt << std::endl;
  //    if(fabs(track->eta - trkTagVar->trackEta) < 0.0001 && fabs(track->pt - trkTagVar->pt) < 0.0001){
  //	trkTagVar->trackPhi = track->phi;
  //	trkTagVar->p.SetPtEtaPhiM(trkTagVar->pt, trkTagVar->trackEta, trkTagVar->trackPhi, trkTagVar->m);
  //	trkTagVar->e = p.E();
  //	break;
  //    }
  //	
  //  }
  //}
  

}

jet::jet(TLorentzVector& vec, float tag){

  p = TLorentzVector(vec);
  pt  = p.Pt();
  eta = p.Eta();
  phi = p.Phi();
  m   = p.M();
  e   = p.E();

  bRegCorr = pt;

  deepB = tag;
  CSVv2 = tag;
  deepFlavB = tag;
}

void jet::bRegression(){
  p  *= bRegCorr;
  pt  = p.Pt();
  eta = p.Eta();
  phi = p.Phi();
  m   = p.M();
  e   = p.E();
}

jet::~jet(){
}


//access tree
jetData::jetData(std::string name, TChain* tree, std::string prefix){

  initBranch(tree, (prefix+"n"+name).c_str(), n );

  initBranch(tree, (prefix+name+"_cleanmask").c_str(), cleanmask );

  initBranch(tree, (prefix+name+"_pt"  ).c_str(), pt  );  
  initBranch(tree, (prefix+name+"_eta" ).c_str(), eta );  
  initBranch(tree, (prefix+name+"_phi" ).c_str(), phi );  
  initBranch(tree, (prefix+name+"_mass").c_str(), m   );  

  initBranch(tree, (prefix+name+"_bRegCorr").c_str(), bRegCorr );  

  initBranch(tree, (prefix+name+"_btagDeepB"    ).c_str(), deepB     );
  initBranch(tree, (prefix+name+"_btagCSVV2"    ).c_str(), CSVv2     );
  initBranch(tree, (prefix+name+"_btagDeepFlavB").c_str(), deepFlavB );

  //
  //  only load the track if the variables are availible
  //
  int nFirstTrackCode = initBranch(tree, (prefix+name+"_nFirstTrack").c_str(),  nFirstTrack);
  int nLastTrackCode  = initBranch(tree, (prefix+name+"_nLastTrack" ).c_str(),  nLastTrack );
  if(nFirstTrackCode != -1 && nLastTrackCode != -1){
    trkData = new trackData(prefix, tree);
  }


  //
  //  Load the btagging data
  //
  btagData = new btaggingData();

  int nFirstSVCode = initBranch(tree, (prefix+name+"_nFirstSV").c_str(),  nFirstSV);
  int nLastSVCode  = initBranch(tree, (prefix+name+"_nLastSV" ).c_str(),  nLastSV );
  if(nFirstSVCode != -1 && nLastSVCode != -1){
    btagData->initSecondaryVerticies(prefix, tree);
  }

  int nFirstTrkTagVarCode = initBranch(tree, (prefix+name+"_nFirstTrkTagVar").c_str(),  nFirstTrkTagVar);
  int nLastTrkTagVarCode  = initBranch(tree, (prefix+name+"_nLastTrkTagVar" ).c_str(),  nLastTrkTagVar );
  if(nFirstTrkTagVarCode != -1 && nLastTrkTagVarCode != -1){
    btagData->initTrkTagVar(prefix, tree);
  }
  

}


std::vector< std::shared_ptr<jet> > jetData::getJets(float ptMin, float ptMax, float etaMax, bool clean, float tagMin, std::string tagger, bool antiTag){
  
  std::vector< std::shared_ptr<jet> > outputJets;
  float *tag = CSVv2;
  if(tagger == "deepB")     tag = deepB;
  if(tagger == "deepFlavB") tag = deepFlavB;

  for(UInt_t i = 0; i < n; ++i){
    if(clean && cleanmask[i] == 0) continue;
    if(          pt[i]  <  ptMin ) continue;
    if(          pt[i]  >= ptMax ) continue;
    if(    fabs(eta[i]) > etaMax ) continue;
    if(antiTag^(tag[i]  < tagMin)) continue; // antiTag XOR (jet fails tagMin). This way antiTag inverts the tag criteria to select untagged jets
    outputJets.push_back(std::make_shared<jet>(jet(i, this)));
  }

  return outputJets;
}

std::vector< std::shared_ptr<jet> > jetData::getJets(std::vector< std::shared_ptr<jet> > inputJets, float ptMin, float ptMax, float etaMax, bool clean, float tagMin, std::string tagger, bool antiTag){
  
  std::vector< std::shared_ptr<jet> > outputJets;

  for(auto &jet: inputJets){
    if(clean && jet->cleanmask == 0) continue;
    if(         jet->pt   <  ptMin ) continue;
    if(         jet->pt   >= ptMax ) continue;
    if(    fabs(jet->eta) > etaMax ) continue;

    if(     tagger == "deepFlavB" && antiTag^(jet->deepFlavB < tagMin)) continue;
    else if(tagger == "deepB"     && antiTag^(jet->deepB     < tagMin)) continue;
    else if(tagger == "CSVv2"     && antiTag^(jet->CSVv2     < tagMin)) continue;
    outputJets.push_back(jet);
  }

  return outputJets;
}


jetData::~jetData(){ 
  std::cout << "jetData::destroyed " << std::endl;
}

