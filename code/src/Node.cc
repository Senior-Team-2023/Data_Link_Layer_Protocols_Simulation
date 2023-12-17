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
#define WS 3
#define TO 10
#define TD 1.0
#define ED 4.0
#define DD 0.1
#define PT 0.5
#define LP 0
#include "Node.h"

Define_Module(Node);

void Node::initialize()
{
    isSender = false;
    // Reading the file
    start = -1;
    end = -1;
    not_processing = true;
    current_index = -1;
    messgs_in_window = 0;

    std::ifstream inputFile;
    current_seq_numb = -1;
    first_seq_numb = -1;
    if (strcmp(this->getName(), "node0") == 0)
    {

        EV << "IN FILE ONE ";
        inputFile.open("../input0.txt");
    }
    else
    {
        EV << "IN FILE TWO ";
        inputFile.open("../input1.txt");
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

        current_index = 0;
        for (int i = 0; i < WS; i++)
        {
            window_messages.push_back(nullptr);
        }
    }

    if (isSender == true && mmsg->getFrame_type() == '1' && !msg->isSelfMessage()) // ack message from reciever
    {

        // if (mmsg->getAck_nack_numb() == -1)
        // {
        //     EV << "TIME " << simTime() << "RECIEVED ACK " << messgs_in_window << endl;
        //     messgs_in_window--;
        //     start++;
        //     start %= WS;
        //     if (start == end)
        //     {
        //         start = -1;
        //         end = -1;
        //     }
        // }
        // else
        EV << "Before ACK RECIEVED " << mmsg->getAck_nack_numb() << "start " << start << endl;

        while (start != mmsg->getAck_nack_numb())

        {
            EV << "In TIME " << simTime() << "RECIEVED ACK " << messgs_in_window << endl;
            start++;
            start %= WS;
            messgs_in_window--;
        }
        EV << "End RECIEVED ACK " << simTime() << " " << messgs_in_window << endl;
    }

    if (mmsg->getFrame_type() == '2' && !msg->isSelfMessage()) // the message is data recieved
    {

        bool acknowledge;
        std::string payload = decode(mmsg->getPayload(), mmsg->getTrailer(), acknowledge);
        MyMessage_Base *ack = new MyMessage_Base();
        if (acknowledge == true)
        {
            ack->setFrame_type('1'); // ack
            ack->setAck_nack_numb((mmsg->getHeader() + 1) % WS);
        }
        else if (acknowledge == false)
        {
            EV << "IN      SHELLLLLLLLLLLLLLLLLLLL" << endl;
            ack->setFrame_type('0'); // nack
            ack->setAck_nack_numb(mmsg->getHeader());
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
            EV << "TIME " << simTime() << "SENT MESG " << mmsg->getPayload() << " Ack " << mmsg->getFrame_type() << " ws " << messgs_in_window << " ack " << mmsg->getAck_nack_numb() << endl;
            send(mmsg, "ino$o");
        }
    }

    if (isSender == true && current_index < lines.size() && not_processing == true && messgs_in_window < WS)
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

        // if (first_seq_numb == -1)
        // first_seq_numb = 0;

        // current_seq_numb++; // not yet
        // current_seq_numb %= WS;

        messgs_in_window++;

        if (start == -1) // lesa bensamy
        {
            start = 0;
            end = 1;
            window_messages.insert(window_messages.begin(), newMesg);
        }
        else // el 3ady
        {

            window_messages.insert(window_messages.begin() + end, newMesg);
            end++;
            end %= WS;
        }

        newMesg->setHeader(end - 1);
        EV << "time to start processing " << simTime() << " WS " << messgs_in_window << " messg " << newMesg->getPayload() << endl;
        current_index++;
        // EV << " sent message " << newMesg->getPayload() << endl;
        not_processing = false;
        scheduleAt(simTime() + PT, newMesg);
    }
}
