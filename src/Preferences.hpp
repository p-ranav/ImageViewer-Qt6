#pragma once
#include <QWidget>
#include <QTabWidget>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QSettings>
#include <QAnyStringView>
#include <QVariant>
#include <QCheckBox>
#include <QColorDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

class Preferences : public QWidget {
    Q_OBJECT

    static inline QSettings *m_settings = new QSettings("p-ranav", "ImageViewer");

    QPushButton *m_colorPickerButton;
    QRect m_backgroundColor; // RGBA, Picked QRect since it can be converted to/from QVariant

    QLineEdit* m_slideshowPeriod;
    QCheckBox* m_slideshowLoop;

    QCheckBox* m_rawHalfSize;
    QCheckBox* m_rawAutoWb;

public:
    constexpr static inline char SETTING_PREVIOUS_OPEN_PATH[] = "openPath";
    constexpr static inline char SETTING_BACKGROUND_COLOR[] = "backgroundColor";
    constexpr static inline char SETTING_SLIDESHOW_PERIOD[] = "slideShowTimerIntervalMs";
    constexpr static inline char SETTING_SLIDESHOW_LOOP[] = "slideShowLoopAfterEnd";
    constexpr static inline char SETTING_RAW_HALF_SIZE[] = "rawHalfSize";
    constexpr static inline char SETTING_RAW_AUTO_WB[] = "rawAutoWb";

public:
    Preferences(QWidget *parent = nullptr);
    static QVariant get(QAnyStringView key, const QVariant &defaultValue);
    static void set(QAnyStringView key, const QVariant &value);

signals:
    void settingChangedBackgroundColor(const QColor& color);
    void settingChangedSlideShowPeriod();
    void settingChangedSlideShowLoop();
    void rawSettingChanged();

private:
    void setupUi();
    QWidget* setupViewTab();
    QWidget* setupSlideshowTab();
    QWidget* setupRawTab();
    void handleEditingFinished_slideshowPeriod();
    void handleEditingFinished_slideshowLoop(int state);
    void handleEditingFinished_halfSize(int state);
    void handleEditingFinished_autoWb(int state);
};
