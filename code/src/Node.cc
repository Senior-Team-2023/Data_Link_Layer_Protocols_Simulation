//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "Node.h"

#define ws 3
#define TO 10
#define TD 1.0
#define ED 4.0
#define DD 0.1
#define PT 0.5
#define LP 0
Define_Module(Node);

void Node::initialize()
{

    isSender = false;
    // Reading the file
    not_processing = true;
    current_index = 0;
    std::ifstream inputFile;

    if (strcmp(this->getName(), "node0") == 0)
    {

        EV << "IN FILE ONE ";
        inputFile.open("E:/fourth year/projects/networks/Data_Link_Layer_Protocols_Simulation/code/input0.txt");
    }
    else
    {
        EV << "IN FILE TWO ";
        inputFile.open("E:/fourth year/projects/networks/Data_Link_Layer_Protocols_Simulation/code/input1.txt");
    }
    // Check if the file is open
    if (!inputFile.is_open())
    {
        std::cerr << "Error opening the file." << std::endl;
    }

    // Read the file line by line
    std::string line;
    while (std::getline(inputFile, line))
    {
        // EV << endl
        //    << "IN LOOP" << endl;
        lines.push_back(line);
    }

    if (strcmp(this->getName(), "node0") == 0)
    {

        for (int i = 0; i < lines.size(); i++)
        {
            EV << " file lines " << lines[i] << endl;
        }
    }
    // Close the file
    inputFile.close();
}

std::string encode(std::string payload, char &checksum)
{
    std::string realFrame = "$";
    for (int i = 0; i < payload.length(); i++)
    {
        if (payload[i] == '/' || payload[i] == '$')
        {
            realFrame += "/";
        }
        realFrame += payload[i];
    }
    realFrame += "$";

    checksum = 0;
    for (char ch : realFrame)
    {
        checksum ^= ch;
    }
    checksum = ~checksum;
    // EV << "REAL FRAME " << realFrame << endl;
    return realFrame;
}

std::string decode(std::string frame, char checksum, bool &acknowledge)
{
    std::string realPayload = "";

    char realchecksum = 0;
    for (char ch : frame)
    {
        realchecksum ^= ch;
    }
    realchecksum ^= checksum;

    for (int i = 1; i < frame.length() - 1; i++)
    {
        if (frame[i] == '/')
        {
            i++;
        }
        realPayload += frame[i];
    }
    // char checksum = frame[frame.length() - 1];

    std::bitset<CHAR_BIT> binaryRepresentation(realchecksum);

    std::bitset<CHAR_BIT> comparisonBitset("11111111");
    if (binaryRepresentation == comparisonBitset) // okay send ack
        acknowledge = true;
    else
        acknowledge = false;
    // EV << "CHECKSUM " << binaryRepresentation << endl;
    // EV << "REAL PAYLOAD " << realPayload << endl;
    return realPayload;
}

void Node::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    MyMessage_Base *mmsg = check_and_cast<MyMessage_Base *>(msg);

    if (mmsg->getFrame_type() == '3') // the message is for start sending from coordinator
    {
        isSender = true;
    }

    if (mmsg->getFrame_type() == '2' && !msg->isSelfMessage()) // the message is data recieved
    {
        bool acknowledge;
        std::string payload = decode(mmsg->getPayload(), mmsg->getTrailer(), acknowledge);
        MyMessage_Base *ack = new MyMessage_Base();
        if (acknowledge == true)
        {
            ack->setFrame_type('1'); // ack
            acl->setAck_nack_numb('');
        }
        else if (acknowledge == false)
        {
            ack->setFrame_type('0'); // nack
            acl->setAck_nack_numb('');
        }
        // send(ack, "ino$o");

        ack->setPayload(payload.c_str());
        // EV << "RECIEVED MESG before scheduling the ack  " << payload << "   MSG ACK " << ack->getFrame_type() << endl;
        scheduleAt(simTime() + PT + TD, ack);
    }

    if (msg->isSelfMessage()) // just forwarding after a delay
    {

        if (strcmp(msg->getName(), "processing_done") == 0) // to separate last loop
        {
            not_processing = true;
            msg->setName("");
            scheduleAt(simTime() + TD, mmsg);
        }
        else
        {
            EV << "TIME " << simTime() << "SENT MESG " << mmsg->getPayload() << " Ack " << mmsg->getFrame_type() << endl;
            send(mmsg, "ino$o");
        }
    }

    if (isSender == true && current_index < lines.size() && not_processing == true && current_index - earliest_unack_index < ws)
    {

        std::istringstream iss(lines[current_index]);
        std::string errors, payload;
        iss >> errors;
        payload = lines[current_index].substr(5, lines[current_index].length());
        char checksum;
        std::string realFrame = encode(payload, checksum);
        MyMessage_Base *newMesg = new MyMessage_Base();
        newMesg->setPayload(realFrame.c_str());
        newMesg->setTrailer(checksum);
        newMesg->setFrame_type('2');
        newMesg->setName("processing_done");

        //         int ptValue = par("PT").intValue();
        //         EV << " PT " << simTime() + ptValue << endl;

        EV << "time to start processing " << simTime() << endl;
        current_index++;
        // EV << " sent message " << newMesg->getPayload() << endl;
        not_processing = false;
        scheduleAt(simTime() + PT, newMesg);
    }
}
