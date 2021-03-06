#include "DlgAddWatchEntry.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QVBoxLayout>
#include <sstream>

#include "../../DolphinProcess/DolphinAccessor.h"
#include "../GUICommon.h"

DlgAddWatchEntry::DlgAddWatchEntry(MemWatchEntry* entry) : m_entry(entry)
{
  m_chkBoundToPointer = new QCheckBox("This is a pointer", this);
  connect(m_chkBoundToPointer, static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),
          this, &DlgAddWatchEntry::onIsPointerChanged);

  QLabel* lblPreview = new QLabel("Value preview: ");
  m_lblValuePreview = new QLabel("", this);
  QHBoxLayout* preview_layout = new QHBoxLayout;
  preview_layout->addWidget(lblPreview);
  preview_layout->addWidget(m_lblValuePreview);
  QWidget* preview_widget = new QWidget();
  preview_widget->setLayout(preview_layout);

  QLabel* lblAddress = new QLabel("Address: ", this);
  m_txbAddress = new QLineEdit(this);
  QHBoxLayout* layout_addressAndPreview = new QHBoxLayout;
  layout_addressAndPreview->addWidget(lblAddress);
  layout_addressAndPreview->addWidget(m_txbAddress);
  QWidget* addressWidget = new QWidget(this);
  addressWidget->setLayout(layout_addressAndPreview);
  connect(m_txbAddress, static_cast<void (QLineEdit::*)(const QString&)>(&QLineEdit::textEdited),
          this, &DlgAddWatchEntry::onAddressChanged);

  m_offsetsLayout = new QGridLayout;
  m_offsets = QVector<QLineEdit*>();
  QWidget* offsetsWidget = new QWidget();
  offsetsWidget->setLayout(m_offsetsLayout);

  QLabel* lblOffset = new QLabel("Offsets (in hex): ", this);

  m_btnAddOffset = new QPushButton("Add offset");
  m_btnRemoveOffset = new QPushButton("Remove offset");
  QHBoxLayout* pointerButtons_layout = new QHBoxLayout;
  pointerButtons_layout->addWidget(m_btnAddOffset);
  pointerButtons_layout->addWidget(m_btnRemoveOffset);
  QWidget* pointerButtons_widget = new QWidget();
  pointerButtons_widget->setLayout(pointerButtons_layout);
  connect(m_btnAddOffset, static_cast<void (QPushButton::*)(bool)>(&QPushButton::clicked), this,
          &DlgAddWatchEntry::addPointerOffset);
  connect(m_btnRemoveOffset, static_cast<void (QPushButton::*)(bool)>(&QPushButton::clicked), this,
          &DlgAddWatchEntry::removePointerOffset);

  QVBoxLayout* pointerOffset_layout = new QVBoxLayout;
  pointerOffset_layout->addWidget(lblOffset);
  pointerOffset_layout->addWidget(offsetsWidget);
  pointerOffset_layout->addWidget(pointerButtons_widget);
  m_pointerWidget = new QWidget(this);
  m_pointerWidget->setLayout(pointerOffset_layout);

  QLabel* lblLabel = new QLabel("Label: ", this);
  m_txbLabel = new QLineEdit(this);
  QHBoxLayout* layout_label = new QHBoxLayout;
  layout_label->addWidget(lblLabel);
  layout_label->addWidget(m_txbLabel);
  QWidget* labelWidget = new QWidget(this);
  labelWidget->setLayout(layout_label);

  QLabel* lblType = new QLabel("Type: ", this);
  m_cmbTypes = new QComboBox(this);
  m_cmbTypes->addItems(GUICommon::g_memTypeNames);
  connect(m_cmbTypes, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
          &DlgAddWatchEntry::onTypeChange);
  QHBoxLayout* layout_type = new QHBoxLayout;
  layout_type->addWidget(lblType);
  layout_type->addWidget(m_cmbTypes);
  QWidget* typeWidget = new QWidget(this);
  typeWidget->setLayout(layout_type);

  QLabel* lblLength = new QLabel(QString("Length: "), this);
  m_spnLength = new QSpinBox(this);
  m_spnLength->setMinimum(1);
  QHBoxLayout* layout_length = new QHBoxLayout;
  layout_length->addWidget(lblLength);
  layout_length->addWidget(m_spnLength);
  m_lengtWidget = new QWidget;
  m_lengtWidget->setLayout(layout_length);
  connect(m_spnLength, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
          &DlgAddWatchEntry::onLengthChanged);

  QDialogButtonBox* buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addWidget(preview_widget);
  main_layout->addWidget(m_chkBoundToPointer);
  main_layout->addWidget(addressWidget);
  main_layout->addWidget(m_pointerWidget);
  main_layout->addWidget(labelWidget);
  main_layout->addWidget(typeWidget);
  main_layout->addWidget(m_lengtWidget);
  main_layout->addWidget(buttonBox);
  setLayout(main_layout);

  connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgAddWatchEntry::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgAddWatchEntry::reject);

  if (entry == nullptr)
  {
    m_entry = new MemWatchEntry();
    m_entry->setLength(1);

    m_cmbTypes->setCurrentIndex(0);
    m_spnLength->setValue(1);
    m_lengtWidget->hide();
    m_lblValuePreview->setText("???");
    m_chkBoundToPointer->setChecked(false);
    m_pointerWidget->hide();

    addPointerOffset();
  }
  else
  {
    m_cmbTypes->setCurrentIndex(static_cast<int>(m_entry->getType()));
    m_spnLength->setValue(static_cast<int>(m_entry->getLength()));
    if (m_entry->getType() == Common::MemType::type_string ||
        m_entry->getType() == Common::MemType::type_byteArray)
      m_lengtWidget->show();
    else
      m_lengtWidget->hide();
    m_txbLabel->setText(QString::fromStdString(m_entry->getLabel()));
    std::stringstream ssAddress;
    ssAddress << std::hex << std::uppercase << m_entry->getConsoleAddress();
    m_txbAddress->setText(QString::fromStdString(ssAddress.str()));
    if (entry->isBoundToPointer())
    {
      m_pointerWidget->show();
      for (int i = 0; i < m_entry->getPointerLevel(); ++i)
      {
        std::stringstream ss;
        ss << std::hex << std::uppercase << m_entry->getPointerOffset(i);
        QLabel* lblLevel =
            new QLabel(QString::fromStdString("Level " + std::to_string(i + 1) + ":"));
        QLineEdit* txbOffset = new QLineEdit();
        txbOffset->setText(QString::fromStdString(ss.str()));
        connect(txbOffset, static_cast<void (QLineEdit::*)(const QString&)>(&QLineEdit::textEdited),
                this, &DlgAddWatchEntry::onOffsetChanged);
        m_offsets.append(txbOffset);
        QLabel* lblAddressOfPath = new QLabel();
        lblAddressOfPath->setText(
            QString::fromStdString(" -> " + m_entry->getAddressStringForPointerLevel(i + 1)));
        m_addressPath.append(lblAddressOfPath);
        m_offsetsLayout->addWidget(lblLevel, i, 0);
        m_offsetsLayout->addWidget(txbOffset, i, 1);
        m_offsetsLayout->addWidget(lblAddressOfPath, i, 2);
      }
    }
    else
    {
      m_pointerWidget->hide();
      addPointerOffset();
    }
    // Implicitly calls updateValuePreview via an event
    m_chkBoundToPointer->setChecked(m_entry->isBoundToPointer());
  }
}

void DlgAddWatchEntry::addPointerOffset()
{
  int level = m_entry->getPointerLevel();
  QLabel* lblLevel = new QLabel(QString::fromStdString("Level " + std::to_string(level + 1) + ":"));
  QLineEdit* txbOffset = new QLineEdit();
  txbOffset->setText(0);
  m_offsets.append(txbOffset);
  QLabel* lblAddressOfPath = new QLabel(" -> ");
  m_addressPath.append(lblAddressOfPath);
  m_offsetsLayout->addWidget(lblLevel, level, 0);
  m_offsetsLayout->addWidget(txbOffset, level, 1);
  m_offsetsLayout->addWidget(lblAddressOfPath, level, 2);
  m_entry->addOffset(0);
  connect(txbOffset, static_cast<void (QLineEdit::*)(const QString&)>(&QLineEdit::textEdited), this,
          &DlgAddWatchEntry::onOffsetChanged);
  if (m_entry->getPointerLevel() > 1)
    m_btnRemoveOffset->setEnabled(true);
  else
    m_btnRemoveOffset->setDisabled(true);
}

void DlgAddWatchEntry::removePointerOffset()
{
  int level = m_entry->getPointerLevel();

  QLayoutItem* lblLevelItem = m_offsetsLayout->takeAt(
      m_offsetsLayout->indexOf(m_offsetsLayout->itemAtPosition(level - 1, 0)->widget()));
  QLayoutItem* txbOffsetItem = m_offsetsLayout->takeAt(
      m_offsetsLayout->indexOf(m_offsetsLayout->itemAtPosition(level - 1, 1)->widget()));
  QLayoutItem* lblAddressOfPathItem = m_offsetsLayout->takeAt(
      m_offsetsLayout->indexOf(m_offsetsLayout->itemAtPosition(level - 1, 2)->widget()));

  delete lblLevelItem->widget();
  delete txbOffsetItem->widget();
  delete lblAddressOfPathItem->widget();

  m_offsetsLayout->setRowMinimumHeight(level - 1, 0);
  m_offsetsLayout->setRowStretch(level - 1, 0);

  m_offsets.removeLast();
  m_addressPath.removeLast();
  m_entry->removeOffset();
  updatePreview();
  if (m_entry->getPointerLevel() == 1)
  {
    m_btnRemoveOffset->setDisabled(true);
    m_btnAddOffset->setFocus();
  }
}

void DlgAddWatchEntry::onOffsetChanged()
{
  QLineEdit* theLineEdit = static_cast<QLineEdit*>(sender());
  int index = 0;
  // Dummy variable for getItemPosition
  int column, rowSpan, columnSpan;
  m_offsetsLayout->getItemPosition(m_offsetsLayout->indexOf(theLineEdit), &index, &column, &rowSpan,
                                   &columnSpan);
  if (validateAndSetOffset(index))
  {
    updatePreview();
  }
}

void DlgAddWatchEntry::onTypeChange(int index)
{
  Common::MemType theType = static_cast<Common::MemType>(index);
  if (theType == Common::MemType::type_string || theType == Common::MemType::type_byteArray)
    m_lengtWidget->show();
  else
    m_lengtWidget->hide();
  m_entry->setType(theType);
  if (validateAndSetAddress())
    updatePreview();
}

void DlgAddWatchEntry::accept()
{
  if (!validateAndSetAddress())
  {
    QString errorMsg = QString(
        "The address you entered is invalid, make sure it is an "
        "hexadecimal number between 0x80000000 and 0x817FFFFF");
    if (DolphinComm::DolphinAccessor::isMem2Enabled())
    {
      errorMsg.append(" or between 0x90000000 and 0x93FFFFFF");
    }
    QMessageBox* errorBox = new QMessageBox(QMessageBox::Critical, QString("Invalid address"),
                                            errorMsg, QMessageBox::Ok, this);
    errorBox->exec();
  }
  else
  {
    if (m_chkBoundToPointer->isChecked())
    {
      bool allOffsetsValid = true;
      int i = 0;
      for (i; i < m_offsets.count(); ++i)
      {
        allOffsetsValid = validateAndSetOffset(i);
        if (!allOffsetsValid)
          break;
      }
      if (!allOffsetsValid)
      {
        QMessageBox* errorBox = new QMessageBox(
            QMessageBox::Critical, "Invalid offset",
            QString::fromStdString("The offset you entered for the level " + std::to_string(i + 1) +
                                   " is invalid, make sure it is an "
                                   "hexadecimal number"),
            QMessageBox::Ok, this);
        errorBox->exec();
        return;
      }
    }
    if (m_txbLabel->text() == "")
      m_entry->setLabel("No label");
    else
      m_entry->setLabel(m_txbLabel->text().toStdString());
    m_entry->setBase(Common::MemBase::base_decimal);
    m_entry->setSignedUnsigned(false);
    setResult(QDialog::Accepted);
    hide();
  }
}

bool DlgAddWatchEntry::validateAndSetAddress()
{
  std::string addressStr = m_txbAddress->text().toStdString();
  std::stringstream ss(addressStr);
  ss >> std::hex;
  u32 address = 0;
  ss >> address;
  if (!ss.fail())
  {
    if (DolphinComm::DolphinAccessor::isValidConsoleAddress(address))
    {
      m_entry->setConsoleAddress(address);
      return true;
    }
  }
  return false;
}

bool DlgAddWatchEntry::validateAndSetOffset(int index)
{
  std::string offsetStr = m_offsets[index]->text().toStdString();
  std::stringstream ss(offsetStr);
  ss >> std::hex;
  int offset = 0;
  ss >> offset;
  if (!ss.fail())
  {
    m_entry->setPointerOffset(offset, index);
    return true;
  }
  return false;
}

void DlgAddWatchEntry::onAddressChanged()
{
  if (validateAndSetAddress())
  {
    updatePreview();
  }
  else
  {
    m_lblValuePreview->setText("???");
  }
}

void DlgAddWatchEntry::updatePreview()
{
  m_entry->readMemoryFromRAM();
  m_lblValuePreview->setText(QString::fromStdString(m_entry->getStringFromMemory()));
  if (m_entry->isBoundToPointer())
  {
    int level = m_entry->getPointerLevel();
    for (int i = 0; i < level; ++i)
    {
      QWidget* test = m_offsetsLayout->itemAtPosition(i, 2)->widget();
      QLabel* lblAddressOfPath =
          static_cast<QLabel*>(m_offsetsLayout->itemAtPosition(i, 2)->widget());
      lblAddressOfPath->setText(
          QString::fromStdString(" -> " + m_entry->getAddressStringForPointerLevel(i + 1)));
    }
  }
}

void DlgAddWatchEntry::onLengthChanged()
{
  m_entry->setLength(m_spnLength->value());
  if (validateAndSetAddress())
    updatePreview();
}

void DlgAddWatchEntry::onIsPointerChanged()
{
  if (m_chkBoundToPointer->isChecked())
    m_pointerWidget->show();
  else
    m_pointerWidget->hide();
  m_entry->setBoundToPointer(m_chkBoundToPointer->isChecked());
  updatePreview();
}

MemWatchEntry* DlgAddWatchEntry::getEntry() const
{
  return m_entry;
}