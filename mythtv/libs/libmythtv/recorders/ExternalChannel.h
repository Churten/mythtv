/// -*- Mode: c++ -*-

#ifndef _EXTERNAL_CHANNEL_H_
#define _EXTERNAL_CHANNEL_H_

#include <cstdint>
#include <utility>
#include <vector>
using namespace std;

// Qt headers
#include <QString>

// MythTV headers
#include "dtvchannel.h"
#include "ExternalStreamHandler.h"

class ExternalChannel : public DTVChannel
{
  public:
    ExternalChannel(TVRec *parent, QString  device)
        : DTVChannel(parent), m_device(std::move(device)),
          m_loc(ExternalChannel::GetDevice()) {}
    ~ExternalChannel(void) override;

    // Commands
    bool Open(void) override; // ChannelBase
    void Close(void) override; // ChannelBase

    // ATSC/DVB scanning/tuning stuff
    using DTVChannel::Tune;
    bool Tune(const DTVMultiplex &/*tuning*/) override // DTVChannel
        { return true; }
    bool Tune(const QString &channum) override; // DTVChannel
    bool Tune(const QString &freqid, int /*finetune*/) override; // DTVChannel

    bool EnterPowerSavingMode(void) override; // DTVChannel

    // Gets
    bool IsOpen(void) const override // ChannelBase
        { return m_streamHandler; }
    QString GetDevice(void) const override // ChannelBase
        { return m_device; }
    bool IsPIDTuningSupported(void) const override // DTVChannel
        { return true; }

    QString UpdateDescription(void);
    QString GetDescription(void);
    bool IsBackgroundTuning(void) const { return m_backgroundTuning; }
    uint GetTuneStatus(void);

  protected:
    bool IsExternalChannelChangeSupported(void) override // ChannelBase
        { return true; }

  private:
    int                      m_tuneTimeout { -1 };
    bool                     m_backgroundTuning {false};
    QString                  m_device;
    QStringList              m_args;
    ExternalStreamHandler   *m_streamHandler {nullptr};
    QString                  m_loc;
};

#endif // _EXTERNAL_CHANNEL_H_
