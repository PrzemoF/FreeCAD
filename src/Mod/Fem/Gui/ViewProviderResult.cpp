/***************************************************************************
 *   Copyright (c) 2013 Jürgen Riegel (FreeCAD@juergen-riegel.net)         *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#include "PreCompiled.h"

#ifndef _PreComp_
# include <Standard_math.hxx>

#endif

#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/lists/SoPickedPointList.h> 
#include <Inventor/details/SoFaceDetail.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoPointSet.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoMaterialBinding.h>
#include <Inventor/errors/SoDebugError.h>

#include "ViewProviderResult.h"
#include <Gui/Command.h>
#include <Gui/Document.h>
#include <Gui/Control.h>

#include <Mod/Fem/App/FemAnalysis.h>

#include "TaskDlgAnalysis.h"

using namespace FemGui;







PROPERTY_SOURCE(FemGui::ViewProviderResult, Gui::ViewProviderDocumentObject)


ViewProviderResult::ViewProviderResult()
{
    sPixmap = "Fem_Result";
    pcColorRoot = new SoSeparator();
    pcColorRoot->ref();
}

ViewProviderResult::~ViewProviderResult()
{
    pcColorRoot->unref();

}

SoSeparator* ViewProviderInspection::getFrontRoot(void) const
{
    return pcColorRoot;
}

void ViewProviderResult::attach(App::DocumentObject *pcFeat)
{
    // creats the standard viewing modes
    inherited::attach(pcFeat);
qDebug("attach");

/*    SoShapeHints * flathints = new SoShapeHints;
    flathints->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE ;
    flathints->shapeType = SoShapeHints::UNKNOWN_SHAPE_TYPE;

    SoGroup* pcColorShadedRoot = new SoGroup();
    pcColorShadedRoot->addChild(flathints);

    // color shaded  ------------------------------------------
    SoDrawStyle *pcFlatStyle = new SoDrawStyle();
    pcFlatStyle->style = SoDrawStyle::FILLED;
    pcColorShadedRoot->addChild(pcFlatStyle);

    pcColorShadedRoot->addChild(pcColorMat);
    pcColorShadedRoot->addChild(pcMatBinding);
    pcColorShadedRoot->addChild(pcLinkRoot);

    addDisplayMaskMode(pcColorShadedRoot, "ColorShaded");

    // Check for an already existing color bar
    Gui::SoFCColorBar* pcBar = ((Gui::SoFCColorBar*)findFrontRootOfType(Gui::SoFCColorBar::getClassTypeId()));
    if (pcBar) {
        float fMin = pcColorBar->getMinValue();
        float fMax = pcColorBar->getMaxValue();
    
        // Attach to the foreign color bar and delete our own bar
        pcBar->Attach(this);
        pcBar->ref();
        pcBar->setRange(fMin, fMax, 3);
        pcBar->Notify(0);
        pcColorBar->Detach(this);
        pcColorBar->unref();
        pcColorBar = pcBar;
    }

    pcColorRoot->addChild(pcColorBar);
*/
}

void ViewProviderResult::OnChange(Base::Subject<int> &rCaller,int rcReason)
{
qDebug("OnChange");
    setActiveMode();
}

void ViewProviderResult::onChanged(const App::Property* prop)
{
qDebug("onChanged");
 /*   if (prop == &OutsideGrayed) {
        if (pcColorBar) {
            pcColorBar->setOutsideGrayed(OutsideGrayed.getValue());
            pcColorBar->Notify(0);
        }
    }
    else if ( prop == &PointSize ) {
        pcPointStyle->pointSize = PointSize.getValue();
    }
    else {
*/
        inherited::onChanged(prop);
/*    }
*/
}

void ViewProviderResult::updateData(const App::Property* prop)
{
}

// Python feature -----------------------------------------------------------------------

namespace Gui {
/// @cond DOXERR
PROPERTY_SOURCE_TEMPLATE(FemGui::ViewProviderResultPython, FemGui::ViewProviderResult)
/// @endcond

// explicit template instantiation
template class FemGuiExport ViewProviderPythonFeatureT<ViewProviderResult>;
}
