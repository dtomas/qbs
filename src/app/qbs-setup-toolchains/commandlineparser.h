/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of Qbs.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms and
** conditions see http://www.qt.io/terms-conditions. For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/
#ifndef QBS_SETUPTOOLCHAINS_COMMANDLINEPARSER_H
#define QBS_SETUPTOOLCHAINS_COMMANDLINEPARSER_H

#include <QStringList>

class CommandLineParser
{
public:
    void parse(const QStringList &commandLine);

    bool helpRequested() const { return m_helpRequested; }
    bool autoDetectionMode() const { return m_autoDetectionMode; }

    QString compilerPath() const { return m_compilerPath; }
    QString toolchainType() const { return m_toolchainType; }
    QString profileName() const { return m_profileName; }
    QString settingsDir() const { return m_settingsDir; }

    QString usageString() const;

private:
    void throwError(const QString &message);
    void assignOptionArgument(const QString &option, QString &argument);
    void complainAboutExtraArguments();

    bool m_helpRequested;
    bool m_autoDetectionMode;
    QString m_compilerPath;
    QString m_toolchainType;
    QString m_profileName;
    QString m_settingsDir;
    QStringList m_commandLine;
    QString m_command;
};

#endif // Include guard
