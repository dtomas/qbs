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
** a written agreement between you and The Qt Company.  For licensing terms and
** conditions see http://www.qt.io/terms-conditions.  For further information
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
#ifndef QBS_BENCHMARKER_BENCHMARKER_H
#define QBS_BENCHMARKER_BENCHMARKER_H

#include "activities.h"

#include <QHash>
#include <QString>
#include <QTemporaryDir>

QT_BEGIN_NAMESPACE
class QStringList;
QT_END_NAMESPACE

namespace qbsBenchmarker {

class BenchmarkResult
{
public:
    qint64 oldInstructionCount;
    qint64 newInstructionCount;
    qint64 oldPeakMemoryUsage;
    qint64 newPeakMemoryUsage;
};
typedef QHash<Activity, BenchmarkResult> BenchmarkResults;

class Benchmarker
{
public:
    Benchmarker(Activities activities, const QString &oldCommit, const QString &newCommit,
                const QString &testProject, const QString &qbsRepo);
    ~Benchmarker();

    void benchmark();

    BenchmarkResults results() const { return m_results; }

private:
    void rememberCurrentRepoState();
    void buildQbs(const QString &buildDir) const;

    const Activities m_activities;
    const QString m_oldCommit;
    const QString m_newCommit;
    const QString m_testProject;
    const QString m_qbsRepo;
    QString m_commitToRestore;
    QTemporaryDir m_baseOutputDir;
    BenchmarkResults m_results;
};

} // namespace qbsBenchmarker

#endif // Include guard.
