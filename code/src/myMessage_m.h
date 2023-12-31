//
// Generated file, do not edit! Created by nedtool 5.6 from myMessage.msg.
//

#ifndef __MYMESSAGE_M_H
#define __MYMESSAGE_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0506
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



/**
 * Class generated from <tt>myMessage.msg:19</tt> by nedtool.
 * <pre>
 * //
 * // TODO generated message class
 * //
 * packet MyMessage
 * {
 *     \@customize(true);  // see the generated C++ header for more info
 * 
 * 
 *     int header;
 *     string payload;
 *     char trailer;//parity
 *     char frame_type;//type 0 1 2 3 
 *     int ack_nack_numb;
 * // Header: the data sequence number.
 * // Payload: the message contents after byte stuffing (in characters).
 * // Trailer: the parity byte. 
 * // Frame type: Data=2/ ACK=1 /NACK=0.
 * // ACK/NACK number.
 * 
 * }
 * </pre>
 *
 * MyMessage_Base is only useful if it gets subclassed, and MyMessage is derived from it.
 * The minimum code to be written for MyMessage is the following:
 *
 * <pre>
 * class MyMessage : public MyMessage_Base
 * {
 *   private:
 *     void copy(const MyMessage& other) { ... }

 *   public:
 *     MyMessage(const char *name=nullptr, short kind=0) : MyMessage_Base(name,kind) {}
 *     MyMessage(const MyMessage& other) : MyMessage_Base(other) {copy(other);}
 *     MyMessage& operator=(const MyMessage& other) {if (this==&other) return *this; MyMessage_Base::operator=(other); copy(other); return *this;}
 *     virtual MyMessage *dup() const override {return new MyMessage(*this);}
 *     // ADD CODE HERE to redefine and implement pure virtual functions from MyMessage_Base
 * };
 * </pre>
 *
 * The following should go into a .cc (.cpp) file:
 *
 * <pre>
 * Register_Class(MyMessage)
 * </pre>
 */
class MyMessage_Base : public ::omnetpp::cPacket
{
  protected:
    int header;
    ::omnetpp::opp_string payload;
    char trailer;
    char frame_type;
    int ack_nack_numb;

  private:
    void copy(const MyMessage_Base& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const MyMessage_Base&);
    // make constructors protected to avoid instantiation

    // make assignment operator protected to force the user override it
    MyMessage_Base& operator=(const MyMessage_Base& other);

  public:
    MyMessage_Base(const char *name=nullptr, short kind=0);
      MyMessage_Base(const MyMessage_Base& other);
    virtual ~MyMessage_Base();
    virtual MyMessage_Base *dup() const override {  return new MyMessage_Base(*this); }
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual int getHeader() const;
    virtual void setHeader(int header);
    virtual const char * getPayload() const;
    virtual void setPayload(const char * payload);
    virtual char getTrailer() const;
    virtual void setTrailer(char trailer);
    virtual char getFrame_type() const;
    virtual void setFrame_type(char frame_type);
    virtual int getAck_nack_numb() const;
    virtual void setAck_nack_numb(int ack_nack_numb);
};


#endif // ifndef __MYMESSAGE_M_H

