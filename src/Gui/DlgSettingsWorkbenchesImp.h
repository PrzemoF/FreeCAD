/***************************************************************************
 *   Copyright (c) 2015 FreeCAD Developers                                 *
 *   Autor: Przemo Firszt <przemo@firszt.eu>                               *
 *   Based on DlgSettingsUnitsImp.h file                                   *
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


#ifndef GUI_DIALOG_DLGSETTINGSWorkbenchesIMP_H
#define GUI_DIALOG_DLGSETTINGSWorkbenchesIMP_H

#include "PropertyPage.h"

namespace Gui {
namespace Dialog {

/**
 * The DlgSettingsWorkbenchesImp class implements a preference page to change
 * visible workbenches
 */
class Ui_DlgSettingsWorkbenches;
class DlgSettingsWorkbenchesImp : public PreferencePage
{
    Q_OBJECT

public:
    DlgSettingsWorkbenchesImp(QWidget* parent = 0);
    ~DlgSettingsWorkbenchesImp();

    void saveSettings();
    void loadSettings();

protected:
    void changeEvent(QEvent *e);

//public Q_SLOTS:
//    void on_comboBox_ViewSystem_currentIndexChanged(int index);

private:
    Ui_DlgSettingsWorkbenches* ui;
};

} // namespace Dialog
} // namespace Gui

#endif // GUI_DIALOG_DLGSETTINGSWorkbenchesIMP_H
