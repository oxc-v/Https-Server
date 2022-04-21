#include "download_file_service.hpp"
#include "upload_file_service.hpp"
#include "web_file_service.hpp"
#include "login_service.hpp"
#include "server.hpp"

using namespace https_server;

int main()
{
    UploadFileService upload_file;
    WebFileService web_file;
    DownloadFileService download_file;
    LoginService login;

    Server s("localhost", "8887");
    s.addService("/UploadFile", upload_file);
    s.addService("/WebFile", web_file);
    s.addService("/DownloadFile", download_file);
    s.addService("/Login", login);

    s.run();

    return 0;
}