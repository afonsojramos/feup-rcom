//c++ includes
#include <iostream>
#include <string>
#include <regex>

#include "download.h"

using namespace std;

int main(int argc, char const ** argv) {

    if (argc != 2){
        printf("Usage: ./download <ftpUrl>\n");
        exit(-1);
    }

    urlParts * url = (urlParts *) malloc(sizeof(urlParts));
    if(!loadUrl(argv[1], url)){
        printf("Unable to parse the url '%s'\n", argv[1]);
        exit(-1);
    }

    return 0;
}

bool loadUrl(string stringUrl, urlParts * url){
    //ftp://[<user>:<password>@]<host>/<url-path>
    regex url_regex ("ftp://([^:]*):([^@]*)@?([^/]+)/(.*/)?([^/]*)");
    smatch url_match_result;

    if (regex_match(stringUrl, url_match_result, url_regex)) {
        url->username = url_match_result[0];
        url->password = url_match_result[1];
        url->hostname = url_match_result[2];
        url->filepath = url_match_result[3];
        url->filename = url_match_result[4];
        for (const auto& res : url_match_result) {
            cout << ": " << res << endl;
        }
    } else {//Malformed url
        return false;
    }
    return true;
}
