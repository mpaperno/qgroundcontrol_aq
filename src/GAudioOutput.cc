/*=====================================================================

QGroundControl Open Source Ground Control Station

(c) 2009, 2010 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>

This file is part of the QGROUNDCONTROL project

    QGROUNDCONTROL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QGROUNDCONTROL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QGROUNDCONTROL. If not, see <http://www.gnu.org/licenses/>.

======================================================================*/

/**
 * @file
 *   @brief Implementation of audio output
 *
 *   @author Lorenz Meier <mavteam@student.ethz.ch>
 *
 */

#include <QApplication>
#include <QSettings>
#include <QTemporaryFile>
#include <QSound>
#include "GAudioOutput.h"
#include "MG.h"

#include <QDebug>


/**
 * This class follows the singleton design pattern
 * @see http://en.wikipedia.org/wiki/Singleton_pattern
 * A call to this function thus returns the only instance of this object
 * the call can occur at any place in the code, no reference to the
 * GAudioOutput object has to be passed.
 */
GAudioOutput* GAudioOutput::instance()
{
    static GAudioOutput* _instance = 0;
    if(_instance == 0)
    {
        _instance = new GAudioOutput();
        // Set the application as parent to ensure that this object
        // will be destroyed when the main application exits
        _instance->setParent(qApp);
    }
    return _instance;
}

#define QGC_GAUDIOOUTPUT_KEY QString("QGC_AUDIOOUTPUT_")

GAudioOutput::GAudioOutput(QObject* parent) : QObject(parent),
    voiceIndex(0),
    emergency(false),
    muted(false),
    isSpeaking(false)
{
    // Load settings
    QSettings settings;
    settings.sync();
    muted = settings.value(QGC_GAUDIOOUTPUT_KEY+"muted", muted).toBool();

#ifndef NO_TEXT_TO_SPEECH
    speech = new QtSpeech(this);
    connect(speech, SIGNAL(finished()), this, SLOT(advanceSpeechQueue()));
#endif

    // Prepare regular emergency signal, will be fired off on calling startEmergency()
    emergencyTimer = new QTimer();
    connect(emergencyTimer, SIGNAL(timeout()), this, SLOT(beep()));

//    switch (voiceIndex) {
//    case 0:
//        selectFemaleVoice();
//        break;
//    default:
//        selectMaleVoice();
//        break;
//    }
}

//GAudioOutput::~GAudioOutput(){}

void GAudioOutput::mute(bool mute)
{
    if (mute != muted)
    {
        this->muted = mute;
        QSettings settings;
        settings.setValue(QGC_GAUDIOOUTPUT_KEY+"muted", this->muted);
        settings.sync();
        emit mutedChanged(muted);
    }
}

bool GAudioOutput::isMuted()
{
    return this->muted;
}

void GAudioOutput::advanceSpeechQueue() {
    isSpeaking = false;
    if (!voiceQueue.isEmpty())
        sayText(voiceQueue.dequeue());
}

void GAudioOutput::sayText(QString text)
{
    if (muted)
        return;

#ifndef NO_TEXT_TO_SPEECH
    speech->tell(text);
    isSpeaking = true;
#endif
}

bool GAudioOutput::say(QString text, int severity)
{
    // TODO Add severity filter
    Q_UNUSED(severity);

    if (muted)
        return false;

    voiceQueue.enqueue(text);
    if (!isSpeaking)
        advanceSpeechQueue();

    return true;
}

/**
 * @param text This message will be played after the alert beep
 */
bool GAudioOutput::alert(QString text)
{
    // Play alert sound
    beep();
    // Say alert message
    return say(text, 2);
}

void GAudioOutput::notifyPositive()
{
    if (!muted)
        QSound::play("files/audio/notify_positive.wav");
}

void GAudioOutput::notifyNegative()
{
    if (!muted)
        QSound::play("files/audio/notify_negative.wav");
}

void GAudioOutput::beep()
{
    if (!muted)
        QSound::play("files/audio/alert.wav");
}

/**
 * The emergency sound will be played continously during the emergency.
 * call stopEmergency() to disable it again. No speech synthesis or other
 * audio output is available during the emergency.
 *
 * @return true if the emergency could be started, false else
 */
bool GAudioOutput::startEmergency()
{
    if (muted)
        return false;
    if (!emergency)
    {
        emergency = true;
        // Beep immediately and then start timer
        beep();
        emergencyTimer->start(3000);
        // FIXME - cancel emergency after a certain time (in case nothing else does?)
        QTimer::singleShot(15000, this, SLOT(stopEmergency()));
    }
    return true;
}

/**
 * Stops the continous emergency sound. Use startEmergency() to start
 * the emergency sound.
 *
 * @return true if the emergency could be stopped, false else
 */
bool GAudioOutput::stopEmergency()
{
    if (emergency) {
        emergency = false;
        emergencyTimer->stop();
    }
    return true;
}

void GAudioOutput::selectFemaleVoice()
{
    // TODO
}

void GAudioOutput::selectMaleVoice()
{
    // TODO
}

/*
void GAudioOutput::selectNeutralVoice()
{
    // TODO
}*/

QStringList GAudioOutput::listVoices(void)
{
    QStringList l;
    // TODO
    return l;
}
