#include "stdafx.h"

#include "CefWpfWebBrowser.h"

namespace CefSharp
{
    void CefWpfWebBrowser::OnInitialized()
    {
        _browserInitialized->Set();
    }

    Size CefWpfWebBrowser::ArrangeOverride(Size size)
    {
        Matrix transform = PresentationSource::FromVisual(this)->CompositionTarget->TransformToDevice;

        int w = size.Width * transform.M11;
        int h = size.Height * transform.M22;

        if (!_bitmap ||
            _bitmap->PixelWidth != w ||
            _bitmap->PixelHeight != h)
        {
            _bitmap = gcnew WriteableBitmap(w, h, 96 * transform.M11, 96 * transform.M22, PixelFormats::Bgr32, nullptr);
        }

        _handlerAdapter->GetCefBrowser()->SetSize(PET_VIEW, size.Width, size.Height);

        return Image::ArrangeOverride(size);
    }

    void CefWpfWebBrowser::OnGotFocus(EventArgs^ e)
    {

    }

    void CefWpfWebBrowser::Paint(const CefRect& dirtyRect, const void* buffer)
    {
        System::Console::WriteLine("wpf paint");
    }
}

/*
#include "stdafx.h"

#include "CefWebBrowser.h"
#include "JsTask.h"
#include "ScriptException.h"

namespace CefSharp
{
    void CefWebBrowser::Load(String^ url)
    {
        WaitForInitialized();

        _loadCompleted->Reset();
        _handlerAdapter->GetCefBrowser()->GetMainFrame()->LoadURL(toNative(url));
    }

    void CefWebBrowser::Stop()
    {
    	WaitForInitialized();

        _handlerAdapter->GetCefBrowser()->StopLoad();
    }

    void CefWebBrowser::Back()
    {
    	WaitForInitialized();

        _handlerAdapter->GetCefBrowser()->GoBack();
    }

    void CefWebBrowser::Forward()
    {
    	WaitForInitialized();

        _handlerAdapter->GetCefBrowser()->GoForward();
    }

    void CefWebBrowser::Reload()
    {
        Reload(false);
    }

    void CefWebBrowser::Reload(bool ignoreCache)
    {
    	WaitForInitialized();

        if(ignoreCache)
        {
            _handlerAdapter->GetCefBrowser()->ReloadIgnoreCache();
        }
        else
        {
            _handlerAdapter->GetCefBrowser()->Reload();
        }
    }

    String^ CefWebBrowser::RunScript(String^ script, String^ scriptUrl, int startLine)
    {
    	WaitForInitialized();

        return RunScript(script, scriptUrl, startLine, -1);
    }

    String^ CefWebBrowser::RunScript(String^ script, String^ scriptUrl, int startLine, int timeout)
    {
    	WaitForInitialized();

        
        _jsError = false;
        _jsResult = nullptr;
/*
        script = 
            "(function() {"
            "   try { "
            "      __js_run_done(" + script + ");"
            "   } catch(e) {"
            "      __js_run_err(e);"
            "   }"
            "})();";

        
        CefRefPtr<JsTask> task = new JsTask(this, toNative(script), toNative(scriptUrl), startLine);
        _handlerAdapter->GetCefBrowser()->GetMainFrame()->ExecuteJavaScriptTask(static_cast<CefRefPtr<CefV8Task>>(task));

        if(!_runJsFinished->WaitOne(timeout))
        {
            throw gcnew TimeoutException(L"Timed out waiting for JavaScript to return");
        }

        if(_jsError == false) 
        {
            return _jsResult;
        }
        throw gcnew ScriptException("An error occurred during javascript execution");
    }

    void CefWebBrowser::OnInitialized()
    {
        BeginInvoke(gcnew Action<EventArgs^>(this, &CefWebBrowser::OnSizeChanged), EventArgs::Empty);
        _browserInitialized->Set();
    }

    void CefWebBrowser::OnHandleCreated(EventArgs^ e)
    {
        if (DesignMode == false) 
        {
            _handlerAdapter = new HandlerAdapter(this);
            CefRefPtr<HandlerAdapter> ptr = _handlerAdapter.get();

            CefString urlStr = toNative(_address);

            CefWindowInfo windowInfo;

            HWND hWnd = static_cast<HWND>(Handle.ToPointer());
            RECT rect;
            GetClientRect(hWnd, &rect);
            windowInfo.SetAsChild(hWnd, rect);

            CefBrowser::CreateBrowser(windowInfo, false, static_cast<CefRefPtr<CefHandler>>(ptr), urlStr);
        }
    }

    void CefWebBrowser::OnSizeChanged(EventArgs^ e)
    {
        if (DesignMode == false && IsInitialized)
        {
            HWND hWnd = static_cast<HWND>(Handle.ToPointer());
            RECT rect;
            GetClientRect(hWnd, &rect);
            HDWP hdwp = BeginDeferWindowPos(1);

            HWND browserHwnd = _handlerAdapter->GetBrowserHwnd();
            hdwp = DeferWindowPos(hdwp, browserHwnd, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
            EndDeferWindowPos(hdwp);
        }
    }

    void CefWebBrowser::OnGotFocus(EventArgs^ e)
    {
        if (IsInitialized && !DesignMode)
        {
            _handlerAdapter->GetCefBrowser()->SetFocus(true);
        }
    }

    void CefWebBrowser::SetTitle(String^ title)
    {
        _title = title;
        PropertyChanged(this, gcnew PropertyChangedEventArgs(L"Title"));
    }

    void CefWebBrowser::SetAddress(String^ address)
    {
        _address = address;
        PropertyChanged(this, gcnew PropertyChangedEventArgs(L"Address"));
    }

    void CefWebBrowser::SetNavState(bool isLoading, bool canGoBack, bool canGoForward)
    {
        if(isLoading != _isLoading) 
        {
            _isLoading = isLoading;
            PropertyChanged(this, gcnew PropertyChangedEventArgs(L"IsLoading"));
        }

        if(canGoBack != _canGoBack) 
        {
            _canGoBack = canGoBack;
            PropertyChanged(this, gcnew PropertyChangedEventArgs(L"CanGoBack"));
        }

        if(canGoForward != _canGoForward)
        {
            _canGoForward = canGoForward;
            PropertyChanged(this, gcnew PropertyChangedEventArgs(L"CanGoForward"));
        }
    }

    void CefWebBrowser::AddFrame(CefRefPtr<CefFrame> frame)
    {
        _loadCompleted->AddCount();
    }

    void CefWebBrowser::FrameLoadComplete(CefRefPtr<CefFrame> frame)
    {
        _loadCompleted->Signal();
    }

    void CefWebBrowser::WaitForLoadCompletion()
    {
        WaitForLoadCompletion(-1);
    }

    void CefWebBrowser::WaitForLoadCompletion(int timeout)
    {
        _loadCompleted->Wait(timeout);
    }

    void CefWebBrowser::SetJsResult(String^ result)
    {
        _jsResult = result;
        _runJsFinished->Set();
    }

    void CefWebBrowser::SetJsError()
    {
        _jsError = true;
        _runJsFinished->Set();
    }

    void CefWebBrowser::RaiseConsoleMessage(String^ message, String^ source, int line)
    {
        ConsoleMessage(this, gcnew ConsoleMessageEventArgs(message, source, line));
    }

    void CefWebBrowser::WaitForInitialized()
    {
        if (IsInitialized) return;

        // TODO: risk of infinite lock
        _browserInitialized->WaitOne();
    }
}
*/