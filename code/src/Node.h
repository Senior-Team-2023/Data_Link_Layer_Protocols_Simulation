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

#ifndef __NETWORKS_PROJECT_NODE_H_
#define __NETWORKS_PROJECT_NODE_H_

#include <omnetpp.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <bitset>
#include <climits>
#include "myMessage_m.h"
using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
protected:
  std::vector<std::string> lines;

  int first_seq_numb;
  int messgs_in_window;
  int current_seq_numb;

  std::vector<MyMessage_Base *> window_messages;
  std::vector<simtime_t> time_outs;
  int start;
  int end;
  int reciever_expected_seq_numb;
  bool no_error_messages;
  bool isSender;
  bool not_processing;

  int current_index;
  virtual void initialize();
  virtual void handleMessage(cMessage *msg);
};

#endif
