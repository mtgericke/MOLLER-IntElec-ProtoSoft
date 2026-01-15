///////////////////////////////////////////////////////////////////////////////
//
// name: CMData.cxx
// date: 06/2025
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
      *SettingsFile >> label >> iSettings.ReversalFreq;
      *SettingsFile >> label >> iSettings.SamplingDelay;

      *SettingsFile >> label >> iSettings.IntMode;   //toggles integration and streaming mode : 1 = integration on
      *SettingsFile >> label >> iSettings.IntModeSingle;   //enables single integration region  (e.g. the enitre helicity window)
      *SettingsFile >> label >> iSettings.streamAll;   //enable streaming all 16 channels, but at much reduced sample factor
      *SettingsFile >> label >> iSettings.initDelay;  //sets the pockels cell settle time delay in  8 ns intervals
      *SettingsFile >> label >> iSettings.totalSamplingDelay;   //sum of the pockles cell and additional adc/sampling delay in 8 ns intervals
      *SettingsFile >> label >> iSettings.numBlocks;   //number of blocks in helcity window
      *SettingsFile >> label >> iSettings.blockSize;   // length of each block currently in terms of 8 ns clock cycles

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
      iSettings.ReversalFreq = 2000; //Hz

      iSettings.IntMode = 0;
      iSettings.IntModeSingle = 1;
      iSettings.streamAll = 0; 
      iSettings.initDelay = 150; //150 * 68 ns ~ 10 us 
      iSettings.totalSamplingDelay = 150;
      iSettings.numBlocks = 1; 
      iSettings.blockSize = 7200; //68 ns * 7200 cycles ~< 490 us       

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
    iSettings.ReversalFreq = 2000; //Hz
    
    iSettings.IntMode = 0;
    iSettings.IntModeSingle = 1;
    iSettings.streamAll = 0; 
    iSettings.initDelay = 150;  
    iSettings.totalSamplingDelay = 150;   
    iSettings.numBlocks = 1; 
    iSettings.blockSize = 7200;       
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
	  //if(tmpf > 0 && tmpf < 10)
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
	  if(tmpi > 0 && tmpi < 17){
	    iSettings.currentData1 = tmpi;
	    
	  }
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
      else if(arg == Form("-aC") && n < nargs-1){
	argp = argv[n+1];
	if(argp.IsFloat()){
	  tmpi =  atoi(argp.Data());
	  if(tmpi >= 0 && tmpi < 2)
	    iSettings.streamAll = tmpi;
	    
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
  context = zmq_ctx_new();

  std::set_new_handler(0);

  StartDataCollection();

}

void* CMData::GetSocket(SockType type)
{
  string tmp = iSettings.IP.data();

  if(type == STREAM){

    //make sure socket is closed before opening them again for a new message/data transfer.
    if(data_socket) {
      zmq_close(data_socket);
      data_socket = NULL;
    }

    server = "tcp://"+ tmp + ":5556";
    cout << "Connecting to server on port 5556:  " << server.data() << endl;

    //context = zmq_ctx_new();
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
  if(type == INTEG){

    //make sure socket is closed before opening them again for a new message/data transfer.
    if(data_socket) {
      zmq_close(data_socket);
      data_socket = NULL;
    }

    server = "tcp://"+ tmp + ":5556";
    cout << "Connecting to server on port 5556:  " << server.data() << endl;

    //context = zmq_ctx_new();
    zmq_ctx_set(context, ZMQ_IO_THREADS, 4);
    data_socket = zmq_socket(context,ZMQ_SUB);
    zmq_setsockopt(data_socket, ZMQ_SUBSCRIBE, "AVG",3);
      
    if(zmq_connect (data_socket, server.data()) != 0) {
      cout << "Failed to Bind ZMQ to port 5556 - quitting this process\n" << endl;;
      return 0;
    }
    else   
      return data_socket;
  }
  else if(type == CNTRL){
    
    //make sure socket is closed before opening them again for a new message/data transfer.
    if(cntr_socket) {
      zmq_close(cntr_socket);
      cntr_socket = NULL;
    }

    errno = 0;
    server = "tcp://"+ tmp + ":5555";
    cout << "Connecting to server on port 5555:  " << server.data() << endl;
    //context = zmq_ctx_new();
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

long CMData::GetPacketFrameSize()
{
  uint32_t prsc = iSettings.PreScFactor;
  long bSize = 0;

  if(!iSettings.IntMode){

    if(!iSettings.streamAll){

      return 8 + 8 + 8; // 8 byte header + 8 byte timestamp

    }

    return 0;
    
  }

  return 0;
}

uint32_t CMData::GetPrescaleFactor(uint32_t *fac)
{
  uint8_t n = 0;
  uint8_t k = 1;
  uint32_t lfac = *fac;
  double tmp = (double)(lfac);
  //int c = 0;

  if(lfac <= 16) {
    k = lfac - 1;
    n = 0;
  }
  else{

    while(tmp > 15){
      tmp = tmp/4;
      n++;
    }
    k = (uint8_t)tmp;
  }

  if(n > 7) n = 7;  //Largest possible prescale factor is kx4^n with n = 7 and k = 15;

  *fac = (uint32_t)(k*TMath::Power(4,n))+1; 
 
  return 0x00000000 | n << 4 | k; 
}

// uint32_t CMData::GetHex(uint32_t *fac)
// {
//   uint8_t n = 0;
//   uint8_t k = 1;
//   uint32_t lfac = *fac;
//   double tmp = (double)(lfac);
//   //int c = 0;

//   if(lfac <= 16) {
//     k = lfac - 1;
//     n = 0;
//   }
//   else{

//     while(tmp > 15){
//       tmp = tmp/4;
//       n++;
//     }
//     k = (uint8_t)tmp;
//   }

//   if(n > 7) n = 7;  //Largest possible prescale factor is kx4^n with n = 7 and k = 15;

//   *fac = (uint32_t)(k*TMath::Power(4,n))+1; 
 
//   return 0x00000000 | n << 4 | k; 
// }

void CMData::StartDataCollection()
{
  uint32_t prsc = GetPrescaleFactor(&(iSettings.PreScFactor));
  uint32_t ch0 = iSettings.currentData0-1; 
  uint32_t ch1 = iSettings.currentData1-1;
  uint32_t integrate = iSettings.IntMode; //bit that toggles integration and streaming mode : 1 = integration on
  uint32_t singleWin = iSettings.IntModeSingle; //enables single integration region  (e.g. the enitre helicity window)
  uint32_t streamAllCh = iSettings.streamAll; //enable streaming all 16 channels, but at much reduced sample factor
  uint32_t initDelay = iSettings.initDelay; //sets the initial time delay in terms of number of samples (should not be entirely zero) 
  uint32_t totalDelay = iSettings.totalSamplingDelay; //sum of the initial delay and an additional adc/sampling delay in terms of samples
  uint32_t numBlocks = iSettings.numBlocks;   //number of blocks in helcity window
  uint32_t blockSize = iSettings.blockSize;   // length of each block in terms of number of samples

  uint32_t fixedDelay = 0;

  int packet_size;
  // if(iSettings.streamAll && !iSettings.IntMode)
  //   packet_size = int(ADC_PACKET_SIZE_SALL);
  // else
  packet_size = int(ADC_PACKET_SIZE);

  //  uint32_t cntrmsg = 0x80000000 | (prsc << 24) | (ch1 << 16) | (ch0 << 20) | (int(ADC_PACKET_SIZE));
  uint32_t cntrmsg = 0x80000000 | ((prsc & 0xFF)  << 24) | ((ch1  & 0xF) << 16) | ((ch0  & 0xF) << 20) | (packet_size & 0xFFFF);
  uint32_t reg1 = 0x44; //FPGA register to set the control parameters
  
  uint32_t ratemsg = 0x80000000 | (iSettings.SamplingDelay << 16); 
  uint32_t reg2 = 0x48; //FPGA register to set the sample delay
  //So there is already a 16*8 ns delay here before the ADC is sampled? Why? Timestamp conversion ... see below ...

  //cout << "tdelay = " << totalDelay << " pSettle = " << initDelay << " streamAllCh = " << streamAllCh << " fDelay = " << fixedDelay << " singleWin = " << singleWin << " integrate = " << integrate << endl;   
  
  uint32_t avgmsg = 0x00000000 | (totalDelay << 20) | (initDelay << 8) | (streamAllCh << 3) | (fixedDelay << 2) | (singleWin << 1) | integrate;
  uint32_t reg3 = 0x104; //FPGA register  to set the control parameters

  uint32_t avgmsg2 = 0x00000000 | ((blockSize & 0xFFFF) << 15) | (numBlocks & 0xF);
  uint32_t reg4 = 0x108; //FPGA register  to set the control parameters
  
  uint32_t  ReadNSamples;
  uint32_t retmsg;
  uint32_t convert_clocks;
  void* csocket;
  void* dsocket;
  
  runData *rDat = NULL;
   
  //Set the board up with the correct channels, prescale, and packet size 
  csocket = GetSocket(CNTRL);
  ADCMessage(WRITE,csocket,reg1,cntrmsg,&retmsg);
  
  ADCMessage(WRITE,csocket,reg2,ratemsg,&retmsg);    
  //Read back the convert time delay used in the conversion of the time stamp
  ADCMessage(READ,csocket,reg2,0,&retmsg);
  convert_clocks = (retmsg >> 16) & 0xFF;
  if(convert_clocks < MIN_CONVERT_CLOCKS)
    convert_clocks = MIN_CONVERT_CLOCKS;
  
  ADCMessage(WRITE,csocket,reg3,avgmsg,&retmsg);
  ADCMessage(READ,csocket,reg3,0,&retmsg);
  ADCMessage(WRITE,csocket,reg4,avgmsg2,&retmsg);
  ADCMessage(READ,csocket,reg4,0,&retmsg);

  SockType sType; // = (integrate) ? INTEG : STREAM;
  
  readThreadArgs = new rArgs;
  fillThreadArgs = new fArgs;
  
  if(!integrate){
    ReadNSamples = (uint32_t)(iSettings.RunLength *SAMPLES_PER_SECOND/iSettings.PreScFactor);
    memcpy(readThreadArgs->dType,"ADC",3);
    memcpy(fillThreadArgs->dType,"ADC",3);
    readThreadArgs->sAll = (streamAllCh) ? true : false;
    // fillThreadArgs->sAll = (streamAllCh) ? true : false;
    sType = STREAM;
  }
  else{
    ReadNSamples = (uint32_t)(iSettings.RunLength * iSettings.ReversalFreq * numBlocks);
    memcpy(readThreadArgs->dType,"AVG",3);
    memcpy(fillThreadArgs->dType,"AVG",3);
    fillThreadArgs->settings = &iSettings;
    sType = INTEG;
    readThreadArgs->integ = 1;
  }
  
  //readThreadArgs->NSamples = ReadNSamples;
  readThreadArgs->socktype = sType;

  zmq_close(csocket);
  
  for(int dNRunSeqCnt = 0; dNRunSeqCnt < dNRunsSeq; dNRunSeqCnt++ ){
          
    SamplesOutFileName = Form("Int_Run_%03d.dat",iSettings.currentRun);
    dsocket = GetSocket(sType);

    rDat = new runData;
    if(!rDat){
      cout << "Cannot create Run data structure. Not enough memory? " << endl << "Aborting Program!" << endl << endl;
      return;
    }
    rDat->convClk = convert_clocks;
    rDat->run = iSettings.currentRun;
    rDat->NSamples = ReadNSamples;
    rDat->FName = SamplesOutFileName.data();
    rDat->Prescale = iSettings.PreScFactor;
    rDat->numBlocks = numBlocks;
   
    //readThreadArgs->FName = SamplesOutFileName.data();
    cout << "Writing to: " << SamplesOutFileName.data() << endl;
    readThreadArgs->rDat = rDat;
    readThreadArgs->sock = dsocket;
             
    if(!GetServerData((void*)readThreadArgs)) {
      cout << "Can't allocate memory to store zmq packet data strcture. Not enough memory? " << endl << "Aborting Program!" << endl << endl;
      return;
    }
    runQue.push(rDat);
    iSettings.currentRun++;
    
    if(dNRunSeqCnt == 0){
      
      fillThreadArgs->rQue = &runQue;
      fillThreadArgs->mExe = this;
      fillThreadArgs->wReduced = dRootFileWriteReduced;
      //fillThreadArgs->dSamples = tmpDataSmpl;
      //fillThreadArgs->tree = DataTree;
      //fillThreadArgs->nRuns = dNRunsSeq;
      if(!integrate){
	if(!streamAllCh)
	  pthread_create(&thread_plot_id, NULL, FillRootTreeThread2Chan, (void*)fillThreadArgs);
	else
	  pthread_create(&thread_plot_id, NULL, FillRootTreeThreadAllChan, (void*)fillThreadArgs);

      }
      else
	pthread_create(&thread_plot_id, NULL, FillRootTreeThreadIntegr, (void*)fillThreadArgs);
   }    
  }

  if(!pthread_join(thread_plot_id, NULL)){
  
    WriteSettings();
    cout << "Done!" << endl;
    return;
  }

  zmq_close(dsocket);

}

Bool_t  CMData::GetServerData(void *vargp)
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
  rawPkt *pkt = NULL;
  char buffer[65536];

  int numBlocks = ((rArgs*)vargp)->rDat->numBlocks;

  //void *poller = zmq_poller_new();
  //zmq_poller_event_t evnts;
  //zmq_poller_add (poller, ((rArgs*)vargp)->sock, NULL, ZMQ_POLLIN);


  
  SamplesOutFile = fopen(((rArgs*)vargp)->rDat->FName.data(), "wb");
  if(!SamplesOutFile) {
    cout << "Can't write raw data to file: " << ((rArgs*)vargp)->rDat->FName.data() << endl <<
	 "Continuing without raw data storage ..." << endl;
    SamplesOutFile = NULL;
  }

  samples_written = 0;
  data_written = 0;
  pkts_rx = 0;

  //cout << std::dec << "NSample = " << ((rArgs*)vargp)->NSamples << endl;

  // while(samples_written < ((rArgs*)vargp)->rDat->NSamples) {
    
  //   ne = zmq_poller_wait_all (poller, &evnts, 1, 5000);
  //   if(ne == 1 && evnts.socket == ((rArgs*)vargp)->sock && (evnts.events & ZMQ_POLLIN)){

  //     zmq_msg_init (&header);
  //     zmq_msg_recv (&header, ((rArgs*)vargp)->sock, 0);
      
  //     data = (uint8_t*)zmq_msg_data(&header);
  //     len = zmq_msg_size(&header);
      
  //      if(strncmp(((rArgs*)vargp)->dType, (char*)data, 3) == 0) {
	
  // 	zmq_msg_init (&samples);
  // 	zmq_msg_recv (&samples, ((rArgs*)vargp)->sock, 0);
	
  // 	pkts_rx++;
  // 	data = (uint8_t*)zmq_msg_data(&samples);
  // 	len = zmq_msg_size(&samples);

  // 	pkt = new rawPkt;
  // 	if(!pkt) return 0;
  // 	pkt->data = (uint8_t*)malloc(len);
  // 	if(!pkt->data) return 0;	
  // 	memcpy(pkt->data, data, len);	
  // 	pkt->length = len;
  // 	if(SamplesOutFile) fwrite(pkt->data, len, 1, SamplesOutFile);
  // 	((rArgs*)vargp)->rDat->dQue.push(pkt);
  // 	data_written += len;
  // 	if(((rArgs*)vargp)->sAll)
  // 	  samples_written += (len - 16) / (5*8);
  // 	else
  // 	  samples_written += (len - 16) / 8;

  // 	zmq_msg_close(&samples);
  //     }
  //     zmq_msg_close (&header);
  //   }
  // }
  // zmq_poller_destroy (&poller);  

  while(samples_written < ((rArgs*)vargp)->rDat->NSamples) {

    len = zmq_recv(((rArgs*)vargp)->sock,buffer,sizeof(buffer),0);
    if(len == 3 && memcmp( buffer, ((rArgs*)vargp)->dType, 3) == 0) {

      len =  zmq_recv(((rArgs*)vargp)->sock,buffer,sizeof(buffer),0);
      pkt = new rawPkt;
      if(!pkt) return 0;
      pkt->data = (uint8_t*)malloc(len);
      if(!pkt->data) return 0;
      memcpy(pkt->data, buffer, len);
      pkt->length = len;
      if(SamplesOutFile) fwrite(pkt->data, len, 1, SamplesOutFile);
      ((rArgs*)vargp)->rDat->dQue.push(pkt);
      data_written += len;
      pkts_rx++;
      if(!((rArgs*)vargp)->integ){
	if(((rArgs*)vargp)->sAll){
	  samples_written += (len - 16) / (5*8);
	}
	else{
	  samples_written += (len - 16) / 8;
	}
      }
      else{
	samples_written += (len - 8) / (115*8);
      }
    }
  }
  
  if(SamplesOutFile) fclose(SamplesOutFile);

  if(!((rArgs*)vargp)->integ && !((rArgs*)vargp)->sAll){
    printf("Pkts: %ld Samples: %ld SamplesPerPacket: %ld Bytes: %ld\n", pkts_rx, samples_written, (data_written / (pkts_rx * len))*2, data_written);
  }
  else
    printf("Pkts: %ld Samples: %ld SamplesPerPacket: %ld Bytes: %ld\n", pkts_rx, samples_written, (data_written / (pkts_rx * len)), data_written);


  return 1;
  
}

void* CMData::FillRootTreeThread2Chan(void *vargp)
{
  sleep(1);
  
  // pkt *data;
  rawPkt *rPkt;

  size_t bi = 0;
  uint16_t num_words;  //2 bytes
  uint32_t num_pkt;    //4 bytes
  uint8_t padding;     //1 byte
  uint8_t id;          //1 byte to unsigned int
  uint64_t tStamp;     //8 bytes

  long *buff;
  uint16_t SampRead = 0;
  int32_t ch0;           //4 bytes
  int32_t ch1;           //4 bytes
  int32_t ch0_data;      //4 bytes
  int32_t ch1_data;      //4 bytes
  uint32_t ch0_num;      //4 bytes
  uint32_t ch1_num;      //4 bytes
  uint32_t PreSc;

  uint32_t gate1, pgate1 = 0;
  uint32_t gate2, pgate2 = 0;

  double ch0_psum = 0;
  double ch0_nsum = 0;
  double ch0_pcnt = 0;
  double ch0_ncnt = 0;
  double ch1_psum = 0;
  double ch1_nsum = 0;
  double ch1_pcnt = 0;
  double ch1_ncnt = 0;

  double ch0_blsum;
  double ch1_blsum;


  int g1cr, flc1;
  int g2cr, flc2;

  int blnum = 4;
  int blsmpls = 1225;
  int smpldelay = 200;
  
  int p = 0, k = 0, blcnt = blnum, blsmpcnt = blsmpls, delaysmplcnt = 0;
  
  tDataSamples *thisData;
  tDataSamples *tmpData;
  TTree *dataTree = NULL;
  TFile* File = NULL;
  Bool_t fOpen = false;

  rawPkt *pkt;
  runData *rDat;
  queue<runData*> *rQue= ((fArgs*)vargp)->rQue;

  TString ROOTFileName;

  int RunStartTime = 0;
  int RunStartIndex = 0;
  int newRun = 0;
  int currentRun = 0;

  double tTime = 0;         //total time for all runs
  double rTime = 0;         //total time for this run
  double prTime = 0;        //previous run time
  uint64_t ptStamp;         //previous packet time stamp
  double dtStamp;           //delta t between consecutive time stamps
  double sTime;             //accummulated time from samples within a packet - the dtStamp should be equal to or greater than  this accummulated time - if it is greater there was deadtime from data transfer  

  int rcnt = 0, pcnt = 0, scnt= 0;
  while(!rQue->empty()){
    rDat = rQue->front();
    //nRuns--;
    currentRun = rDat->run;
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
    RunStartTime = rTime;
    
    cout << "Filling tree with data packets for run: " << currentRun << endl;
    rTime = 0;
    pcnt = 0;
    while(!rDat->dQue.empty()){

      pkt = rDat->dQue.front();

      if(pkt){
    
	buff = (long*)pkt->data;
	bi = 0;
	k = (pkt->length/8) - 2;    //packet length is given in bytes ; the packet header is two bytes. 
	  
	num_words = buff[0] & 0xFFFF;
	num_pkt = (buff[0] >> 16) & 0xFFFFFFFF;
	id = (buff[0] >> 56) & 0xFF;
	tStamp = buff[1];

	SampRead += k;

	if(!pcnt) {
	  ptStamp = tStamp;
	  prTime = rTime;
	}
	dtStamp = (double)(tStamp - ptStamp)*TS_TO_NS;
	rTime += (dtStamp  - sTime);

	if(dtStamp > sTime)
	  thisData->tStmpDiffLarger.push_back(dtStamp*1e-6);
	if(dtStamp < sTime)
	  thisData->tStmpDiffSmaller.push_back(dtStamp*1e-6);
	thisData->tStmpDiffTime.push_back(dtStamp*1e-6);
	thisData->PacketSmplTimeSum.push_back(sTime*1e-6);
	thisData->Packet.push_back(pcnt+1);
	thisData->PacketNSamp.push_back(k);
	  
	sTime = 0;
	for(int n = 0; n < k; n++){//nSamp; n++){
	    
	  ch0 = buff[2+n];
	  ch1 = (buff[2+n] >> 32);
	  // cout << "ch0 = " << std::hex << ch0 << std::dec << endl;
	  // cout << "ch1 = " << std::hex << ch1 << std::dec << endl;

	  
	  ch0_data = (ch0 >> 14);
	  //cout << "ch0_data = " << std::hex << ch0_data << std::dec << endl;
	  ch1_data = (ch1 >> 14);
	  //cout << "ch1_data = " << std::hex << ch1_data << std::dec << endl;
	  PreSc = ((ch0 >> 4) & 0x7F)+1;
	  ch0_num = ch0 & 0xF;
	  ch1_num = ch1 & 0xF;
	  
	  gate1 = (ch0 >> 12) & 0x1;
	  gate2 = (ch0 >> 13) & 0x1;
	  
	  // if(ch0_num == ch1_num){
	  //   sTime = (tStamp + ((n*2) * TS_CONVERSION * PreSc)) *  TS_TO_NS;
	  // }
	  // else{
	  //   sTime = (tStamp + (n * TS_CONVERSION * PreSc)) * TS_TO_NS;
	  // }
	  
	  rTime += TS_CONVERSION*PreSc*TS_TO_NS;
	  sTime += TS_CONVERSION*PreSc*TS_TO_NS;
	  // c++;
	  // cout << std::dec << c << "  " << "n = " << n << ", pcnt = " << pcnt << "  tStamp = " << tStamp << "  ptStamp = " << ptStamp << "  dtStamp = " << dtStamp*1e-6 << "  sTime = " << sTime*1e-6  << "  rTime = " << rTime*1e-6  << " drTime = " <<  (rTime-prTime)*1e-6 << endl;
	  
	  
	  if(!p){
	    // iTime = rTime;
	    // sTimeP = rTime;
	    // cTimeP = cTime;
	    // tStampP = tStamp;
	    g1cr = gate1;
	    flc1 = 0;
	    g2cr = gate2;
	    flc2 = 0;
	  }
	    	    
	  thisData->ch0_data.push_back(ch0_data*ADC_CONVERSION);
	  thisData->ch1_data.push_back(ch1_data*ADC_CONVERSION);
	  thisData->gate1.push_back(gate1);
	  thisData->gate2.push_back(gate2);
	  thisData->tStmp.push_back(rTime*1e-6);
	  
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

	  if(!pgate1 && gate1){
	    blcnt = 0;
	    blsmpcnt = 0;
	    ch0_blsum = 0;
	    ch1_blsum = 0;
	    delaysmplcnt = 0;
	  }
	  pgate1 = gate1;

	  if(blcnt < blnum && delaysmplcnt >= smpldelay){
	    
	    if(blsmpcnt < blsmpls){
	      ch0_blsum += ch0_data*ADC_CONVERSION;
	      ch1_blsum += ch1_data*ADC_CONVERSION;
	      blsmpcnt++;
	    }

	    if(blsmpcnt == blsmpls){
	      thisData->ch0_blockmean.push_back(ch0_blsum/blsmpls);
	      thisData->ch1_blockmean.push_back(ch1_blsum/blsmpls);
	      blsmpcnt = 0;
	      ch0_blsum = 0;
	      ch1_blsum = 0;
	      blcnt++;
	    }

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
	  
	  // sTimeP = rTime;
	  // cTimeP = cTime;
	  // tStampP = tStamp;
	  prTime = rTime;
	  p++;
	  delaysmplcnt++;
	  newRun = 0;
	}	
	
	free(pkt->data);
	pkt = NULL;
	rDat->dQue.pop();
	ptStamp = tStamp;
	pcnt++;
      }
    }
    
    RunStartIndex += p;
    thisData->PreScF = PreSc;
    thisData->ch0_num = ch0_num;
    thisData->ch1_num = ch1_num;
    thisData->ch0_mean = thisData->ch0_sum/thisData->ch0_data.size(); 
    thisData->ch1_mean = thisData->ch1_sum/thisData->ch1_data.size(); 
    thisData->ch0_sig = sqrt(thisData->ch0_ssq/thisData->ch0_data.size()-thisData->ch0_mean*thisData->ch0_mean); 
    thisData->ch1_sig = sqrt(thisData->ch1_ssq/thisData->ch1_data.size()-thisData->ch1_mean*thisData->ch1_mean);
    thisData->RunLength = rTime;
    thisData->NSamples = SampRead;//ReadNSamples;
    tmpData = thisData;

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

    delete rDat;
    rQue->pop();
    rcnt++;
    gSystem->ProcessEvents();
  }

  return NULL;
}



void* CMData::FillRootTreeThreadAllChan(void *vargp)
{
  sleep(1);
  
  //pkt *data;
  rawPkt *rPkt;

  size_t bi = 0;
  uint16_t num_words;  //2 bytes
  uint32_t num_pkt;    //4 bytes
  uint8_t padding;     //1 byte
  uint8_t id;          //1 byte to unsigned int
  uint64_t tStamp;     //8 bytes
  
  uint16_t SampRead = 0;

  uint64_t *buff;
  uint64_t *tmpb;
  int32_t ch_data[16];   //4*16 bytes

  uint32_t PreSc;

  uint32_t gate1;
  uint32_t gate2;
      
  int k = 0, p = 0;  
  
  tDataAllChanSamples *thisData;
  tDataAllChanSamples *tmpData;
  TTree *dataTree = NULL;
  TFile* File = NULL;
  Bool_t fOpen = false;

  rawPkt *pkt;
  runData *rDat;
  queue<runData*> *rQue= ((fArgs*)vargp)->rQue;

  TString ROOTFileName;

  int RunStartTime = 0;
  int RunStartIndex = 0;
  int newRun = 0;
  int currentRun = 0;
  
  double tTime = 0;         //total time for all runs
  double rTime = 0;         //total time for this run
  double prTime = 0;        //previous run time
  uint64_t ptStamp;         //previous packet time stamp
  double dtStamp;           //delta t between consecutive time stamps
  double sTime;             //accummulated time from samples within a packet - the dtStamp should be equal to or greater than  this accummulated time - if it is greater there was deadtime from data transfer  
  
  int rcnt = 0, pcnt = 0;
  while(!rQue->empty()){
    rDat = rQue->front();
    PreSc = rDat->Prescale;
    currentRun = rDat->run;
    ROOTFileName = Form("Int_Run_%03d.root",currentRun);
    cout << "Setting ROOT file name: " << ROOTFileName << endl;
    File = new TFile(ROOTFileName,"RECREATE");
    dataTree = new TTree("DataTree","Integrating ADC Streaming Data - All Channels");
    tmpData = NULL;
    dataTree->Branch("SampleStreamAll","tDataAllChanSamples",&tmpData,64000,99);     
    fOpen = true;
    newRun = 1;
    
    thisData = new tDataAllChanSamples;
    
    SampRead = 0;
    cout << "Filling tree with data packets for run: " << std::dec << currentRun << endl;
    
    RunStartTime = rTime;
    rTime = 0;
    
    pcnt = 0;
    while(!rDat->dQue.empty()){

      pkt = rDat->dQue.front();      
      if(pkt){

	buff = (uint64_t*)pkt->data;

	k = (pkt->length/8) - 2;    //Packet length is given in bytes ; the packet header is 2 bytes long

	num_words = buff[0] & 0xFFFF;
	num_pkt = (buff[0] >> 16) & 0xFFFFFFFF;
	id = (buff[0] >> 56) & 0xFF;
	tStamp = buff[1];
	SampRead += (int)(k/5);
	if(!pcnt) {
	  ptStamp = tStamp;
	  prTime = rTime;
	}
	dtStamp = (double)((tStamp - ptStamp)*TS_TO_NS);
	rTime += (dtStamp - sTime);

	if(dtStamp > sTime)
	  thisData->tStmpDiffLarger.push_back(dtStamp*1e-6);
	if(dtStamp < sTime)
	  thisData->tStmpDiffSmaller.push_back(dtStamp*1e-6);
	thisData->tStmpDiffTime.push_back(dtStamp*1e-6);
	thisData->PacketSmplTimeSum.push_back(sTime*1e-6);
	thisData->Packet.push_back(pcnt+1);
	thisData->PacketNSamp.push_back((int)(k/5));
	
	sTime = 0;  //accumulate this only over the samples in a given packet
	for(int n = 0; n < k; n += 5){

	  //Note: The kludgy code below is used to implement a compiler inependent way to bit shift signed values.
	  //OR-ing the data with 0xFFFC0000 below ensures that negative values are properly bit shifted to
	  //higher precision variables (e.g. the 18 bit ADC samples into 32 bit integers). The first condition
	  //(*tmpb & 0x20000) tests if the MSB (sign bit) of the 18 bit sample is 1 or 0;
	  
	  tmpb = &(buff[n+2]);
	  ch_data[0] = !(*tmpb & 0x20000) ? (*tmpb & 0x3FFFF) : (0xFFFC0000 | (*tmpb & 0x3FFFF));
	  ch_data[1] = !((*tmpb >>18) & 0x20000) ? ((*tmpb >> 18) & 0x3FFFF) : (0xFFFC0000 | ((*tmpb >> 18) & 0x3FFFF));
	  ch_data[2] = !((*tmpb >>36) & 0x20000) ? ((*tmpb >> 36) & 0x3FFFF) : (0xFFFC0000 | ((*tmpb >> 36) & 0x3FFFF));

	  tmpb = &(buff[n+2+1]);
	  ch_data[3] = !(*tmpb & 0x20000) ? (*tmpb & 0x3FFFF) : (0xFFFC0000 | (*tmpb & 0x3FFFF));
	  ch_data[4] = !((*tmpb >>18) & 0x20000) ? ((*tmpb >> 18) & 0x3FFFF) : (0xFFFC0000 | ((*tmpb >> 18) & 0x3FFFF));
	  ch_data[5] = !((*tmpb >>36) & 0x20000) ? ((*tmpb >> 36) & 0x3FFFF) : (0xFFFC0000 | ((*tmpb >> 36) & 0x3FFFF));

	  tmpb = &(buff[n+2+2]);
	  ch_data[6] = !(*tmpb & 0x20000) ? (*tmpb & 0x3FFFF) : (0xFFFC0000 | (*tmpb & 0x3FFFF));
	  ch_data[7] = !((*tmpb >>18) & 0x20000) ? ((*tmpb >> 18) & 0x3FFFF) : (0xFFFC0000 | ((*tmpb >> 18) & 0x3FFFF));
	  ch_data[8] = !((*tmpb >>36) & 0x20000) ? ((*tmpb >> 36) & 0x3FFFF) : (0xFFFC0000 | ((*tmpb >> 36) & 0x3FFFF));

	  tmpb = &(buff[n+2+3]);
	  ch_data[9] = !(*tmpb & 0x20000) ? (*tmpb & 0x3FFFF) : (0xFFFC0000 | (*tmpb & 0x3FFFF));
	  ch_data[10] = !((*tmpb >>18) & 0x20000) ? ((*tmpb >> 18) & 0x3FFFF) : (0xFFFC0000 | ((*tmpb >> 18) & 0x3FFFF));
	  ch_data[11] = !((*tmpb >>36) & 0x20000) ? ((*tmpb >> 36) & 0x3FFFF) : (0xFFFC0000 | ((*tmpb >> 36) & 0x3FFFF));

	  tmpb = &(buff[n+2+4]);
	  ch_data[12] = !(*tmpb & 0x20000) ? (*tmpb & 0x3FFFF) : (0xFFFC0000 | (*tmpb & 0x3FFFF));
	  ch_data[13] = !((*tmpb >>18) & 0x20000) ? ((*tmpb >> 18) & 0x3FFFF) : (0xFFFC0000 | ((*tmpb >> 18) & 0x3FFFF));
	  ch_data[14] = !((*tmpb >>36) & 0x20000) ? ((*tmpb >> 36) & 0x3FFFF) : (0xFFFC0000 | ((*tmpb >> 36) & 0x3FFFF));

	  ch_data[15] = ((buff[n+2+2] >> 54) & 0x3) << 16 | ((buff[n+2+1] >> 54) & 0xFF) << 8 | ((buff[n+2] >> 54) & 0xFF);
	  ch_data[15] = ((buff[n+2+2] >> 54) & 0x2) ? 0xFFFC0000 | ch_data[15] : ch_data[15];
	  
	  gate1 = (buff[n+2+3] >> 55) & 0x1;
	  gate2 = (buff[n+2+3] >> 56) & 0x1;
	  
	  rTime += TS_CONVERSION*PreSc*TS_TO_NS;
	  sTime += TS_CONVERSION*PreSc*TS_TO_NS;
	  
	    // cout << std::dec << "n = " << n << ", pcnt = " << pcnt << "  tStamp = " << tStamp << "  ptStamp = " << ptStamp << "  dtStamp = " << dtStamp*1e-6 << "  sTime = " << sTime*1e-6  << "  rTime = " << rTime*1e-6  << " drTime = " <<  (rTime-prTime)*1e-6 << endl;
	    
	  for(int c = 0; c < 16; c++){
	    thisData->ch_data[c].push_back(ch_data[c]*ADC_CONVERSION);
	  }
	  thisData->gate1.push_back(gate1);
	  thisData->gate2.push_back(gate2);
	  thisData->tStmp.push_back(rTime*1e-6);
	  
	  p++;
	  newRun = 0;
	}	
	
	free(pkt->data);
	pkt = NULL;
	rDat->dQue.pop();
	ptStamp = tStamp;
	tTime += rTime;	
	pcnt++;
      }
    }
    
    RunStartIndex += p;
    thisData->PreScF = PreSc;
    thisData->RunLength = rTime;
    thisData->NSamples = SampRead;//ReadNSamples;
    tmpData = thisData;

    if(fOpen){
      if(dataTree){
	dataTree->Fill();	
	cout << "Done filling this packet: " <<  std::dec << dataTree->GetTotBytes() << endl;
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

    delete rDat;
    rQue->pop();
    
    gSystem->ProcessEvents();
    rcnt++;
  }
  
  return NULL;
}
  
  


void* CMData::FillRootTreeThreadIntegr(void *vargp)
{
  sleep(1);
  
  //pkt *data;
  rawPkt *rPkt;

  size_t bi = 0;
  uint16_t num_words;  //2 bytes
  uint32_t num_pkt;    //4 bytes
  uint8_t padding;     //1 byte
  uint8_t id;          //1 byte to unsigned int
  uint64_t tStamp;     //8 bytes
  
  uint16_t SampRead = 0;

  uint64_t *buff;
  int64_t ch_sum[16];   //4*16 bytes
  int64_t ch_ssq[16];   //4*16 bytes
  int64_t ch_snum[16];   //4*16 bytes
  int64_t ch_misc[16];   //4*16 bytes
  int32_t ch_max[16];   //4*16 bytes
  int32_t ch_min[16];   //4*16 bytes
  double  ch_std[16];

  int64_t ch_tsum[16];   //4*16 bytes
  int64_t ch_tssq[16];   //4*16 bytes
  int64_t ch_tsnum[16];   //4*16 bytes

  uint32_t PreSc;

  uint32_t gate1;
  uint32_t gate2;

      
  int k = 0;// p = 0;  
  
  tDataAverageSamples *thisData;
  tDataAverageSamples *tmpData;
  TTree *dataTree = NULL;
  TFile* File = NULL;
  Bool_t fOpen = false;

  rawPkt *pkt;
  runData *rDat;
  queue<runData*> *rQue= ((fArgs*)vargp)->rQue;
  uint32_t SingleWin = ((fArgs*)vargp)->settings->IntModeSingle;
  uint32_t numBlocks = ((fArgs*)vargp)->settings->numBlocks;
  uint32_t blockSize = ((fArgs*)vargp)->settings->blockSize;
  if(SingleWin) numBlocks = 1;

  int block;
  uint64_t pcktCnt, prevPckt;
  uint64_t tSamples;

  TString ROOTFileName;

  int RunStartTime = 0;
  //int RunStartIndex = 0;
  int newRun = 0;
  int currentRun = 0;
  
  double tTime = 0;         //total time for all runs
  double rTime = 0;         //total time for this run
  double prTime = 0;        //previous run time
  uint64_t ptStamp;         //previous packet time stamp
  double dtStamp;           //delta t between consecutive time stamps
  double sTime;             //accummulated time from samples within a packet - the dtStamp should be equal to or greater than  this accummulated time - if it is greater there was deadtime from data transfer

  double RunSeqStartTime;
  double RunningSampleTime;
  
  int rcnt = 0, pcnt = 0, bcnt = 0;
  while(!rQue->empty()){
    rDat = rQue->front();
    PreSc = rDat->Prescale;
    currentRun = rDat->run;
    ROOTFileName = Form("Int_Run_%03d.root",currentRun);
    cout << "Setting ROOT file name: " << ROOTFileName << endl;
    File = new TFile(ROOTFileName,"RECREATE");
    dataTree = new TTree("DataTree","Integrating ADC Integration Data - All Channels");
    tmpData = NULL;
    dataTree->Branch("SampleStreamAvg","tDataAverageSamples",&tmpData,64000,99);     
    fOpen = true;
    newRun = 1;
    
    thisData = new tDataAverageSamples;
    // IntegrationData *intData;
    // for(int b = 0; b < numBlocks; b++){
    //   intData = new IntegrationData;
    //   thisData->dataBlocks.push_back(intData);
    //   cout << "block " << b << endl;
    // }
    
    SampRead = 0;
    cout << "Filling tree with data packets for run: " << std::dec << currentRun << endl;
    
    RunStartTime = rTime;
    rTime = 0;
    
    pcnt = 0;
    while(!rDat->dQue.empty()){   //Loop over the number of data packets in this run

      pkt = rDat->dQue.front();      
      if(pkt){

	buff = (uint64_t*)pkt->data;

	//Packet length is given in bytes ; the packet header is 1 byte long. In this case each packet only contains the data for one integration block.
	//Here "sample" means one helicity block, including all associated data over all channels.
	
	k = (pkt->length/8) - 1;    

	num_words = buff[0] & 0xFFFF;  
	//Number of data words (in this case that would be 67 64 bit words)
	//word 0: packet header
	//word 1: 64 bit time stamp
	//word 2: 60 bit packet counter + 4 bit block number (within helicity window)
	//word 3: 64 bit total number of samples captured in the helicity window (for all channels)
	//words  4-19: 20 bit max + 20 bit min + 2 bit trigger (two TTL lemo inputs) + 22 bit empty for each of 16 channels (trigger info copied) 
	//words 20-35: 64 bit channel sample count for the block for each of 16 channels
	//words 36-51: 64 bit channel sum for the block for each of 16 channels
	//words 52-67: 64 bit channel sum-of-squares the block for each of 16 channels	
	num_pkt = (buff[0] >> 16) & 0xFFFFFFFF;   //Not really used
	id = (buff[0] >> 56) & 0xFF;  //Only available if the board ID has been set
	
	SampRead += k/(num_words);  //Would be just +1 in this case. k = 67 here since only one sample is sent per packet

	bcnt = 0;
	sTime = 0;  //accumulate this only over the samples in a given packet
	//make this a "silly" loop, just in case we ever change the number samples sent per packet - at the moment this will only "loop" once.
	for(int n = 0; n < k; n += num_words){

	  tStamp = buff[n+1];
	  block  = (buff[n+2] >> 60) & 0xF;
	  pcktCnt = buff[n+2] & 0x0FFFFFFFFFFFFFFF;	  	  
	  tSamples = buff[n+3];

	  if(!rcnt && !pcnt){
	    RunSeqStartTime = tStamp;
	    ptStamp = tStamp;
	    prevPckt = pcktCnt;
		
	  }
	  RunningSampleTime = (tStamp - RunSeqStartTime)*TS_TO_NS;
	  

	  // if(!pcnt) {
	  //   ptStamp = tStamp;
	  //   //prTime = rTime;
	  // }
	  // dtStamp = (double)((tStamp - ptStamp)*TS_TO_NS);
	  // rTime += dtStamp ;//(dtStamp - sTime);
	  
	  thisData->tStmp.push_back(RunningSampleTime*1e-6);
	  thisData->block.push_back(block);
	  thisData->NSamples.push_back(tSamples);
	  thisData->pckCntDiff.push_back(pcktCnt -  prevPckt);
	  thisData->pckCnt.push_back(pcktCnt);

	  cout << "pckt = " << pcktCnt << endl;
	  cout << "block = " << block << endl;
	  cout << "tstmp = " << tStamp << endl;
	  cout << "tstmpdiff = " << tStamp -  ptStamp << endl;
	  cout << "pcktdiff = " << pcktCnt -  prevPckt << endl;
	  prevPckt = pcktCnt;

	  for(int c = 0; c < 16; c++){
	    ch_misc[c] = buff[n+c+4];
	    //std::bitset<64> tmp(ch_misc[c]);
	    //cout << "ch_misc = " << tmp << " or in hex: " << std::hex << ch_misc[c] << std::dec << endl;
	    ch_max[c] =  !(ch_misc[c] & 0x80000) ? (ch_misc[c] & 0xFFFFF) : (0xFFF00000 | (ch_misc[c] & 0xFFFFF));
	    ch_min[c] =  !((ch_misc[c] >> 20) & 0x80000) ? ((ch_misc[c] >> 20)& 0xFFFFF) : (0xFFF00000 | ((ch_misc[c] >> 20) & 0xFFFFF));
	    thisData->ch_max[c].push_back(ch_max[c]*ADC_CONVERSION);
	    thisData->ch_min[c].push_back(ch_min[c]*ADC_CONVERSION);

	    ch_tsnum[c] = buff[n+c+20];
	    thisData->ch_WindowNSamples[c].push_back(ch_tsnum[c]);

	    ch_tsum[c] = buff[n+c+36];
	    thisData->ch_WindowSum[c].push_back(ch_tsum[c]);
	    
	    ch_tssq[c] = buff[n+c+52];
	    thisData->ch_WindowSumSq[c].push_back(ch_tssq[c]);

	    ch_snum[c] = buff[n+c+68];
	    thisData->ch_NSamples[c].push_back(ch_snum[c]);
	    
	    ch_sum[c] = buff[n+c+84];
	    //cout << "ch_sum in hex = " << std::hex << buff[n+c+36] << std::dec << endl;
	    thisData->ch_Sum[c].push_back(ch_sum[c]*ADC_CONVERSION);
	    
	    ch_ssq[c] = buff[n+c+100];
	    thisData->ch_SumSq[c].push_back(ch_ssq[c]*ADC_CONVERSION*ADC_CONVERSION);

	    ch_std[c] = sqrt(ch_ssq[c]*ADC_CONVERSION*ADC_CONVERSION/ch_snum[c] - ch_sum[c]*ADC_CONVERSION*ch_sum[c]*ADC_CONVERSION/ch_snum[c]/ch_snum[c]);
	    thisData->ch_Sig[c].push_back(ch_std[c]);
	    
	    thisData->ch_Mean[c].push_back(ch_sum[c]*ADC_CONVERSION/ch_snum[c]);
	    
	    //cout << "Block " << block << " time " << tStamp << " runTime " << RunningSampleTime*1e-6 << " Samples " << ch_snum[c] << " channel " << c << " mean = " << ch_sum[c]*ADC_CONVERSION/ch_snum[c]  << " std = " << ch_std[c] << " min = " << ch_min[c]*ADC_CONVERSION  << " max = " << ch_max[c]*ADC_CONVERSION << endl;
	  }
	  	  
	  gate1 = (ch_misc[0] >> 40) & 0x1;
	  gate2 = (ch_misc[0] >> 41) & 0x1;
	  thisData->gate1.push_back(gate1);
	  thisData->gate2.push_back(gate2);
	  
	  bcnt++;
	  if(bcnt == numBlocks) bcnt = 0;

	  //cout << std::dec << " pcnt = " << pcnt << "  tStamp = " << tStamp << "  ptStamp = " << ptStamp << "  dtStamp = " << dtStamp*1e-6 << "  sTime = " << sTime*1e-6  << "  rTime = " << rTime*1e-6  << " drTime = " <<  (rTime-prTime)*1e-6 << endl;
	  
	  
	  //p++;
	  tTime += rTime;	
	  ptStamp = tStamp;
	  newRun = 0;
	}	
	
	free(pkt->data);
	pkt = NULL;
	rDat->dQue.pop();
	pcnt++;
	      
      }

    }
    
    //RunStartIndex += p;
    thisData->PreScF = PreSc;
    thisData->RunLength = rTime;
    //thisData->NSamples = SampRead;//ReadNSamples;
    tmpData = thisData;

    if(fOpen){
      if(dataTree){
	dataTree->Fill();	
	cout << "Done filling this packet: " <<  std::dec << dataTree->GetTotBytes() << endl;
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

    delete rDat;
    rQue->pop();
    
    gSystem->ProcessEvents();
    rcnt++;
  }

  return NULL;
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
  zmq_ctx_destroy(context);
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
      *SettingsOutFile << "RunLength[s] " << iSettings.RunLength  << '\n';
      *SettingsOutFile << "ReversalFrequency " << iSettings.ReversalFreq  << '\n';     
      *SettingsOutFile << "SamplingDelay " << iSettings.SamplingDelay << '\n';

      *SettingsOutFile << "IntMode " << iSettings.IntMode << '\n';   //toggles integration and streaming mode : 1 = integration on
      *SettingsOutFile << "SingleBlock " << iSettings.IntModeSingle << '\n';  //enables single integration region  (e.g. the enitre helicity window)
      *SettingsOutFile << "AllChannels " << iSettings.streamAll << '\n';   //enable streaming all 16 channels, but at much reduced sample factor
      *SettingsOutFile << "InitDelay " << iSettings.initDelay << '\n';  //sets the pockels cell settle time delay in  8 ns intervals
      *SettingsOutFile << "TotalSamplingDelay " << iSettings.totalSamplingDelay << '\n';   //sum of the pockles cell and additional adc/sampling delay in 8 ns intervals
      *SettingsOutFile << "NumBlocks " << iSettings.numBlocks << '\n';   //number of blocks in helcity window
      *SettingsOutFile << "BlockLength " << iSettings.blockSize << '\n';   // length of each block currently in terms of 8 ns clock cycles
      
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


