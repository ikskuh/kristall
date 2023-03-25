#include "mediaplayer.hpp"
#include "ui_mediaplayer.h"

#include "kristall.hpp"

#include <QtGlobal>
#include <QToolButton>
#include <QTime>
#include <QIcon>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QMediaContent>
#else
#include <QAudioOutput>
#include <QUrl>
#endif

MediaPlayer::MediaPlayer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MediaPlayer),
    media_stream(),
    player(),
    manual_seek(false)
{
    ui->setupUi(this);

    connect( // connect with "this" as context, so the connection will die when the window is destroyed
        kristall::globals().localization.get(), &Localization::translationChanged,
        this, [this]() { this->ui->retranslateUi(this); },
        Qt::DirectConnection
    );

    this->player.setVideoOutput(this->ui->video_out);

    connect(&this->player, &QMediaPlayer::durationChanged, this->ui->media_progress, &QSlider::setMaximum);
    connect(&this->player, &QMediaPlayer::positionChanged, this->ui->media_progress, &QSlider::setValue);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(&this->player, &QMediaPlayer::audioAvailableChanged, this->ui->mute_button, &QToolButton::setEnabled);
#else
    connect(&this->player, &QMediaPlayer::hasAudioChanged, this->ui->mute_button, &QToolButton::setEnabled);
#endif
    // connect(&this->player, &QMediaPlayer::videoAvailableChanged, this->ui->video_out, &QVideoWidget::setVisible);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(&this->player, &QMediaPlayer::stateChanged, this, &MediaPlayer::on_media_playbackChanged);
#else
    connect(&this->player, &QMediaPlayer::playbackStateChanged, this, &MediaPlayer::on_media_playbackChanged);
#endif
    connect(&this->player, &QMediaPlayer::mediaStatusChanged, [](QMediaPlayer::MediaStatus status) {
        qDebug() << "media status changed" << status;
    });

    connect(&this->player, &QMediaPlayer::positionChanged, this, &MediaPlayer::on_media_positionChanged);

    connect(this->ui->media_progress, &QSlider::valueChanged, this, &MediaPlayer::on_seekChanged);
    connect(this->ui->media_progress, &QSlider::sliderPressed, this, &MediaPlayer::on_seekPressed);
    connect(this->ui->media_progress, &QSlider::sliderReleased, this, &MediaPlayer::on_seekReleased);
}

MediaPlayer::~MediaPlayer()
{
    delete ui;
}

void MediaPlayer::setMedia(QByteArray const & data, QUrl const & ref_url, QString const & mime)
{
    this->player.stop();

    this->mime = mime;

    this->media_stream.close();
    this->media_stream.setData(data); // = QBuffer { &this->backing_buffer };
    this->media_stream.open(QIODevice::ReadOnly);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QMediaContent content { ref_url };
    this->player.setMedia(content, &this->media_stream);
#else
    this->player.setSource(ref_url);
#endif
}

void MediaPlayer::stopPlaying()
{
    this->player.stop();
}

void MediaPlayer::on_playpause_button_clicked()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if(this->player.state() != QMediaPlayer::PlayingState) {
        this->player.play();
    } else {
        this->player.pause();
    }
#else
    if(this->player.playbackState() != QMediaPlayer::PlayingState) {
        this->player.play();
    } else {
        this->player.pause();
    }
#endif
}

void MediaPlayer::on_mute_button_clicked(bool checked)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    this->player.setMuted(checked);
#else
    this->player.audioOutput()->setMuted(checked);
#endif
}

void MediaPlayer::on_media_positionChanged(qint64 pos)
{
    auto time = QTime::fromMSecsSinceStartOfDay(pos);

    this->ui->media_position->setText(time.toString());
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void MediaPlayer::on_media_playbackChanged(QMediaPlayer::State status)
{
    this->ui->playpause_button->setIcon((status == QMediaPlayer::PlayingState)
        ? QIcon::fromTheme("media-playback-pause")
        : QIcon::fromTheme("media-playback-start")
    );
}
#else
void MediaPlayer::on_media_playbackChanged(QMediaPlayer::PlaybackState status)
{
    this->ui->playpause_button->setIcon((status == QMediaPlayer::PlayingState)
        ? QIcon::fromTheme("media-playback-pause")
        : QIcon::fromTheme("media-playback-start")
    );
}
#endif

void MediaPlayer::on_seekChanged(qint64 pos)
{
    if (this->manual_seek) {
        player.setPosition(pos);
    }
}

void MediaPlayer::on_seekPressed()
{
    this->manual_seek = true;
}

void MediaPlayer::on_seekReleased()
{
    this->manual_seek = false;
}
