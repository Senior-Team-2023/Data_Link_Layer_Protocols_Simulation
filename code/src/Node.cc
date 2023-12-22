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
#define LP 0.8
#include "Node.h"

Define_Module(Node);

void Node::initialize()
{
    isSender = false;
    // Reading the file
    no_error_messages = 0;
    start = -1;
    end = -1;
    reciever_expected_seq_numb = 0;
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

// FOR ACK NACK
// if set header = -1 so it is loss
// if set header =-2 so it is not loss
void Node::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    MyMessage_Base *mmsg = check_and_cast<MyMessage_Base *>(msg);

    // at sneder start from coordinator
    if (mmsg->getFrame_type() == '3') // the message is for start sending from coordinator
    {
        isSender = true;

        current_index = 0;
        for (int i = 0; i < WS + 1; i++)
        {
            window_messages.push_back(nullptr);
            time_outs.push_back(-1);
        }
    }

    //   at sendet nack found
    if (isSender == true && mmsg->getFrame_type() == '0' && !msg->isSelfMessage()) // nack message from reciever
    {

        while (start != mmsg->getAck_nack_numb())

        {
            EV << "In TIME " << simTime() << "RECIEVED NACK " << endl;
            start++;
            start %= (WS + 1);
            // we are gonna consider all before nacks as acks
            time_outs.insert(time_outs.begin() + start, -1);
        }

        EV << "NACK!!!!! " << mmsg->getAck_nack_numb() << "start " << start << endl;
        // get the count of elemnts before the acks to move the index in file
        int fake_start = mmsg->getAck_nack_numb();

        int counter = 0;
        while (fake_start != end)

        {
            counter++;

            EV << "In TIME " << simTime() << "RECIEVED NACK " << messgs_in_window << endl;
            fake_start++;
            fake_start %= (WS + 1);
        }
        current_index -= counter;

        start = mmsg->getAck_nack_numb();

        end = start;

        no_error_messages += 1;
        EV << "NACK!!!!! " << simTime() << "   " << mmsg->getAck_nack_numb() << "start " << start << endl;

        messgs_in_window = 0;
    }

    // at sender ack found
    if (isSender == true && mmsg->getFrame_type() == '1' && !msg->isSelfMessage()) // ack message from reciever
    {

        EV << "Before ACK RECIEVED " << mmsg->getAck_nack_numb() << "start " << start << " " << end << endl;

        // if(start == mmsg->getAck_nack_numb())
        while (start != mmsg->getAck_nack_numb())

        {
            EV << "In TIME " << simTime() << "RECIEVED ACK " << messgs_in_window << endl;
            start++;
            start %= (WS + 1);
            messgs_in_window--;
            // we are gonna consider all before ack as acked (accumulative acks)
            time_outs.insert(time_outs.begin() + start, -1);
        }

        EV << "End RECIEVED ACK " << simTime() << " " << messgs_in_window << endl;
    }

    // AT reciever data found and sending ack and nack
    if (mmsg->getFrame_type() == '2' && !msg->isSelfMessage()) // the message is data recieved
    {

        bool acknowledge;
        std::string payload = decode(mmsg->getPayload(), mmsg->getTrailer(), acknowledge);
        MyMessage_Base *ack = new MyMessage_Base();
        EV << ack->getAck_nack_numb() << " IN   ACKKKKKKKKKKKKK " << mmsg->getPayload() << "  " << simTime() << " expec " << reciever_expected_seq_numb << " header " << mmsg->getHeader() << endl;
        if (acknowledge == true && reciever_expected_seq_numb == mmsg->getHeader())
        {
            ack->setFrame_type('1'); // ack
            reciever_expected_seq_numb += 1;
            reciever_expected_seq_numb %= (WS + 1);
            ack->setAck_nack_numb((mmsg->getHeader() + 1) % (WS + 1));
            ack->setPayload(payload.c_str());
            ack->setName("processing_done");
            float er = uniform(0, 1);
            EV << " ERROR " << er << " " << LP << endl;
            if (er <= LP)
            {
                EV << "HERE IS A LOSS " << er << " msg seq " << ack->getAck_nack_numb() << endl;
                // so it's a loss
                ack->setHeader(-1);
            }
            else
            {
                EV << "No LOSS " << er << " msg seq " << ack->getAck_nack_numb() << endl;
                // not loss
                ack->setHeader(-2);
            }
            // EV << "RECIEVED MESG before scheduling the ack  " << payload << "   MSG ACK " << ack->getFrame_type() << endl;
            scheduleAt(simTime() + PT, ack);
            // cancelAndDelete(mmsg);
            cancelAndDelete(msg);
        }
        else if (acknowledge == false || reciever_expected_seq_numb == mmsg->getHeader())
        {
            ack->setFrame_type('0'); // nack
            ack->setAck_nack_numb(reciever_expected_seq_numb);
            EV << ack->getAck_nack_numb() << " IN      HELLLLLLLLLLLLLLLLLLLL " << mmsg->getPayload() << "  " << simTime() << " expec " << reciever_expected_seq_numb << " header " << mmsg->getHeader() << endl;

            ack->setPayload(payload.c_str());
            ack->setName("processing_done");
            float er = uniform(0, 1);
            EV << " ERROR " << er << " " << LP << endl;
            if (er <= LP)
            {
                EV << "HERE IS A LOSS " << er << " msg seq " << ack->getAck_nack_numb() << endl;
                // so it's a loss
                ack->setHeader(-1);
            }
            else
            {
                EV << "No LOSS " << er << " msg seq " << ack->getAck_nack_numb() << endl;
                // not loss
                ack->setHeader(-2);
            }
            // EV << "RECIEVED MESG before scheduling the ack  " << payload << "   MSG ACK " << ack->getFrame_type() << endl;
            scheduleAt(simTime() + PT, ack);
            // cancelAndDelete(mmsg);
            cancelAndDelete(msg);
        }
        else if (reciever_expected_seq_numb != mmsg->getHeader())
        {
            ack->setFrame_type('1'); // ack

            ack->setAck_nack_numb(reciever_expected_seq_numb);
            ack->setPayload(payload.c_str());
            ack->setName("processing_done");
            ack->setHeader(-2);
            scheduleAt(simTime() + PT, ack);
        }
        // send(ack, "ino$o");
    }

    // at both forwarding either wit td or not
    if (msg->isSelfMessage()) // just forwarding after a delay
    {

        // processing done so we need to schedule it  and schedule the timeout
        if (strcmp(msg->getName(), "processing_done") == 0) // to separate last loop
        {
            not_processing = true;
            msg->setName("");
            scheduleAt(simTime() + TD, mmsg);
            // an alarm to wake up after the TO delay
            if (mmsg->getFrame_type() == '2')
            {
                MyMessage_Base *wake_up = new MyMessage_Base();
                wake_up->setName("timeout");
                wake_up->setHeader(mmsg->getHeader());
                simtime_t current_time = simTime();
                scheduleAt(current_time + TO, wake_up);

                time_outs.insert(time_outs.begin() + mmsg->getHeader(), current_time + TO);
            }
        }
        // it's after a timeout
        else if (strcmp(msg->getName(), "timeout") == 0 && isSender == true)
        {

            if (!(time_outs[mmsg->getHeader()] == -1 || time_outs[mmsg->getHeader()] > simTime()))
            {
                EV << "TIMEOUT!!!!! " << mmsg->getHeader() << "start " << start << endl;
                int fake_start = mmsg->getHeader();
                int counter = 0;
                while (fake_start != end)

                {
                    counter++;

                    EV << "In TIME " << simTime() << "RECIEVED NACK " << messgs_in_window << endl;
                    fake_start++;
                    fake_start %= (WS + 1);
                    messgs_in_window--;
                    time_outs.insert(time_outs.begin() + fake_start, -1);
                }
                current_index -= counter;

                // start = mmsg->getHeader();

                // end = start;
                end = mmsg->getHeader();

                no_error_messages += 1;
                EV << "TIMEOUT!!!!! " << simTime() << "   " << mmsg->getHeader() << "start " << start << endl;
            }
        }

        else
        {
            EV << "OHHHHHHHHHH" << endl;
            EV << "TIME " << simTime() << "SENT MESG " << mmsg->getPayload() << " Ack " << mmsg->getFrame_type() << " ws " << messgs_in_window << " ack " << mmsg->getAck_nack_numb() << endl;

            // if ack or nack
            if (mmsg->getFrame_type() == '1' || mmsg->getFrame_type() == '0')
            {
                // it's  not a loss
                if (mmsg->getHeader() == -2)
                {
                    send(mmsg, "ino$o");
                }
                else
                {
                    EV << " LOSSS IN before send directly " << mmsg->getAck_nack_numb();
                }
            }
            else
            {
                send(mmsg, "ino$o");
            }
        }
    }

    // at sender sending data
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
            newMesg->setHeader(end - 1);
            window_messages.insert(window_messages.begin(), newMesg);
        }
        else // el 3ady
        {

            window_messages.insert(window_messages.begin() + end, newMesg);
            newMesg->setHeader(end);
            end++;

            end %= (WS + 1);
        }

        EV << "time to start processing " << simTime() << " WS " << messgs_in_window << " messg " << newMesg->getPayload() << endl;
        current_index++;
        // EV << " sent message " << newMesg->getPayload() << endl;
        not_processing = false;
        scheduleAt(simTime() + PT, newMesg);
    }
}
