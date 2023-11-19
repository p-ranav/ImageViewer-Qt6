#include "ExportWidget.hpp"

ExportWidget::ExportWidget(const QPixmap &pixmap, QWidget *parent)
    : QWidget(parent), pixmap(pixmap) {
  // Set up the main layout
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  // Create the form layout
  QFormLayout *formLayout = new QFormLayout;

  // Create a label for the image preview
  previewLabel = new QLabel;
  previewLabel->setPixmap(
      pixmap.scaled(500, 500, Qt::KeepAspectRatio, Qt::SmoothTransformation));

  // Create the dropdown for image file formats
  formatComboBox = new QComboBox;
  formatComboBox->addItem("PNG");
  formatComboBox->addItem("JPG"); // Set JPG as the default format

  // Create the quality slider
  qualitySlider = new QSlider(Qt::Horizontal);
  qualitySlider->setMinimum(0);
  qualitySlider->setMaximum(10); // 10 distinct values
  qualitySlider->setSingleStep(1);
  qualitySlider->setTickInterval(1);
  qualitySlider->setTickPosition(QSlider::NoTicks); // Hide ticks
  qualitySlider->hide(); // Initially hide the quality slider
  constexpr static int initialValue = 8;
  qualitySlider->setValue(initialValue);
  qualitySlider->setStyleSheet("color: white");

  // Create a label for the slider
  qualityLabel = new QLabel("Quality:");
  qualityLabel->hide(); // Initially hide the quality label
  updateQualityLabel(initialValue);

  // Create the export button
  exportButton = new QPushButton("Export");

  // Add widgets to the form layout
  formLayout->addRow(previewLabel);
  formLayout->addRow("Format:", formatComboBox);
  formLayout->addRow(qualityLabel);
  formLayout->addRow(qualitySlider);
  formLayout->addWidget(exportButton);

  // Add the form layout to the main layout
  mainLayout->addLayout(formLayout);

  // Connect the signal-slot for format change
  connect(formatComboBox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(handleFormatChange(int)));

  // Connect the signal-slot for export button click
  connect(exportButton, &QPushButton::clicked, this,
          &ExportWidget::exportPixmap);

  // Connect the signal-slot for slider value change
  connect(qualitySlider, &QSlider::valueChanged, this,
          &ExportWidget::updateQualityLabel);
}

void ExportWidget::handleFormatChange(int index) {
  // Show or hide the quality slider and label based on the selected format
  if (index == 1) { // JPG selected
    qualitySlider->show();
    qualityLabel->show();
  } else {
    qualitySlider->hide();
    qualityLabel->hide();
  }
}

void ExportWidget::updateQualityLabel(int value) {
  // Update the label to show the current quality setting
  qualityLabel->setText(QString("Quality: %1%").arg(value * 10));
}

void ExportWidget::exportPixmap() {
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"), "",
                                                  tr("Images (*.png *.jpg)"));
  if (!fileName.isEmpty()) {
    // Determine the file format based on the selected index
    QString format = (formatComboBox->currentIndex() == 1) ? "JPG" : "PNG";

    // Save the QPixmap to the selected file format
    pixmap.save(fileName, format.toUtf8(), qualitySlider->value() * 10);

    hide();
  }
}