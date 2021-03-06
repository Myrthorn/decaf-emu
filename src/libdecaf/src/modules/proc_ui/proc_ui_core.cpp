#include "proc_ui.h"
#include "proc_ui_core.h"

namespace proc_ui
{

static ProcUISaveCallback
gSaveCallback;

static ProcUISaveCallbackEx
gSaveCallbackEx;

static void *
gSaveCallbackExArg;

void
ProcUIInit(ProcUISaveCallback saveCallback)
{
   gSaveCallback = saveCallback;
   gSaveCallbackEx = nullptr;
   gSaveCallbackExArg = nullptr;
}

void
ProcUIInitEx(ProcUISaveCallbackEx saveCallbackEx, void *arg)
{
   gSaveCallback = nullptr;
   gSaveCallbackEx = saveCallbackEx;
   gSaveCallbackExArg = arg;
}

ProcUIStatus
ProcUIProcessMessages(BOOL block)
{
   // TODO: ProcUIProcessMessages
   return ProcUIStatus::InForeground;
}

ProcUIStatus
ProcUISubProcessMessages(BOOL block)
{
   // TODO: ProcUISubProcessMessages
   return ProcUIStatus::InForeground;
}

void
ProcUIRegisterCallback(ProcUICallbackType type,
                       ProcUICallback callback,
                       void *param,
                       uint32_t unk)
{
   // TODO: ProcUIRegisterCallback
}

void
ProcUISetMEM1Storage(void *buffer, uint32_t size)
{
   // TODO: ProcUISetMEM1Storage
}

void
Module::registerCoreFunctions()
{
   RegisterKernelFunction(ProcUIInit);
   RegisterKernelFunction(ProcUIInitEx);
   RegisterKernelFunction(ProcUIProcessMessages);
   RegisterKernelFunction(ProcUISubProcessMessages);
   RegisterKernelFunction(ProcUIRegisterCallback);
   RegisterKernelFunction(ProcUISetMEM1Storage);
}

} // namespace proc_ui
