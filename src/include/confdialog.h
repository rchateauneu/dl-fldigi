// generated by Fast Light User Interface Designer (fluid) version 1.0108

#ifndef confdialog_h
#define confdialog_h
#include <FL/Fl.H>
#include "globals.h"
#include "modem.h"
#include "configuration.h"
extern Fl_Double_Window *dlgConfig; 
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tabs.H>
extern Fl_Tabs *tabsConfigure;
#include <FL/Fl_Group.H>
extern Fl_Group *tabOperator;
#include <FL/Fl_Input.H>
extern Fl_Input *inpMyCallsign;
extern Fl_Input *inpMyName;
extern Fl_Input *inpMyQth;
extern Fl_Input *inpMyLocator;
#include <FL/Fl_Check_Button.H>
extern Fl_Check_Button *btnUseLeadingZeros;
#include <FL/Fl_Value_Input.H>
extern Fl_Value_Input *nbrContestStart;
extern Fl_Value_Input *nbrContestDigits;
extern Fl_Group *tabWaterfall;
#include "colorbox.h"
extern colorbox *WF_Palette;
#include <FL/Fl_Button.H>
extern Fl_Button *btnColor[9];
extern Fl_Button *btnLoadPalette;
extern Fl_Button *btnSavePalette;
#include <FL/Fl_Counter.H>
extern Fl_Counter *cntLowFreqCutoff;
extern Fl_Check_Button *btnWFaveraging;
extern Fl_Check_Button *btnBlackman;
extern Fl_Check_Button *btnHamming;
extern Fl_Check_Button *btnHanning;
extern Fl_Check_Button *btnTriangular;
extern Fl_Counter *valLatency;
extern Fl_Check_Button *btnUseCursorLines;
extern Fl_Check_Button *btnUseBWTracks;
extern Fl_Check_Button *btnUseCursorCenterLine;
#include <FL/Fl_Color_Chooser.H>
extern Fl_Button *btnCursorBWcolor;
extern Fl_Button *btnCursorCenterLineColor;
extern Fl_Button *btnBwTracksColor;
extern Fl_Group *tabVideo;
extern Fl_Check_Button *btnsendid;
extern Fl_Check_Button *btnsendvideotext;
extern Fl_Input *valVideotext;
#include <FL/Fl_Value_Slider.H>
extern Fl_Value_Slider *sldrVideowidth;
extern Fl_Check_Button *chkID_SMALL;
extern Fl_Check_Button *btnViewXmtSignal;
extern Fl_Group *sld;
extern Fl_Check_Button *btnCWID;
extern Fl_Value_Slider *sldrCWIDwpm;
extern Fl_Group *tabRig;
#include <FL/Fl_Round_Button.H>
extern Fl_Input *inpTTYdev;
extern Fl_Round_Button *btnRTSptt;
extern Fl_Round_Button *btnDTRptt;
extern Fl_Round_Button *btnRTSplusV;
extern Fl_Round_Button *btnDTRplusV;
#include <FL/Fl_Box.H>
extern Fl_Round_Button *btnPTT[5];
extern Fl_Check_Button *chkUSERIGCAT;
extern Fl_Check_Button *chkUSEHAMLIB;
extern Fl_ComboBox *cboHamlibRig;
extern Fl_Input *inpRIGdev;
#include <FL/Fl_Choice.H>
extern Fl_Choice *mnuBaudRate;
extern Fl_Check_Button *chkUSEMEMMAP;
extern Fl_Button *btnInit_Interface;
extern Fl_Group *tabQRZ;
extern Fl_Check_Button *btnQRZnotavailable;
extern Fl_Check_Button *btnQRZsocket;
extern Fl_Check_Button *btnQRZcdrom;
extern Fl_Check_Button *btnHAMCALLsocket;
extern Fl_Input *inpQRZusername;
extern Fl_Input *inpQRZuserpassword;
extern Fl_Button *btnQRZpasswordShow;
extern Fl_Group *tabSoundCard;
extern Fl_Tabs *tabsSoundCard;
extern Fl_Group *tabAudio;
extern Fl_Group *AudioOSS;
#include <FL/Fl_Input_Choice.H>
extern Fl_Input_Choice *menuOSSDev;
extern Fl_Group *AudioPort;
extern Fl_Choice *menuPortInDev;
extern Fl_Choice *menuPortOutDev;
extern Fl_Group *AudioPulse;
extern Fl_Input *inpPulseServer;
extern Fl_Group *AudioNull;
extern Fl_Round_Button *btnAudioIO[4];
extern Fl_Group *tabAudioOpt;
#include <FL/Fl_Spinner.H>
extern Fl_Spinner *cntRxRateCorr;
extern Fl_Spinner *cntTxRateCorr;
extern Fl_Spinner *cntTxOffset;
extern Fl_Group *AudioSampleRate;
extern Fl_Choice *menuOutSampleRate;
extern Fl_Choice *menuInSampleRate;
extern Fl_Choice *menuSampleConverter;
extern Fl_Group *tabMixer;
#include <FL/Fl_Light_Button.H>
extern void setMixerInput(int);
extern Fl_Light_Button *btnLineIn;
extern Fl_Light_Button *btnMicIn;
extern void setPCMvolume(double);
extern Fl_Value_Slider *valPCMvolume;
extern Fl_Input_Choice *menuMix;
extern void resetMixerControls();
extern Fl_Check_Button *btnMixer;
extern Fl_Group *tabMisc;
extern Fl_Value_Input *valCWsweetspot;
extern Fl_Value_Input *valRTTYsweetspot;
extern Fl_Value_Input *valPSKsweetspot;
extern Fl_Check_Button *btnStartAtSweetSpot;
extern Fl_Check_Button *chkTransmitRSid;
extern Fl_Check_Button *chkSlowCpu;
extern Fl_Group *tabModems;
extern Fl_Tabs *tabsModems;
extern Fl_Group *tabCW;
extern Fl_Value_Slider *sldrCWbandwidth;
extern Fl_Counter *cntCWrange;
extern Fl_Check_Button *btnCWrcvTrack;
#include <FL/Fl_Value_Output.H>
extern Fl_Value_Output *valCWrcvWPM;
#include <FL/Fl_Progress.H>
extern Fl_Progress *prgsCWrcvWPM;
extern Fl_Value_Slider *sldrCWxmtWPM;
extern Fl_Counter *cntCWlowerlimit;
extern Fl_Counter *cntCWupperlimit;
extern Fl_Counter *cntCWweight;
extern Fl_Counter *cntCWdash2dot;
extern Fl_Counter *cntCWrisetime;
extern Fl_Counter *cntCWdefWPM;
extern Fl_Group *tabCWQSK;
extern Fl_Check_Button *btnQSK;
extern Fl_Counter *cntPreTiming;
extern Fl_Counter *cntPostTiming;
extern Fl_Group *tabTHOR;
extern Fl_Input *txtTHORSecondary;
extern Fl_Counter *valTHOR_BW;
extern Fl_Check_Button *valTHOR_FILTER;
extern Fl_Counter *valTHOR_PATHS;
extern Fl_Check_Button *valTHOR_SOFT;
extern Fl_Group *tabDomEX;
extern Fl_Input *txtSecondary;
extern Fl_Counter *valDominoEX_BW;
extern Fl_Check_Button *valDominoEX_FILTER;
extern Fl_Check_Button *chkDominoEX_FEC;
extern Fl_Counter *valDominoEX_PATHS;
extern Fl_Group *tabFeld;
#include "fontdef.h"
extern Fl_Choice *selHellFont;
extern Fl_Value_Slider *sldrHellBW;
extern Fl_Check_Button *btnHellXmtWidth;
extern Fl_Check_Button *btnHellRcvWidth;
extern Fl_Check_Button *btnBlackboard;
extern Fl_Check_Button *btnHellFastAttack;
extern Fl_Check_Button *btnHellSlowAttack;
extern Fl_Check_Button *btnFeldHellIdle;
extern Fl_Group *tabOlivia;
extern Fl_Choice *mnuOlivia_Tones;
extern Fl_Choice *mnuOlivia_Bandwidth;
extern Fl_Button *btnRestartOlivia;
extern Fl_Counter *cntOlivia_smargin;
extern Fl_Counter *cntOlivia_sinteg;
extern Fl_Check_Button *btnOlivia_8bit;
extern Fl_Group *tabPSK;
extern Fl_Counter *cntSearchRange;
extern Fl_Check_Button *btnPSKmailSweetSpot;
extern Fl_Counter *cntServerOffset;
extern Fl_Check_Button *btnMarquee;
extern Fl_Check_Button *btnShowFrequencies;
extern Fl_Spinner *cntChannels;
extern Fl_Spinner *cntStartFrequency;
extern Fl_Spinner *cntTimeout;
extern Fl_Counter *cntACQsn;
extern Fl_Group *tabMT63;
extern Fl_Check_Button *btnMT63_8bit;
extern Fl_Check_Button *btnmt63_interleave;
extern Fl_Group *tabRTTY;
extern Fl_Choice *selShift;
extern Fl_Choice *selBaud;
extern Fl_Choice *selBits;
extern Fl_Choice *selParity;
extern Fl_Choice *selStopBits;
extern Fl_Check_Button *chkPseudoFSK;
extern Fl_Check_Button *btnCRCRLF;
extern Fl_Check_Button *btnAUTOCRLF;
extern Fl_Counter *cntrAUTOCRLF;
extern Fl_Check_Button *btnRTTY_USB;
extern Fl_Round_Button *btnRTTYafc[3];
extern Fl_Check_Button *btnPreferXhairScope;
extern Fl_Check_Button *chkXagc;
extern Fl_Check_Button *chkUOSrx;
extern Fl_Check_Button *chkUOStx;
#include <FL/Fl_Return_Button.H>
extern Fl_Return_Button *btnCloseConfig;
extern Fl_Button *btnSaveConfig;
Fl_Double_Window* ConfigureDialog();
void openConfig();
void closeDialog();
void createConfig();
#endif
