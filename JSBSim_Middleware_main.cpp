#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")

struct Parameter
{
    int type;
    int pad1;
    double value;
    int valid;
    int pad2;
};

std::string trim(std::string s)
{
    s.erase(
        s.begin(),
        std::find_if(
            s.begin(),
            s.end(),
            [](unsigned char ch)
            {
                return !std::isspace(ch);
            }));

    s.erase(
        std::find_if(
            s.rbegin(),
            s.rend(),
            [](unsigned char ch)
            {
                return !std::isspace(ch);
            }).base(),
        s.end());

    return s;
}

std::vector<std::string> split(
        const std::string& s,
        char delimiter)
{
    std::vector<std::string> tokens;

    std::stringstream ss(s);

    std::string token;

    while(getline(ss,token,delimiter))
    {
        tokens.push_back(
            trim(token));
    }

    return tokens;
}

bool isTextPacket(char* buffer,int len)
{
    for(int i=0;i<len;i++)
    {
        unsigned char c=buffer[i];

        if(c==0)
            continue;

        if(c<32 || c>126)
            return false;
    }

    return true;
}

int main()
{
    WSADATA wsa;

    WSAStartup(
        MAKEWORD(2,2),
        &wsa);

    SOCKET sock=
    socket(
        AF_INET,
        SOCK_DGRAM,
        IPPROTO_UDP);

    sockaddr_in server;

    server.sin_family=AF_INET;
    server.sin_port=htons(5500);
    server.sin_addr.s_addr=INADDR_ANY;

    if(bind(
        sock,
        (sockaddr*)&server,
        sizeof(server))
        == SOCKET_ERROR)
    {
        std::cout
        <<"Bind failed : "
        <<WSAGetLastError()
        <<"\n";

        return -1;
    }

    std::cout
    <<"Waiting for JSBSim...\n";

    char buffer[2048];

    while(true)
    {
        sockaddr_in sender;

        int senderSize=
        sizeof(sender);

        int len=
        recvfrom(
            sock,
            buffer,
            sizeof(buffer),
            0,
            (sockaddr*)&sender,
            &senderSize);

        if(len<=0)
            continue;

        std::cout
        <<"\nReceived bytes="
        <<len
        <<"\n";

        //----------------------------------
        // TEXT XML/CSV MODE
        //----------------------------------

        if(isTextPacket(buffer,len))
        {
            buffer[len]='\0';

            std::string packet(buffer);

            if(packet.find("<LABELS>")
               !=std::string::npos)
            {
                std::cout
                <<"Skipping LABEL packet\n";

                continue;
            }

            std::vector<std::string>
            tokens=
            split(packet,',');

            if(tokens.size()>=7)
            {
                std::cout
                <<"\nDetected : XML/CSV\n";

                std::cout
                <<"Time      : "
                <<tokens[1]
                <<"\n";

                std::cout
                <<"Altitude  : "
                <<tokens[2]
                <<" ft\n";

                std::cout
                <<"Speed     : "
                <<tokens[3]
                <<" fps\n";

                std::cout
                <<"Pitch     : "
                <<tokens[4]
                <<" rad\n";

                std::cout
                <<"Roll      : "
                <<tokens[5]
                <<" rad\n";

                std::cout
                <<"Heading   : "
                <<tokens[6]
                <<" rad\n";
            }

            continue;
        }

        //----------------------------------
        // PYTHON BINARY MODE
        //----------------------------------

        if(len>=136)
        {
            std::cout
            <<"\nDetected : Python Struct\n";

            Parameter* p=
            (Parameter*)
            (buffer+16);

            double altitude=
            p[0].value;

            double speed=
            p[1].value;

            double pitch=
            p[2].value;

            double roll=
            p[3].value;

            double heading=
            p[4].value;

            std::cout
            <<"Altitude : "
            <<altitude
            <<" ft\n";

            std::cout
            <<"Speed    : "
            <<speed
            <<" fps\n";

            std::cout
            <<"Pitch    : "
            <<pitch
            <<" rad\n";

            std::cout
            <<"Roll     : "
            <<roll
            <<" rad\n";

            std::cout
            <<"Heading  : "
            <<heading
            <<" rad\n";
        }
    }

    closesocket(sock);

    WSACleanup();

    return 0;
}