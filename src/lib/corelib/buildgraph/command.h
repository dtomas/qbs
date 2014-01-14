/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Build Suite.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#ifndef QBS_COMMAND_H
#define QBS_COMMAND_H

#include <tools/codelocation.h>

#include <QProcessEnvironment>
#include <QStringList>
#include <QVariantMap>
#include <QScriptValue>

namespace qbs {
namespace Internal {

class AbstractCommand
{
public:
    virtual ~AbstractCommand();

    enum CommandType {
        ProcessCommandType,
        JavaScriptCommandType
    };

    static AbstractCommand *createByType(CommandType commandType);
    static QString defaultDescription() { return QString(); }
    static QString defaultHighLight() { return QString(); }
    static bool defaultIsSilent() { return false; }

    virtual CommandType type() const = 0;
    virtual bool equals(const AbstractCommand *other) const;
    virtual void fillFromScriptValue(const QScriptValue *scriptValue, const CodeLocation &codeLocation);
    virtual void load(QDataStream &s);
    virtual void store(QDataStream &s);

    const QString description() const { return m_description; }
    const QString highlight() const { return m_highlight; }
    bool isSilent() const { return m_silent; }
    CodeLocation codeLocation() const { return m_codeLocation; }

protected:
    AbstractCommand();

private:
    QString m_description;
    QString m_highlight;
    bool m_silent;
    CodeLocation m_codeLocation;
};

class ProcessCommand : public AbstractCommand
{
public:
    static void setupForJavaScript(QScriptValue targetObject);

    ProcessCommand();

    CommandType type() const { return ProcessCommandType; }
    bool equals(const AbstractCommand *otherAbstractCommand) const;
    void fillFromScriptValue(const QScriptValue *scriptValue, const CodeLocation &codeLocation);
    void load(QDataStream &s);
    void store(QDataStream &s);

    const QString program() const { return m_program; }
    const QStringList arguments() const { return m_arguments; }
    const QString workingDir() const { return m_workingDir; }
    int maxExitCode() const { return m_maxExitCode; }
    QString stdoutFilterFunction() const { return m_stdoutFilterFunction; }
    QString stderrFilterFunction() const { return m_stderrFilterFunction; }
    int responseFileThreshold() const { return m_responseFileThreshold; }
    QString responseFileUsagePrefix() const { return m_responseFileUsagePrefix; }
    QProcessEnvironment environment() const { return m_environment; }

private:
    void getEnvironmentFromList(const QStringList &envList);

    QString m_program;
    QStringList m_arguments;
    QString m_workingDir;
    int m_maxExitCode;
    QString m_stdoutFilterFunction;
    QString m_stderrFilterFunction;
    int m_responseFileThreshold; // When to use response files? In bytes of (program name + arguments).
    QString m_responseFileUsagePrefix;
    QProcessEnvironment m_environment;
};

class JavaScriptCommand : public AbstractCommand
{
public:
    static void setupForJavaScript(QScriptValue targetObject);

    JavaScriptCommand();

    virtual CommandType type() const { return JavaScriptCommandType; }
    bool equals(const AbstractCommand *otherAbstractCommand) const;
    void fillFromScriptValue(const QScriptValue *scriptValue, const CodeLocation &codeLocation);
    void load(QDataStream &s);
    void store(QDataStream &s);

    const QString &sourceCode() const { return m_sourceCode; }
    void setSourceCode(const QString &str) { m_sourceCode = str; }

    const QVariantMap &properties() const { return m_properties; }

private:
    QString m_sourceCode;
    QVariantMap m_properties;
};

} // namespace Internal
} // namespace qbs

#endif // QBS_COMMAND_H