/**********************************************************************
*  Copyright (c) 2008-2015, Alliance for Sustainable Energy.
*  All rights reserved.
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License as published by the Free Software Foundation; either
*  version 2.1 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**********************************************************************/

#ifndef OPENSTUDIO_CONSTRUCTIONGRIDVIEW_HPP
#define OPENSTUDIO_CONSTRUCTIONGRIDVIEW_HPP

#include "../shared_gui_components/OSGridController.hpp"
#include "OSItem.hpp"

#include "../model/Model.hpp"

#include <QWidget>

class QComboBox;

class QLabel;

namespace openstudio{

class ModelSubTabView;
class ConstructionGridController;

class ConstructionGridView : public QWidget
{
  Q_OBJECT

public:

  ConstructionGridView(bool isIP, const model::Model & model, QWidget * parent = 0);

  virtual ~ConstructionGridView() {}

  virtual std::vector<model::ModelObject> selectedObjects() const;

  void enableFilter();

  void disableFilter();

  QComboBox * m_filters = nullptr;

private:

  REGISTER_LOGGER("openstudio.ConstructionGridView");

  bool m_isIP;

  ConstructionGridController * m_gridController = nullptr;

  QLabel * m_filterLabel = nullptr;

signals:

  void toggleUnitsClicked(bool displayIP);

  void dropZoneItemClicked(OSItem* item);

  void itemSelected(OSItem * item);

  void selectionCleared();

  void gridRowSelected(OSItem*);

private slots:

  void onDropZoneItemClicked(OSItem* item);

};

class ConstructionGridController : public OSGridController
{

  Q_OBJECT

public:

  ConstructionGridController(bool isIP,
    const QString & headerText,
    IddObjectType iddObjectType,
    model::Model model,
    std::vector<model::ModelObject> modelObjects);

  virtual ~ConstructionGridController() {}

  virtual void refreshModelObjects();

  virtual void categorySelected(int index);

protected:

  virtual void setCategoriesAndFields();

  virtual void addColumns(const QString &category, std::vector<QString> & fields);

  virtual void checkSelectedFields();

  virtual QString getColor(const model::ModelObject & modelObject);

public slots:

  virtual void onItemDropped(const OSItemId& itemId);

  virtual void onComboBoxIndexChanged(int index);

  void filterChanged(const QString & text);

private:
 
  ConstructionGridView * constructionGridView();

};

} // openstudio

#endif // OPENSTUDIO_CONSTRUCTIONGRIDVIEW_HPP

