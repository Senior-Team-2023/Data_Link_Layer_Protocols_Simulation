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

//
// TODO generated message class
//
packet MyMessage {
    @customize(true);  // see the generated C++ header for more info
   
    
    int header;
    string payload;
    char trailer;//parity
    char frame_type;//type 0 1 2 3 
    char ack_nack_numb;
// Header: the data sequence number.
// Payload: the message contents after byte stuffing (in characters).
// Trailer: the parity byte. 
// Frame type: Data=2/ ACK=1 /NACK=0.
// ACK/NACK number.

}
