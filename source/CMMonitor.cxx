///////////////////////////////////////////////////////////////////////////////
//
// name: CMMonitor.cxx
// date: 5-29-2021
// auth: Michael Gericke 
//
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <CMMonitor.h>

//ClassImp(CMMonitor)  

CMMonitor::CMMonitor(const TGWindow *p, UInt_t w, UInt_t h)
  : TGMainFrame(p, w, h)
{
  dMWWidth = w;
  dMWHeight = h;
  
  std::set_new_handler(0);

  MakeMenuLayout();
  MakeCurrentModeTab(); 
  MapSubwindows();
  Resize(GetDefaultSize());
  MapWindow();
  
}

void CMMonitor::MakeMenuLayout()
{
  
  dMenuFile = new TGPopupMenu(fClient->GetRoot());  
  dMenuFile->AddEntry("&New (New text output)...", M_FILE_NEW);
  dMenuFile->AddEntry("N&ew (New root output)...", M_ROOT_FILE_NEW);  
  dMenuFile->AddEntry("&Open (Text output)...", M_FILE_OPEN);
  dMenuFile->AddEntry("O&pen (Root out file)...", M_ROOT_FILE_OPEN);
  dMenuFile->AddEntry("Open Data Window",M_OPEN_DATAWINDOW);
  dMenuFile->AddEntry("&Close (Text out file)", M_FILE_CLOSE);
  dMenuFile->AddEntry("C&lose (ROOT out file)", M_ROOT_FILE_CLOSE);  
  dMenuFile->AddSeparator();
  dMenuFile->AddEntry("Root File Browser", M_VIEW_BROWSER);
  dMenuFile->AddSeparator();
  dMenuFile->AddSeparator();
  dMenuFile->AddEntry("E&xit", M_FILE_EXIT);

  dMenuSettings = new TGPopupMenu(fClient->GetRoot());
  dMenuSettings->AddEntry("Set Connection Options...", M_SET_CONNECT);
  
  dMenuFile->Associate(this);
  dMenuSettings->Associate(this);
  
  dMenuBar = new TGMenuBar(this, 1, 1, kHorizontalFrame);
  dMenuBar->AddPopup("&File", dMenuFile, new TGLayoutHints(kLHintsTop | kLHintsLeft ,0, 4, 0, 0));
  dMenuBar->AddPopup("&Settings",dMenuSettings, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0));
  
  AddFrame(dMenuBar, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,0, 0, 1, 1));
}

void CMMonitor::MakeCurrentModeTab()
{
  TGTab *Tab = new TGTab(this,dMWWidth-15,dMWHeight-80);  
  TGCompositeFrame *tf = Tab->AddTab("Current Mode");
  TGVerticalFrame *vf1 = new TGVerticalFrame(tf,10,10);
  
  TGHorizontalFrame *hf[NUM_CHANNELS];
  TGLabel *dChanLabel[NUM_CHANNELS];
  TGLabel *dRateLabel[NUM_CHANNELS];

  for(int n = 0; n < NUM_CHANNELS; n++){

    hf[n] = new TGHorizontalFrame(vf1,10,10);
    vLabF[n] = new TGVerticalFrame(hf[n],10,10);
    vLabF[n]->Resize(400,10); //this doesn't seem to be doing anything in the case :(
    dChanLabel[n] = new TGLabel(vLabF[n],Form("Channel %02d                    ",n)); //leave the empty space to stretch the label
    vLabF[n]->AddFrame(dChanLabel[n],new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
    dRateFrame[n] = new TGHorizontalFrame(vLabF[n],10,10);
    dRateLabel[n] = new TGLabel(dRateFrame[n],"Rate:  "); //leave the empty space to stretch the label
    dRateCounter[n] = new TGLabel(dRateFrame[n],"1.0");
    dRateFrame[n]->AddFrame(dRateLabel[n],new TGLayoutHints(kLHintsTop | kLHintsLeft, 10, 2, 2, 2));
    dRateFrame[n]->AddFrame(dRateCounter[n],new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
    vLabF[n]->AddFrame(dRateFrame[n],new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));   
    dCurrMSmplGrCv[n] = new TRootEmbeddedCanvas(Form("SmplGr_Ch%d",n), hf[n], 10, 10);
    dCurrMSmplHstCv[n] = new TRootEmbeddedCanvas(Form("SmplHst_Ch%d",n), hf[n], 10, 10);
    dCurrMSmplFFTCv[n] = new TRootEmbeddedCanvas(Form("SmplFFT_Ch%d",n), hf[n], 10, 10);
    hf[n]->AddFrame(vLabF[n], new TGLayoutHints( kLHintsLeft | kLHintsTop | kLHintsExpandY, 2, 2, 5, 5));
    hf[n]->AddFrame(dCurrMSmplGrCv[n],new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 2, 2, 5, 5));
    hf[n]->AddFrame(dCurrMSmplHstCv[n],new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY,2, 2, 5, 5));
    hf[n]->AddFrame(dCurrMSmplFFTCv[n],new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY,2, 2, 5, 5));
    vf1->AddFrame(hf[n], new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX | kLHintsExpandY));
    if(n < NUM_CHANNELS-1){
      TGHorizontal3DLine *Line = new TGHorizontal3DLine(vf1);
      vf1->AddFrame(Line, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
    }
  }
  
  vf1->Resize(dMWWidth-15,dMWHeight-15);
  tf->AddFrame(vf1,new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX | kLHintsExpandY));
  AddFrame(Tab, new TGLayoutHints(kLHintsBottom | kLHintsExpandX | kLHintsExpandY,2, 2, 5, 1));
  Tab->MapSubwindows();
  Tab->Layout();

}


void CMMonitor::OnObjClose(char *obj)
{
  TString str = obj;

  if(str.Contains("dataWindow")) CloseWindow();
}

CMMonitor::~CMMonitor()
{

}

void CMMonitor::CloseWindow()
{
  gApplication->Terminate(0);
}

void CMMonitor::PadIsPicked(TPad* selpad, TObject* selected, Int_t event)
{
  printf("Pad number = %d\n",selpad->GetNumber());
}

void CMMonitor::MainTabEvent(Int_t event, Int_t x, Int_t y, TObject* selobject)
{
  if(event == kButton1Double){
    // Int_t pad = dMainCanvas->GetCanvas()->GetSelectedPad()->GetNumber();

    // if(pad == 3){
    //   // if(!dM2M1RatioGraph) return;
    //   // PlotMiscData(dM2M1RatioGraph);
    // }
    // if(pad == 4){
    //   // if(!dPolGraph) return;
    //   // PlotMiscData(dPolGraph);
    // }
    // if(pad == 5){
    //   // if(!dM3M2RatioGraph) return;
    //   // PlotMiscData(dM3M2RatioGraph);
    // }
  }
}


Bool_t CMMonitor::ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2)
{
  // Handle messages send to the MainFrame object. E.g. all menu button
  // messages.
  switch (GET_MSG(msg)){

  case kC_TEXTENTRY:
    switch (GET_SUBMSG(msg)) {
    case kTE_ENTER:
      switch (parm1) {

      case M_RUN_SELECT:

      default:
	break;
      }

    default:
      break;
    }

  case kC_COMMAND:
    switch (GET_SUBMSG(msg)) {
      
    case kCM_COMBOBOX:
      {
	switch (parm1) {
	case M_TBIN_SELECT:
	  break;
	}
      }
      break;

    case kCM_MENUSELECT:
      break;
      
    case kCM_MENU:
      switch (parm1) {
      
      case M_ROOT_FILE_OPEN:
	//OpenRootFile();
	break;
	
      case M_FILE_OPEN:

	break;

      case M_OPEN_DATAWINDOW:
	break;
	
      case M_FILE_EXIT:
	CloseWindow();   // this also terminates theApp
	break;
	
      case M_ROOT_FILE_CLOSE:
	// CloseRootFile();
	break;
	
      case M_FILE_CLOSE:
	break;

      case M_SET_CONNECT:
	// DoConfigSettings();
	CountTest();
	break;


      case M_VIEW_BROWSER:
	// OpenRootBrowser();
	break;

      case M_HELP_CONTENTS:
	// Open the online help manual in a web browser.
        { 
		// pid_t child = fork(); 
		// if (-1 == child) perror("couldn't fork to open web browser"); 
		// if (0 == child) {
		// 	execl("/bin/sh", "/bin/sh", "-c", 
		// 	      "mozilla "
		// 	      "http://sns.phys.utk.edu/svn/npdg/trunk/analysis/online/Manual/OnLineAnalysisTasks.html",
		// 	      (char*)0); 
		// 	perror("couldn't exec shell for web browser"); 
		// 	exit(1); 
		// }
	}
	break;

      case M_HELP_SEARCH:
	 {
// #		define URL "http://sns.phys.utk.edu/svn/npdg/trunk/analysis/online"
// 		pid_t child = fork(); 
// 		if (-1 == child) perror("couldn't fork to show change history"); 
// 		if (0 == child) {
// 			execl("/bin/sh", "/bin/sh", "-c", 
// 			      "xterm -title 'Online Analysis Change History' "
// 			      " -e \" "
// 			      "(echo changes to " URL " && svn log " URL " ) | less"
// 			      " \" "
// 			      , (char*)0); 
// 			perror("couldn't exec shell for web browser"); 
// 			exit(1); 
// 		}
	 }
	 break;

      case M_HELP_ABOUT:
	// were I able to make a cute box, I would put it here
	break;

      default:
	break;
      }
      
    default:
      break;
    }
    
  default:
    break;
  }
  
  return kTRUE;
}


void CMMonitor::CountTest()
{
  time_t inittime = time(NULL);
  time_t currtime;
  time_t delta;
  uint32_t counter = 0;

  while(1){
    currtime = time(NULL);
    delta = currtime - inittime;
      
    if (delta >= 0.1){
      
      dRateCounter[0]->SetText(Form("%d",counter));
      dRateFrame[0]->Layout();
      //cout << counter << endl;
      //vLabF[0]->Layout();
      counter++;
      inittime = time(NULL);
      gSystem->ProcessEvents();
    }
  }
}

int main(int argc, char **argv)
{
   TApplication theApp("CMMonitor", &argc, argv);

   if (gROOT->IsBatch()) {
      fprintf(stderr, "%s: cannot run in batch mode\n", argv[0]);
      return 1;
   }

   CMMonitor mainWindow(gClient->GetRoot(), 1200,800);

   theApp.Run();


   return 0;
}
