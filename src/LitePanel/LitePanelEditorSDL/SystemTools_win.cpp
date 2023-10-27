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
}
