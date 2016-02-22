/**
 * @file
 *
 * This file defines a command line interface for the Package Manager
 */

/******************************************************************************
 * Copyright (c) 2014, AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <getopt.h>
#include <alljoyn/gateway/PackageManager.h>
#include <qcc/String.h>

using namespace std;
using namespace ajn::gw;

const string Usage()
{
    stringstream usage;
    usage << "Packet Manager Usage:\n";
    usage << "-pname";
    usage << "[install | -uninstall | -upgrade]\n";
    usage << "-version\n";
    usage << "-url\n";
    usage << "-size\n";
    usage << "-uid\n";
    usage << "-help\n";

    return usage.str();
}

const int APP_ID =     'a';
const int PNAME =      'p';
const int INSTALL =    'i';
const int UPGRADE =    'u';
const int UNINSTALL =  'r';
const int VERSION =    'v';
const int URL =        'l';
const int SIZE =       's';
const int UID =        'd';
const int HELP =       'h';


static struct option long_options[] = {
    { "appid",      required_argument,  0,  APP_ID    },
    { "pname",      required_argument,  0,  PNAME     },
    { "install",    no_argument,        0,  INSTALL   },
    { "upgrade",    no_argument,        0,  UPGRADE   },
    { "uninstall",  no_argument,        0,  UNINSTALL },
    { "version",    required_argument,  0,  VERSION   },
    { "url",        required_argument,  0,  URL       },
    { "size",       required_argument,  0,  SIZE      },
    { "uid",        required_argument,  0,  UID       },
    { "help",       no_argument,        0,  HELP      },
    { 0, 0, 0, 0 }
};

int main(int argc, char* argv[])
{
    using namespace qcc;
    int index(0);
    int result(0);
    String appId;
    bool install(false);
    bool upgrade(false);
    bool uninstall(false);
    String packageName;
    String version;
    String url;
    unsigned int size(0);
    String uid;
    String fileUrl;

    while (result != -1) {
        result = getopt_long_only(argc, argv, "p:a:iunv:l:s:d:f:h", long_options, &index);
        switch (result) {

        case APP_ID:
            cout << "App Id: " << optarg << endl;
            appId = optarg;
            break;

        case PNAME:
            cout << "pname: " << optarg << endl;
            packageName = optarg;
            break;

        case INSTALL:
            install = true;
            if (upgrade || uninstall) {
                cout << "install is incompatible with upgrade or uninstall\n";
                return 1;
            }
            break;

        case UPGRADE:
            install = true;
            upgrade = true;
            if (install || uninstall) {
                cout << "upgrade is incompatible with install or uninstall\n";
                return 1;
            }
            cout << "upgrade\n";
            break;

        case UNINSTALL:
            uninstall = true;
            if (install || upgrade) {
                cout << "uninstall is incompatible with install or upgrade\n";
                return 1;
            }
            cout << "uninstall\n";
            break;

        case VERSION:
            cout << "version: " << optarg << endl;
            version = optarg;
            break;

        case URL:
            cout << "url: " << optarg << endl;
            url = optarg;
            break;

        case SIZE:
            size = stoul(optarg);
            cout << "size: " << size << endl;
            break;

        case UID:
            cout << "uid: " << optarg << endl;
            uid = optarg;
            break;

        case HELP:
            cout << Usage() << endl;
            break;

        default:
            if (result != -1) {
                cout << "unrecognised option: " << index << endl;
                return 1;
            }
        }
    }

    PackageManager mgr;
    QStatus responseStatus(ER_FAIL);
    if (install) {
        cout << "pmcl install mode\n";
        if (argc != 14) {
            cout << "incorrect number of arguments\n";
            responseStatus = ER_BAD_ARG_COUNT;
        } else {
            mgr.InstallApp(appId, packageName, version, url, upgrade, uid, responseStatus);
        }

    } else if (uninstall) {
        cout << "pmcl uninstall mode\n";
        if (argc != 6) {
            cout << "incorrect number of arguments\n";
            responseStatus = ER_BAD_ARG_COUNT;
        } else {
            mgr.UninstallApp(packageName, responseStatus);
        }

        if (responseStatus != ER_OK) {
            cout << "Package Manager command failed: " << QCC_StatusText(responseStatus) << endl;
        }
    }

    return responseStatus;
}
