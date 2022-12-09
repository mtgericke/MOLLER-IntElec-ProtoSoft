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

  SettingsFile = NULL;
  SettingsFile = new ifstream("CMMonitorSettings.txt");
  if(SettingsFile){
    if(SettingsFile->is_open()){

      *SettingsFile >> iSettings.IP;
      *SettingsFile >> iSettings.currentRun;
      *SettingsFile >> iSettings.currentData0;
      *SettingsFile >> iSettings.currentData1;
      *SettingsFile >> iSettings.PreScFactor;
      *SettingsFile >> iSettings.RunLength;

      //cout << iSettings.IP.data() << endl;
      //cout << iSettings.currentRun << endl;
      SettingsFile->close();
    }    
  }
  else{
    iSettings.IP = "192.168.2.227";
    iSettings.currentRun = 1;
    iSettings.currentData0 = 1;
    iSettings.currentData1 = 2;
    iSettings.PreScFactor = 1;
    iSettings.RunLength = 1;
  }
  
  dNRunsSeq = 1;
  dNRunSeqCnt = 0;
  
  SamplesOutFileName = Form("Int_Run_%03d.dat",iSettings.currentRun);
  ReadNSamples = iSettings.RunLength*SAMPLES_PER_SECOND/iSettings.PreScFactor;
  RUN_START = false;
  RUN_STOP = false;
  RUN_ON = false;
  
  RunStartTime = 0;
  RunStartIndex = 0;
  
  readThreadArgs = new rArgs;
  socket = NULL;
  context = NULL;

  SetDataFileOpen(false);
  
  std::set_new_handler(0);

  MakeMenuLayout();
  MakeUtilityLayout();
  MakeCurrentModeTab();
  MapSubwindows();
  Resize(GetDefaultSize());
  MapWindow();
}

Bool_t CMMonitor::ConnectBoard()
{  
  TString ip;
  Bool_t retflag;
  //string server = "tcp://10.32.182.93:5556";
  string tmp = iSettings.IP.data();
  server = "tcp://"+ tmp + ":5556";
  cout << "Connecting to server:  " << server.data() << endl;

  context = zmq_ctx_new();
  zmq_ctx_set(context, ZMQ_IO_THREADS, 4);


  return 1;
}

void CMMonitor::StartDataCollection()
{
  if(IsDataFileOpen())
    CloseDataFile();
  
  string tmp = iSettings.IP.data();
  char c0[10];
  memset(c0,'\0',10);
  sprintf(c0, "%X", iSettings.currentData0-1);
  string ch0 = c0;
  char c1[10];
  memset(c1,'\0',10);
  sprintf(c1, "%X", iSettings.currentData1-1);
  string ch1 = c1;

  rawPkt *pkt = NULL;

  int flag = 0;
    
  while(1){

    if(dNRunSeqCnt < dNRunsSeq && dNRunSeqCnt > 0){
      sleep(10); //seconds
      RUN_START = true;
    }
    
    if(RUN_START){

      iSettings.currentRun++;
      dRunEntry->SetNumber(iSettings.currentRun);

      if(socket) {
	zmq_close(socket);
	socket = NULL;
      }

      string contrl = "python3 moller_ctrl.py "+ tmp + " write 0x44 " + "0x8" + std::to_string(iSettings.PreScFactor-1) + ch0 + ch1 +"1000"+'\n';
      
      socket = zmq_socket(context,ZMQ_SUB);
      
      zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "ADC",3);
      zmq_setsockopt(socket, ZMQ_RCVBUF, "", SAMPLES_PER_SECOND);
      
      if(zmq_connect (socket, server.data()) != 0) {
	printf("Failed to Bind ZMQ to port 5556 - quitting this process\n");
	return;
      }
           
      cout << contrl << endl;
      system(contrl.data());
      
      pkt = new rawPkt;
      if(!pkt){
	return;
      }
      pkt->data = (uint8_t*)malloc(MAX_ALLOC);
      if(!pkt->data) {
	return;
      }
  
      readThreadArgs->FName = SamplesOutFileName.data();
      cout << "Writing to: " << SamplesOutFileName.data() << endl;
      readThreadArgs->NSamples = ReadNSamples;
      readThreadArgs->sock = socket;
      readThreadArgs->pkt = pkt;
      
      pthread_create(&thread_cap_id, NULL, GetServerData, (void*)readThreadArgs);
      if(!pthread_join(thread_cap_id, NULL)){
	dataQue.push(pkt);
	//cout << "this queue size = " << dataQue.size() << endl;
	RUN_START = false;
	RUN_ON = false;
	
      }
      // flag = 1;
      if(dNRunsSeq == 1)
	FillDataPlots();
      else{
	
	if(!dataQue.empty()){
	  
	  pkt = (rawPkt*)dataQue.front();
	  free(pkt->data);
	  dataQue.pop();
	  // delete pkt;
	  pkt = NULL;
	  
	  dNRunSeqCnt++;
	  
	  if(dNRunSeqCnt == dNRunsSeq){
	    RUN_START = false;
	    RUN_ON = false;

	    cout << endl;
	    cout << "Sqeuence of " << dNRunSeqCnt << " done!" << endl;
	    cout << endl;
	  }
	}
      }
      if(!IsDataFileOpen())
	WriteSettings();
      
    }
    gSystem->ProcessEvents();
    
    if(RUN_STOP)
      break;
  }
  RUN_STOP = false;
}

void *CMMonitor::GetServerData(void *vargp)
{
  FILE *SamplesOutFile;
  uint8_t *data;
  size_t len;
  uint64_t samples_written;
  uint64_t data_written;
  uint64_t pkts_rx;
  zmq_msg_t message;
  zmq_msg_t msg_samples;

  
  SamplesOutFile = fopen(((rArgs*)vargp)->FName.data(), "wb");
  if(!SamplesOutFile) {
    return NULL;
  }

  samples_written = 0;
  data_written = 0;
  pkts_rx = 0;

  cout << "NSample = " << ((rArgs*)vargp)->NSamples << endl;
  
  while(samples_written < 2.0*((rArgs*)vargp)->NSamples) {
    //The factor of 2 needs to be there, because I want to specify the number of samples per channel.
    //However the way the ZMQ code is written below, it reads both channels at the same time. Which means
    //that NSamples above would be split between the two channels.

    zmq_msg_init (&message);
    zmq_msg_recv (&message, ((rArgs*)vargp)->sock, 0);
    
    data = (uint8_t*)zmq_msg_data(&message);
    len = zmq_msg_size(&message);
    
    if(strncmp("ADC", (char*)data, 3) == 0) {

      zmq_msg_init (&msg_samples);
      zmq_msg_recv (&msg_samples, ((rArgs*)vargp)->sock, 0);
      
      pkts_rx++;
      data = (uint8_t*)zmq_msg_data(&msg_samples);
      len = zmq_msg_size(&msg_samples);
      
      memcpy(&(((rArgs*)vargp)->pkt->data)[data_written], data, len);
      
      data_written += len;
      samples_written += (len - 16) / 4;

      // memcpy(pkt->data,data,len);
      
      zmq_msg_close(&msg_samples);
    }
    zmq_msg_close (&message);
    // data_written += 100000;
    // samples_written += (100000-16)/4;
  }
  
  fwrite(((rArgs*)vargp)->pkt->data, data_written, 1, SamplesOutFile);
  fclose(SamplesOutFile);
  ((rArgs*)vargp)->pkt->length = data_written;
  
  printf("Pkts: %ld Samples: %ld SamplesPerPacket: %ld Bytes: %ld\n", pkts_rx, samples_written, (data_written / (pkts_rx * 16)) * 2, data_written);
  
  //free(sample_data);
  
  pthread_exit(NULL);
  
}

void CMMonitor::FillDataPlots()
{
  //pkt *data;
  pkt *data;
  rawPkt *rPkt;

  size_t bi = 0;
  uint16_t num_words;  //2 bytes
  uint32_t num_pkt;    //4 bytes
  uint8_t padding;     //1 byte
  uint8_t id;          //1 byte to unsigned int
  uint64_t tStamp;     //8 bytes
  
  uint16_t nSamp;
  uint16_t SampRead = 0;
  int32_t ch0;           //4 bytes
  int32_t ch1;           //4 bytes
  int32_t ch0_data;      //4 bytes
  int32_t ch1_data;      //4 bytes
  uint32_t ch0_num;      //4 bytes
  uint32_t ch1_num;      //4 bytes
  uint32_t PreSc;
  uint64_t sTime;          //absolute sample time stamp for each run
  uint64_t sTimeP = 0;     //dumy
  uint64_t iTime;          //run start time stamp  
  uint64_t cTime = 0;      //current time stamp within run relative to run start time
  uint64_t cTimeP = 0;
  uint64_t tStampP = 0;
  
  
  double t1 = 0, t2 = 0;
  int p = 0, k = 0;

  DataSamples *thisData;

  if(!dataQue.empty()){

    thisData = new DataSamples;
    thisData->ch0_sum = 0;
    thisData->ch1_sum = 0;
    thisData->ch0_ssq = 0;
    thisData->ch1_ssq = 0;
    PlotData.push_back(thisData);
    
    rPkt = dataQue.front();
    if(rPkt){     
      while(bi < rPkt->length){
	
	memcpy(&num_words,&(rPkt->data)[bi+0],2);
	memcpy(&num_pkt,&(rPkt->data)[bi+2],4);
	memcpy(&padding,&(rPkt->data)[bi+6],1);
	memcpy(&id,&(rPkt->data)[bi+7],1);
	memcpy(&tStamp,&(rPkt->data)[bi+8],8);	
 	nSamp = num_words - 1;
	
	for(int n = 0; n < nSamp; n++){

	  //data = new pkt;
	  
	  memcpy(&ch0,&(rPkt->data)[bi+16+n*8],4);
	  memcpy(&ch1,&(rPkt->data)[bi+16+n*8+4],4);

	  ch0_data = ch0 >> 14;
	  ch1_data = ch1 >> 14;
	  PreSc = ((ch0 >> 4) & 0x7F)+1;
	  ch0_num = ch0 & 0xF;
	  ch1_num = ch1 & 0xF;

	  // if(bi == 0 && n == 0){
	  //   if(IsDataFileOpen()){
	  //     iSettings.currentData0 = ch0_num;
	  //     iSettings.currentData1 = ch1_num;
	  //     iSettings.PreScFactor = PreSc;
	  //     Ch0ListBox->Select(iSettings.currentData0+3100,false);
	  //     Ch1ListBox->Select(iSettings.currentData1+3100,false);
	  //     dSmplDivEntry->SetNumber((double)PreSc);
	  //     gSystem->ProcessEvents(); 
	  //   }
	  // }
	  
	  if(ch0_num == ch1_num){
	    sTime = (tStamp + ((n*2) * TS_CONVERSION * PreSc)) *  TS_TO_NS;
	  }
	  else{
	    sTime = (tStamp + (n * TS_CONVERSION * PreSc)) * TS_TO_NS;
	  }

	  if(!p){
	    iTime = sTime;
	    sTimeP = sTime;
	    cTimeP = cTime;
	    tStampP = tStamp;
	  }

	  cTime = sTime-iTime + RunStartTime;

	  thisData->ch0_data.push_back(ch0_data*ADC_CONVERSION);
	  thisData->ch1_data.push_back(ch1_data*ADC_CONVERSION);
	  thisData->ch0_sum += ch0_data*ADC_CONVERSION;
	  thisData->ch1_sum += ch1_data*ADC_CONVERSION;
	  thisData->ch0_ssq += ch0_data*ADC_CONVERSION*ch0_data*ADC_CONVERSION;
	  thisData->ch1_ssq += ch1_data*ADC_CONVERSION*ch1_data*ADC_CONVERSION;
	  thisData->tStmp.push_back(cTime*1e-6);

	  ChSigHst[0]->Fill(ch0_data*ADC_CONVERSION);
	  ChSigHst[1]->Fill(ch1_data*ADC_CONVERSION);
	  ChSigGr[0]->SetPoint(p+RunStartIndex,cTime*1e-6,ch0_data*ADC_CONVERSION);
	  ChSigGr[1]->SetPoint(p+RunStartIndex,cTime*1e-6,ch1_data*ADC_CONVERSION);
	  // if(cTime - cTimeP < 0){
	  //   cout.precision(17);
	  //   cout << "bi = " << bi << " p = " << p << " tStamp = " << tStamp << " tStampP = " << tStampP << " tStampDiv = " << tStamp-tStampP << "  sTime = " << (sTime-iTime) << "  sTimeDiv = " << (sTime - sTimeP)*1e-6 << "  cTime = " << cTime*1e-6 << " ch0 =  "<<  ch0_data*ADC_CONVERSION << endl;
	  // }
	  
	  sTimeP = sTime;
	  cTimeP = cTime;
	  tStampP = tStamp;
	  if(!(p%50000)){
	    dCurrMSmplHstCv[0]->GetCanvas()->cd();
	    gPad->Modified();
	    gPad->Update();
	    dCurrMSmplHstCv[1]->GetCanvas()->cd();
	    gPad->Modified();
	    gPad->Update();
	  }
	  gSystem->ProcessEvents();
	  p++;
	  // RunStartIndex++;
	}
	
	k++;
	bi = bi + 16 + nSamp*8;
      }
      
      RunStartTime = cTime;
      RunStartIndex += p;
      thisData->PreScF = PreSc;
      thisData->ch0_num = ch0_num;
      thisData->ch1_num = ch1_num;
      thisData->ch0_num = ch0_num;
      thisData->ch0_mean = thisData->ch0_sum/thisData->ch0_data.size(); 
      thisData->ch1_mean = thisData->ch1_sum/thisData->ch1_data.size(); 
      thisData->ch0_sig = sqrt(thisData->ch0_ssq/thisData->ch0_data.size()-thisData->ch0_mean*thisData->ch0_mean); 
      thisData->ch1_sig = sqrt(thisData->ch1_ssq/thisData->ch1_data.size()-thisData->ch1_mean*thisData->ch1_mean);
      if(IsDataFileOpen()){
	iSettings.currentData0 = ch0_num;
	iSettings.currentData1 = ch1_num;
	iSettings.PreScFactor = PreSc;
	Ch0ListBox->Select(iSettings.currentData0+3100,false);
	Ch1ListBox->Select(iSettings.currentData1+3100,false);
	dSmplDivEntry->SetNumber((double)PreSc);
	//gSystem->ProcessEvents();
	iSettings.RunLength = ceil(p*PreSc/SAMPLES_PER_SECOND);
	dRunTimeEntry->SetNumber(p*PreSc/SAMPLES_PER_SECOND);
      }
      thisData->RunLength = iSettings.RunLength;
      thisData->NSamples = ReadNSamples;
      
      dataQue.pop();
      free(rPkt->data);
      delete rPkt;

      ChSigHstHR[0]->FillN(thisData->ch0_data.size(),thisData->ch0_data.data(),0);     
      ChSigHstHR[1]->FillN(thisData->ch1_data.size(),thisData->ch1_data.data(),0);

      dCurrMSmplHstCv[0]->GetCanvas()->cd();
      gPad->Modified();
      gPad->Update();
      dCurrMSmplHstCv[1]->GetCanvas()->cd();
      gPad->Modified();
      gPad->Update();

      gSystem->ProcessEvents();
    }
  }
}

Int_t CMMonitor::OpenDataFile(ERFileStatus status, const char* file)
{
  if(IsDataFileOpen()) CloseDataFile();
  char filename[NAME_STR_MAX];
  if(!file){
    if(GetFilenameFromDialog(filename,"dat",status) == PROCESS_FAILED)
      return PROCESS_FAILED;
  }
  else
    strcpy(filename,file);

  if(strcmp(".dat",strrchr(filename,'.'))) return PROCESS_FAILED;
  
  string name = filename;
  int num;

  int p1 = name.find("Run_");
  int p2 = name.find(".");
  string number = name.substr(p1 + 4, p2 - p1 +3);
  iSettings.currentRun = atoi(number.c_str());
  cout << "Opening run: " << iSettings.currentRun << endl;
  
  SetDataFileOpen(kTrue);
  SetDataFileName(filename);

  // FILE *ifile = fopen(filename,"rb");
  ifstream ifile(filename, std::ifstream::binary);
  
  if(ifile.is_open()){

    ifile.seekg (0, ios::end);
    size_t length = ifile.tellg();
    rawPkt *pkt = new rawPkt;
    pkt->data = (uint8_t*)malloc(length);
   
    // char * memblock;    
    // memblock = new char [length];
    
    ifile.seekg (0, ios::beg);
    ifile.read ((char*)pkt->data,length);
    ifile.close();
    pkt->length = length;
    dataQue.push(pkt);
    
    

    // memcpy(&(((rArgs*)vargp)->pkt->data)[data_written], data, len);
    
    
    // size_t nb = 0;
    
    // while(nb < length){

      

    // }

    return PROCESS_OK;
  
  }
  
  return PROCESS_FAILED;

}

Int_t CMMonitor::SaveDataFile(ERFileStatus status, const char* file)
{
  if(!IsDataFileOpen()) return PROCESS_FAILED;
  char filename[NAME_STR_MAX];
  if(!file){
    if(GetFilenameFromDialog(filename,"dat",status) == PROCESS_FAILED)
      return PROCESS_FAILED;
  }
  else
    strcpy(filename,file);

  if(strcmp(".dat",strrchr(filename,'.'))) return PROCESS_FAILED;

  return PROCESS_OK;
}


void CMMonitor::CloseDataFile()
{
  SetDataFileOpen(false);
  SetDataFileName("");
  ClearData();
  ClearPlots();
}

void CMMonitor::ClearData()
{
  DataSamples *run;
  
  for(int n = 0; n < PlotData.size(); n++){
    
    run = PlotData[n];

    if(run){
      run->tStmp.clear();
      run->tStmp.resize(0);
      run->ch0_data.clear();
      run->ch0_data.resize(0);
      run->ch1_data.clear();
      run->ch1_data.resize(0);
    }
  }

  PlotData.clear();
  PlotData.resize(0);

}

void CMMonitor::ClearPlots()
{
  if(ChSigHst[0])
    ChSigHst[0]->Reset();     
  if(ChSigHst[1])
    ChSigHst[1]->Reset();

  if(ChSigHstHR[0])
    ChSigHstHR[0]->Reset();     
  if(ChSigHstHR[1])
    ChSigHstHR[1]->Reset();
  
  if(ChSigGr[0]){
    delete ChSigGr[0];
    ChSigGr[0] = new TGraph();
    ChSigGr[0]->SetName(Form("ChSigGr_%02d",0));
    
  }
  if(ChSigHstHR[1]){
    delete ChSigGr[1];
    ChSigGr[1] = new TGraph();
    ChSigGr[1]->SetName(Form("ChSigGr_%02d",1));
  }


  RunStartTime = 0;
  RunStartIndex = 0;

  
  if(fftTmpCh0)
    fftTmpCh0->Reset();
  if(fftTmpCh1)
    fftTmpCh1->Reset();
  if(fftCh0)
    fftCh0->Reset();
  if(fftCh1)
    fftCh1->Reset();
    

  dCurrMSmplHstCv[0]->GetCanvas()->cd();
  gPad->Modified();
  gPad->Update();
  dCurrMSmplHstCv[1]->GetCanvas()->cd();
  gPad->Modified();
  gPad->Update();
  dCurrMSmplHstHRCv[0]->GetCanvas()->cd();
  gPad->Modified();
  gPad->Update();
  dCurrMSmplHstHRCv[1]->GetCanvas()->cd();
  gPad->Modified();
  gPad->Update();
  dCurrMSmplGrCv[0]->GetCanvas()->cd();
  gPad->Modified();
  gPad->Update();
  dCurrMSmplGrCv[1]->GetCanvas()->cd();
  gPad->Modified();
  gPad->Update();
  dCurrMSmplFFTCv[0]->GetCanvas()->cd();
  gPad->Modified();
  gPad->Update();
  dCurrMSmplFFTCv[1]->GetCanvas()->cd();
  gPad->Modified();
  gPad->Update();


}


Int_t CMMonitor::GetFilenameFromDialog(char *file, const char *ext,
				      ERFileStatus status, Bool_t kNotify,
				      const char *notifytext)
{
  int retval = 0;

  if(kNotify && notifytext){

    new TGMsgBox(fClient->GetRoot(), this,"File Open Operation",
		 notifytext,kMBIconQuestion, kMBOk | kMBCancel, &retval);
    if(retval == kMBCancel) return PROCESS_FAILED;
  }

  if(!ext || !*ext) return PROCESS_FAILED;
  if(!file) return PROCESS_FAILED;
  TString fext = ext;
  const char *filetypes[12];
  Int_t index = 0;


  if(fext.Contains("dat")){
     filetypes[index] = "Data files"; index++;
     filetypes[index] = "*.dat"; index++;
  }
  if(fext.Contains("txt")){
     filetypes[index] = "Data files"; index++;
     filetypes[index] = "*.txt"; index++;
  }
  if(fext.Contains("csv")){
     filetypes[index] = "Data files"; index++;
     filetypes[index] = "*.csv"; index++;
  }
  if(fext.Contains("root")){
     filetypes[index] = "Root files"; index++;
     filetypes[index] = "*.root"; index++;
  }
  if(fext.Contains("log")){
     filetypes[index] = "Log files"; index++;
     filetypes[index] = "*.log"; index++;
  }
  filetypes[index] = "All files"; index++;
  filetypes[index] = "*"; index++;
  filetypes[index] = 0; index++;
  filetypes[index] = 0; index++;

  string tmp = gSystem->Getenv("USER");
  string misc =  "/home/%s/scratch" + tmp;
  static TString dir(misc);
  TGFileInfo fi;

  if(status == FS_OLD){
    fi.fFileTypes = (const char **)filetypes;
    fi.fIniDir    = StrDup(dir);
    new TGFileDialog(fClient->GetRoot(), this, kFDOpen, &fi);
    dir = fi.fIniDir;
    if(!fi.fFilename) {return PROCESS_FAILED;};
  }

  if(status == FS_NEW){
    fi.fFileTypes = (const char **)filetypes;
    fi.fIniDir    = StrDup(dir);
    new TGFileDialog(fClient->GetRoot(), this, kFDSave, &fi);
    dir = fi.fIniDir;

    if(!fi.fFilename) {return PROCESS_FAILED;};
  }
  strcpy(file,fi.fFilename);
  return PROCESS_OK;
}

void CMMonitor::PlotDataGraph()
{
  
  dCurrMSmplGrCv[0]->GetCanvas()->cd();
  ChSigGr[0]->Draw("ap");
  gPad->Modified();
  gPad->Update();
  dCurrMSmplGrCv[1]->GetCanvas()->cd();
  ChSigGr[1]->Draw("ap");
  gPad->Modified();
  gPad->Update();


}

void CMMonitor::PlotDataHRHst()
{
  
  dCurrMSmplHstHRCv[0]->GetCanvas()->cd();
  ChSigHstHR[0]->Draw();
  gPad->Modified();
  gPad->Update();
  dCurrMSmplHstHRCv[1]->GetCanvas()->cd();
  ChSigHstHR[1]->Draw();
  gPad->Modified();
  gPad->Update();

}

void CMMonitor::PlotDataFFT()
{

  Int_t RATE = CalculateFFT();

  if(!RATE) return;

  dCurrMSmplFFTCv[0]->GetCanvas()->cd();
  fftCh0->Draw();
  gPad->SetLogx();
  gPad->SetLogy();
  gPad->SetGridx();
  gPad->SetGridy();  
  fftCh0->GetXaxis()->SetRangeUser(10,RATE/2);
  fftCh0->GetYaxis()->SetRangeUser(1e-6,0.02);
  gPad->Modified();
  gPad->Update();

  
  dCurrMSmplFFTCv[1]->GetCanvas()->cd();
  fftCh1->Draw();
  gPad->SetLogx();
  gPad->SetLogy();
  gPad->SetGridx();
  gPad->SetGridy();  
  fftCh1->GetXaxis()->SetRangeUser(10,RATE/2);
  fftCh1->GetYaxis()->SetRangeUser(1e-6,0.02);
  gPad->Modified();
  gPad->Update();

}



Int_t CMMonitor::CalculateFFT()
{
                       
  // TH1D *Ch0Hst;
  // TH1D *Ch1Hst;

  int binning; 
  int RATE;
  double mean[2] = {0,0};
  double lmean[2];

  int smpl_range; 
  int N = 0;
 
  int Nruns = PlotData.size();
  
  if(!Nruns) return 0; //do nothing - no data

  fftTmpCh0 = new TH1D(Form("FFTTmpCh0_%d",iSettings.currentRun),"",100,0,100);
  fftTmpCh1 = new TH1D(Form("FFTTmpCh1_%d",iSettings.currentRun),"",100,0,100);
  
  fftCh0 = new TProfile(Form("FFTCh0_%d",iSettings.currentRun),"",100,0,100);
  fftCh1 = new TProfile(Form("FFTCh1_%d",iSettings.currentRun),"",100,0,100);

  for(int l = 0; l < Nruns; l++){

    DataSamples *data = PlotData[l];

    if(l == 0){
      binning = (int)(data->NSamples/1000);
      RATE = SAMPLES_PER_SECOND/data->PreScF;
      smpl_range = (int)data->NSamples;

      fftTmpCh0->SetBins(binning/2-1,0,(Int_t)(RATE/2.0));
      fftTmpCh1->SetBins(binning/2-1,0,(Int_t)(RATE/2.0));
      
      fftCh0->SetBins(binning/2-1,0,(Int_t)(RATE/2.0));
      fftCh1->SetBins(binning/2-1,0,(Int_t)(RATE/2.0));
  
    }
   
    lmean[0] = data->ch0_mean;
    lmean[1] = data->ch1_mean;

    mean[0] += lmean[0];
    mean[1] += lmean[1];
    
    if(data->NSamples < smpl_range){cout << "Range flag!" << endl; smpl_range = data->NSamples;}
    
    AddFFT(fftTmpCh0,data,lmean[0], RATE, 0, binning);
    AddFFT(fftTmpCh1,data,lmean[1], RATE, 1, binning);
    // AddFFT(fftTmpCh0,data,lmean[0], RATE, 0, smpl_range);
    // AddFFT(fftTmpCh1,data,lmean[1], RATE, 1, smpl_range);
    
    for(int i = 0; i < smpl_range/2-1; i++) {
      fftCh0->Fill(fftTmpCh0->GetBinCenter(i),fftTmpCh0->GetBinContent(i));
      fftCh1->Fill(fftTmpCh1->GetBinCenter(i),fftTmpCh1->GetBinContent(i));
      // Ch0Hst->Fill(data->ch0_data[i] - lmean[0]);
      // Ch1Hst->Fill(data->ch1_data[i] - lmean[1]);
      N++;
    }
    fftTmpCh0->Reset();
    fftTmpCh1->Reset();   
  }  

  // mean[0] = mean[0]/PlotData.size();
  // mean[1] = mean[1]/PlotData.size();

  // Double_t MaxAmp0 = 0;
  // Double_t Amp0;
  // Double_t MaxAmp1 = 0;
  // Double_t Amp1;
  
  // for(int i = 0; i < binning/2-1; i++) {
  //   Amp0 = fftCh0->GetBinContent(i);
  //   Amp1 = fftCh1->GetBinContent(i);
  //   if(MaxAmp0 < Amp0) MaxAmp0 = Amp0;
  //   if(MaxAmp1 < Amp1) MaxAmp1 = Amp1;
  // }

  // double ssqCh0 = 0;
  // double ssqCh1 = 0;
  // for(int i = 0; i < binning/2-1; i++) {
  //   fftGainCh0->Fill(fftCh0->GetBinCenter(i),fftCh0->GetBinContent(i)/MaxAmp0);
  //   fftGainCh1->Fill(fftCh1->GetBinCenter(i),fftCh1->GetBinContent(i)/MaxAmp1);
  //   fftTmpCh0->SetBinContent(i,fftCh0->GetBinContent(i));
  //   fftTmpCh1->SetBinContent(i,fftCh1->GetBinContent(i));
  //   ssqCh0 += pow(fftCh0->GetBinContent(i),2);
  //   ssqCh1 += pow(fftCh1->GetBinContent(i),2);
  // }

  return RATE;

}

bool CMMonitor::AddFFT(TH1D *fftTmp, DataSamples *data, double mean, double RATE, int ch, int smpls)
{
  vector<double> *tStmp;
  vector<double> *SmplCh;

  uint64_t N = data->NSamples;

  tStmp = &(data->tStmp);
  if(ch == 0)
    SmplCh = &(data->ch0_data);
  else if(ch == 1)
    SmplCh = &(data->ch1_data);
  else
    return false;

  TH1D *hst = new TH1D("hst","",smpls,0,smpls/RATE);
  for(int n = 0; n < N; n++){
    hst->SetBinContent(n+1,(*SmplCh)[n]-mean);
  }
    
  TH1 *fftmag = NULL;
  TVirtualFFT::SetTransform(0);
  fftmag = ((TH1D*)hst)->FFT(fftmag,"MAG");
  fftmag->SetName("FFT_mag");
  Int_t bin;

  for(int i = 0; i < smpls/2-1; i++) {

    bin = fftTmp->FindBin(fftmag->GetBinCenter(i)*RATE/(smpls));
    fftTmp->SetBinContent(bin,2.0*fftmag->GetBinContent(i)/smpls);
  }

  TVirtualFFT *transf = TVirtualFFT::GetCurrentTransform();
  delete transf;
  delete hst;
  delete fftmag;

  return true;
  
}

void CMMonitor::MakeMenuLayout()
{
  
  dMenuFile = new TGPopupMenu(fClient->GetRoot());  
  // dMenuFile->AddEntry("&New (New text output)...", M_FILE_NEW);
  // dMenuFile->AddEntry("N&ew (New root output)...", M_ROOT_FILE_NEW);  
  dMenuFile->AddEntry("&Open (Data File)...", M_FILE_OPEN);
  // dMenuFile->AddEntry("O&pen (Root out file)...", M_ROOT_FILE_OPEN);
  // dMenuFile->AddEntry("Open Data Window",M_OPEN_DATAWINDOW);
  dMenuFile->AddEntry("&Save (Settings)", M_FILE_SAVE_SETTINGS);
  // dMenuFile->AddEntry("C&lose (ROOT out file)", M_ROOT_FILE_CLOSE);  
  dMenuFile->AddSeparator();
  // dMenuFile->AddEntry("Root File Browser", M_VIEW_BROWSER);
  // dMenuFile->AddSeparator();
  // dMenuFile->AddSeparator();
  dMenuFile->AddEntry("E&xit", M_FILE_EXIT);

  dMenuSettings = new TGPopupMenu(fClient->GetRoot());
  dMenuSettings->AddEntry("Set IP...", M_SET_IP);
  dMenuSettings->AddEntry("Connect...", M_CONNECT);
  //dMenuSettings->AddEntry("Start...", M_RUN_START);
  dMenuSettings->AddEntry("Stop...", M_RUN_STOP);
  // dMenuSettings->AddEntry("Pause...", M_RUN_PAUSE);
  
  dMenuFile->Associate(this);
  dMenuSettings->Associate(this);
  
  dMenuBar = new TGMenuBar(this, 1, 1, kHorizontalFrame);
  dMenuBar->AddPopup("&File", dMenuFile, new TGLayoutHints(kLHintsTop | kLHintsLeft ,0, 4, 0, 0));
  dMenuBar->AddPopup("&Data",dMenuSettings, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0));
  
  AddFrame(dMenuBar, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,0, 0, 1, 1));
}

void CMMonitor::MakeCurrentModeTab()
{
  TGTab *Tab = new TGTab(this,dMWWidth-15,dMWHeight-80);  
  TGCompositeFrame *tf = Tab->AddTab("Current Mode");
  TGVerticalFrame *vf1 = new TGVerticalFrame(tf,10,10);
  
  TGHorizontalFrame *hf[NUM_CHANNELS];
  // TGLabel *dChanLabel[NUM_CHANNELS];
  // TGLabel *dRateLabel[NUM_CHANNELS];

  for(int n = 0; n < NUM_CHANNELS; n++){

    hf[n] = new TGHorizontalFrame(vf1,10,10);
    vLabF[n] = new TGVerticalFrame(hf[n],10,10);
    vLabF[n]->Resize(100,10); //this doesn't seem to be doing anything in the case :(
    dChanLabel[n] = new TGLabel(vLabF[n],Form("Channel %d          ",n)); //leave the empty space to stretch the label
    vLabF[n]->AddFrame(dChanLabel[n],new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
    // dRateFrame[n] = new TGHorizontalFrame(vLabF[n],10,10);
    // dRateLabel[n] = new TGLabel(dRateFrame[n],"Rate:  "); //leave the empty space to stretch the label
    // dRateCounter[n] = new TGLabel(dRateFrame[n],"1.0");
    // dRateFrame[n]->AddFrame(dRateLabel[n],new TGLayoutHints(kLHintsTop | kLHintsLeft, 10, 2, 2, 2));
    // dRateFrame[n]->AddFrame(dRateCounter[n],new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
    // vLabF[n]->AddFrame(dRateFrame[n],new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));   
    dCurrMSmplGrCv[n] = new TRootEmbeddedCanvas(Form("SmplGr_Ch%d",n), hf[n], 10, 10);
    dCurrMSmplHstCv[n] = new TRootEmbeddedCanvas(Form("SmplHst_Ch%d",n), hf[n], 10, 10);
    dCurrMSmplHstHRCv[n] = new TRootEmbeddedCanvas(Form("SmplHst_Ch%d",n), hf[n], 10, 10);
    
    dCurrMSmplFFTCv[n] = new TRootEmbeddedCanvas(Form("SmplFFT_Ch%d",n), hf[n], 10, 10);
    hf[n]->AddFrame(vLabF[n], new TGLayoutHints( kLHintsLeft | kLHintsTop | kLHintsExpandY, 2, 2, 5, 5));
    hf[n]->AddFrame(dCurrMSmplGrCv[n],new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 2, 2, 5, 5));
    hf[n]->AddFrame(dCurrMSmplHstCv[n],new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY,2, 2, 5, 5));
    hf[n]->AddFrame(dCurrMSmplHstHRCv[n],new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY,2, 2, 5, 5));
    hf[n]->AddFrame(dCurrMSmplFFTCv[n],new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY,2, 2, 5, 5));
    vf1->AddFrame(hf[n], new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX | kLHintsExpandY));
    if(n < NUM_CHANNELS-1){
      TGHorizontal3DLine *Line = new TGHorizontal3DLine(vf1);
      vf1->AddFrame(Line, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
    }

    ChSigHst[n] = new TH1D(Form("ChSigHst_%02d",n),"",2*4096,-4.096,4.096);
    ChSigHstHR[n] = new TH1D(Form("ChSigHstHR_%02d",n),"",(int)(2*4.096/ADC_CONVERSION),-4.096,4.096);
    dCurrMSmplHstCv[n]->GetCanvas()->cd();
    ChSigHst[n]->Draw();
    gPad->Modified();
    gPad->Update();
    ChSigGr[n] = new TGraph();
    ChSigGr[n]->SetName(Form("ChSigGr_%02d",n));
  }
  
  vf1->Resize(dMWWidth-15,dMWHeight-15);

  TGHorizontal3DLine *anotherLine = new TGHorizontal3DLine(tf);
  tf->AddFrame(anotherLine, new TGLayoutHints(kLHintsTop | kLHintsExpandX));

  
  tf->AddFrame(vf1,new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX | kLHintsExpandY));
  AddFrame(Tab, new TGLayoutHints(kLHintsBottom | kLHintsExpandX | kLHintsExpandY,2, 2, 5, 1));
  Tab->MapSubwindows();
  Tab->Layout();

  dChanLabel[0]->SetText(Form("Channel %d",iSettings.currentData0));
  dChanLabel[1]->SetText(Form("Channel %d",iSettings.currentData1));

}

void CMMonitor::MakeUtilityLayout()
{
  TGHorizontal3DLine *dHorizontal3DLine = new TGHorizontal3DLine(this);
  AddFrame(dHorizontal3DLine, new TGLayoutHints(kLHintsTop | kLHintsExpandX));

  dHorizontal3DLine = new TGHorizontal3DLine(this);
  AddFrame(dHorizontal3DLine, new TGLayoutHints(kLHintsTop | kLHintsExpandX));

  dUtilityFrame = new TGHorizontalFrame(this,60,10);


  dRunEntry = new TGNumberEntry(dUtilityFrame,iSettings.currentRun,6,M_RUN_SELECT,
				TGNumberFormat::kNESInteger,
				TGNumberFormat::kNEANonNegative,
				TGNumberFormat::kNELLimitMinMax,1,999999);
  if(dRunEntry){
    dRunEntryLabel = new TGLabel(dUtilityFrame,"Run:");
    if(dRunEntryLabel){
      dUtilityFrame->AddFrame(dRunEntryLabel, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 100, 2, 2, 2));
    }
    dRunEntry->Associate(this);
    dUtilityFrame->AddFrame(dRunEntry,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 10, 2, 2, 2));
  }

  dRunTimeEntry = new TGNumberEntry(dUtilityFrame,iSettings.RunLength,6,M_RUN_TIME_SELECT,
				    TGNumberFormat::kNESReal,
				    TGNumberFormat::kNEANonNegative,
				    TGNumberFormat::kNELLimitMinMax,1,5);
  if(dRunTimeEntry){
    dRunTimeEntryLabel = new TGLabel(dUtilityFrame,"Run Time (s):");
    if(dRunTimeEntryLabel){
      dUtilityFrame->AddFrame(dRunTimeEntryLabel, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 30, 2, 2, 2));
    }
    dRunTimeEntry->Associate(this);
    dUtilityFrame->AddFrame(dRunTimeEntry,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 10, 2, 2, 2));
  }

  dSmplDivEntry = new TGNumberEntry(dUtilityFrame,iSettings.PreScFactor,6,M_SDIV_SELECT,
				TGNumberFormat::kNESInteger,
				TGNumberFormat::kNEANonNegative,
				TGNumberFormat::kNELLimitMinMax,1,14);
  if(dSmplDivEntry){
    dSmplDivEntryLabel = new TGLabel(dUtilityFrame,"Prescale:");
    if(dSmplDivEntryLabel){
      dUtilityFrame->AddFrame(dSmplDivEntryLabel,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 30, 2, 2, 2));
    }
    dSmplDivEntry->Associate(this);
    dUtilityFrame->AddFrame(dSmplDivEntry,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 10, 2, 2, 2));
  }

  Ch0ListBox = new TGComboBox(dUtilityFrame,CMB_CH0SEL);
  if(Ch0ListBox){
    TGLabel *dCh0Label = new TGLabel(dUtilityFrame,"Ch0:");
    if(dCh0Label){
      dUtilityFrame->AddFrame(dCh0Label,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 30, 2, 2, 2));
    }
    Ch0ListBox->Resize(50,20);
    for(int n = 1; n < 17; n++){
      Ch0ListBox->AddEntry(Form("%d",n),3100+n);
    }
    Ch0ListBox->Select(iSettings.currentData0+3100,false);
    Ch0ListBox->Associate(this);
    dUtilityFrame->AddFrame(Ch0ListBox, new TGLayoutHints(kLHintsLeft | kLHintsCenterY ,10,2,2,2));
  }
  

  Ch1ListBox = new TGComboBox(dUtilityFrame,CMB_CH1SEL);
  if(Ch1ListBox){
    TGLabel *dCh1Label = new TGLabel(dUtilityFrame,"Ch1:");
    if(dCh1Label){
      dUtilityFrame->AddFrame(dCh1Label,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 30, 2, 2, 2));
    }
    Ch1ListBox->Resize(50,20);
    for(int n = 1; n < 17; n++){
      Ch1ListBox->AddEntry(Form("%d",n),5100+n);
    }
    Ch1ListBox->Select(iSettings.currentData1+5100,false);
    Ch1ListBox->Associate(this);
    dUtilityFrame->AddFrame(Ch1ListBox, new TGLayoutHints(kLHintsLeft | kLHintsCenterY ,10,2,2,2));
  }


  TGTextButton* dBtnRunStart = new TGTextButton(dUtilityFrame, "&Start", BTN_RUN_START);
  dBtnRunStart->Associate(this);
  dUtilityFrame->AddFrame(dBtnRunStart, new TGLayoutHints( kLHintsLeft | kLHintsCenterY, 20, 0, 5, 5 ));

  TGTextButton* dBtnTGraph = new TGTextButton(dUtilityFrame, "&Time Graph", BTN_TGRAPH);
  dBtnTGraph->Associate(this);
  dUtilityFrame->AddFrame(dBtnTGraph, new TGLayoutHints( kLHintsLeft | kLHintsCenterY, 20, 0, 5, 5 ));

  TGTextButton* dBtnHstHR = new TGTextButton(dUtilityFrame, "&Histo (HR)", BTN_HSTHR);
  dBtnHstHR->Associate(this);
  dUtilityFrame->AddFrame(dBtnHstHR, new TGLayoutHints( kLHintsLeft | kLHintsCenterY, 20, 0, 5, 5 ));

  TGTextButton* dBtnFFT = new TGTextButton(dUtilityFrame, "&FFT", BTN_FFT);
  dBtnFFT->Associate(this);
  dUtilityFrame->AddFrame(dBtnFFT, new TGLayoutHints( kLHintsLeft | kLHintsCenterY, 20, 0, 5, 5 ));

  TGTextButton* dBtnClear = new TGTextButton(dUtilityFrame, "&Clear Plots", BTN_CLEAR_PL);
  dBtnClear->Associate(this);
  dUtilityFrame->AddFrame(dBtnClear, new TGLayoutHints( kLHintsLeft | kLHintsCenterY, 20, 0, 5, 5 ));


  dNumRunSeqEntry = new TGNumberEntry(dUtilityFrame,dNRunsSeq,6,M_RUN_SEQ_SELECT,
				      TGNumberFormat::kNESReal,
				      TGNumberFormat::kNEANonNegative,
				      TGNumberFormat::kNELLimitMinMax,1,1000);
  if(dNumRunSeqEntry){
    TGLabel* dRunTimeEntryLabel = new TGLabel(dUtilityFrame,"Num of Runs:");
    if(dRunTimeEntryLabel){
      dUtilityFrame->AddFrame(dRunTimeEntryLabel, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 30, 2, 2, 2));
    }
    dNumRunSeqEntry->Associate(this);
    dUtilityFrame->AddFrame(dNumRunSeqEntry,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 10, 2, 2, 2));
  }


  
  AddFrame(dUtilityFrame, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 2,2,2,2));
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

void CMMonitor::WriteSettings()
{
  
  SettingsOutFile = NULL;
  SettingsOutFile = new ofstream("CMMonitorSettings.txt");
  if(SettingsOutFile){
    if(SettingsOutFile->is_open()){
      
      *SettingsOutFile << iSettings.IP << '\n';
      *SettingsOutFile << iSettings.currentRun  << '\n';
      *SettingsOutFile << iSettings.currentData0  << '\n';
      *SettingsOutFile << iSettings.currentData1  << '\n';
      *SettingsOutFile << iSettings.PreScFactor  << '\n';
      *SettingsOutFile << iSettings.RunLength  << '\n';
      
      SettingsOutFile->close();
    }    
  }
  
}


Bool_t CMMonitor::ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2)
{
  // Handle messages send to the MainFrame object. E.g. all menu button
  // messages.
  switch (GET_MSG(msg)){

  case kC_TEXTENTRY:
    switch (GET_SUBMSG(msg)) {

    case kTE_TEXTCHANGED:
      switch(parm1){

      case M_RUN_SELECT:
	//cout << dRunEntry->GetNumber() << endl;
	iSettings.currentRun = (Int_t)dRunEntry->GetNumber();
	SamplesOutFileName = Form("Int_Run_%03d.dat",iSettings.currentRun);
	
	//cout << SamplesOutFileName  << endl;
	break;

      case M_RUN_TIME_SELECT:
	//cout << dRunEntry->GetNumber() << endl;
	iSettings.RunLength = dRunTimeEntry->GetNumber();
	ReadNSamples = iSettings.RunLength*SAMPLES_PER_SECOND/iSettings.PreScFactor;
	cout << "Run Time: " << iSettings.RunLength  << endl;
	break;

      case M_RUN_SEQ_SELECT:
	dNRunsSeq = dNumRunSeqEntry->GetNumber();
	break;

      case M_SDIV_SELECT:
	iSettings.PreScFactor =  dSmplDivEntry->GetNumber();
	cout << "Pre Scale: " << iSettings.PreScFactor << endl;
	if(iSettings.PreScFactor > 0)
	  ReadNSamples = iSettings.RunLength*SAMPLES_PER_SECOND/iSettings.PreScFactor;
	break;

      }
      
    case kTE_ENTER:
      switch (parm1) {

      default:
	break;
      }

    default:
      break;
    }
    break;
    
  case kC_COMMAND:
    switch (GET_SUBMSG(msg)) {
      
    case kCM_COMBOBOX:
      {
	switch(parm1){

	case CMB_CH0SEL:
	  for(int n = 1; n < 17; n++){
	    
	    if(Ch0ListBox->GetSelected() == 3100+n){
	      dChanLabel[0]->SetText(Form("Channel %d",n));
	      iSettings.currentData0 = n;
	      break;
	    }	    
	  }
	break;

	case CMB_CH1SEL:
	  for(int n = 1; n < 17; n++){
	    
	    if(Ch1ListBox->GetSelected() == 5100+n){
	      dChanLabel[1]->SetText(Form("Channel %d",n));
	      iSettings.currentData1 = n;
	      break;
	    }	    
	  }
	  break;
	  
	}
	break;

      }
      break;
      
    case kCM_BUTTON:
      {
	switch(parm1){
	case BTN_TGRAPH:
	  cout << "Turn on graph" << endl;
	  PlotDataGraph();
	  break;
	
	case BTN_HSTHR:
	  cout << "Turn on high resol. Hist" << endl; 
	  PlotDataHRHst();
	  break;
	  
	case BTN_FFT:
	  cout << "Turn on FFT" << endl;
	  PlotDataFFT();
	  break;

	case M_RUN_SELECT:
	  cout << "Run increased" << endl;
	  break;

	case BTN_RUN_START:
	  RUN_START = true;
	  RUN_STOP = false;
	  RUN_ON = true;

	  break;

	case  BTN_CLEAR_PL:
	  ClearData();
	  ClearPlots();
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
	if(OpenDataFile(FS_OLD) == PROCESS_OK){
	  FillDataPlots();
	}
	else{

	  cout << "Cannot open data file: " << DataFileName.data() << endl;
	}
	break;

      case M_OPEN_DATAWINDOW:
	break;
	
      case M_FILE_EXIT:
	zmq_close(socket);

	CloseWindow();   // this also terminates theApp
	break;
	
      case M_ROOT_FILE_CLOSE:
	// CloseRootFile();
	break;

      case M_FILE_SAVE_SETTINGS:
	WriteSettings();
	break;
	
      case M_FILE_CLOSE:
	break;

      case M_SET_IP:
	// DoConfigSettings();
	SetIP();
	break;

      case M_CONNECT:
	ConnectBoard();
	StartDataCollection();
	break;

      case M_RUN_START:
	RUN_START = true;
	RUN_STOP = false;
	RUN_PAUSE = false;
	RUN_ON = true;

	//	StartDataCollection();
	//FillDataPlots();
	break;

      case M_RUN_STOP:
	RUN_STOP = true;
	RUN_START = false;
	RUN_ON = false;
	break;

      case M_RUN_PAUSE:
	if(RUN_PAUSE){
	  RUN_PAUSE = false;
	  RUN_ON = true;
	}
	else{
	  RUN_PAUSE = true;
	  RUN_ON = false;
	}
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


void CMMonitor::SetIP()
{
  TString lip;
  Bool_t retflag;

  RStringEntryDialog *IPEntry = new RStringEntryDialog(fClient->GetRoot(), this, "IPEntry", "CMMonitor",
						       &retflag,&lip,"IP Address",400,200);
  if(!retflag) {cout << "Connection Canceled" << endl; return;}
  iSettings.IP = lip.Data();
  cout << "Connecting to ADC board IP:  " << iSettings.IP.data() << endl;


  
  // time_t inittime = time(NULL);
  // time_t currtime;
  // time_t delta;
  // uint32_t counter = 0;

  // while(1){
  //   currtime = time(NULL);
  //   delta = currtime - inittime;
      
  //   if (delta >= 0.1){
      
  //     dRateCounter[0]->SetText(Form("%d",counter));
  //     dRateFrame[0]->Layout();
  //     //cout << counter << endl;
  //     //vLabF[0]->Layout();
  //     counter++;
  //     inittime = time(NULL);
  //     gSystem->ProcessEvents();
  //   }
  // }
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


