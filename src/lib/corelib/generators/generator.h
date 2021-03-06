/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2015 Jake Petroules.
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

#ifndef GENERATORPLUGIN_H
#define GENERATORPLUGIN_H

#include <api/project.h>
#include <QList>
#include <QString>

namespace qbs {

/*!
 * \class ProjectGenerator
 * \brief The \c ProjectGenerator class is an abstract base class for generators which generate
 * arbitrary output given a resolved Qbs project.
 */

class ProjectGenerator
{
public:
    virtual ~ProjectGenerator()
    {
    }

    /*!
     * Returns the name of the generator used to create the external build system files.
     */
    virtual QString generatorName() const = 0;

    virtual void generate(const InstallOptions &installOptions) = 0;

    QList<Project> projects() const
    {
        return m_projects;
    }

    void addProject(const Project &project)
    {
        m_projects << project;
    }

    void addProjects(const QList<Project> &projects)
    {
        m_projects << projects;
    }

    void removeProject(const Project &project)
    {
        m_projects.removeOne(project);
    }

    void clearProjects()
    {
        m_projects.clear();
    }

    QList<QVariantMap> buildConfigurations() const
    {
        return m_buildConfigurations;
    }

    void addBuildConfiguration(const QVariantMap &configuration)
    {
        m_buildConfigurations << configuration;
    }

    void addBuildConfigurations(const QList<QVariantMap> &configurations)
    {
        m_buildConfigurations << configurations;
    }

    void removeBuildConfiguration(const QVariantMap &configuration)
    {
        m_buildConfigurations.removeOne(configuration);
    }

    void clearBuildConfigurations()
    {
        m_buildConfigurations.clear();
    }

protected:
    ProjectGenerator()
    {
    }

private:
    QList<Project> m_projects;
    QList<QVariantMap> m_buildConfigurations;
};

} // namespace qbs

#ifdef __cplusplus
extern "C" {
#endif

typedef qbs::ProjectGenerator **(*getGenerators_f)();

#ifdef __cplusplus
}
#endif

#endif // GENERATORPLUGIN_H
