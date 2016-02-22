void OptParser::parse(int argc, char* argv[] )
{
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
}

