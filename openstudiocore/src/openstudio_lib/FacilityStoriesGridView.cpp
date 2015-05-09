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

#include "FacilityStoriesGridView.hpp"

#include "OSDropZone.hpp"
#include "OSItemSelectorButtons.hpp"
#include "RenderingColorWidget.hpp"

#include "../shared_gui_components/OSDoubleEdit.hpp"
#include "../shared_gui_components/OSGridView.hpp"

#include "../model/BuildingStory.hpp"
#include "../model/BuildingStory_Impl.hpp"
#include "../model/DefaultConstructionSet.hpp"
#include "../model/DefaultConstructionSet_Impl.hpp"
#include "../model/DefaultScheduleSet.hpp"
#include "../model/DefaultScheduleSet_Impl.hpp"
#include "../model/Model.hpp"
#include "../model/Model_Impl.hpp"
#include "../model/ModelObject.hpp"
#include "../model/ModelObject_Impl.hpp"
#include "../model/RenderingColor.hpp"
#include "../model/RenderingColor_Impl.hpp"

#include "../utilities/core/Assert.hpp"
#include "../utilities/idd/IddEnums.hxx"
#include "../utilities/idd/OS_BuildingStory_FieldEnums.hxx"

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>

// These defines provide a common area for field display names
// used on column headers, and other grid widgets

#define NAME "Story Name"
#define SELECTED "All"

// GENERAL
#define NOMINALZCOORDINATE "Nominal Z Coordinate"
#define NOMINALFLOORTOFLOORHEIGHT "Nominal Floor to Floor Height"
#define DEFAULTCONSTRUCTIONSETNAME "Default Construction Set Name"
#define DEFAULTSCHEDULESETNAME "Default Schedule Set Name"
#define GROUPRENDERINGNAME "Group Rendering Name"
#define NOMINALFLOORTOCEILINGHEIGHT "Nominal Floor to Ceiling Height"
#define SPACES "Spaces"

// FILTERS
#define NOMINALZCOORDINATEGREATERTHAN "Nominal Z Coordinate >"
#define NOMINALZCOORDINATELESSTHAN "Nominal Z Coordinate <"

namespace openstudio {

  struct ModelObjectNameSorter{
    // sort by name
    bool operator()(const model::ModelObject & lhs, const model::ModelObject & rhs){
      return (lhs.name() < rhs.name());
    }
  };

  FacilityStoriesGridView::FacilityStoriesGridView(bool isIP, const model::Model & model, QWidget * parent)
    : GridViewSubTab(isIP, model, parent)
  {
    auto buildingStories = model.getModelObjects<model::BuildingStory>();
    auto buildingStoriesModelObjects = subsetCastVector<model::ModelObject>(buildingStories);

    m_gridController = new FacilityStoriesGridController(isIP, "Building Stories", IddObjectType::OS_BuildingStory, model, buildingStoriesModelObjects);
    m_gridView = new OSGridView(m_gridController, "Building Stories", "Drop\nStory", false, parent);

    setGridController(m_gridController);
    setGridView(m_gridView);

    // Filters

    QLabel * label = nullptr;

    QVBoxLayout * layout = nullptr;

    auto filterGridLayout = new QGridLayout();
    filterGridLayout->setContentsMargins(7, 4, 0, 8);
    filterGridLayout->setSpacing(5);

    label = new QLabel();
    label->setText("Filters:");
    label->setObjectName("H2");
    filterGridLayout->addWidget(label, filterGridLayout->rowCount(), filterGridLayout->columnCount(), Qt::AlignTop | Qt::AlignLeft);

    // NOMINALZCOORDINATEGREATERTHAN

    layout = new QVBoxLayout();

    label = new QLabel();
    label->setText(NOMINALZCOORDINATEGREATERTHAN);
    label->setObjectName("H3");
    layout->addWidget(label, Qt::AlignTop | Qt::AlignLeft);

    m_greaterThanFilter = new QLineEdit();
    m_greaterThanFilter->setFixedWidth(OSItem::ITEM_WIDTH);
    connect(m_greaterThanFilter, &QLineEdit::editingFinished, this, &openstudio::FacilityStoriesGridView::greaterThanFilterChanged);

    auto greaterThanValidator = new QDoubleValidator();
    m_greaterThanFilter->setValidator(greaterThanValidator);

    layout->addWidget(m_greaterThanFilter, Qt::AlignTop | Qt::AlignLeft);
    layout->addStretch();
    filterGridLayout->addLayout(layout, filterGridLayout->rowCount() - 1, filterGridLayout->columnCount());

    // NOMINALZCOORDINATELESSTHAN

    layout = new QVBoxLayout();

    label = new QLabel();
    label->setText(NOMINALZCOORDINATELESSTHAN);
    label->setObjectName("H3");
    layout->addWidget(label, Qt::AlignTop | Qt::AlignLeft);

    m_lessThanFilter = new QLineEdit();
    m_lessThanFilter->setFixedWidth(OSItem::ITEM_WIDTH);
    connect(m_lessThanFilter, &QLineEdit::editingFinished, this, &openstudio::FacilityStoriesGridView::lessThanFilterChanged);

    auto lessThanValidator = new QDoubleValidator();
    m_lessThanFilter->setValidator(lessThanValidator);

    layout->addWidget(m_lessThanFilter, Qt::AlignTop | Qt::AlignLeft);
    layout->addStretch();
    filterGridLayout->addLayout(layout, filterGridLayout->rowCount() - 1, filterGridLayout->columnCount());

    filterGridLayout->setRowStretch(filterGridLayout->rowCount(), 100);
    filterGridLayout->setColumnStretch(filterGridLayout->columnCount(), 100);

    m_gridView->m_contentLayout->addLayout(filterGridLayout);

    m_gridView->m_contentLayout->addSpacing(7);
  }

  void FacilityStoriesGridView::onDropZoneItemClicked(OSItem* item)
  {
  }

  void FacilityStoriesGridView::addObject(const IddObjectType& iddObjectType)
  {
    if (IddObjectType::OS_BuildingStory == iddObjectType.value()) {
      model::BuildingStory(this->m_model);
    }
  }

  void FacilityStoriesGridView::purgeObjects(const IddObjectType& iddObjectType)
  {
    for (auto mo : this->m_model.getConcreteModelObjects<model::BuildingStory>()){
      mo.remove();
    }
  }

  void FacilityStoriesGridView::greaterThanFilterChanged()
  {
    filterChanged();
  }

  void FacilityStoriesGridView::lessThanFilterChanged()
  {
    filterChanged();
  }

  void FacilityStoriesGridView::filterChanged()
  {
    auto objectSelector = this->m_gridController->getObjectSelector();

    auto upperLimit = this->m_lessThanFilter->text().toDouble();
    auto lowerLimit = this->m_greaterThanFilter->text().toDouble();

    for (auto obj : objectSelector->m_selectorObjects) {
      auto nominalZCoordinate = obj.cast<model::BuildingStory>().nominalZCoordinate();
      if (nominalZCoordinate && (nominalZCoordinate > upperLimit || nominalZCoordinate < lowerLimit)) {
        objectSelector->m_filteredObjects.insert(obj).second;
      }
    }

    this->m_gridView->requestRefreshAll();
  }

  FacilityStoriesGridController::FacilityStoriesGridController(bool isIP,
    const QString & headerText,
    IddObjectType iddObjectType,
    model::Model model,
    std::vector<model::ModelObject> modelObjects) :
    OSGridController(isIP, headerText, iddObjectType, model, modelObjects)
  {
    setCategoriesAndFields();
  }

  void FacilityStoriesGridController::setCategoriesAndFields()
  {
    {
      std::vector<QString> fields;
      fields.push_back(GROUPRENDERINGNAME);
      fields.push_back(NOMINALZCOORDINATE);
      fields.push_back(NOMINALFLOORTOFLOORHEIGHT);
      fields.push_back(NOMINALFLOORTOCEILINGHEIGHT);
      fields.push_back(DEFAULTCONSTRUCTIONSETNAME);
      fields.push_back(DEFAULTSCHEDULESETNAME);
      //fields.push_back(SPACES);
      std::pair<QString, std::vector<QString> > categoryAndFields = std::make_pair(QString("General"), fields);
      m_categoriesAndFields.push_back(categoryAndFields);
    }

    OSGridController::setCategoriesAndFields();
  }

  void FacilityStoriesGridController::categorySelected(int index)
  {
    OSGridController::categorySelected(index);
  }

  void FacilityStoriesGridController::addColumns(const QString &category, std::vector<QString> & fields)
  {
    // always show name and selected columns
    fields.insert(fields.begin(), { NAME, SELECTED });

    m_baseConcepts.clear();

    for (const auto &field : fields) {

      if (field == NAME) {
        addNameLineEditColumn(Heading(QString(NAME), false, false),
          false,
          false,
          CastNullAdapter<model::BuildingStory>(&model::BuildingStory::name),
          CastNullAdapter<model::BuildingStory>(&model::BuildingStory::setName)
          );
      }
      else if (field == SELECTED) {
        auto checkbox = QSharedPointer<QCheckBox>(new QCheckBox());
        checkbox->setToolTip("Check to select all rows");
        connect(checkbox.data(), &QCheckBox::stateChanged, this, &FacilityStoriesGridController::selectAllStateChanged);

        addSelectColumn(Heading(QString(SELECTED), false, false, checkbox), "Check to select this row");
      }
      else if (field == NOMINALZCOORDINATE) {
        addQuantityEditColumn(Heading(QString(NOMINALFLOORTOFLOORHEIGHT)),
          QString("m"),
          QString("m"),
          QString("ft"),
          m_isIP,
          NullAdapter(&model::BuildingStory::nominalZCoordinate),
          NullAdapter(&model::BuildingStory::setNominalZCoordinate)
          );
      }
      else if (field == NOMINALFLOORTOFLOORHEIGHT) {
        addQuantityEditColumn(Heading(QString(NOMINALFLOORTOFLOORHEIGHT)),
          QString("m"),
          QString("m"),
          QString("ft"),
          m_isIP,
          NullAdapter(&model::BuildingStory::nominalFloortoFloorHeight),
          NullAdapter(&model::BuildingStory::setNominalFloortoFloorHeight)
          );
      }
      else if (field == DEFAULTCONSTRUCTIONSETNAME) {
        addDropZoneColumn(Heading(QString(DEFAULTCONSTRUCTIONSETNAME)),
          CastNullAdapter<model::BuildingStory>(&model::BuildingStory::defaultConstructionSet),
          CastNullAdapter<model::BuildingStory>(&model::BuildingStory::setDefaultConstructionSet),
          boost::optional<std::function<void(model::BuildingStory*)>>(CastNullAdapter<model::BuildingStory>(&model::BuildingStory::resetDefaultConstructionSet))
          );
      }
      else if (field == DEFAULTSCHEDULESETNAME) {
        addDropZoneColumn(Heading(QString(DEFAULTSCHEDULESETNAME)),
          CastNullAdapter<model::BuildingStory>(&model::BuildingStory::defaultScheduleSet),
          CastNullAdapter<model::BuildingStory>(&model::BuildingStory::setDefaultScheduleSet),
          boost::optional<std::function<void(model::BuildingStory*)>>(CastNullAdapter<model::BuildingStory>(&model::BuildingStory::resetDefaultScheduleSet))
          );
      }
      else if (field == GROUPRENDERINGNAME) {
        addRenderingColorColumn(Heading(QString(GROUPRENDERINGNAME), true, false),
          CastNullAdapter<model::BuildingStory>(&model::BuildingStory::renderingColor),
          CastNullAdapter<model::BuildingStory>(&model::BuildingStory::setRenderingColor)
          );
      }
      else if (field == NOMINALFLOORTOCEILINGHEIGHT) {
        addQuantityEditColumn(Heading(QString(NOMINALFLOORTOCEILINGHEIGHT)),
          QString("m"),
          QString("m"),
          QString("ft"),
          m_isIP,
          NullAdapter(&model::BuildingStory::nominalFloortoCeilingHeight),
          NullAdapter(&model::BuildingStory::setNominalFloortoCeilingHeight)
          );
      }
      else if (field == SPACES) {

      }
      else {
        // unhandled
        OS_ASSERT(false);
      }
    }
  }

  QString FacilityStoriesGridController::getColor(const model::ModelObject & modelObject)
  {
    QColor defaultColor(Qt::lightGray);
    return defaultColor.name();
  }

  void FacilityStoriesGridController::checkSelectedFields()
  {
    if (!this->m_hasHorizontalHeader) return;

    OSGridController::checkSelectedFields();
  }

  void FacilityStoriesGridController::onItemDropped(const OSItemId& itemId)
  {
  }

  void FacilityStoriesGridController::refreshModelObjects()
  {
    auto exteriorLights = m_model.getModelObjects<model::BuildingStory>();
    m_modelObjects = subsetCastVector<model::ModelObject>(exteriorLights);
    std::sort(m_modelObjects.begin(), m_modelObjects.end(), ModelObjectNameSorter());
  }

} // openstudio