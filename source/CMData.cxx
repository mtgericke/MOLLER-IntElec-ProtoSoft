///////////////////////////////////////////////////////////////////////////////
//
// name: CMData.cxx
// date: 11-13-2023
// auth: Michael Gericke 
//
///////////////////////////////////////////////////////////////////////////////

//#include <stdlib.h>
#include <CMData.h>

CMData::CMData(int *argc, char **argv)
{
  string label;
  TString arg;
  TString argp;
  int nargs = *argc, n = 0;
  int rlen = 0;
  float tmpf;
  int tmpi;
  
  SettingsFile = NULL;
  SettingsFile = new ifstream("CMDataSettings.txt");
  if(SettingsFile){
    if(SettingsFile->is_open()){

      *SettingsFile >> label >> iSettings.IP;
      *SettingsFile >> label >> iSettings.currentRun;
      *SettingsFile >> label >> iSettings.currentData0;
      *SettingsFile >> label >> iSettings.currentData1;
      *SettingsFile >> label >> iSettings.PreScFactor;
      *SettingsFile >> label >> iSettings.RunLength;
      *SettingsFile >> label >> iSettings.SamplingDelay;

      SettingsFile->close();
    }
    else{

      iSettings.IP = "192.168.2.227";
      iSettings.currentRun = 1;
      iSettings.currentData0 = 1;
      iSettings.currentData1 = 2;
      iSettings.PreScFactor = 1;
      iSettings.SamplingDelay = 0;
      iSettings.RunLength = 1;

      delete SettingsFile;
    }
  }
  else{
    iSettings.IP = "192.168.2.227";
    iSettings.currentRun = 1;
    iSettings.currentData0 = 1;
    iSettings.currentData1 = 2;
    iSettings.PreScFactor = 1;
    iSettings.SamplingDelay = 0;
    iSettings.RunLength = 1;
  }

  dNRunsSeq = 1;
  dNRunSeqCnt = 0;

  DataTree = NULL;
  DataRootFile = NULL;
  
  RunStartTime = 0;
  RunStartIndex = 0;

  dRootFileWriteReduced = false;


  if(nargs >= 2){
    while(n < nargs){
      
      arg = argv[n];
      if(arg == Form("-l") && n < nargs-1){
	argp = argv[n+1];
	if(argp.IsFloat()){
	  tmpf =  atof(argp.Data());
	  if(tmpf > 0 && tmpf < 10)
	    iSettings.RunLength = tmpf;
	  n += 2;
	}
      }
      else if(arg == Form("-n") && n < nargs-1){
	argp = argv[n+1];
	if(argp.IsFloat()){
	  tmpi =  atoi(argp.Data());
	  if(tmpi > 0)
	    dNRunsSeq = tmpi;
	  n += 2;
	}
      }
      else if(arg == Form("-s") && n < nargs-1){
	argp = argv[n+1];
	if(argp.IsFloat()){
	  tmpi =  atoi(argp.Data());
	  if(tmpi > 0)
	    iSettings.PreScFactor = tmpi;
	    
	  n += 2;
	}
      }
      else if(arg == Form("-c1") && n < nargs-1){
	argp = argv[n+1];
	if(argp.IsFloat()){
	  tmpi =  atoi(argp.Data());
	  if(tmpi > 0 && tmpi < 17)
	    iSettings.currentData0 = tmpi;
	    
	  n += 2;
	}
      }
      else if(arg == Form("-c2") && n < nargs-1){
	argp = argv[n+1];
	if(argp.IsFloat()){
	  tmpi =  atoi(argp.Data());
	  if(tmpi > 0 && tmpi < 17)
	    iSettings.currentData1 = tmpi;
	    
	  n += 2;
	}
      }
      else if(arg == Form("-r") && n < nargs-1){
	argp = argv[n+1];
	if(argp.IsFloat()){
	  tmpi =  atoi(argp.Data());
	  if(tmpi > 0)
	    iSettings.currentRun = tmpi;
	    
	  n += 2;
	}
      }
      else if(arg == Form("-red") && n < nargs){
	n++;
	dRootFileWriteReduced = true;
      }
      else
	n++;
    }
  }

  
  //cout << iSettings.IP << endl;
  //cout << iSettings.currentRun << endl;
  //cout << iSettings.currentData0 << endl;
  //cout << iSettings.currentData1 << endl;
  //cout << iSettings.PreScFactor << endl;
  //cout << iSettings.RunLength << endl;
  //cout << iSettings.SamplingDelay << endl << endl;

  //cout << dNRunsSeq << endl << endl;
      
  cntr_socket = NULL;
  data_socket = NULL;
  context = NULL;


  std::set_new_handler(0);

  StartDataCollection();

}

void* CMData::GetSocket(SockType type)
{
  string tmp = iSettings.IP.data();

  if(type == DATA){
    server = "tcp://"+ tmp + ":5556";
    cout << "Connecting to server on port 5556:  " << server.data() << endl;

    context = zmq_ctx_new();
    zmq_ctx_set(context, ZMQ_IO_THREADS, 4);
    data_socket = zmq_socket(context,ZMQ_SUB);
    zmq_setsockopt(data_socket, ZMQ_SUBSCRIBE, "ADC",3);
      
    if(zmq_connect (data_socket, server.data()) != 0) {
      cout << "Failed to Bind ZMQ to port 5556 - quitting this process\n" << endl;;
      return 0;
    }
    else   
      return data_socket;
  }
  else if(type == CNTRL){
    errno = 0;
    server = "tcp://"+ tmp + ":5555";
    cout << "Connecting to server on port 5555:  " << server.data() << endl;
    context = zmq_ctx_new();
    zmq_ctx_set(context, ZMQ_IO_THREADS, 4);
    cntr_socket = zmq_socket(context,ZMQ_REQ);

    if(zmq_connect (cntr_socket, server.data()) != 0) {
      cout << "Failed to Bind ZMQ to port 5555 - quitting this process\n" << endl;
      
      switch (errno){
      case EAGAIN:
	cout << "ZMQ Error = EAGAIN" << endl;
	break;
      case ENOTSUP:
	cout << "ZMQ Error = ENOTSUP" << endl;
	break;
      case EFSM:
	cout << "ZMQ Error = EFSM" << endl;
	break;
      case ETERM:
	cout << "ZMQ Error = ETERM" << endl;
	break;
      case ENOTSOCK:
	cout << "ZMQ Error = ENOTSOCK" << endl;
	break;
      case EINTR:
	cout << "ZMQ Error = EINTR" << endl;
	break;
      }


      return 0;
    }
    else   {
      return cntr_socket;
    }
  }
  return 0;
}

Bool_t CMData::ADCMessage(ActType Act, void* socket, uint32_t addr, uint32_t data, uint32_t *msgret)
{
  if(!socket) return 0;

  int len;
  zmq_msg_t Resp;
  zmq_msg_t msg;
  uint8_t *respm;
  int fl;
  
  if(Act == WRITE){
    cntrMsg[0] = (uint32_t)('w');
    cntrMsg[1] = (uint32_t)(addr/4);
    cntrMsg[2] = data;
  }
  if(Act == READ){
    cntrMsg[0] = (uint32_t)('r');
    cntrMsg[1] = (uint32_t)(addr/4);
    cntrMsg[2] = 0;  
  }

  int al = zmq_msg_init_data(&msg,cntrMsg,12,0,0);//zmq_msg_init_size(&msg,12);
  if(!al){

    zmq_msg_send(&msg,socket,0);
  
    al = zmq_msg_init(&Resp);
    if(!al){
      errno = 0;
      fl = zmq_msg_recv (&Resp,socket,0);
      if(fl == -1){
	switch (errno){
	case EAGAIN:
	  cout << "ZMQ Error = EAGAIN" << endl;
	  break;
	case ENOTSUP:
	  cout << "ZMQ Error = ENOTSUP" << endl;
	  break;
	case EFSM:
	  cout << "ZMQ Error = EFSM" << endl;
	  break;
	case ETERM:
	  cout << "ZMQ Error = ETERM" << endl;
	  break;
	case ENOTSOCK:
	  cout << "ZMQ Error = ENOTSOCK" << endl;
	  break;
	case EINTR:
	  cout << "ZMQ Error = EINTR" << endl;
	  break;
	}
      }else{
   
	respm = (uint8_t*)zmq_msg_data(&Resp);
	len = zmq_msg_size(&Resp);

	if(len > 0 && respm[0] == 'r'){	
	  uint32_t tmp2;
	  memcpy(&tmp2,&(respm)[4],4);
	  *msgret = tmp2;
	  
	  if(Act == WRITE){
	    cout << "Message: 0x" << std::hex << data << " written." << endl;
	  }
	  if(Act == READ){
	    cout << "Message: 0x" << std::hex << tmp2 << " received." << endl;
	  }
	  	  
	  return 1;

	}
	else
	  return 0;
      }          
    }
  }
  else{
    cout << "WriteADCMessage: Can't allocate message size" << endl;
    return 0;
  }
  return 0;
}

void CMData::StartDataCollection()
{
  uint32_t prsc = iSettings.PreScFactor-1;
  uint32_t ch0 = iSettings.currentData0-1; 
  uint32_t ch1 = iSettings.currentData1-1; 
  uint32_t cntrmsg = 0x80000000 | (prsc << 24) | (ch0 << 16) | (ch1 << 20) | (int(ADC_PACKET_SIZE));
  uint32_t reg1 = 0x44; //ADC register to set the control parameters in message above
  uint32_t ratemsg = 0x80000000 | (iSettings.SamplingDelay << 16);
  uint32_t reg2 = 0x48; //ADC register to set the sample rate
  uint32_t retmsg;
  uint32_t convert_clocks;
  void* csocket;
  void* dsocket;
  rawPkt *pkt = NULL;

  int flag = 0;
  //int dNRunSeqCnt = 0;

  
  ReadNSamples = iSettings.RunLength *SAMPLES_PER_SECOND/iSettings.PreScFactor;
  //dNRunSeqCnt = 0;
  for(int dNRunSeqCnt = 0; dNRunSeqCnt < dNRunsSeq; dNRunSeqCnt++ ){
      
    // if(!IsRootFileOpen()){
      
    //   ROOTFileName = Form("Int_Run_%03d-%03d.root",iSettings.currentRun,iSettings.currentRun+dNRunsSeq-1);	
    //   OpenRootFile(ROOTFileName.Data());
    // }
    
    SamplesOutFileName = Form("Int_Run_%03d.dat",iSettings.currentRun);
    
    //GetSocket(...) sets the global pointers cntr_socket or data_socket
    //make sure they are close before opening them again for a new message/data transfer.
    if(data_socket) {
      zmq_close(data_socket);
      data_socket = NULL;
    }
    if(cntr_socket) {
      zmq_close(cntr_socket);
      cntr_socket = NULL;
    }
    
    //Set the board up with the correct channels, prescale, and packet size 
    csocket = GetSocket(CNTRL);
    ADCMessage(WRITE,csocket,reg1,cntrmsg,&retmsg);
    ADCMessage(WRITE,csocket,reg2,ratemsg,&retmsg);
    
    //Read back the convert time delay used in the conversion of the time stamp
    ADCMessage(READ,csocket,reg2,0,&retmsg);
    convert_clocks = (retmsg >> 16) & 0xFF;
    if(convert_clocks < MIN_CONVERT_CLOCKS)
      convert_clocks = MIN_CONVERT_CLOCKS;
    
    dsocket = GetSocket(DATA);      
    
    pkt = new rawPkt;
    if(!pkt){
      return;
    }
    pkt->convClk = convert_clocks;
    pkt->run = iSettings.currentRun;
    pkt->data = (uint8_t*)malloc(MAX_ALLOC);
    if(!pkt->data) {
      return;
    }

    readThreadArgs = new rArgs;
    readThreadArgs->FName = SamplesOutFileName.data();
    cout << "Writing to: " << SamplesOutFileName.data() << endl;
    readThreadArgs->NSamples = ReadNSamples;
    readThreadArgs->sock = dsocket;
    readThreadArgs->pkt = pkt;
          
    GetServerData((void*)readThreadArgs);
    dataQue.push(pkt); 	
      // pthread_create(&thread_cap_id, NULL, GetServerData, (void*)readThreadArgs);
      // dNRunSeqCnt++;
    iSettings.currentRun++; 
    if(dNRunSeqCnt == 0){
      
      fillThreadArgs = new fArgs;
      fillThreadArgs->mQue = &dataQue;
      fillThreadArgs->mExe = this;
      fillThreadArgs->wReduced = dRootFileWriteReduced;
      fillThreadArgs->dSamples = tmpDataSmpl;
      fillThreadArgs->tree = DataTree;
      fillThreadArgs->nRuns = dNRunsSeq;
      // fillThreadArgs->rStartInd = RunStartIndex;
      // fillThreadArgs->rStartTime = RunStartTime;
      
      // fillThreadArgs->FName = SamplesOutFileName.data();
      // cout << "Writing to: " << SamplesOutFileName.data() << endl;
      // readThreadArgs->NSamples = ReadNSamples;
      // readThreadArgs->sock = dsocket;
      // readThreadArgs->pkt = pkt;
      pthread_create(&thread_plot_id, NULL, FillRootTreeThread, (void*)fillThreadArgs);
   }
      
    // if(!dataQue.empty()){
    //   FillRootTreeThread();
      
    //   pkt = (rawPkt*)dataQue.front();
    //   free(pkt->data);
    //   dataQue.pop();
    //   // delete pkt;
    //   pkt = NULL;      
    // }


    
  }
    // if(!pthread_join(thread_cap_id, NULL)){
    //dataQue.push(pkt);
    //cout << "this queue size = " << dataQue.size() << endl;
    //RUN_START = false;
    //RUN_ON = false;      
    // }
    
    // gSystem->ProcessEvents();

  if(!pthread_join(thread_plot_id, NULL)){
  
    WriteSettings();
    cout << "Done!" << endl;
    return;
  }
  

  // int qsz = dataQue.size();
  // while(!dataQue.empty()){
  //   cout << "Filling tree with data packets: " << qsz - dataQue.size()+1 << endl;

  //   FillRootTree();
  //   pkt = (rawPkt*)dataQue.front();
  //   free(pkt->data);
  //   dataQue.pop();
  //   // delete pkt;
  //   pkt = NULL;
  //   gSystem->ProcessEvents();
     
  // }
  // cout << "Done filling tree." << endl;
         
  //CloseRootFile();


}

void *CMData::GetServerData(void *vargp)
{
  FILE *SamplesOutFile;
  uint8_t *data;
  size_t len;
  uint64_t samples_written;
  uint64_t data_written;
  uint64_t pkts_rx;
  zmq_msg_t header;
  zmq_msg_t samples;
  int ne;

  void *poller = zmq_poller_new();
  zmq_poller_event_t evnts;
  zmq_poller_add (poller, ((rArgs*)vargp)->sock, NULL, ZMQ_POLLIN);


  
  SamplesOutFile = fopen(((rArgs*)vargp)->FName.data(), "wb");
  if(!SamplesOutFile) {
    return NULL;
  }

  samples_written = 0;
  data_written = 0;
  pkts_rx = 0;

  cout << std::dec << "NSample = " << ((rArgs*)vargp)->NSamples << endl;
  
  while(samples_written < 2.0*((rArgs*)vargp)->NSamples) {
    //The factor of 2 needs to be there, because I want to specify the number of samples per channel.
    //However the way the ZMQ code is written below, it reads both channels at the same time. Which means
    //that NSamples above would be split between the two channels.
    
    ne = zmq_poller_wait_all (poller, &evnts, 1, 5000);
    if(ne == 1 && evnts.socket == ((rArgs*)vargp)->sock && (evnts.events & ZMQ_POLLIN)){

      zmq_msg_init (&header);
      zmq_msg_recv (&header, ((rArgs*)vargp)->sock, 0);
      
      data = (uint8_t*)zmq_msg_data(&header);
      len = zmq_msg_size(&header);
      
      if(strncmp("ADC", (char*)data, 3) == 0) {
	
	zmq_msg_init (&samples);
	zmq_msg_recv (&samples, ((rArgs*)vargp)->sock, 0);
	
	pkts_rx++;
	data = (uint8_t*)zmq_msg_data(&samples);
	len = zmq_msg_size(&samples);
	
	memcpy(&(((rArgs*)vargp)->pkt->data)[data_written], data, len);
	
	data_written += len;
	samples_written += (len - 16) / 4;
      
	zmq_msg_close(&samples);
      }
      zmq_msg_close (&header);
    }
  }
  zmq_poller_destroy (&poller);  
  
  fwrite(((rArgs*)vargp)->pkt->data, data_written, 1, SamplesOutFile);
  fclose(SamplesOutFile);
  ((rArgs*)vargp)->pkt->length = data_written;
  
  printf("Pkts: %ld Samples: %ld SamplesPerPacket: %ld Bytes: %ld\n", pkts_rx, samples_written, (data_written / (pkts_rx * 16)) * 2, data_written);


  return NULL;
  
}

void* CMData::FillRootTreeThread(void *vargp)
{
  sleep(1);
  
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

  uint32_t gate1;
  uint32_t gate2;

  double ch0_psum = 0;
  double ch0_nsum = 0;
  double ch0_pcnt = 0;
  double ch0_ncnt = 0;
  double ch1_psum = 0;
  double ch1_nsum = 0;
  double ch1_pcnt = 0;
  double ch1_ncnt = 0;

  int g1cr, flc1;
  int g2cr, flc2;
  
  double t1 = 0, t2 = 0;
  int p = 0, k = 0;

  vector<double_t> tStmpDiff;
  vector<uint64_t> tStmpDiffTime;
  vector<uint64_t> tStmpDiffRun;
  
  
  tDataSamples *thisData;
  tDataSamples *tmpData;
  TTree *dataTree = NULL;
  TFile* File = NULL;
  Bool_t fOpen = false;
  int nRuns = ((fArgs*)vargp)->nRuns;

  queue<rawPkt*> *lQue = ((fArgs*)vargp)->mQue;

  TString ROOTFileName;

  int RunStartTime = 0;
  int RunStartIndex = 0;
  int newRun = 0;
  int currentRun = 0;
  
  while(!lQue->empty()){
    rPkt = lQue->front();
    //nRuns--;
    currentRun = rPkt->run;
    ROOTFileName = Form("Int_Run_%03d.root",currentRun);
    cout << "Setting ROOT file name: " << ROOTFileName << endl;
    File = new TFile(ROOTFileName,"RECREATE");
    dataTree = new TTree("DataTree","Integrating ADC Streaming Data");
    tmpData = NULL;
    dataTree->Branch("SampleStream","tDataSamples",&tmpData,64000,99);     
    fOpen = true;
    newRun = 1;
    
    thisData = new tDataSamples;
    thisData->ch0_sum = 0;
    thisData->ch1_sum = 0;
    thisData->ch0_ssq = 0;
    thisData->ch1_ssq = 0;
    
    SampRead = 0;
    RunStartTime = cTime;
    
    cout << "Filling tree with data packets for run: " << rPkt->run << endl;
    bi = 0;
    while(bi < rPkt->length){
      
      memcpy(&num_words,&(rPkt->data)[bi+0],2);
      memcpy(&num_pkt,&(rPkt->data)[bi+2],4);
      memcpy(&padding,&(rPkt->data)[bi+6],1);
      memcpy(&id,&(rPkt->data)[bi+7],1);
      memcpy(&tStamp,&(rPkt->data)[bi+8],8);	
      nSamp = num_words - 1;
      SampRead += nSamp;
      
      for(int n = 0; n < nSamp; n++){
	
	memcpy(&ch0,&(rPkt->data)[bi+16+n*8],4);
	memcpy(&ch1,&(rPkt->data)[bi+16+n*8+4],4);
	
	ch0_data = ch0 >> 14;
	ch1_data = ch1 >> 14;
	PreSc = ((ch0 >> 4) & 0x7F)+1;
	ch0_num = ch0 & 0xF;
	ch1_num = ch1 & 0xF;
	
	gate1 = (ch0 >> 12) & 0x1;
	gate2 = (ch0 >> 13) & 0x1;
	
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
	  g1cr = gate1;
	  flc1 = 0;
	  g2cr = gate2;
	  flc2 = 0;
	}
	
	cTime = sTime-iTime + RunStartTime;
	
	if(!((fArgs*)vargp)->wReduced){
	  thisData->ch0_data.push_back(ch0_data*ADC_CONVERSION);
	  thisData->ch1_data.push_back(ch1_data*ADC_CONVERSION);
	  thisData->gate1.push_back(gate1);
	  thisData->gate2.push_back(gate2);
	  thisData->tStmp.push_back(cTime*1e-6);
	}
	if(!newRun){
	  if((sTime - sTimeP) > TS_TO_NS*TS_CONVERSION*PreSc){	    
	    thisData->tStmpDiff.push_back((sTime - sTimeP)*1e-6);
	    thisData->tStmpDiffTime.push_back(cTime*1e-6);
	    
	    tStmpDiff.push_back((sTime - sTimeP)*1e-6);
	    tStmpDiffTime.push_back(cTime*1e-6);
	    tStmpDiffRun.push_back(rPkt->run);
	  }
	}
	
	thisData->ch0_sum += ch0_data*ADC_CONVERSION;
	thisData->ch1_sum += ch1_data*ADC_CONVERSION;
	thisData->ch0_ssq += ch0_data*ADC_CONVERSION*ch0_data*ADC_CONVERSION;
	thisData->ch1_ssq += ch1_data*ADC_CONVERSION*ch1_data*ADC_CONVERSION;
	
	if(gate1 != g1cr){g1cr = gate1; flc1++;}
	
	if(flc1 == 2){
	  flc1 = 0;
	  
	  thisData->ch0_asym.push_back((ch0_psum/ch0_pcnt - ch0_nsum/ch0_ncnt)/(ch0_psum/ch0_pcnt + ch0_nsum/ch0_ncnt));
	  thisData->ch1_asym.push_back((ch1_psum/ch1_pcnt - ch1_nsum/ch1_ncnt)/(ch1_psum/ch1_pcnt + ch1_nsum/ch1_ncnt));
	  
	  thisData->ch0_asym_num.push_back((ch0_psum/ch0_pcnt - ch0_nsum/ch0_ncnt));
	  thisData->ch1_asym_num.push_back((ch1_psum/ch1_pcnt - ch1_nsum/ch1_ncnt));
	  thisData->ch0_asym_den.push_back((ch0_psum/ch0_pcnt + ch0_nsum/ch0_ncnt));
	  thisData->ch1_asym_den.push_back((ch1_psum/ch1_pcnt + ch1_nsum/ch1_ncnt));
	  
	  ch0_psum = 0;
	  ch1_psum = 0;
	  ch0_pcnt = 0;
	  ch1_pcnt = 0;
	  ch0_nsum = 0;
	  ch1_nsum = 0;
	  ch0_ncnt = 0;
	  ch1_ncnt = 0;	  
	}
	
	if(gate1) {
	  ch0_psum += ch0_data*ADC_CONVERSION;
	  ch1_psum += ch1_data*ADC_CONVERSION;
	  ch0_pcnt++;
	  ch1_pcnt++;
	}
	if(!gate1) {
	  ch0_nsum += ch0_data*ADC_CONVERSION;
	  ch1_nsum += ch1_data*ADC_CONVERSION;
	  ch0_ncnt++;
	  ch1_ncnt++;
	}
	
	sTimeP = sTime;
	cTimeP = cTime;
	tStampP = tStamp;
	p++;
	newRun = 0;
      }	
      k++;
      bi = bi + 16 + nSamp*8;
    }
    
    RunStartIndex += p;
    thisData->PreScF = PreSc;
    thisData->ch0_num = ch0_num;
    thisData->ch1_num = ch1_num;
    thisData->ch0_mean = thisData->ch0_sum/thisData->ch0_data.size(); 
    thisData->ch1_mean = thisData->ch1_sum/thisData->ch1_data.size(); 
    thisData->ch0_sig = sqrt(thisData->ch0_ssq/thisData->ch0_data.size()-thisData->ch0_mean*thisData->ch0_mean); 
    thisData->ch1_sig = sqrt(thisData->ch1_ssq/thisData->ch1_data.size()-thisData->ch1_mean*thisData->ch1_mean);
    // if(IsDataFileOpen()){
    // 	iSettings.currentData0 = ch0_num;
    // 	iSettings.currentData1 = ch1_num;
    // 	iSettings.PreScFactor = PreSc;
    // 	iSettings.RunLength = ceil(p*PreSc/SAMPLES_PER_SECOND);
    // 	iSettings.SamplingDelay = 0;
    // }
    thisData->RunLength = cTime;
    thisData->NSamples = SampRead;//ReadNSamples;
    tmpData = thisData;
    //cout << "ch0 buffer = " << tmpData->ch0_data.size() << endl;
    if(fOpen){
      if(dataTree){
	dataTree->Fill();	
	cout << "Done filling this packet: " <<  dataTree->GetTotBytes() << endl;
	dataTree->AutoSave("FlushBaskets");
      }
      if(File != NULL){
	File->Write("",TObject::kOverwrite);
	File->Close(kFalse);
	delete File;
	File = NULL;
      }
      fOpen = false;
    }
    if(thisData)
      delete thisData;
    free(rPkt->data);
    rPkt = NULL;
    lQue->pop();
    gSystem->ProcessEvents();
    //previousRun = currentRun;
  }

  ofstream *fMissedSmpls = new ofstream("MissedSamples.dat",std::ios_base::app);
  if(fMissedSmpls){
    if(fMissedSmpls->is_open()){

      for(int s = 0 ; s < tStmpDiff.size(); s++){
      
	*fMissedSmpls << tStmpDiff[s] << " " << tStmpDiffTime[s] << " " << tStmpDiffRun[s] << '\n';

      }
      fMissedSmpls->close();
    }
  }


  
  // gSystem->ProcessEvents();
  // if(rPkt){
  //   free(rPkt->data);
  //   rPkt = NULL;
  //   lQue->pop();
  //   cout << "Done filling this packet." << endl;
  // }

  return NULL;
}

void CMData::FillRootTree()
{
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

  uint32_t gate1;
  uint32_t gate2;

  double ch0_psum = 0;
  double ch0_nsum = 0;
  double ch0_pcnt = 0;
  double ch0_ncnt = 0;
  double ch1_psum = 0;
  double ch1_nsum = 0;
  double ch1_pcnt = 0;
  double ch1_ncnt = 0;

  int g1cr, flc1;
  int g2cr, flc2;
  
  double t1 = 0, t2 = 0;
  int p = 0, k = 0;

  tDataSamples *thisData;

  if(!dataQue.empty()){
 
    thisData = new tDataSamples;
    thisData->ch0_sum = 0;
    thisData->ch1_sum = 0;
    thisData->ch0_ssq = 0;
    thisData->ch1_ssq = 0;
    PlotData.push_back(thisData);
    
    rPkt = dataQue.front();
    if(rPkt){     
      cout << "Filling tree with data packets for run: " << rPkt->run << endl;
      while(bi < rPkt->length){
	
	memcpy(&num_words,&(rPkt->data)[bi+0],2);
	memcpy(&num_pkt,&(rPkt->data)[bi+2],4);
	memcpy(&padding,&(rPkt->data)[bi+6],1);
	memcpy(&id,&(rPkt->data)[bi+7],1);
	memcpy(&tStamp,&(rPkt->data)[bi+8],8);	
 	nSamp = num_words - 1;
	
	for(int n = 0; n < nSamp; n++){
	  
	  memcpy(&ch0,&(rPkt->data)[bi+16+n*8],4);
	  memcpy(&ch1,&(rPkt->data)[bi+16+n*8+4],4);

	  ch0_data = ch0 >> 14;
	  ch1_data = ch1 >> 14;
	  PreSc = ((ch0 >> 4) & 0x7F)+1;
	  ch0_num = ch0 & 0xF;
	  ch1_num = ch1 & 0xF;

	  gate1 = (ch0 >> 12) & 0x1;
	  gate2 = (ch0 >> 13) & 0x1;
	  
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
	    g1cr = gate1;
	    flc1 = 0;
	    g2cr = gate2;
	    flc2 = 0;
	  }

	  cTime = sTime-iTime + RunStartTime;

	  if(!dRootFileWriteReduced){
	    thisData->ch0_data.push_back(ch0_data*ADC_CONVERSION);
	    thisData->ch1_data.push_back(ch1_data*ADC_CONVERSION);
	    thisData->gate1.push_back(gate1);
	    thisData->gate2.push_back(gate2);
	    thisData->tStmp.push_back(cTime*1e-6);
	  }
	  if((sTime - sTimeP) > TS_TO_NS*TS_CONVERSION*PreSc){	    
	    thisData->tStmpDiff.push_back((sTime - sTimeP)*1e-6);
	    thisData->tStmpDiffTime.push_back(cTime*1e-6);
	  }

	  thisData->ch0_sum += ch0_data*ADC_CONVERSION;
	  thisData->ch1_sum += ch1_data*ADC_CONVERSION;
	  thisData->ch0_ssq += ch0_data*ADC_CONVERSION*ch0_data*ADC_CONVERSION;
	  thisData->ch1_ssq += ch1_data*ADC_CONVERSION*ch1_data*ADC_CONVERSION;

	  if(gate1 != g1cr){g1cr = gate1; flc1++;}

	  if(flc1 == 2){
	    flc1 = 0;
	    
	    thisData->ch0_asym.push_back((ch0_psum/ch0_pcnt - ch0_nsum/ch0_ncnt)/(ch0_psum/ch0_pcnt + ch0_nsum/ch0_ncnt));
	    thisData->ch1_asym.push_back((ch1_psum/ch1_pcnt - ch1_nsum/ch1_ncnt)/(ch1_psum/ch1_pcnt + ch1_nsum/ch1_ncnt));

	    thisData->ch0_asym_num.push_back((ch0_psum/ch0_pcnt - ch0_nsum/ch0_ncnt));
	    thisData->ch1_asym_num.push_back((ch1_psum/ch1_pcnt - ch1_nsum/ch1_ncnt));
	    thisData->ch0_asym_den.push_back((ch0_psum/ch0_pcnt + ch0_nsum/ch0_ncnt));
	    thisData->ch1_asym_den.push_back((ch1_psum/ch1_pcnt + ch1_nsum/ch1_ncnt));
	    
	    ch0_psum = 0;
	    ch1_psum = 0;
	    ch0_pcnt = 0;
	    ch1_pcnt = 0;
	    ch0_nsum = 0;
	    ch1_nsum = 0;
	    ch0_ncnt = 0;
	    ch1_ncnt = 0;
	  }
  
	  if(gate1) {
	    ch0_psum += ch0_data*ADC_CONVERSION;
	    ch1_psum += ch1_data*ADC_CONVERSION;
	    ch0_pcnt++;
	    ch1_pcnt++;
	  }
	  if(!gate1) {
	    ch0_nsum += ch0_data*ADC_CONVERSION;
	    ch1_nsum += ch1_data*ADC_CONVERSION;
	    ch0_ncnt++;
	    ch1_ncnt++;
	  }
	  
	  sTimeP = sTime;
	  cTimeP = cTime;
	  tStampP = tStamp;
	  p++;
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
	iSettings.RunLength = ceil(p*PreSc/SAMPLES_PER_SECOND);
	iSettings.SamplingDelay = 0;
      }
      thisData->RunLength = iSettings.RunLength;
      thisData->NSamples = ReadNSamples;
      
      if(IsRootFileOpen()){
	tmpDataSmpl = thisData;
	if(DataTree)
	  DataTree->Fill();	
      }
      cout << "Done filling packet(s)." << endl;

      gSystem->ProcessEvents();
    }
    DataTree->AutoSave("FlushBaskets");    
  }

}


Int_t CMData::OpenRootFile(const char* file)
{
  if(!file) return 0;

  Int_t flag = 0;

  if(IsRootFileOpen()) CloseRootFile();
  ROOTFileName = file;
  cout << "Setting ROOT file name: " << ROOTFileName << endl;
  
  DataRootFile = new TFile(ROOTFileName,"RECREATE");
  if(!DataRootFile){SetRootFileOpen(kFalse); return 0;}
  DataTree = new TTree("DataTree","Integrating ADC Streaming Data");
  tmpDataSmpl = NULL;
  DataTree->Branch("SampleStream","tDataSamples",&tmpDataSmpl,64000,99);     
  
  SetRootFileOpen(kTrue);

  return 1;
}

void CMData::CloseRootFile()
{

  if(DataRootFile != NULL){
    DataRootFile->Write("",TObject::kOverwrite);
    DataRootFile->Close(kFalse);
    delete DataRootFile;
    DataRootFile = NULL;
  }

  SetRootFileOpen(kFalse);

}

CMData::~CMData()
{   
  if(IsRootFileOpen()){
    cout << "Close ROOT File" << endl;
    CloseRootFile();
  }
}


void CMData::WriteSettings()
{
  
  SettingsOutFile = NULL;
  SettingsOutFile = new ofstream("CMDataSettings.txt");
  if(SettingsOutFile){
    if(SettingsOutFile->is_open()){
      
      *SettingsOutFile << "IP "  << iSettings.IP << '\n';
      *SettingsOutFile << "Run " << iSettings.currentRun  << '\n';
      *SettingsOutFile << "ReadChannel1 " << iSettings.currentData0  << '\n';
      *SettingsOutFile << "ReadChannel2 " << iSettings.currentData1  << '\n';
      *SettingsOutFile << "PrescaleFactor " << iSettings.PreScFactor  << '\n';
      *SettingsOutFile << "RunLength(s) " << iSettings.RunLength  << '\n';
      *SettingsOutFile << "SamplingDelay " << iSettings.SamplingDelay << '\n';
      SettingsOutFile->close();
    }    
  }
  
}

int main(int argc, char **argv)
{
  // TApplication theApp("CMData", &argc, argv);

  //if (gROOT->IsBatch()) {
  //   fprintf(stderr, "%s: cannot run in batch mode\n", argv[0]);
  //    return 1;
  //}

   CMData thisCMData(&argc, argv);

  // theApp.Run();

   exit(0);
   //return 0;
}


