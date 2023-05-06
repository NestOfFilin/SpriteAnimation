#include "SpriteAnimation/SpriteAnimation.h"

#include <windows.h>
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
		, m_SfmlWin(m_Win32Window)
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
		UnregisterClassW(L"SpriteAnimation", m_hInstance);

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
		case WM_SIZE:
		{
			const unsigned sx = LOWORD(lParam);
			const unsigned sy = HIWORD(lParam);
			if (m_SfmlWin.isOpen())
			{
				m_SfmlWin.setView(sf::View(sf::FloatRect(
					0.F,
					0.F,
					static_cast<float>(sx),
					static_cast<float>(sy)
				)));
				m_SfmlWin.setSize({ sx, sy });

				if (m_pSpriteAnimation)
				{
					auto& sprite = m_pSpriteAnimation->GetSprite();
					const auto scale = sprite.getScale();
					sprite.setPosition(
						static_cast<float>(sx) / 2.F,
						static_cast<float>(sy) / 2.F
					);
				}
			}
			break;
		}
		case WM_TIMER:
		{
			if (m_pSpriteAnimation)
			{
				m_pSpriteAnimation->PutNextSprite();
			}
		}
		case WM_PAINT:
		{
			if (m_SfmlWin.isOpen())
			{
				m_SfmlWin.clear(sf::Color(123U, 191U, 96U, 255U));
				if (m_pSpriteAnimation)
				{
					m_SfmlWin.draw(m_pSpriteAnimation->GetSprite());
				}
				m_SfmlWin.display();
			}
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
			uCountFiles = DragQueryFileW(hdrop, 0xFFFFFFFF, nullptr, 0Ui32);

			for (UINT uFile = 0Ui32; uFile < uCountFiles; uFile++)
			{
				UINT uBufferSize = DragQueryFileW(hdrop, uFile, nullptr, 0Ui32);
				if (uBufferSize == 0Ui32)
				{
					throw EXIT_FAILURE;
				}
				std::wstring wstrFileName;
				wstrFileName.resize(static_cast<size_t>(uBufferSize) + 1Ui64);

				UINT uReadedChars = DragQueryFileW(
					hdrop,
					uFile,
					(TCHAR*)wstrFileName.data(),
					uBufferSize + 1Ui32
				);

				if (uReadedChars == 0Ui32)
				{
					throw EXIT_FAILURE;
				}

				std::string strFileName(wstrFileName.length(), 0Ui64);
				std::transform(
					wstrFileName.begin(),
					wstrFileName.end(),
					strFileName.begin(),
					[](wchar_t c)
					{
						return static_cast<char>(c);
					}
				);

				sf::Image image;
				if (!image.loadFromFile(strFileName))
				{
					throw EXIT_FAILURE;
				}

				m_pSpriteAnimation = std::make_unique<CSpriteAnimation>(
					image,
					ESCGerenatorType::ColorColumn
				);
			}

			// Free up memory.
			DragFinish(hdrop);

			auto& sprite = m_pSpriteAnimation->GetSprite();

			sprite.setScale(sprite.getScale() * 4.F);
			const auto spriteRect = sprite.getLocalBounds();
			sprite.setOrigin(spriteRect.width / 2.F, spriteRect.height / 2.F);
			const auto wndSize = m_SfmlWin.getSize();
			sprite.setPosition(
				static_cast<float>(wndSize.x) / 2.F,
				static_cast<float>(wndSize.y) / 2.F
			);

			SetTimer(handle, m_TimerID, 16Ui32, nullptr); // 1000 / 60 ~= 16
			break;
		}
		}

		return DefWindowProc(handle, message, wParam, lParam);
	}

private:
	unsigned __int64 m_TimerID{ 1Ui64 };

	HINSTANCE m_hInstance;

	WNDCLASS m_Win32WindowStructClass;
	HWND m_Win32Window;

	sf::RenderWindow m_SfmlWin;

	std::unique_ptr<CSpriteAnimation> m_pSpriteAnimation;

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
	while (GetMessageW(&message, nullptr, 0Ui32, 0Ui32))
	{
		if (!TranslateAcceleratorW(message.hwnd, hAccelTable, &message))
		{
			TranslateMessage(&message);
			DispatchMessageW(&message);
		}
	}

	return EXIT_SUCCESS;
}
