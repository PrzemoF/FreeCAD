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

#include <Base/Exception.h>
#include <Base/Quantity.h>
#include <App/PropertyLinks.h>
#include <App/PropertyUnits.h>
#include <App/GeoFeature.h>
#include <Gui/Application.h>
#include <Gui/Document.h>
#include <Gui/Flag.h>
#include <Gui/MainWindow.h>
#include <Gui/SoFCColorBar.h>
#include <Gui/SoFCSelection.h>
#include <Gui/ViewProviderGeometryObject.h>
#include <Gui/View3DInventorViewer.h>
#include <Gui/Widgets.h>
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
    pcColorStyle = new SoDrawStyle(); 
    pcColorRoot->addChild(pcColorStyle);
    pcColorBar = new Gui::SoFCColorBar;
    pcColorBar->Attach(this);
    pcColorBar->ref();
    pcColorBar->setRange( -0.1f, 0.1f, 3 );
}

ViewProviderResult::~ViewProviderResult()
{
    pcColorRoot->unref();
    pcColorBar->Detach(this);
    pcColorBar->unref();

}

SoSeparator* ViewProviderResult::getFrontRoot(void) const
{
qDebug("getFrontRoot");
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
*/
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
}

void ViewProviderResult::hide(void)
{
    inherited::hide();
    pcColorStyle->style = SoDrawStyle::INVISIBLE;
}

void ViewProviderResult::show(void)
{
    inherited::show();
    pcColorStyle->style = SoDrawStyle::FILLED;
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
	if (prop->getTypeId() == App::PropertyString::getClassTypeId()) {
		QString name = QString::fromStdString(prop->getName());
		qDebug("updateData: name: ");
		qDebug(name.toUtf8());
		if (strcmp(prop->getName(), "DataType") == 0) {
			std::string fem_dt = ((App::PropertyString*)prop)->getValue();
			QString fem_data_type = QString::fromStdString(fem_dt);
			qDebug("updateData: fem_data_type: ");
			qDebug(fem_data_type.toUtf8());
			/*this->search_radius = fSearchRadius;
			  pcColorBar->setRange( -fSearchRadius, fSearchRadius, 4 );
			  pcColorBar->Notify(0);*/
		}
	}
	if (prop->getTypeId() == App::PropertyQuantity::getClassTypeId()) {
		QString name = QString::fromStdString(prop->getName());
		qDebug("updateData: unit: ");
		qDebug(name.toUtf8());
		if (strcmp(prop->getName(), "Unit") == 0) {
			Base::Quantity fem_u = static_cast<const App::PropertyQuantity*>(prop)->getQuantityValue();
			double fem_unit_value = fem_u.getValue();
			qDebug("updateData: Unit_value: ");
			qDebug("%f", fem_unit_value);
			Base::Unit fem_unit = fem_u.getUnit();
			QString fem_unit_string = fem_unit.getString();
			qDebug("updateData: Unit: ");
			qDebug(fem_unit_string.toUtf8());
			/*this->search_radius = fSearchRadius;
			  pcColorBar->setRange( -fSearchRadius, fSearchRadius, 4 );
			  pcColorBar->Notify(0);*/
		}
	}
}

// Python feature -----------------------------------------------------------------------

namespace Gui {
/// @cond DOXERR
PROPERTY_SOURCE_TEMPLATE(FemGui::ViewProviderResultPython, FemGui::ViewProviderResult)
/// @endcond

// explicit template instantiation
template class FemGuiExport ViewProviderPythonFeatureT<ViewProviderResult>;
}
