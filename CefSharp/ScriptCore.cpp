#include "stdafx.h"
#include "include/cef_runnable.h"
#include "ScriptCore.h"
#include "ScriptException.h"

namespace CefSharp
{
    bool ScriptCore::TryGetMainFrame(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame>& frame)
    {
        if (browser != nullptr)
        {
            frame = browser->GetMainFrame();
            return frame != nullptr;
        }
        else
        {
            return false;
        }
    }

    void ScriptCore::UIT_Execute(CefRefPtr<CefBrowser> browser, CefString script)
    {
        CefRefPtr<CefFrame> mainFrame;
        if (TryGetMainFrame(browser, mainFrame))
        {
            mainFrame->ExecuteJavaScript(script, "about:blank", 0);
        }
    }

    void ScriptCore::UIT_Evaluate(CefRefPtr<CefBrowser> browser, CefString script)
    {
        CefRefPtr<CefFrame> mainFrame;
        if (TryGetMainFrame(browser, mainFrame))
        {
            CefRefPtr<CefV8Context> context = mainFrame->GetV8Context();

            if (context.get() && context->Enter())
            {
                CefRefPtr<CefV8Value> global = context->GetGlobal();
                CefRefPtr<CefV8Value> eval = global->GetValue("eval");
                CefRefPtr<CefV8Value> arg = CefV8Value::CreateString(script);
                CefRefPtr<CefV8Value> result;
                CefRefPtr<CefV8Exception> exception;

                CefV8ValueList args;
                args.push_back(arg);

                if (eval->ExecuteFunctionWithContext(context, global, args,
                    result, exception, false))
                {
                    if (exception)
                    {
                        CefString message = exception->GetMessage();
                        _exceptionMessage = toClr(message);
                    }
                    else
                    {
                        try
                        {
                            _result = convertFromCef(result);
                        }
                        catch (Exception^ ex)
                        {
                            _exceptionMessage = ex->Message;
                        }
                    }
                }

                context->Exit();
            }
        }
        else
        {
            _exceptionMessage = "Failed to obtain reference to main frame";
        }

        SetEvent(_event);
    }

    void ScriptCore::Execute(CefRefPtr<CefBrowser> browser, CefString script)
    {
        if (CefCurrentlyOn(TID_UI))
        {
            UIT_Execute(browser, script);
        }
        else
        {
            CefPostTask(TID_UI, NewCefRunnableMethod(this, &ScriptCore::UIT_Execute,
                browser, script));
        }
    }

    gcroot<Object^> ScriptCore::Evaluate(CefRefPtr<CefBrowser> browser, CefString script, double timeout)
    {
        AutoLock lock_scope(this);
        _result = nullptr;
        _exceptionMessage = nullptr;

        if (CefCurrentlyOn(TID_UI))
        {
            UIT_Evaluate(browser, script);
        }
        else
        {
            CefPostTask(TID_UI, NewCefRunnableMethod(this, &ScriptCore::UIT_Evaluate,
                browser, script));
        }

        switch (WaitForSingleObject(_event, timeout))
        {
        case WAIT_TIMEOUT:
            throw gcnew ScriptException("Script timed out");
        case WAIT_ABANDONED:
        case WAIT_FAILED:
            throw gcnew ScriptException("Script error");
        }

        if (_exceptionMessage)
        {
            throw gcnew ScriptException(_exceptionMessage);
        }
        else
        {
            return _result;
        }
    }
}