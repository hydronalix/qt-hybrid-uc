/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "settingsdialog.h"

#include <QLabel>
#include <QMessageBox>
#include <QString>

//! [0]
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_status(new QLabel),
    m_eValue(new QLabel),
    m_sValue(new QLabel),
    m_SpwmValue(new QLabel),
    m_EpwmValue(new QLabel),
    m_ErpmValue(new QLabel),
    m_EstaValue(new QLabel),
    m_staValue(new QLabel),
    m_ch3Value(new QLabel),
    m_RPMValue(new QLabel),
    m_CANValue(new QLabel),
    m_console(new Console),
    m_settings(new SettingsDialog),
//! [1]
    m_serial(new QSerialPort(this))
//! [1]
{
//! [0]
    m_ui->setupUi(this);

    m_console->setEnabled(false);
    setCentralWidget(m_console);

    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionQuit->setEnabled(true);
    m_ui->actionConfigure->setEnabled(true);

    m_ui->statusBar->addWidget(m_status);

    m_ui->eArea->addWidget(m_eValue);
    m_ui->sArea->addWidget(m_sValue);
    m_ui->SpwmArea->addWidget(m_SpwmValue);
    m_ui->EpwmArea->addWidget(m_EpwmValue);
    m_ui->ErpmArea->addWidget(m_ErpmValue);
    m_ui->EstaArea->addWidget(m_EstaValue);
    m_ui->staArea->addWidget(m_staValue);
    m_ui->ch3Area->addWidget(m_ch3Value);
    m_ui->RPMArea->addWidget(m_RPMValue);
    m_ui->CANArea->addWidget(m_CANValue);

    initActionsConnections();

    connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);

//! [2]
    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);
//! [2]
    connect(m_console, &Console::getData, this, &MainWindow::writeData);
//! [3]
}
//! [3]

MainWindow::~MainWindow()
{
    delete m_settings;
    delete m_ui;
}

//! [4]
void MainWindow::openSerialPort()
{
    const SettingsDialog::Settings p = m_settings->settings();
    m_serial->setPortName(p.name);
    m_serial->setBaudRate(p.baudRate);
    m_serial->setDataBits(p.dataBits);
    m_serial->setParity(p.parity);
    m_serial->setStopBits(p.stopBits);
    m_serial->setFlowControl(p.flowControl);
    if (m_serial->open(QIODevice::ReadWrite)) {
        m_console->setEnabled(true);
        m_console->setLocalEchoEnabled(p.localEchoEnabled);
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);
        m_ui->actionConfigure->setEnabled(false);
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    } else {
        QMessageBox::critical(this, tr("Error"), m_serial->errorString());

        showStatusMessage(tr("Open error"));
    }
}
//! [4]

//! [5]
void MainWindow::closeSerialPort()
{
    if (m_serial->isOpen())
        m_serial->close();
    m_console->setEnabled(false);
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionConfigure->setEnabled(true);
    showStatusMessage(tr("Disconnected"));
}
//! [5]

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Simple Terminal"),
                       tr("The <b>Simple Terminal</b> example demonstrates how to "
                          "use the Qt Serial Port module in modern GUI applications "
                          "using Qt, with a menu bar, toolbars, and a status bar."));
}

//! [6]
void MainWindow::writeData(const QByteArray &data)
{
    m_serial->write(data);
}
//! [6]

//! [7]
void MainWindow::readData()
{
    const QByteArray data = m_serial->readAll();
    m_console->putData(data);
    inputWord(data);
}
//! [7]

//! [8]
void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), m_serial->errorString());
        closeSerialPort();
    }
}
//! [8]

void MainWindow::initActionsConnections()
{
    connect(m_ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(m_ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(m_ui->actionConfigure, &QAction::triggered, m_settings, &SettingsDialog::show);
    connect(m_ui->actionClear, &QAction::triggered, m_console, &Console::clear);
    connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(m_ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
}

void MainWindow::showStatusMessage(const QString &message)
{
    m_status->setText(message);
}

void MainWindow::inputWord(const QByteArray &data)
{
    QString dataString = QString(data);
    m_eValue->setText(dataString);
    m_sValue->setText(dataString);
    //parse dataString
    QRegExp rx("[: ]");
    QStringList list = dataString.split(rx, Qt::SkipEmptyParts);

    //find which part of the string to put where
//    for(int i = 0; i < list.size(); i++)
//    {
//        if(QString::compare(list.at(i), "e", Qt::CaseInsensitive))
//        {
//            m_eValue->setText(list.at(5));
//        }
//        else if(QString::compare(list.at(i), "s", Qt::CaseInsensitive))
//        {
//            m_sValue->setText(list.at(0));
//        }
//        else if(QString::compare(list.at(i), "Spwm", Qt::CaseInsensitive))
//        {
//            m_SpwmValue->setText(list.at(i));
//        }
//        else if(QString::compare(list.at(i), "Epwm", Qt::CaseInsensitive))
//        {
//            m_EpwmValue->setText(list.at(i));
//        }
//        else if(QString::compare(list.at(i), "Erpm", Qt::CaseInsensitive))
//        {
//            m_ErpmValue->setText(list.at(i));
//        }
//        else if(QString::compare(list.at(i), "Esta", Qt::CaseInsensitive))
//        {
//            m_EstaValue->setText(list.at(i));
//        }
//        else if(QString::compare(list.at(i), "sta", Qt::CaseInsensitive))
//        {
//            m_staValue->setText(list.at(i));
//        }
//        else if(QString::compare(list.at(i), "ch3", Qt::CaseInsensitive))
//        {
//            m_ch3Value->setText(list.at(i));
//        }
//        else if(QString::compare(list.at(i), "RPM", Qt::CaseInsensitive))
//        {
//            m_RPMValue->setText(list.at(i));
//        }
//        else if(QString::compare(list.at(i), "CAN", Qt::CaseInsensitive))
//        {
//            m_CANValue->setText(list.at(i));
//        }
//        else
//        {

//        }
//    }
}

