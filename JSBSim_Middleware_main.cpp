#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")

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
        tokens.push_back(trim(token));
    }

    return tokens;
}

int main()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2),&wsa);

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
        ==SOCKET_ERROR)
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

    static double prevAltitude=-9999;
    static double prevSpeed=-9999;
    static double prevPitch=-9999;
    static double prevRoll=-9999;
    static double prevHeading=-9999;

    while(true)
    {
        sockaddr_in sender;
        int senderSize=sizeof(sender);

        int len=
        recvfrom(
            sock,
            buffer,
            sizeof(buffer)-1,
            0,
            (sockaddr*)&sender,
            &senderSize);

        if(len<=0)
            continue;

        buffer[len]='\0';

        std::string packet(buffer);

        std::cout
        <<"\nRAW:\n"
        <<packet
        <<"\n";

        if(packet.find("<LABELS>")!=std::string::npos)
        {
            std::cout
            <<"Skipping LABEL packet\n";

            continue;
        }

        std::vector<std::string>
        tokens=
        split(packet,',');

        if(tokens.size()<7)
        {
            std::cout
            <<"Invalid packet\n";

            continue;
        }

        try
        {
            // Correct indexes
            double time=
            std::stod(tokens[1]);

            double altitude=
            std::stod(tokens[2]);

            double speed=
            std::stod(tokens[3]);

            double pitch=
            std::stod(tokens[4]);

            double roll=
            std::stod(tokens[5]);

            double heading=
            std::stod(tokens[6]);

            std::cout
            <<"\n===== RECEIVED JSBSim DATA =====\n";

            std::cout
            <<"Time      : "
            <<time
            <<" sec\n";

            std::cout
            <<"Altitude  : "
            <<altitude
            <<" ft\n";

            std::cout
            <<"Speed     : "
            <<speed
            <<" fps\n";

            std::cout
            <<"Pitch     : "
            <<pitch
            <<" rad\n";

            std::cout
            <<"Roll      : "
            <<roll
            <<" rad\n";

            std::cout
            <<"Heading   : "
            <<heading
            <<" rad\n";

            std::cout
            <<"\nValidation:\n";

            std::cout
            <<"Altitude : "
            <<((altitude>=-1000 && altitude<=50000)
            ?"OK":"INVALID")
            <<"\n";

            std::cout
            <<"Speed : "
            <<((speed>=0 && speed<=2000)
            ?"OK":"INVALID")
            <<"\n";

            std::cout
            <<"Pitch : "
            <<((pitch>=-3.14 && pitch<=3.14)
            ?"OK":"INVALID")
            <<"\n";

            std::cout
            <<"Roll : "
            <<((roll>=-3.14 && roll<=3.14)
            ?"OK":"INVALID")
            <<"\n";

            std::cout
            <<"Heading : "
            <<((heading>=0 && heading<=6.28)
            ?"OK":"INVALID")
            <<"\n";

            if(prevAltitude!=-9999)
            {
                std::cout
                <<"\nPacket Change:\n";

                std::cout
                <<"Altitude Change : "
                <<altitude-prevAltitude
                <<"\n";

                std::cout
                <<"Speed Change : "
                <<speed-prevSpeed
                <<"\n";

                std::cout
                <<"Pitch Change : "
                <<pitch-prevPitch
                <<"\n";

                std::cout
                <<"Roll Change : "
                <<roll-prevRoll
                <<"\n";

                std::cout
                <<"Heading Change : "
                <<heading-prevHeading
                <<"\n";
            }

            prevAltitude=altitude;
            prevSpeed=speed;
            prevPitch=pitch;
            prevRoll=roll;
            prevHeading=heading;
        }
        catch(...)
        {
            std::cout
            <<"Packet conversion error\n";
        }
    }

    closesocket(sock);

    WSACleanup();

    return 0;
}