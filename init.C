{
TTree *gTree;
Int_t gRunInProgress = 0;
//TCanvas c1("c1","Amplitudes");
//TCanvas *gc1;
//Int_t gFirstEntry=1;
//Int_t gNEntries=999999;

    cout<<"gSystem.Load(""Tdq"")"<<endl;
    gSystem->Load("Tdq");
    Tdq *gdq=new Tdq();

    //gh = new TH2S("adc-chn","",128,0,128,256,0,256);
    gStyle->SetPalette(1);

    //load script to be called by run.C
    gROOT->ProcessLine(".L process_file.C");

    //obsolete//cout<<"Now you can start the analysis using '.x run.C or .x check_for_file.C()'"<<endl;

    // Set the run-specific globals
    gdq->gDebug = 0;

    // The following line may be necessary to account for extra words
    gdq->gExtraWords = 0;

    // if mapping is on then analysis will be in dtector channels space, otherwise - SVX4 space
    gdq->gStripMapping = 1;
    
    // Enable/disable clustering
    gdg->gClustering = 1;
    
    // Pedestal subtraction
    gdq->gSubtractPeds = 0;
    
    // Common Mode Subtraction
    // 1: calculate but not subtract
    // 2: subtract
    gdq->gCMNControl = 2;
    
    // Number of events to process
    gdq->gMaxEntries = 1000000;

    // Process the newest file
    // last_file();
    
    //''''''''''''''''''''''''''''''''''''''''''
    // Cuts
    // select events unly with good Preamp Reset
    TCut cutPT = "hPARTime<7 || hPARTime>9";
    // non-dead channels in chain 0
    // in svx space:
    //TCut cutDead0 = "chn!=62 && chn!=64 && chn!=128 && chn!=129 && chn!=95";
    // cut noisy hits:
    TCut cutDead = "hitp[0]!=63 && hitp[0]!=64 && hitp[1]!=0 && hitp[1]!=97";
    //
    // cut noise
    TCut cutHitThr = "hitv[0]>15 && hitv[1]>15";
    //
    // final cut
    TCut cut = cutPT;
    
    // popular histograms, these lines need to be executed manually after process_file(), 
    hTitle = ""; hTitle2 = "";
    TH2S *h2 = new TH2S("h2",hTitle,256,0,256,250,0,256); TH1S *h1 = new TH1S("h1",hTitle,257,-1,256);
    //
}


