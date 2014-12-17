#include <QMenu>
#include <QContextMenuEvent>
#include <QSettings>
#include <QDebug>

#ifdef QGC_USE_VLC
    #include <QMessageBox>
    #include <QMenuBar>
    #include <QLayout>
    #include <QFileDialog>
    #include <vlc/vlc.h>
#endif

#include "QGCRGBDView.h"
#include "UASManager.h"


QGCRGBDView::QGCRGBDView(int width, int height, QWidget *parent) :
    HUD(width, height, parent),
    rgbEnabled(false),
    depthEnabled(false)
{
    enableRGBAction = new QAction(tr("Enable RGB Image"), this);
    enableRGBAction->setStatusTip(tr("Show the RGB image live stream in this window"));
    enableRGBAction->setCheckable(true);
    enableRGBAction->setChecked(rgbEnabled);
    connect(enableRGBAction, SIGNAL(triggered(bool)), this, SLOT(enableRGB(bool)));

    enableDepthAction = new QAction(tr("Enable Depthmap"), this);
    enableDepthAction->setStatusTip(tr("Show the Depthmap in this window"));
    enableDepthAction->setCheckable(true);
    enableDepthAction->setChecked(depthEnabled);
    connect(enableDepthAction, SIGNAL(triggered(bool)), this, SLOT(enableDepth(bool)));

#ifdef QGC_USE_VLC
    VIDEOWIDTH = 640;
    VIDEOHEIGHT = 480;
    vlcInstance = NULL;
    VLCEnabled = false;
    enableVLCAction = new QAction(tr("VLC"), this);
    enableVLCAction->setStatusTip(tr("Show video with vlc"));
    enableVLCAction->setCheckable(true);
    enableVLCAction->setChecked(VLCEnabled);
    connect(enableVLCAction, SIGNAL(triggered(bool)), this, SLOT(enableVLC(bool)));
#endif

    connect(UASManager::instance(), SIGNAL(activeUASSet(UASInterface*)), this, SLOT(setActiveUAS(UASInterface*)));

    clearData();
    loadSettings();
}

QGCRGBDView::~QGCRGBDView()
{
    storeSettings();

#ifdef QGC_USE_VLC
    if ( vlcInstance ) {
        if(vlcPlayer) {
            libvlc_media_player_stop(vlcPlayer);
            /* release the media player */
            libvlc_media_player_release(vlcPlayer);
            /* Reset application values */
            vlcPlayer = NULL;
        }
        vlcInstance = NULL;
    }
#endif
}

void QGCRGBDView::storeSettings()
{
    QSettings settings;
    settings.beginGroup("QGC_RGBDWIDGET");
    settings.setValue("STREAM_RGB_ON", rgbEnabled);
    settings.setValue("STREAM_DEPTH_ON", depthEnabled);
    settings.endGroup();
}

void QGCRGBDView::loadSettings()
{
    QSettings settings;
    settings.beginGroup("QGC_RGBDWIDGET");
    rgbEnabled = settings.value("STREAM_RGB_ON", rgbEnabled).toBool();
    // Only enable depth if RGB is not on
    if (!rgbEnabled) depthEnabled = settings.value("STREAM_DEPTH_ON", depthEnabled).toBool();
    settings.endGroup();
}

void QGCRGBDView::setActiveUAS(UASInterface* uas)
{
    if (this->uas != NULL)
    {
        // Disconnect any previously connected active MAV
        disconnect(this->uas, SIGNAL(rgbdImageChanged(UASInterface*)), this, SLOT(updateData(UASInterface*)));

        clearData();
    }

    if (uas)
    {
        // Now connect the new UAS
        // Setup communication
        connect(uas, SIGNAL(rgbdImageChanged(UASInterface*)), this, SLOT(updateData(UASInterface*)));
    }

    HUD::setActiveUAS(uas);
}

void QGCRGBDView::clearData(void)
{
    QImage offlineImg;
    qDebug() << offlineImg.load(":/files/images/status/colorbars.png");

    glImage = offlineImg;
}

void QGCRGBDView::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    // Update actions
    enableHUDAction->setChecked(HUDInstrumentsEnabled);
    //enableVideoAction->setChecked(videoEnabled);
    enableRGBAction->setChecked(rgbEnabled);
    enableDepthAction->setChecked(depthEnabled);

    menu.addAction(enableHUDAction);
    menu.addAction(enableRGBAction);
    menu.addAction(enableDepthAction);


#ifdef QGC_USE_VLC

    enableVLCAction->setChecked(VLCEnabled);
    menu.addAction(enableVLCAction);
#endif

    //menu.addAction(selectHUDColorAction);
    //menu.addAction(enableVideoAction);
    //menu.addAction(selectOfflineDirectoryAction);
    //menu.addAction(selectVideoChannelAction);
    menu.exec(event->globalPos());
}

void QGCRGBDView::enableRGB(bool enabled)
{
    rgbEnabled = enabled;
    videoEnabled = rgbEnabled | depthEnabled;
    QWidget::resize(size().width(), size().height());
}

void QGCRGBDView::enableDepth(bool enabled)
{
    depthEnabled = enabled;
    videoEnabled = rgbEnabled | depthEnabled;
    QWidget::resize(size().width(), size().height());
}


#ifdef QGC_USE_VLC

void QGCRGBDView::enableVLC(bool enabled)
{
    VLCEnabled = enabled;

    VIDEOWIDTH = glImage.size().width();
    VIDEOHEIGHT = glImage.size().height();
    if ( VLCEnabled ) {
        VlcEnabledFlip = true;
        initVlcUI();
        if ( vlcInstance == NULL) {
            play();
        }
    }
    else if ( vlcInstance ) {
        VLCEnabled = false;
        VlcEnabledFlip = false;
        if(vlcPlayer) {
            libvlc_media_player_stop(vlcPlayer);
            /* release the media player */
            libvlc_media_player_release(vlcPlayer);
            /* Reset application values */
            vlcPlayer = NULL;
        }
        vlcInstance = NULL;
    }

    resize(size());
}

void QGCRGBDView::initVlcUI() {
    /* Buttons for the UI */
    openBut = new QPushButton("Open");
    QObject::connect(openBut,    SIGNAL(clicked()), this, SLOT(openFile()));

    playBut = new QPushButton("Play");
    QObject::connect(playBut, SIGNAL(clicked()), this, SLOT(play()));

    QPushButton *stopBut = new QPushButton("Stop");
    QObject::connect(stopBut, SIGNAL(clicked()), this, SLOT(stop()));

    QPushButton *muteBut = new QPushButton("Mute");
    QObject::connect(muteBut, SIGNAL(clicked()), this, SLOT(mute()));

    volumeSlider = new QSlider(Qt::Horizontal);
    QObject::connect(volumeSlider, SIGNAL(sliderMoved(int)), this, SLOT(changeVolume(int)));
    volumeSlider->setValue(80);

    slider = new QSlider(Qt::Horizontal);
    slider->setMaximum(1000);
    QObject::connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(changePosition(int)));

    /* A timer to update the sliders */
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateInterface()));
    timer->start(100);

    /* Central Widgets */
    //videoWidget = new QWidget;

    //videoWidget->setAutoFillBackground( true );
    //QPalette plt = palette();
    //plt.setColor( QPalette::Window, Qt::black );
    //videoWidget->setPalette( plt );
    hSpacer = new QSpacerItem(20,20,QSizePolicy::Minimum,QSizePolicy::Expanding);


    /* Put all in layouts */
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(playBut);
    layout->addWidget(stopBut);
    layout->addWidget(muteBut);
    layout->addWidget(openBut);
    layout->addWidget(volumeSlider);

    QVBoxLayout *layout2 = new QVBoxLayout;
    layout2->setMargin(0);
    layout2->addSpacerItem(hSpacer);
    layout2->addWidget(slider);
    layout2->addLayout(layout);
    this->setLayout(layout2);
    //setCentralWidget(centralWidget);
    //resize( 600, 400);
}


void QGCRGBDView::openFile() {
    /* The basic file-select box */
    QString fileOpen = QFileDialog::getOpenFileName(this, tr("Load a file"), "~");
    /* Stop if something is playing */
    if (vlcPlayer && libvlc_media_player_is_playing(vlcPlayer))
        stop();
}

void QGCRGBDView::play() {

    const char * const vlc_args[] = {
              "-I", "dummy", /* Don't use any interface */
              "--ignore-config", /* Don't use VLC's config */
              "--extraintf=logger",
              "--verbose=2"
              };
    //"--extraintf=logger", //log anything
    //"--verbose=2", //be much more verbose then normal for debugging purpose
    /* Initialize libVLC */
    //"--plugin-path=.\\plugins\\" };
    VlcEnabledFlip = true;
    vlcInstance = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);
    vlcPlayer = libvlc_media_player_new(vlcInstance);
    vlcMedia = libvlc_media_new_path(vlcInstance, "C:\\Wildlife.wmv");

    libvlc_media_player_set_media(vlcPlayer,vlcMedia);

    libvlc_video_set_callbacks(vlcPlayer, lock, unlock, 0, this);
    libvlc_video_set_format(vlcPlayer,"RV32",VIDEOWIDTH,VIDEOHEIGHT, VIDEOWIDTH*4);
    libvlc_media_player_play(vlcPlayer);

}

void *QGCRGBDView::lock( void *data, void**pp_ret )
{
    Q_ASSERT(data);
    //qDebug()<<"lock";
    QGCRGBDView *c = (QGCRGBDView*)data;
    c->mutex.lock();
    *pp_ret = c->glImage.bits();
    return NULL;
}

void QGCRGBDView::unlock( void *data, void *id, void *const *p_pixels )
{
    //qDebug()<<"Unlock";
    QGCRGBDView *c = (QGCRGBDView*)data;
    c->mutex.unlock();
}

int QGCRGBDView::changeVolume(int vol) { /* Called on volume slider change */
    if (vlcPlayer)
        return libvlc_audio_set_volume (vlcPlayer,vol);
    return 0;
}

void QGCRGBDView::changePosition(int pos) { /* Called on position slider change */
    if (vlcPlayer)
        libvlc_media_player_set_position(vlcPlayer, (float)pos/1000.0);
}

void QGCRGBDView::stop() {
    if(vlcPlayer) {
        /* stop the media player */
        libvlc_media_player_stop(vlcPlayer);

        /* release the media player */
        libvlc_media_player_release(vlcPlayer);

        /* Reset application values */
        vlcPlayer = NULL;
        slider->setValue(0);
        playBut->setText("Play");
    }
}

void QGCRGBDView::mute() {
    if(vlcPlayer) {
        if(volumeSlider->value() == 0) { //if already muted...
            this->changeVolume(80);
            volumeSlider->setValue(80);
        } else { //else mute volume
            this->changeVolume(0);
            volumeSlider->setValue(0);
        }
    }
}

#endif

float colormapJet[128][3] = {
    {0.0f,0.0f,0.53125f},
    {0.0f,0.0f,0.5625f},
    {0.0f,0.0f,0.59375f},
    {0.0f,0.0f,0.625f},
    {0.0f,0.0f,0.65625f},
    {0.0f,0.0f,0.6875f},
    {0.0f,0.0f,0.71875f},
    {0.0f,0.0f,0.75f},
    {0.0f,0.0f,0.78125f},
    {0.0f,0.0f,0.8125f},
    {0.0f,0.0f,0.84375f},
    {0.0f,0.0f,0.875f},
    {0.0f,0.0f,0.90625f},
    {0.0f,0.0f,0.9375f},
    {0.0f,0.0f,0.96875f},
    {0.0f,0.0f,1.0f},
    {0.0f,0.03125f,1.0f},
    {0.0f,0.0625f,1.0f},
    {0.0f,0.09375f,1.0f},
    {0.0f,0.125f,1.0f},
    {0.0f,0.15625f,1.0f},
    {0.0f,0.1875f,1.0f},
    {0.0f,0.21875f,1.0f},
    {0.0f,0.25f,1.0f},
    {0.0f,0.28125f,1.0f},
    {0.0f,0.3125f,1.0f},
    {0.0f,0.34375f,1.0f},
    {0.0f,0.375f,1.0f},
    {0.0f,0.40625f,1.0f},
    {0.0f,0.4375f,1.0f},
    {0.0f,0.46875f,1.0f},
    {0.0f,0.5f,1.0f},
    {0.0f,0.53125f,1.0f},
    {0.0f,0.5625f,1.0f},
    {0.0f,0.59375f,1.0f},
    {0.0f,0.625f,1.0f},
    {0.0f,0.65625f,1.0f},
    {0.0f,0.6875f,1.0f},
    {0.0f,0.71875f,1.0f},
    {0.0f,0.75f,1.0f},
    {0.0f,0.78125f,1.0f},
    {0.0f,0.8125f,1.0f},
    {0.0f,0.84375f,1.0f},
    {0.0f,0.875f,1.0f},
    {0.0f,0.90625f,1.0f},
    {0.0f,0.9375f,1.0f},
    {0.0f,0.96875f,1.0f},
    {0.0f,1.0f,1.0f},
    {0.03125f,1.0f,0.96875f},
    {0.0625f,1.0f,0.9375f},
    {0.09375f,1.0f,0.90625f},
    {0.125f,1.0f,0.875f},
    {0.15625f,1.0f,0.84375f},
    {0.1875f,1.0f,0.8125f},
    {0.21875f,1.0f,0.78125f},
    {0.25f,1.0f,0.75f},
    {0.28125f,1.0f,0.71875f},
    {0.3125f,1.0f,0.6875f},
    {0.34375f,1.0f,0.65625f},
    {0.375f,1.0f,0.625f},
    {0.40625f,1.0f,0.59375f},
    {0.4375f,1.0f,0.5625f},
    {0.46875f,1.0f,0.53125f},
    {0.5f,1.0f,0.5f},
    {0.53125f,1.0f,0.46875f},
    {0.5625f,1.0f,0.4375f},
    {0.59375f,1.0f,0.40625f},
    {0.625f,1.0f,0.375f},
    {0.65625f,1.0f,0.34375f},
    {0.6875f,1.0f,0.3125f},
    {0.71875f,1.0f,0.28125f},
    {0.75f,1.0f,0.25f},
    {0.78125f,1.0f,0.21875f},
    {0.8125f,1.0f,0.1875f},
    {0.84375f,1.0f,0.15625f},
    {0.875f,1.0f,0.125f},
    {0.90625f,1.0f,0.09375f},
    {0.9375f,1.0f,0.0625f},
    {0.96875f,1.0f,0.03125f},
    {1.0f,1.0f,0.0f},
    {1.0f,0.96875f,0.0f},
    {1.0f,0.9375f,0.0f},
    {1.0f,0.90625f,0.0f},
    {1.0f,0.875f,0.0f},
    {1.0f,0.84375f,0.0f},
    {1.0f,0.8125f,0.0f},
    {1.0f,0.78125f,0.0f},
    {1.0f,0.75f,0.0f},
    {1.0f,0.71875f,0.0f},
    {1.0f,0.6875f,0.0f},
    {1.0f,0.65625f,0.0f},
    {1.0f,0.625f,0.0f},
    {1.0f,0.59375f,0.0f},
    {1.0f,0.5625f,0.0f},
    {1.0f,0.53125f,0.0f},
    {1.0f,0.5f,0.0f},
    {1.0f,0.46875f,0.0f},
    {1.0f,0.4375f,0.0f},
    {1.0f,0.40625f,0.0f},
    {1.0f,0.375f,0.0f},
    {1.0f,0.34375f,0.0f},
    {1.0f,0.3125f,0.0f},
    {1.0f,0.28125f,0.0f},
    {1.0f,0.25f,0.0f},
    {1.0f,0.21875f,0.0f},
    {1.0f,0.1875f,0.0f},
    {1.0f,0.15625f,0.0f},
    {1.0f,0.125f,0.0f},
    {1.0f,0.09375f,0.0f},
    {1.0f,0.0625f,0.0f},
    {1.0f,0.03125f,0.0f},
    {1.0f,0.0f,0.0f},
    {0.96875f,0.0f,0.0f},
    {0.9375f,0.0f,0.0f},
    {0.90625f,0.0f,0.0f},
    {0.875f,0.0f,0.0f},
    {0.84375f,0.0f,0.0f},
    {0.8125f,0.0f,0.0f},
    {0.78125f,0.0f,0.0f},
    {0.75f,0.0f,0.0f},
    {0.71875f,0.0f,0.0f},
    {0.6875f,0.0f,0.0f},
    {0.65625f,0.0f,0.0f},
    {0.625f,0.0f,0.0f},
    {0.59375f,0.0f,0.0f},
    {0.5625f,0.0f,0.0f},
    {0.53125f,0.0f,0.0f},
    {0.5f,0.0f,0.0f}
};

void QGCRGBDView::updateData(UASInterface *uas)
{
	Q_UNUSED(uas);
}
