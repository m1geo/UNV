//
//      (C) 30/Apr/2011 - Obada
//		The UNV Project, Electronic Enginering, University College London
//
//		Based on
//			http://tldp.org/LDP/LG/issue74/tougher.html#4
//			http://www.ibm.com/developerworks/linux/tutorials/l-sock2/section4.html
//			http://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
//

void Die(char* errMsg);
void resetBuffer(int);
void sendHeader();
void sendRTP_Packet(const char *, int );
void startUDP();
void setupUDP(int iUDP_Port);
void dontDie();
void threadStartServerRTSP(int , int ,int );
void startServerRTSP(int , int , int );
void addFrame(char* , int );
void addHeader(char*, int);
void addFrameByFile(const char *, const char *);
