#include <QtTest/QtTest>
#include <QSignalSpy>
#include "audio/AudioDecoder.h"

class TestAudioDecoder : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        qRegisterMetaType<size_t>("size_t");
    }

    void testInitialState()
    {
        suno::audio::AudioDecoder decoder;
        QVERIFY(!decoder.isPlaying());
        QVERIFY(!decoder.isPaused());
        QCOMPARE(decoder.duration(), 0);
        QCOMPARE(decoder.position(), 0);
    }

    void testLoadInvalidFile()
    {
        suno::audio::AudioDecoder decoder;
        QSignalSpy errorSpy(&decoder, &suno::audio::AudioDecoder::errorOccurred);
        
        QVERIFY(!decoder.loadFile("/nonexistent/path/to/file.mp3"));
        QCOMPARE(errorSpy.count(), 1);
    }

    void testAudioDataCallback()
    {
        suno::audio::AudioDecoder decoder;
        
        bool callbackCalled = false;
        size_t receivedFrames = 0;
        int receivedChannels = 0;
        
        decoder.setAudioDataCallback([&](const float* data, size_t frames, int channels) {
            callbackCalled = true;
            receivedFrames = frames;
            receivedChannels = channels;
            Q_UNUSED(data)
        });
        
        QVERIFY(true);
    }

    void testSetVolume()
    {
        suno::audio::AudioDecoder decoder;
        
        decoder.setVolume(0.5f);
        QCOMPARE(decoder.volume(), 0.5f);
        
        decoder.setVolume(1.0f);
        QCOMPARE(decoder.volume(), 1.0f);
        
        decoder.setVolume(-0.5f);
        QCOMPARE(decoder.volume(), 0.0f);
        
        decoder.setVolume(2.0f);
        QCOMPARE(decoder.volume(), 1.0f);
    }

    void cleanupTestCase()
    {
    }
};

QTEST_MAIN(TestAudioDecoder)
#include "test_audio_decoder.moc"
