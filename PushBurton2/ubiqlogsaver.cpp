/*
 * Copyright (c) 2011 Nokia Corporation
 *
 * This file is part of the Push Snowboarding Project, More info at:
 * www.pushsnowboading.com
 *
 * Author: Clovis Scotti <scotti@ieee.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#include "ubiqlogsaver.h"

#include "LogSender.h"

namespace {
    const int TICK_BUFFER_SIZE = 100;
}

UbiqLogSaver::UbiqLogSaver(uint a_start_time)
{
    start_time = a_start_time;
    CreateRunDir();

    QString rawLogFilename = (dirName + FSC_FSYS_SLASH) + "RawLog.xml";

    qDebug() << "RawLog Filename: " << rawLogFilename;

    dataFile = new QFile(rawLogFilename);

    dataFile->open(QFile::WriteOnly | QFile::Text);
    xml.setDevice(dataFile);
    xml.setAutoFormatting(true);

    xml.writeStartDocument();
    xml.writeStartElement("N8SensorsLog");

    // The buffer to be sent online
    this->bufferFile = new QBuffer();
    this->bufferFile->open(QBuffer::ReadWrite);

    this->bufferXml.setDevice(this->bufferFile);
    this->bufferXml.setAutoFormatting(true);

    this->tickBufferCount = 0;

    this->sender = new LogSender();

    runEnd = false;
    this->start();
}

UbiqLogSaver::~UbiqLogSaver()
{
    this->run_end();//will return immediately if everything went ok since runEnd will be true
}

void UbiqLogSaver::run()
{
    while(true)
    {
        bool leaving = false;
        runControlLock.lock();
        if(!runEnd)
            runControlSignal.wait(&runControlLock);
        else
            leaving = true;
        runControlLock.unlock();


        ticksLock.lock();
        while(ticksBuffer.count() > 0){
            NPushLogTick * tick = ticksBuffer.dequeue();
            ticksLock.unlock();
            tick->dump_to_xml(xml);
            dataFile->flush();

            tick->dump_to_xml(this->bufferXml);
            this->tickBufferCount++;
            if(this->tickBufferCount == TICK_BUFFER_SIZE) {
                this->sendResults();
                this->tickBufferCount = 0;
            }

            ticksLock.lock();
        }
        ticksLock.unlock();

        reportsLock.lock();
        while(reportBuffer.count() > 0){
            qDebug() << "report being dumped";
            NPushGenReport * repr = reportBuffer.dequeue();

            reportsLock.unlock();
            repr->save_to_dir(dirName);
            reportsLock.lock();
        }
        reportsLock.unlock();


        if(leaving){
            break;
        }
    }
    exit(0);
}

void UbiqLogSaver::run_end()
{
    if(runEnd)
        return;
    qDebug() << "before signaling exit";
    runControlLock.lock();
    runEnd = true;
    runControlLock.unlock();
    qDebug() << "waking up saver";
    runControlSignal.wakeAll();
    this->wait();//Adding a timeout here? not!?
    qDebug() << "saver exited";
    xml.writeEndElement();//N8SensorsLog
    xml.writeEndDocument();
    dataFile->close();
    this->sendResults();
    this->bufferFile->close();
    this->tickBufferCount = 0;
    qDebug() << "File closed";
}

void UbiqLogSaver::CreateRunDir()
{
    dirName = QString(FSC_RUNS_FOLDERS_ROOT)+QString(FSC_RUNS_PREFIX);

    dirName += QString::number(start_time);
    dirName += QDateTime::fromTime_t(start_time).toString("_hhmm");
    if(!QDir().mkpath(dirName))
        qDebug() << "Could not create run directory";
}

void UbiqLogSaver::sendResults()
{
    qDebug() << "Ticks in buffer: " << this->tickBufferCount;
    qDebug() << "Size: " << this->bufferFile->size();

    this->sender->setTextToSend(QString(this->bufferFile->buffer()));
    this->sender->start();

    this->bufferFile->close();
    this->bufferFile->setBuffer(0);
    this->bufferFile->open(QBuffer::ReadWrite);

    qDebug() << "Buffer after close/open: " << QString(this->bufferFile->buffer());
    qDebug() << "Size: " << this->bufferFile->size();
}

void UbiqLogSaver::log_tick_in(NPushLogTick* tick)
{
    ticksLock.lock();
    ticksBuffer.enqueue(tick);
    ticksLock.unlock();
    runControlSignal.wakeAll();
}

void UbiqLogSaver::report_in(NPushGenReport* rep)
{
    reportsLock.lock();
    reportBuffer.enqueue(rep);
    reportsLock.unlock();
    runControlSignal.wakeAll();
}
