//
// Filename: NErrorLog
//  This file defines the interface for the NErrorLog class.  The
//   class implements a thread-safe, per process error log, whose
//   name is determined by the executing program's name.      
//
// Copyright 2020 embeddedKeith
// -*^*-
//-------------------------------------------------------------------------
#ifndef _NErrorLog_
#define _NErrorLog_

#include "osinc.hpp"
#include "ntime.hpp"
#include "nevent.hpp"
#include "embeddedSeqList.hpp"
#include "nreslock.hpp"

class NErrorLog;

class _MSExport NErrorLog
{

	public:
		NErrorLog( const NString& pszHdr, const NString& ErrExt);
		virtual ~NErrorLog();
		void log(DWORD Error, const char* pszCharData);
		void checkPoint();
		void dump(NString strDumpFile);
		static Boolean isInit();
		static NErrorLog& getClass();

	protected:
		class ErrorLogData
		{
			public:
				enum ErrOp {LogOp, ChkPntOp, DumpOp, CloseOp};
				ErrOp  Operation;
				NTime  clTime;
				DWORD  dwError;
				PCHAR  cpCharData;

				ErrorLogData(ErrOp Op, DWORD dwErr, const char* cpData);
				ErrorLogData(const ErrorLogData &em);
				ErrorLogData &operator= (const ErrorLogData & );
				~ErrorLogData();
		};

		embeddedSeqList <ErrorLogData> ErrorQueue;
		NPrivateResource QueueLock;
		NEvent Evt, threadDoneEvent;	
		NString strFileHdr, strFileExt;
		pthread_t *pcs;
		static Boolean bInit;
		static NErrorLog *clpThis;

		static void startProcessLoop(NErrorLog* clpPtr);
		void processLoop();
		NString GetFileName();
		void storeMsg(ErrorLogData &em);
		Boolean getMsg(ErrorLogData &em);
};

#endif
