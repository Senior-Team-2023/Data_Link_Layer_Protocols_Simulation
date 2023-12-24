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

        EV << "msg name " << msg->getName() << " !!!!!!!!!!!!\n";
        // !!!!!!!!!!!!!!!!!handle other processing done for errors!!!!!!!!!!!!!!!
        // processing done so we need to schedule it  and schedule the timeout
        if (strcmp(msg->getName(), "processing_done") == 0) // to separate last loop
        {
            not_processing = true;
            msg->setName("");
            scheduleAt(simTime() + TD, mmsg);

            EV << " IN FALLING THUNDRE " << endl;
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
        else if (strcmp(msg->getName(), "processing_done_delay") == 0) // to separate last loop
        {
            not_processing = true;
            msg->setName("");
            scheduleAt(simTime() + TD + ED, mmsg);
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
        else if (strcmp(msg->getName(), "processing_done_dup") == 0) // to separate last loop
        {
            not_processing = true;
            msg->setName("");
            MyMessage_Base *dup_msg = new MyMessage_Base();
            dup_msg->setPayload(mmsg->getPayload());
            dup_msg->setTrailer(mmsg->getTrailer());
            dup_msg->setFrame_type(mmsg->getFrame_type());
            dup_msg->setHeader(mmsg->getHeader());
            dup_msg->setName("");
            scheduleAt(simTime() + TD, mmsg);
            scheduleAt(simTime() + TD + DD, dup_msg);
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
        else if (strcmp(msg->getName(), "processing_done_dup_delay") == 0) // to separate last loop
        {
            not_processing = true;
            msg->setName("");
            MyMessage_Base *dup_msg = new MyMessage_Base();
            dup_msg->setPayload(mmsg->getPayload());
            dup_msg->setTrailer(mmsg->getTrailer());
            dup_msg->setFrame_type(mmsg->getFrame_type());
            dup_msg->setHeader(mmsg->getHeader());
            dup_msg->setName("");
            scheduleAt(simTime() + TD + ED, mmsg);
            scheduleAt(simTime() + TD + DD + ED, dup_msg);
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
        else if (strcmp(msg->getName(), "processing_done_loss") == 0) // to separate last loop
        {
            not_processing = true;
            // msg->setName("");
            // MyMessage_Base *dup_msg = new MyMessage_Base();
            // dup_msg->setPayload(mmsg->getPayload());
            // dup_msg->setTrailer(mmsg->getTrailer());
            // dup_msg->setFrame_type(mmsg->getFrame_type());
            // dup_msg->setHeader(mmsg->getHeader());
            // dup_msg->setName("");
            // scheduleAt(simTime() + TD + ED, mmsg);
            // scheduleAt(simTime() + TD + DD + ED, dup_msg);
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

            // if ack or nack with LP
            if (mmsg->getFrame_type() == '1' || mmsg->getFrame_type() == '0')
            {
                // it's  not a loss in ackk or nack
                if (mmsg->getHeader() == -2)
                {
                    send(mmsg, "ino$o");
                }
                else
                {
                    EV << " LOSSS IN before send directly " << mmsg->getAck_nack_numb();
                }
            }
            // A nOrmal MEssage
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
        not_processing = false;

        // no errors

        char mod = errors[0];
        char loss = errors[1];
        char dup = errors[2];
        char delay = errors[3];

        EV << "Mod " << mod << " Loss " << loss << " DUP " << dup << " Delay " << delay << endl;

        //    no errors
        if (mod == '0' && loss == '0' && dup == '0' && delay == '0')
        {
            newMesg->setName("processing_done");
            scheduleAt(simTime() + PT, newMesg);
        }

        // if modification and no other errors so ensd it else just modify and continue
        if (mod == '1' && loss == '0')
        {

            newMesg->setName("processing_done");
            int count = realFrame.length();
            int random_index = int(uniform(0, count));
            int random_shift = int(uniform(0, 7));

            // for (int i = 7; i >= 0; --i)
            // {
            //     EV << ((realFrame[random_index] >> i) & 1);
            // }
            // EV << "PRE SHIFT " << realFrame[random_index] << endl;
            realFrame[random_index] ^= 1 << random_shift;
            // EV << " REAL FRAME " << realFrame << " random index " << random_index << " random shift  " << random_shift << endl;
            // for (int i = 7; i >= 0; --i)
            // {
            //     EV << ((realFrame[random_index] >> i) & 1);
            // }

            newMesg->setPayload(realFrame.c_str());
            EV << " Before SHIT !!!!!!!!!!!" << endl;
            // no other errors so just send it
            if (dup == '0' && delay == '0')
            {
                EV << " IN SHIT !!!!!!!!!!!" << endl;
                scheduleAt(simTime() + PT, newMesg);
            }
        }

        // dup but no delay
        if (dup == '1' && delay == '0' && loss == '0')
        {
            newMesg->setName("processing_done_dup");

            scheduleAt(simTime() + PT, newMesg);
        }
        // delay but no dup
        if (delay == '1' && dup == '0' && loss == '0')
        {
            newMesg->setName("processing_done_delay");
            scheduleAt(simTime() + PT, newMesg);
        }

        // dup and delay
        if (delay == '1' && dup == '1' && loss == '0')
        {
            newMesg->setName("processing_done_dup_delay");
            scheduleAt(simTime() + PT, newMesg);
        }
        if (loss == '1')
        {
            newMesg->setName("processing_done_loss");
            scheduleAt(simTime() + PT, newMesg);
        }
        // if (first_seq_numb == -1)
        // first_seq_numb = 0;

        // current_seq_numb++; // not yet
        // current_seq_numb %= WS;
    }
}
