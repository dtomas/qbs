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
#include "artifactvisitor.h"

#include "artifact.h"
#include "productbuilddata.h"
#include <language/language.h>
#include <tools/qbsassert.h>

namespace qbs {
namespace Internal {

ArtifactVisitor::ArtifactVisitor(int artifactType) : m_artifactType(artifactType)
{
}

void ArtifactVisitor::visitProduct(const ResolvedProductConstPtr &product)
{
    if (!product->buildData)
        return;
    foreach (BuildGraphNode *node, product->buildData->nodes)
        node->accept(this);
}

void ArtifactVisitor::visitProject(const ResolvedProjectConstPtr &project)
{
    foreach (const ResolvedProductConstPtr &product, project->allProducts())
        visitProduct(product);
}

bool ArtifactVisitor::visit(RuleNode *ruleNode)
{
    Q_UNUSED(ruleNode);
    return false;
}

bool ArtifactVisitor::visit(Artifact *artifact)
{
    QBS_CHECK(artifact);
    if (m_artifactType & artifact->artifactType)
        doVisit(artifact);
    return false;
}

} // namespace Internal
} // namespace qbs
