// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "SystemTools.h"

#include <codecvt>
#include <string>
#include <type_traits>

#include <Windows.h>
#include <shobjidl.h> 

#ifdef max
#undef max
#endif

#include <algorithm>

namespace dcclite::panel_editor
{

//https://stackoverflow.com/questions/2573834/c-convert-string-or-char-to-wstring-or-wchar-t/18597384#18597384
#pragma warning(push)
#pragma warning(disable : 4996)

    std::string wstring_to_utf8(std::wstring const &str)
    {
        std::wstring_convert<std::conditional_t<
            sizeof(wchar_t) == 4,
            std::codecvt_utf8<wchar_t>,
            std::codecvt_utf8_utf16<wchar_t>>> converter;
        return converter.to_bytes(str);
    }

    std::wstring utf8_to_wstring(std::string const &str)
    {
        std::wstring_convert<std::conditional<
            sizeof(wchar_t) == 4,
            std::codecvt_utf8<wchar_t>,
            std::codecvt_utf8_utf16<wchar_t>>::type> converter;
        return converter.from_bytes(str);
    }
#pragma warning(pop)
    
    std::optional<dcclite::fs::path> OpenFileDialog()
	{
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

        if (!SUCCEEDED(hr))
            return {};
        
        IFileOpenDialog *pFileOpen;

        // Create the FileOpenDialog object.
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void **>(&pFileOpen));

        dcclite::fs::path result;

        if (SUCCEEDED(hr))
        {
            // Show the Open dialog box.
            hr = pFileOpen->Show(NULL);

            // Get the file name from the dialog box.
            if (SUCCEEDED(hr))
            {
                IShellItem *pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    // Display the file name to the user.
                    if (SUCCEEDED(hr))
                    {
                        result = wstring_to_utf8(pszFilePath);
                        
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
            
        CoUninitialize();

        if (result.empty())
            return {};
        else
            return result;
	}

    std::optional<dcclite::fs::path> SaveFileDialog(const dcclite::fs::path &filePath, const std::string &extension, const std::string &filter)
    {       
        constexpr size_t MY_MAX_PATH = 2048;

        OPENFILENAMEW   ofw = { 0 };
        std::vector<WCHAR> fileName;
        
        {
            auto wfilePath = utf8_to_wstring(filePath.string());

            fileName.resize(std::max(wfilePath.length() + 1, MY_MAX_PATH));
            memcpy(&fileName[0], &wfilePath[0], (wfilePath.length() + 1) * sizeof(WCHAR));
        }        

        ofw.lStructSize = sizeof(ofw);        

        ofw.lpstrFile = &fileName[0];
        ofw.nMaxFile = static_cast<DWORD>(fileName.size());

        ofw.Flags = OFN_DONTADDTORECENT | OFN_NOCHANGEDIR;

        std::wstring wextension = utf8_to_wstring(extension);
        ofw.lpstrDefExt = wextension.length() ? &wextension[0] : nullptr;

        std::vector<WCHAR> vecfilter;

        if (filter.length() && wextension.length())
        {
            std::wstring wfilter = utf8_to_wstring(filter);
         
            //back to 90's with some basic string operations... strings lengths plus \0 *. \0\0
            vecfilter.resize(wfilter.length() + 1 + wextension.length() + 4);            
            memcpy(&vecfilter[0], &wfilter[0], wfilter.length() * sizeof(WCHAR));

            //close first string - vector already initialized to zero
            //vecfilter[wfilter.length()] = WCHAR('\0');

            //add a dot to extension
            vecfilter[wfilter.length() + 1] = WCHAR('*');
            vecfilter[wfilter.length() + 2] = WCHAR('.');

            //copy extension after dot
            memcpy(&vecfilter[wfilter.length() + 3], &wextension[0], wextension.length() * sizeof(WCHAR));

            //const auto len = wfilter.length() + 3 + wextension.length();

            //double null... - vector already initialized to zero
            //vecfilter[len] = WCHAR('\0');
            //vecfilter[len + 1] = WCHAR('\0');

            ofw.lpstrFilter = &vecfilter[0];
        }                

        if (GetSaveFileNameW(&ofw) == 0)
        {
            auto err = CommDlgExtendedError();

            throw std::exception("[SaveFileDialog] GetSaveFileNameW failed with error code: {}", err);
        }

        auto result = wstring_to_utf8(ofw.lpstrFile);

        return result;
    }
}
