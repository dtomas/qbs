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

#ifndef QBS_EXECUTORJOB_H
#define QBS_EXECUTORJOB_H

#include <language/forward_decls.h>
#include <tools/commandechomode.h>
#include <tools/error.h>

#include <QObject>

namespace qbs {
class CodeLocation;
class ProcessResult;

namespace Internal {
class AbstractCommandExecutor;
class ProductBuildData;
class JsCommandExecutor;
class Logger;
class ProcessCommandExecutor;
class ScriptEngine;
class Transformer;

class ExecutorJob : public QObject
{
    Q_OBJECT
public:
    ExecutorJob(const Logger &logger, QObject *parent);
    ~ExecutorJob();

    void setMainThreadScriptEngine(ScriptEngine *engine);
    void setDryRun(bool enabled);
    void setEchoMode(CommandEchoMode echoMode);
    void run(Transformer *t);
    void cancel();

signals:
    void reportCommandDescription(const QString &highlight, const QString &message);
    void reportProcessResult(const qbs::ProcessResult &result);
    void finished(const qbs::ErrorInfo &error = ErrorInfo()); // !hasError() <=> command successful

private:
    void runNextCommand();
    void onCommandFinished(const qbs::ErrorInfo &err);

    void setFinished();
    void reset();

    AbstractCommandExecutor *m_currentCommandExecutor;
    ProcessCommandExecutor *m_processCommandExecutor;
    JsCommandExecutor *m_jsCommandExecutor;
    Transformer *m_transformer;
    int m_currentCommandIdx;
    ErrorInfo m_error;
};

} // namespace Internal
} // namespace qbs

#endif // QBS_EXECUTORJOB_H
