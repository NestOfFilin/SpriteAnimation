#include <SFML/Graphics.hpp>
#include <windows.h>
#include <cmath>
#include <set>


class CWin32SFMLWindow
{
public:
    CWin32SFMLWindow(HINSTANCE hInstance, int nCmdShow)
        : m_hInstance(InitWin32WindowStruct(hInstance))
        , m_Win32Window(CreateWindowExW(
            0L,
            L"SpriteAnimation",
            L"SpriteAnimation",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            200,
            200,
            407,
            680,
            nullptr,
            nullptr,
            hInstance,
            nullptr))
        , m_view1(CreateWindowW(
            L"STATIC",
            nullptr,
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
            20,
            20,
            352,
            560,
            m_Win32Window,
            nullptr,
            hInstance,
            nullptr))
        , m_SFMLView1(m_view1)
    {
        ShowWindow(m_Win32Window, nCmdShow);
        UpdateWindow(m_Win32Window);

        int err = GetLastError();
    }

    ~CWin32SFMLWindow()
    {
        // Destroy the main window (all its child controls will be destroyed)
        DestroyWindow(m_Win32Window);

        // Don't forget to unregister the window class
        UnregisterClassW(L"SFML App", m_hInstance);

        s_WindowPointers.erase(this);
    }

private:
    HINSTANCE InitWin32WindowStruct(HINSTANCE hInstance)
    {
        s_WindowPointers.insert(this);

        m_Win32WindowStructClass.style = CS_HREDRAW | CS_VREDRAW;
        m_Win32WindowStructClass.lpfnWndProc = &CWin32SFMLWindow::StaticOnEvent;
        m_Win32WindowStructClass.cbClsExtra = 0;
        m_Win32WindowStructClass.cbWndExtra = 0;
        m_Win32WindowStructClass.hInstance = hInstance;
        m_Win32WindowStructClass.hIcon = nullptr;
        m_Win32WindowStructClass.hCursor = 0;
        m_Win32WindowStructClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BACKGROUND);
        m_Win32WindowStructClass.lpszMenuName = nullptr;
        m_Win32WindowStructClass.lpszClassName = L"SpriteAnimation";
        RegisterClassW(&m_Win32WindowStructClass);

        return hInstance;
    }

    static LRESULT CALLBACK StaticOnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
    {
        for (const auto& pWnd : s_WindowPointers)
        {
            LRESULT itemRes = pWnd->OnEvent(handle, message, wParam, lParam);
            if (itemRes != 0Ui64)
            {
                return itemRes;
            }
        }
        return 0Ui64;
    }

    LRESULT CALLBACK OnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_CREATE:
        {
            DragAcceptFiles(handle, TRUE); // enabled WM_DROPFILES
            break;
        }
        case WM_TIMER:
        {

            m_CurSpriteNum++;
            if (m_CurSpriteNum == m_SpritesCount)
            {
                m_CurSpriteNum = 0;
            }

            const auto& textureRect = m_Sprite1.getTextureRect();
            m_Sprite1.setTextureRect(
                sf::IntRect(
                    (textureRect.width) * m_CurSpriteNum,
                    textureRect.top,
                    textureRect.width,
                    textureRect.height)
            );

            m_SFMLView1.clear();
            m_SFMLView1.draw(m_Sprite1);
            m_SFMLView1.display();
            break;
        }
        // Quit when we close the main window
        case WM_DESTROY:
        {
            KillTimer(handle, m_TimerID);
            PostQuitMessage(0);
            break;
        }
        case WM_DROPFILES:
        {
            HDROP hdrop = reinterpret_cast<HDROP>(wParam);
            UINT  uCountFiles;

            // Get the # of files being dropped.
            uCountFiles = DragQueryFileW(hdrop, -1, nullptr, 0);

            for (UINT uFile = 0; uFile < uCountFiles; uFile++)
            {
                UINT uBufferSize = DragQueryFileW(hdrop, uFile, nullptr, 0);
                if (uBufferSize == 0)
                {
                    throw EXIT_FAILURE;
                }
                std::wstring wstrFileName;
                wstrFileName.resize(uBufferSize + 1);

                UINT uReadedChars = DragQueryFileW(
                    hdrop,
                    uFile,
                    (TCHAR*)wstrFileName.data(),
                    uBufferSize + 1
                );

                if (uReadedChars == 0)
                {
                    throw EXIT_FAILURE;
                }

                std::string strFileName(wstrFileName.begin(), wstrFileName.end());
                if (!m_Texture1.loadFromFile(strFileName))
                {
                    throw EXIT_FAILURE;
                }
            }

            // Free up memory.
            DragFinish(hdrop);

            m_Sprite1.setTexture(m_Texture1, true);
            const auto& textureRect = m_Sprite1.getTextureRect();
            m_Sprite1.setTextureRect(
                sf::IntRect(textureRect.left,
                    textureRect.top,
                    textureRect.width / 4,
                    textureRect.height)
            );
            m_Sprite1.setScale(m_Sprite1.getScale() * 4.F);

            SetTimer(handle, m_TimerID, 16u, nullptr); // 1000 / 60 ~= 16
            break;
        }
        }

        return DefWindowProc(handle, message, wParam, lParam);
    }

private:
    unsigned __int64 m_TimerID{ 1U };

    HINSTANCE m_hInstance;

    WNDCLASS m_Win32WindowStructClass;
    HWND m_Win32Window;

    HWND m_view1;
    sf::RenderWindow m_SFMLView1;

    sf::Texture m_Texture1;

    sf::Sprite m_Sprite1;

    sf::Clock m_Clock;

    unsigned m_CurSpriteNum{ 0 };
    unsigned m_SpritesCount{ 4 };

private:
    static std::set<CWin32SFMLWindow*> s_WindowPointers;
};

std::set<CWin32SFMLWindow*> CWin32SFMLWindow::s_WindowPointers;


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    CWin32SFMLWindow window(hInstance, nCmdShow);

    HACCEL hAccelTable = LoadAcceleratorsW(hInstance, L"SFML_WIN32");

    // Loop until a WM_QUIT message is received
    MSG message;
    while (GetMessageW(&message, nullptr, 0, 0))
    {
        if (!TranslateAcceleratorW(message.hwnd, hAccelTable, &message))
        {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }

    return EXIT_SUCCESS;
}
