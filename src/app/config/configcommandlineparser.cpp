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
#include "configcommandlineparser.h"

#include <logging/translator.h>
#include <tools/error.h>

#include <cstdio>

using namespace qbs;
using namespace Internal;

void ConfigCommandLineParser::parse(const QStringList &commandLine)
{
    m_command = ConfigCommand();
    m_helpRequested = false;
    m_settingsDir.clear();

    m_commandLine = commandLine;
    if (m_commandLine.isEmpty())
        throw ErrorInfo(Tr::tr("No parameters supplied."));
    if (m_commandLine.count() == 1 && (m_commandLine.first() == QLatin1String("--help")
                              || m_commandLine.first() == QLatin1String("-h"))) {
        m_helpRequested = true;
        return;
    }

    while (!m_commandLine.isEmpty() && m_commandLine.first().startsWith(QLatin1String("--"))) {
        const QString arg = m_commandLine.takeFirst().mid(2);
        if (arg == QLatin1String("list"))
            setCommand(ConfigCommand::CfgList);
        else if (arg == QLatin1String("unset"))
            setCommand(ConfigCommand::CfgUnset);
        else if (arg == QLatin1String("export"))
            setCommand(ConfigCommand::CfgExport);
        else if (arg == QLatin1String("import"))
            setCommand(ConfigCommand::CfgImport);
        else if (arg == QLatin1String("settings-dir"))
            assignOptionArgument(arg, m_settingsDir);
        else
            throw ErrorInfo(Tr::tr("Unknown option for config command."));
    }

    switch (command().command) {
    case ConfigCommand::CfgNone:
        if (m_commandLine.isEmpty())
            throw ErrorInfo(Tr::tr("No parameters supplied."));
        if (m_commandLine.count() > 2)
            throw ErrorInfo(Tr::tr("Too many arguments."));
        m_command.varNames << m_commandLine.first();
        if (m_commandLine.count() == 1) {
            setCommand(ConfigCommand::CfgList);
        } else {
            m_command.varValue = m_commandLine.at(1);
            setCommand(ConfigCommand::CfgSet);
        }
        break;
    case ConfigCommand::CfgUnset:
        if (m_commandLine.isEmpty())
            throw ErrorInfo(Tr::tr("Need name of variable to unset."));
        m_command.varNames = m_commandLine;
        break;
    case ConfigCommand::CfgExport:
        if (m_commandLine.count() != 1)
            throw ErrorInfo(Tr::tr("Need name of file to which to export."));
        m_command.fileName = m_commandLine.first();
        break;
    case ConfigCommand::CfgImport:
        if (m_commandLine.count() != 1)
            throw ErrorInfo(Tr::tr("Need name of file from which to import."));
        m_command.fileName = m_commandLine.first();
        break;
    case ConfigCommand::CfgList:
        m_command.varNames = m_commandLine;
        break;
    default:
        break;
    }
}

void ConfigCommandLineParser::setCommand(ConfigCommand::Command command)
{
    if (m_command.command != ConfigCommand::CfgNone)
        throw ErrorInfo(Tr::tr("You cannot specify more than one command."));
    m_command.command = command;
}

void ConfigCommandLineParser::printUsage() const
{
    puts("Usage:\n"
        "    qbs config [--settings-dir <settings directory] <options>\n"
        "    qbs config [--settings-dir <settings directory] <key>\n"
        "    qbs config [--settings-dir <settings directory] <key> <value>"
        "\n"
         "Options:\n"
         "    --list [<root> ...] list keys under key <root> or all keys\n"
         "    --unset <name>      remove key with given name\n"
         "    --import <file>     import settings from given file\n"
         "    --export <file>     export settings to given file\n");
}

void ConfigCommandLineParser::assignOptionArgument(const QString &option, QString &argument)
{
    if (m_commandLine.isEmpty())
        throw ErrorInfo(Tr::tr("Option '%1' needs an argument.").arg(option));
    argument = m_commandLine.takeFirst();
    if (argument.isEmpty())
        throw ErrorInfo(Tr::tr("Argument for option '%1' must not be empty.").arg(option));
}
