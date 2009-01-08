#ifndef MANAGERS_H
#define MANAGERS_H

/* There are a number of objects and variables (the Managers) that need to   */
/* be available from almost all code modules.  Rather than try to pass them  */
/* around as arguments to functions (which makes for long and unwieldy       */
/* parameter lists), they are made global.  The actual definitions are all   */
/* in main.cpp, or called from it.                                           */

#include "NodeInfo.h"
extern NodeInfo *NI;

#include "Brain.h"
extern Brain *TheBrain;

#include "Input.h"
extern Input *In;

#include "MessageManager.h"
extern MessageManager *MsgMgr;

#include "MessageBus.h"
extern MessageBus *MsgBus;

#include "CellManager.h"
extern CellManager *CellMgr;

#include "ActiveSyn.h"
extern ActiveSynPool *ActiveSynMgr;

//Includes for doc++
//@Include CellManager.h

#endif
