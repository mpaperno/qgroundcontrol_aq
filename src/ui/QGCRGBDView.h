#ifndef QGCRGBDVIEW_H
#define QGCRGBDVIEW_H

#include "HUD.h"
#ifdef Q_OS_WIN
#include <vlc/vlc.h>
#include <QMutex>
#include <QMainWindow>
#include <QSlider>
#include <QPushButton>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QLayout>
#include <QSpacerItem>
#endif

class QGCRGBDView : public HUD
{
    Q_OBJECT
public:
    explicit QGCRGBDView(int width=640, int height=480, QWidget *parent = 0);
    ~QGCRGBDView();

signals:

public slots:
    void setActiveUAS(UASInterface* uas);

    void clearData(void);
    void enableRGB(bool enabled);
    void enableDepth(bool enabled);
    void enableVLC(bool enabled);
    void updateData(UASInterface *uas);

protected:
    bool rgbEnabled;
    bool depthEnabled;
    bool VLCEnabled;
    QAction* enableRGBAction;
    QAction* enableDepthAction;
    void contextMenuEvent (QContextMenuEvent* event);
    /** @brief Store current configuration of widget */
    void storeSettings();
    /** @brief Load configuration of widget */
    void loadSettings();


private:
#ifdef Q_OS_WIN
    /*
    struct ctx {
        uchar* pixels;
        HANDLE mutex;
        HUD* mainWindow;
    };
    */
    struct ctx {
        QImage* pixels;
        QMutex* mutex;
        HUD* mainWindow;
    };

    struct ctx* cont;
    //QImage *vlcImage;
    QMutex mutex;
    void initVlcUI();
    void setMenuBar(QMenuBar *menubar);
    QWidget  *menuWidget() const;
    void setMenuWidget(QWidget *menubar);
    int VIDEOWIDTH;
    int VIDEOHEIGHT;
    libvlc_instance_t *vlcInstance;
    libvlc_media_player_t *vlcPlayer;
    libvlc_media_t *vlcMedia;
    static void *lock(void *data, void **pp_ret );
    static void unlock(void *data , void *id, void *const *p_pixels);
    static void setPixmap(void *data , void *id);
    QPushButton *playBut;
    QPushButton *openBut;
    QPushButton *stopBut;
    QSpacerItem *hSpacer;
    QSlider *volumeSlider;
    QSlider *slider;
    QAction* enableVLCAction;
private slots:
       void openFile();
       void play();
       void stop();
       void mute();
       int changeVolume(int);
       void changePosition(int);
#endif

};

#endif // QGCRGBDVIEW_H
