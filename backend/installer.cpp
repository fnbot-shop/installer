#pragma once
#include "installer.h"

#include "use_http.h"
#include "streams.h"
#include "registry.h"

#include <shlobj.h>

#define UNUSED_RETURN(x) __pragma(warning(suppress:6031)) x

namespace installer {
    static void create_shortcut(std::string target, std::string targetArgs, std::wstring output, std::string pszDescription, int iShowmode, std::string pszCurdir, std::string pszIconfile, int iIconindex)
    {
        IShellLink* pShellLink;
        IPersistFile* pPersistFile;
        UNUSED_RETURN(CoInitialize(NULL));
        if (iShowmode >= 0 && iIconindex >= 0) {
            if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&pShellLink)))
            {
                pShellLink->SetPath(target.c_str());
                pShellLink->SetArguments(targetArgs.c_str());
                if (pszDescription.size() > 0)
                {
                    pShellLink->SetDescription(pszDescription.c_str());
                }
                if (iShowmode > 0)
                {
                    pShellLink->SetShowCmd(iShowmode);
                }
                if (pszCurdir.size() > 0)
                {
                    pShellLink->SetWorkingDirectory(pszCurdir.c_str());
                }
                if (pszIconfile.size() > 0 && iIconindex >= 0)
                {
                    pShellLink->SetIconLocation(pszIconfile.c_str(), iIconindex);
                }
                if (SUCCEEDED(pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile)))
                {
                    pPersistFile->Save(output.c_str(), TRUE);
                    pPersistFile->Release();
                }
                pShellLink->Release();
            }
        }
        CoUninitialize();
        return;
    }

    InstallManager::InstallManager(INSTALL_INFO* info) {
        this->info = info;
    }

    void InstallManager::Run() {
        thread = std::thread(&InstallManager::run_thread, this);
    }

    void InstallManager::run_thread() {
        update_info(STATE::INIT_PINNING, 0, nullptr);

        auto pinner = SSLPinner();
        if (!pinner.valid) {
            info->error_callback(info->lParam, "Could not create a secure connection.");
            return;
        }

        update_info(STATE::INIT_STREAMS, .02, nullptr);

        Header header;
        bool error = false;
        ArchiveFileStream stream(info->install_location, &header, [&error, this](const char* err) {
            error = true;
            this->info->error_callback(info->lParam, err);
        });
        LZ4_DecompressionStream decompStream(&stream);

        httplib::SSLClient cli(DOWNLOAD_HOST);
        cli.set_compress(true);
        cli.set_ca_cert_path(pinner.filename);
        cli.enable_server_certificate_verification(true);
        cli.set_follow_location(true);

        update_info(STATE::INIT_DOWNLOAD, .03, nullptr);
        bool started = false; // the html buffers a bit with the github html response, wait till the file magic comes through
        auto resp = cli.Get(DOWNLOAD_PATH, [&decompStream, &started, &header, &error, this](const char* data, uint64_t data_length) {
            if (!started) {
                if (*((int*)data) == FILE_MAGIC) {
                    started = true;
                    header.InstallSize = *(((int*)data) + 1);
                    decompStream.recieve(data + 8, data_length - 8);
                }
                return true;
            }
            decompStream.recieve(data, data_length);
            if (error) {
                return false;
            }
            return true;
            }, [&stream, &started, this](uint64_t len, uint64_t total) {
                static int val = 1;
                --val;
                if (started && !val) {
                    update_info(STATE::DOWNLOAD, .05 + (((double)len / total) * .9), stream.filename.c_str());
                    val = 5;
                }
                return true;
            });
        if (error) {
            return;
        }
        update_info(STATE::REGISTRY, .95, nullptr);
        registry_uninstall();

        ProgramRegistryInfo uninstall_info;
        startExe = info->install_location / header.StartupExe;
        auto startup_exe = startExe.string();
        uninstall_info.DisplayIcon = startup_exe;
        uninstall_info.DisplayName = DOWNLOAD_REPO;
        uninstall_info.DisplayVersion = DOWNLOAD_VERSION;
        uninstall_info.Publisher = DOWNLOAD_AUTHOR;
        uninstall_info.HelpLink = header.HelpLink;
        uninstall_info.InstallDate = registry_datetime();
        uninstall_info.InstallLocation = info->install_location.string();
        uninstall_info.URLInfoAbout = header.AboutLink;
        uninstall_info.URLUpdateInfo = header.PatchNotesLink;
        uninstall_info.EstimatedSize = header.InstallSize;// / 1024;
        uninstall_info.NoModify = 1;
        uninstall_info.NoRepair = 0;
        uninstall_info.ModifyPath = "\"" + startup_exe + "\" " + header.ModifyPath;
        uninstall_info.UninstallString = "\"" + startup_exe + "\" " + header.UninstallString;
        registry_install(&uninstall_info);

        update_info(STATE::SHORTCUT, .98, nullptr);
        fs::path shortcut_path = fs::path(getenv("PROGRAMDATA")) / "Microsoft\\Windows\\Start Menu\\Programs\\" DOWNLOAD_REPO ".lnk";
        fs::remove(shortcut_path); // remove if the file exists
        create_shortcut(startup_exe, std::string(), shortcut_path.native(), std::string(), 1, info->install_location.string(), startup_exe, 0);

        update_info(STATE::DONE, 1, nullptr);
    }

    void InstallManager::update_info(STATE state, double progress, const char* state_info) {
        info->state = state;
        info->progress = progress;
        if (state_info) {
            strcpy(info->state_info, state_info);
        }
        info->callback(info);
    }
}